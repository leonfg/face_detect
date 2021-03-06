/* <title of the code in this file>
   Copyright (C) 2012 Adapteva, Inc.
   
   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program, see the file COPYING.  If not, see
   <http://www.gnu.org/licenses/>. */

/**
 * C interface for Adapteva implementation of LBP face detection algorithm
 */

#ifndef EP_CASCADE_DETECTOR
#define EP_CASCADE_DETECTOR



#ifdef __cplusplus
extern "C" {
#endif
#include "ep_data_types.h"

////////////////////////////////////////////////////////////////////////////////
//                             IMAGE FUNCTIONS                                //
////////////////////////////////////////////////////////////////////////////////

/**
 * Create empty EpImage.
 * @return value that is recognized by other functions as "empty"
 */
EpImage ep_image_create_empty(void);

/**
 * Check whether image is empty.
 * @param image: pointer to valid image structure.
 * @return non-zero value for empty image, otherwise zero .
 */
int ep_image_is_empty(EpImage const *const image);

/**
 * Create EpImage given its width and height.
 * @param width: required image width;
 * @param height: required image height.
 * @return image with required size and undefined contents,
 *   or empty image on memory allocation failure. 
 */
EpImage ep_image_create(int const width, int const height);

/**
 * Get subimage of specified image without data copying.
 *   No checks on specified coordinates are performed.
 * @param image: pointer to valid image structure;
 * @param x: horizontal coordinate of upper-left corner of required subimage;
 * @param y: vertical coordinate of upper-left corner of required subimage;
 * @param width: width of required subimage;
 * @param height: height of required subimage.
 * @return subimage required.
 */
EpImage ep_subimage_get (
    EpImage const *const image,
    int const x, int const y,
    int const width, int const height
);

/**
 * Get subimage of specified image with data copying.
 *   No checks on specified coordinates are performed.
 * @param image: pointer to valid image structure;
 * @param x: horizontal coordinate of upper-left corner of required subimage;
 * @param y: vertical coordinate of upper-left corner of required subimage;
 * @param width: width of required subimage;
 * @param height: height of required subimage.
 * @return subimage required; on memory allocation failure empty image will be returned
 */
EpImage ep_subimage_clone (
    EpImage const*const image,
    int const x, int const y,
    int const width, int const height
);

/**
 * Copy the whole image and its data.
 * @param image: pointer to valid image structure.
 * @return copy of the image and its data; on memory allocation failure empty image will be returned
 */
EpImage ep_image_clone(EpImage const *const image);

/**
 * Calculates image checksum for debugging purpose.
 *   Images with the same data will get the same checksums,
 *   but it is not guaranteed that images with different data will
 *   get different checksums.
 * @param image: pointer to valid image structure.
 * @return checksum (int value).
 */
int ep_image_checksum(EpImage const *const image);

/**
 * Save image to simple binary file for debug purpose.
 * @param image: pointer to valid image structure.
 * @param file_name: pointer to file name (null-terminated string)
 * @return ERR_SUCCESS on success;
 *         ERR_ARGUMENT if image is empty (empty images cannot be saved);
 *         ERR_FILE if file cannot be opened or written.
 */
EpErrorCode ep_image_save(EpImage const *const image, char const *const file_name);

/**
 * Load image from simple binary file ( previously written by ep_image_save() )
 * @param file_name: pointer to file name (null-terminated string)
 * @param error_code: pointer to integer value which will receive the error code.
 *                  If this pointer is NULL then no error code is stored.
 *     Error codes: ERR_SUCCESS -- success;
 *                  ERR_FILE -- file cannot be opened or read;
 *                  ERR_FILE_CONTENTS -- wrong file contents;
 *                  ERR_MEMORY -- cannot allocate memory buffer.
 * @return image loaded. Empty image is returned in case of any error.
 */
EpImage ep_image_load(char const *const file_name, EpErrorCode *const error_code);

/**
 * Release image. After calling this function image will be empty. 
 * @param image: pointer to valid image structure.
 */
void ep_image_release(EpImage *const image);

////////////////////////////////////////////////////////////////////////////////
// IMAGE LIST FUNCTIONS (Control list of images held in shared memory buffer) //
////////////////////////////////////////////////////////////////////////////////

/**
 * Create empty images list
 */
EpImgList ep_img_list_create_empty(int const start_offset);

/**
 * Add item to images list.
 * @param img_list: pointer to valid images list;
 * @return ERR_SUCCESS on success;
 *         ERR_MEMORY on memory allocation failure.
 */
EpErrorCode ep_img_list_add (
    EpImgList *const img_list,
    int const step,
    int const width,
    int const height
);

/**
 * Release data hold by image list.
 * After calling this function list becomes empty. Images properties can be inserted to such list.
 * @param img_list: pointer to valid image list.
 */
void ep_img_list_release(EpImgList *const img_list);

////////////////////////////////////////////////////////////////////////////////
//                        RECTANGLES LIST FUNCTIONS                           //
////////////////////////////////////////////////////////////////////////////////

/**
 * Create empty rectangles list.
 */
EpRectList ep_rect_list_create_empty(void);

/**
 * Add item to rectangles list.
 * @param rect_list: pointer to valid rectangles list;
 * @param x: horizontal coordinate of left rectangle side;
 * @param y: vertical coordinate of top rectangle size;
 * @width: width of the rectangle;
 * @height: height of the rectangle.
 * @return ERR_SUCCESS on success;
 *         ERR_MEMORY on memory allocation failure; list is not changed in this case.
 */
EpErrorCode ep_rect_list_add (
    EpRectList *const rect_list,
    float const x    , float const y    ,
    float const width, float const height
);

/**
 * Reserve some space in rectangles list to speed up future rectangles insertions.
 * @param rect_list: pointer to valid rectangles list;
 * @param count: required number of rectangles that are planned to be added.
 * @return ERR_SUCCESS on success;
 *         ERR_MEMORY on memory allocation failure; list is not changed in this case.
 */
EpErrorCode ep_rect_list_reserve(EpRectList *const rect_list, int const count);

/**
 * Release data hold by rectangles list.
 * After calling this function list becomes empty. Rectangles can be inserted to such list.
 * @param rect_list: pointer to valid rectangles list.
 */
void ep_rect_list_release(EpRectList *const rectList);


////////////////////////////////////////////////////////////////////////////////
//                        TASK LIST FUNCTIONS                           //
////////////////////////////////////////////////////////////////////////////////

/**
 * Create empty task list
 */
EpTaskList ep_task_list_create_empty(void);

/**
 * Add item to tasks list.
 * @param task_list: pointer to valid rectangles list;
 * @param area       : tile area (must be  width * step)
 * @param width      : width of tile
 * @param height     : height of tile
 * @param step       : step in tile (must be round_up_to8(width))
 * @param scan_mode  : Scan mode of pixels (even pixels, odd pixels, or all pixels)
 * @param items_count: count of detected items (must be 0)
 * @param image_index: index of processing image
 * @return ERR_SUCCESS on success;
 *         ERR_MEMORY on memory allocation failure.
 */
EpErrorCode ep_task_list_add (
    EpTaskList *const task_list,
    int area,
    int width,
    int height,
    int step,
    int scan_mode,
    int items_count,
    int image_index
);

/**
 * Reserve some space in task list to speed up future tasks insertions.
 * @param task_list: pointer to valid task list;
 * @param count: required number of tasks that are planned to be added.
 * @return ERR_SUCCESS on success;
 *         ERR_MEMORY on memory allocation failure; list is not changed in this case.
 */
EpErrorCode ep_task_list_reserve(EpTaskList *const task_list, int const count);

/**
 * Release data hold by task list.
 * After calling this function list becomes empty. Tasks can be inserted to such list.
 * @param task_list: pointer to valid task list.
 */
void ep_task_list_release(EpTaskList *const task_list);
////////////////////////////////////////////////////////////////////////////////
//                          CLASSIFIER FUNCTIONS                              //
////////////////////////////////////////////////////////////////////////////////

/**
 * Create classifier which is recognized by other functions as "empty".
 * @return required classifier structure.
 */
EpCascadeClassifier ep_classifier_create_empty(void);

/**
 * Check whether classifier is empty.
 * @param classifier: pointer to the valid classifier structure.
 * @return non-zero value for empty classifier; zero for non-empty.
 */
int ep_classifier_is_empty(EpCascadeClassifier const *const classifier);

/**
 * Check classifier data for validity. Empty classifier is considered invalid!
 *   Use ep_classifier_is_empty() function to check whether classifier is empty.
 * @param classifier: pointer to tested classifier. classifier->data must be safely dereferencable!
 * @return zero value for good classifier data; non-zero value for bad data.
 */
int ep_classifier_check(EpCascadeClassifier const *const classifier);

/**
 * Clone classifier data into new buffer.
 * @param classifier: pointer to the classifier to be cloned.
 * @return cloned classifier structure; empty classifier is returned in case of any error.
 */
EpCascadeClassifier ep_classifier_clone(EpCascadeClassifier const *const classifier);

/**
 * Calculate classifier checksum for debug purpose.
 *   Classifiers with the same data will get the same checksums,
 *   but it is not guaranteed that classifiers with different data will
 *   get different checksums.
 * @param classifier: pointer to valid classifier structure.
 * @return checksum (int value).
 */
int ep_classifier_checksum(EpCascadeClassifier const *const classifier);

/**
 * Save classifier to binary file.
 * @param classifier: pointer to valid classifier structure.
 * @param file_name: pointer to file name (null-terminated string).
 * @return ERR_SUCCESS on success;
 *         ERR_ARGUMENT if classifier is empty (empty classifiers cannot be saved);
 *         ERR_FILE if file cannot be opened or written.
 */
EpErrorCode ep_classifier_save (
    EpCascadeClassifier const *const classifier,
    char const *const file_name
);

/**
 * Load classifier from binary file ( previously written by ep_classifier_save() )
 * @param file_name: pointer to file name (null-terminated string)
 * @param error_code: pointer to integer value which will receive the error code.
 *                  If this pointer is NULL then no error code is stored.
 *     Error codes: ERR_SUCCESS -- success;
 *                  ERR_FILE -- file cannot be opened or read;
 *                  ERR_FILE_CONTENTS -- wrong file contents;
 *                  ERR_MEMORY -- cannot allocate memory buffer.
 * @return classifier loaded. Empty classifier is returned in case of any error.
 */
EpCascadeClassifier ep_classifier_load(char const *const file_name, EpErrorCode *const EpErrorCode);

/**
 * Release memory hold by classifier.
 * After calling this function classifier is empty.
 * @param classifier: pointer to valid classifier structure. 
 */
void ep_classifier_release(EpCascadeClassifier *const classifier);

////////////////////////////////////////////////////////////////////////////////
//                          MAIN DETECTION FUNCTION                           //
////////////////////////////////////////////////////////////////////////////////

/**
 * Multiscale object detection
 *
 * Image is iteratively scaled down until it became less than native object size.
 * On each scale detection is performed.
 *
 * @param image     : Image to process (pointer to valid image structure). Image contents is modified during detection!
 * @param classifier: Classifier to use (pointer to valid classifier structure).
 * @param objects   : Detections will be added to this list (pointer to valid rectangles list structure).
 * @param scan_mode : Which image pixels to test; @see EpScanMode.
 * @param num_cores : Number of cores to use.
 * @param log_file  : Name of time-log file (if 0  then time logging is off).
 *
 * @return ERR_SUCCESS : successful detection;
 *         ERR_ARGUMENT: empty image, or invalid classifier, or unknown detection_mode, or unknown scan_mode.
 *         ERR_MEMORY  : cannot allocate required memory (memory checks are not implemented yet).
 *         ERR_OTHER  : classifier is too large and cannot be uploaded to core.
 */
EpErrorCode ep_detect_multi_scale_device (
    EpImage                   *const image,
    EpCascadeClassifier const *const classifier,
    EpRectList                *const objects,
    EpScanMode                 const scan_mode,
    int                        const num_cores,
    char                const *const log_file
);

EpErrorCode ep_detect_multi_scale_host (
    EpImage                   *const image,
    EpCascadeClassifier const *const classifier,
    EpRectList                *const objects,
    EpScanMode                 const scan_mode
);

#ifdef __cplusplus
}
#endif

#endif
