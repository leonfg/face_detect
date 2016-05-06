Face Detect
===================================
Cascade classifier based face detection modified from http://www.adapteva.com/white-papers/face-detection-using-the-epiphany-multicore-processor to fit the parallella board and current eSDK.    

Envirment
-----------------------------------
Image: ubuntu-15.04-esdk-2016.3-headless-z7020    
eSDK: 2016.3    

Build and run
-----------------------------------
### OpenCV install:
sudo apt-get install libopencv-dev    


### Build: 
cd code    
./buils.sh    

### Run:
cd code/release    
run.sh    

### Para:
        "{ i | input | | Input image file }"   
        "{ c | classifier | lbpcascade_frontalface.xml }"    
        "{ g | grouping | 3 | Number of detections in group }"    
        "{ o | output | | Output filename }"    
        "{ h | host | 0 | Run detection on Epiphany | 1 | Run detection on ARM }"    
        "{ n | numcores | 16 | Number of working cores }"   
        "{ l | log | | Name of log-file }"    
    example:    
    ./EpFaceHost i g20.jpg c lbpcascade_frontalface.xml g 3 o t1.jpg h 0 n 12 l 1.log    

### Results:
The green circle is Epiphany classify result, the red rectangle is opencv classify result    

Remaining problems:
-----------------------------------
The working principle is: resizing the input image into several different sizes, then dividing the resized images into many subimages with a given size window, the cascade classifier will do feature matching calculation to distinguish if a subimage is a human face. We use Epiphany cores to do the feature matching work parallelly to achieve speedup. There are some problems in current code:     
1. Core number: When use most of the cores like 16 or 15, the device program always hang. I have to use less cores to prevent this.     
2. Number of loop iterations: There are two main "for" loop sequences in the device program, one is dmacopy the subimages from shared memory, the other is classifier calculation. When there are too many dmacopy loop iterations in core the program will hang too. So I have to use more cores to reduce the loop iterations in each core.     
The hang situation will happen randomly, but the workflow and logic are definitely correct, I can not figure out the reason of the problem, so I have to use nether too less nor too more cores to balance the core number and the loop iterations, 4~12 cores will get successful execution most time. Someone please help me to solve this!
