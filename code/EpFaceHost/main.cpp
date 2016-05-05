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
 * Example application that demonstrates Adapteva face detector
 */

#include <cassert>
#include <iostream>

#ifndef DEVICE_EMULATION
   // #include <e_host.h>
#else //DEVICE_EMULATION
    #include "c/ep_emulator.h"
#endif //DEVICE_EMULATION

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#include "cpp/ep_cascade_detector.hpp"

int main(int argc, char **argv) {

    char const *const keys (
        "{ i | input | | Input image or video file }"
        "{ c | classifier | lbpcascade_frontalface.dat | Epiphany LBP classifier }"
        "{ g | grouping | 3 | Number of detections in group }"
        "{ o | output | | Output filename }"
        "{ h | host | 0 | Run detection on host }"
        "{ n | numcores | 16 | Number of working cores }"
        "{ l | log | | Name of log-file }"
    );

    cv::CommandLineParser cmd(argc, argv, keys);
    std::string const fn_image( cmd.get<std::string>("input") ),
                      fn_classifier( cmd.get<std::string>("classifier") ),
                      fn_log( cmd.get<std::string>("log") );
    std::string fn_output( cmd.get<std::string>("output") );
    int const detections_group( cmd.get<int>("grouping") );
    int const num_cores( cmd.get<int>("numcores") );
    bool const host_only(cmd.get<int>("host") != 0);

    if( !host_only ) {
        /*      
        std::string const server_ip( "127.0.0.1" );
        unsigned short const port(50999);

        std::cout << "Connecting to e-server " << server_ip << ":" << port << "..." << std::flush;
        if( e_open( const_cast<char *>( server_ip.c_str() ), port ) ) {
            std::cout << " Can't establish connection to e-server." << std::endl;
            return -1;
        }
        std::cout << " Done." << std::endl; 
		*/
    }

    std::cout << "Loading image " << fn_image << "..." << std::flush;
    cv::Mat image(cv::imread(fn_image, CV_LOAD_IMAGE_GRAYSCALE));
    cv::VideoCapture capture;
    cv::VideoWriter writer;

    bool f_video(false);

    if( image.empty() ) {
        std::cout << " Error loading image." << std::endl;
        std::cout << "Loading video " << fn_image << "..." << std::flush;
        if( !capture.open(fn_image) ) {
            std::cout << " Error loading video." << std::endl;
            return -1;
        }
        f_video = true;
        if( fn_output.empty() )
            fn_output = "result.avi";
    } else {
        if( fn_output.empty() )
            fn_output = "result.png";
    }
    std::cout << " Done." << std::endl;

    std::cout << "Loading cascade " << fn_classifier << "..." << std::flush;

//See cpp/ep_cascade_detector.hpp for enabling/disabling integration with OpenCV object detector
#ifdef __OPENCV_OBJDETECT_HPP__
    cv::CascadeClassifier classifier_cv;
    ep::CascadeClassifier classifier_ep;

    //When loading .xml cascade both OpenCV and Epiphany detectors are tested.
    //When loading .dat cascade only Epiphany detector is tested.
    if( fn_classifier.size() > 4 && fn_classifier.substr(fn_classifier.size() - 4) == ".xml" ) {
        classifier_cv.load(fn_classifier);
        classifier_ep = classifier_cv;
    } else {
        classifier_ep.load(fn_classifier);
    }
#else
    ep::CascadeClassifier const classifier_ep(fn_classifier);
#endif

    if( classifier_ep.empty() ) {
        std::cout << " Error loading cascade." << std::endl;
        return -1;
    }

    std::cout << " Done. Classifier size is " << classifier_ep.get_size() << " bytes." << std::endl;
    //classifier.save("lbpcascade_frontalface.dat");

    if(f_video) {
        capture >> image;
        cv::cvtColor(image, image, CV_BGR2GRAY);
        if( !image.empty() ) {
            writer.open (
                fn_output,
                CV_FOURCC('M', 'J', 'P', 'G'),
                capture.get(CV_CAP_PROP_FPS),
                cv::Size(image.cols, image.rows)
            );
        } else {
            std::cout << " Error reading video." << std::endl;
            return -1;
        }
    }

    cv::Mat canvas;

    while(true) {
        std::vector<cv::Rect> objects_ep, objects_cv;

        {
            std::cout << "Detecting objects via ep::detect_multi_scale..." << std::endl;
            int64 const timeStart( cv::getTickCount() );

            ep::detect_multi_scale (
                image,
                classifier_ep,
                objects_ep,
                detections_group,
                SCAN_EVEN,
                host_only ? DET_HOST : DET_DEVICE,
                num_cores,
                fn_log
            );

            int64 const timeStop( cv::getTickCount() );
            std::cout << "Done in " << (timeStop - timeStart) / cv::getTickFrequency() << " sec." << std::endl;
        }

#ifdef __OPENCV_OBJDETECT_HPP__
        if( !classifier_cv.empty() ) {
            std::cout << "Detecting objects via cv::detect_multi_scale..." << std::endl;
            int64 const timeStart( cv::getTickCount() );

            classifier_cv.detectMultiScale(image, objects_cv, 1.19, detections_group);

            int64 const timeStop( cv::getTickCount() );
            std::cout << "Done in " << (timeStop - timeStart) / cv::getTickFrequency() << " sec." << std::endl;
        }
#endif

        cv::cvtColor(image, canvas, CV_GRAY2BGR);

        //Visualizing OpenCV detections
        for(int i(0); i < static_cast<int>( objects_cv.size() ); ++i) {
            cv::rectangle(canvas, objects_cv[i], cv::Scalar(0, 0, 255), detections_group ? 2 : 1);
        }

        //Visualizing Epiphany detections
        for(int i(0); i < static_cast<int>( objects_ep.size() ); ++i) {
            cv::Rect const &r(objects_ep[i]);
            cv::Point const c(r.x + r.width / 2, r.y + r.height / 2);
            cv::circle(canvas, c, r.width / 2, cv::Scalar(0, 255, 0), detections_group ? 2 : 1, CV_AA);
        }

        if(f_video) {
            writer << canvas;
            capture >> image;
            if( image.empty() )
                break; //End of video
            cv::cvtColor(image, image, CV_BGR2GRAY);
        } else {
            std::cout << "Saving result to " << fn_output << "..." << std::flush;
            if( !cv::imwrite(fn_output, canvas) ) {
                std::cout << " Error saving result." << std::endl;
                return -1;
            }
            break; //Single image only
        }
    }

    std::cout << " Done." << std::endl;

    if( !host_only ) {
		/*
		std::cout << "Disconnecting from e-server..." << std::flush;
        if( e_close() ) {
            std::cout << " Can't disconnect." << std::endl;
            return -1;
        }
        std::cout << " Done." << std::endl;
		*/
    }

    return 0;
}
