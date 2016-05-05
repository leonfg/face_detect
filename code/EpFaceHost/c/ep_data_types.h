/*
 * Common data types
 */

#ifndef EP_DATA_TYPES_H
#define EP_DATA_TYPES_H

//#include <e-hal.h>


/**
 * Constants
 */
typedef enum {
    /// Size of Epiphany memory bank in bytes.
    BANK_SIZE = 8192,
    /// Recommended vertical and horizontal size of tile used to detect objects.
    /// Tiles are overlapped in order to not miss detections at edges
    RECOMMENDED_TILE_SIZE = 128,
    /// Maximal allowed detections per tile. If more will be detected then some detections will be discarded
    /// Must be even value because transmitted data size is rounded up to the nearest 64 bits boundary
    MAX_DETECTIONS_PER_TILE = 16,
    /// Classifier should occupy less than one memory bank; some space is reserved for stack.
    /// This value must be dividible by 8
    MAX_CLASSIFIER_BYTES = BANK_SIZE - 512,
    /// Identifier (4 bytes) written to the beginning of image file (simple binary format is used)
    FILE_ID_IMAGE = 1734438217,
    /// Identifier (4 bytes) written to the beginning of classifier file (binary format is used)
    FILE_ID_CLASSIFIER = 1935764547,
    /// Core frequency in MHz to convert tics to seconds
    CORE_FREQUENCY = 400,
    /// Timer divisor to prevent unsigned int overflow of total core time
    TIMER_VALUE_SHIFT = 7
} EpConstants1;

/**
 * Object detection mode
 */
typedef enum {
    /// Detection on host CPU. Fast parallel implementation
    DET_HOST = 0,
    /// Detection of Epiphany (single or multiple cores)
    DET_DEVICE
} EpDetectionMode;

/**
 * Image scan mode
 */
typedef enum {
    /// Checkerboard scan order; even pixels
    SCAN_EVEN = 0,
    /// Checkerboard scan order; odd pixels
    SCAN_ODD = 1,
    /// Scan all pixels
    SCAN_FULL = 2
} EpScanMode;

/**
 * Error codes may be returned by functions in this library
 */
typedef enum {
    /// No error occurred:
    ERR_SUCCESS = 0,
    /// One of the arguments provided to a function is invalid
    ERR_ARGUMENT,
    /// File operation error (error during opening file, reading file or closing it)
    ERR_FILE,
    /// Data read from file does not meet required format.
    ERR_FILE_CONTENTS,
    /// Not enough memory to perform operation 
    ERR_MEMORY,
    /// Other error occurred
    ERR_OTHER
} EpErrorCode;

typedef unsigned int EpCoreId;

/**
 * Image data usable by functions in this library
 */
typedef struct {
    /// Pointer to image buffer
    unsigned char *data;
    /// Image dimensions
    int width, height;
    /// Step in bytes from one image line to the next one. May be larger than width
    int step;
} EpImage;

/**
 * Rectangle to describe object detection
 */
typedef struct {
    /// Position of the top-left corner
    float x, y;
    /// Width and height of the rectangle
    float width, height;
} EpRect;

/**
 * List of detected objects (rectangles)
 */
typedef struct {
    /// Rectangles buffer
    EpRect *data;
    /// Memory amount allocated
    int capacity;
    /// Number of rectangles stored
    int count;
} EpRectList;

/**
 * Classifier is just binary buffer
 */
typedef struct {
    char *data;
    int size;
} EpCascadeClassifier; 

/**
 * Type of classifier node
 */
typedef enum {
    /// First classifier node containing some useful information about classifier itself
    NODE_META = 1635018061,
    /// Decision node; contains feature description and score dependent on feature value
    NODE_DECISION = 0, //Must be zero; other values are arbitrary
    /// End of classifier stage. Contains rule to reject object or go to the next stage
    NODE_STAGE = 1734440019,
    /// Last node of the classifier, meaning successful detection. May only go after the NODE_STAGE node
    NODE_FINAL = 1819175238
} EpNodeType;

/**
 * First classifier node containing some useful information about classifier itself
 */
typedef struct {
    /// id == NODE_META for EpNodeMeta structure
    int id;

    /// Native width and height of detected objects in pixels. To detect larger objects image is scaled down.
    int window_width, window_height;
} __attribute__((packed)) EpNodeMeta;

/**
 * Decision node; contains feature description and score dependent on feature value
 */
typedef struct {
    /// id == NODE_DECISION for EpNodedecision structure
    int id;

    /**
     * feature_width  =  feature        & 255
     * feature_height = (feature >> 8 ) & 255
     * feature_x      = (feature >> 16) & 255
     * feature_y      =  feature >> 24
     */
    int feature;
    /// Score for object if feature value will be in specified subset
    int score;
    /**
     * One bit for each possible value of LBP feature.
     * Bits which are equal to "1" mean that object gets score
     */
    int subsets[8];
} __attribute__((packed)) EpNodeDecision;

typedef struct {
    /// id == NODE_STAGE for EpNodeStage structure
    int id;

    /**
     * If sum of all decisions is less than this threshold then no detection
     * is assumed and classifier execution will be terminated
     */
     int threshold;
} __attribute__((packed)) EpNodeStage;

/**
 * Last node of the classifier, meaning successful detection. May only go after the EpNodeStage node
 */
typedef struct {
    /// id == NODE_FINAL for EpNodeFinal structure
    int id;
} __attribute__((packed)) EpNodeFinal;

/**
 * Structure of timer
 */
typedef struct {
    /// Current value of timer
    unsigned int value;
    /// ID of current core
    unsigned int core_id;
} __attribute__((packed)) EpTimerBuf;

/**
 * Structure of task
 */
typedef struct {
    /// Offset of tile
    int offset;
    /// Tile width
    int width;
    /// Tile height
    int height;
    /// Size of detection area ( = step * height)
    int area;
    /// Tile step
    int step;
    /// Scan mode of pixels (even pixels, odd pixels, or all pixels)
    int scan_mode;
    /// Count of objects
    int items_count;
    /// Index of processing image
    int image_index;
    /// Detection result
    int objects[MAX_DETECTIONS_PER_TILE];
} __attribute__((packed)) EpTaskItem;

/**
 * List of tasks
 */
typedef struct {
    /// Tasks buffer
    EpTaskItem *data;
    /// Memory amount allocated
    int capacity;
    /// Number of tasks stored
    int count;
} EpTaskList;

typedef enum {
    /// Maximal allowed memory occupied by tile -- 2 banks of Epiphany memory in this case
    MAX_TILE_BYTES = BANK_SIZE * 2 - sizeof(EpTaskItem) - sizeof(EpTimerBuf) - 1024,
    /// Maximal allowed images count in scale pyramid
    MAX_IMGS_COUNT = 30,
    /// Maximal allowed memory occupied by pyramid
    MAX_IMGS_BUF   = 16480000,
    /// Maximal cores count
    MAX_CORES_NUM  = 16,
    /// Maximal tasks count
    MAX_TASK_BUF   = 2048
} EpConstants2;

typedef struct {
    /// Timer service info
    EpTimerBuf timer;
    /// Data structure for exchanging control data
    EpTaskItem task_item;
    /// Begin of tile buffer
    unsigned char buf_tile[BANK_SIZE - sizeof(EpTaskItem) - sizeof(EpTimerBuf)]; // First part of image data
} __attribute__((packed)) EpCoreBank1;

typedef struct {
    ///Almost two memory banks are used to store image data:
    unsigned char buf_tile[BANK_SIZE]; //Second part of image data
} __attribute__((packed)) EpCoreBank2;

typedef struct {
    ///Part of last memory bank is for classifier:
    char buf_classifier[MAX_CLASSIFIER_BYTES]; //It is supposed that stack is less than 512 bytes!
} __attribute__((packed)) EpCoreBank3;

/**
 * Properties of image held is shared buffer
 */
typedef struct {
    /// offset of this image in shared buffer
    int data_offset;
    /// step (must be multiple of 8)
    int step;
    /// width of image
    int width;
    /// height of image
    int height;
} __attribute__((packed)) EpImageProp;

typedef struct {
    /// Images properties buffer
    EpImageProp *data;
    /// Memory amount allocated
    int capacity;
    /// Number of images stored
    int count;
    /// current position offset
    int cur_offset;
    /// previous position offset
    int prev_offset;
} EpImgList;

/**
 * Control memory structure.
 */
typedef struct {
    /// total tasks count
    int task_count;
    /// current task number
    int task_to_take;
    /// number of processed tasks
    int task_finished;
    /// number of cores to be started
    int start_cores;
    /// current index in timers queue
    int timer_index;
    int unused;
} __attribute__((packed)) EpControlInfo;

typedef struct {
    /// Control information for core task manager
    EpControlInfo control_info;
    /// Images properties
    EpImageProp   imgs_prop[MAX_IMGS_COUNT];
    /// Classifier buffer
    char          buf_classifier[MAX_CLASSIFIER_BYTES];
    /// Images buffer
    unsigned char imgs_buf[MAX_IMGS_BUF];
    /// Tasks list
    EpTaskItem    tasks[MAX_TASK_BUF];
    /// Timers list
    EpTimerBuf    timers[MAX_CORES_NUM];
} __attribute__((packed)) EpDRAMBuf;

#endif /* EP_DATA_TYPES_H */
