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

Directories
-----------------------------------
### code/EpFaceHost
Host code    
Launch device program in c/ep_cascade_detector.c, function ep_detect_multi_scale_device.

### code/EpFaceCore_commonlib
Device code
