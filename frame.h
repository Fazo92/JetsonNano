#pragma once
#include <vector>
#include <iostream>
#include <opencv2/features2d/features2d.hpp>
#include <opencv2/xfeatures2d.hpp>
#include <opencv2/xfeatures2d/cuda.hpp>
#include <opencv2/cudafeatures2d.hpp>
#include <opencv4/opencv2/core/types.hpp>
#include "opencv2/calib3d.hpp"
#include "opencv2/highgui.hpp"
#include <netinet/tcp.h>
#include <chrono>
#include <thread>
#include "pthread.h"
#include <chrono>
#include <mutex>
#include <netinet/in.h> 
#include <opencv2/cudaoptflow.hpp>
#include <opencv2/cudafeatures2d.hpp>
#include <opencv2/cudawarping.hpp>
#include <opencv2/cudaimgproc.hpp>
#include <opencv2/imgproc.hpp>
using namespace cv;
class frame{
    public:
        Mat m_img;
        cuda::GpuMat dsc;
        cuda::GpuMat m_img_gpu;
        frame(Mat img){
            this->m_img=img;
            m_img_gpu.upload(img);
        }
        Ptr<cuda::ORB> orb=cv::cuda::ORB::create(5000);
        Ptr<cuda::SURF_CUDA> surf=cuda::SURF_CUDA::create(1,5,2,true);
        void compute_features_orb();
        void compute_features_surf();
        void compute_features_sift();
        void getPointsOptFlow(Mat prev_img);
        std::vector<KeyPoint> kp;
        std::vector<Point2f> prev_pts,next_pts;
};