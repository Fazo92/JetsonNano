#include <iostream>
#include <string>
#include <sys/types.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <string.h>
#include <string>
#include <librealsense2/rs.hpp> // Include RealSense Cross Platform API
#include <vector>
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

#include <opencv2/cudacodec.hpp>
#include <opencv2/video/tracking.hpp>
#include <opencv2/cudaoptflow.hpp>
#include "sock.h"
#include <opencv2/features2d.hpp>
#include <fstream>
#define IPADRESS "192.168.100.176"
#define GROUPIP "226.1.1.2"
#define GROUPIPIMG "226.1.1.1"
#define PORTMULTICASTIMG 54222
#define PORTFRAME 55000+4
#define PORTDETECT 56000+4
#define PORTHINDERNIS 54221
using namespace cv;
using namespace cv::xfeatures2d;
using namespace std;
class rsfeatures{
    public:
        pthread_mutex_t mu_img,mu_dsc_ref,mu_dsc;
    pthread_mutex_t mu_H,mu_H2;
    pthread_mutex_t mu_pts;
      pthread_mutex_t mu_rcvimg;
    vector<Rect> m_rect;
    vector<KeyPoint> m_kp,m_kp_ref;
    cuda::GpuMat m_gpu_dsc_ref,m_gpu_dsc; 
    Mat m_img_ref;
    Mat m_H2;
    int m_width=640;
    int m_height=480;

    void compute_fts();
    void homography();
    void send_warped_img();
    double calcError(vector<Point2f> scene,vector<Point2f> obj,Mat H);
    Mat m_img1,m_img2,m_img3,m_img;
    void optFlow();
    void warpImage();
    double data_HBL[9] = { 0.7995172821202939,0.03769600438210419,38.74621023335813,-0.06696498169437112,0.8802728675937656,200.8930512025859,-1.40858823353097e-06,7.370026188769675e-05,1};
   Mat m_H=Mat(3,3,CV_64F,data_HBL);
    Mat staticH=m_H.clone();

    bool hindernis=false;
    void getdepth_and_colorFrame();
    void send_homography(int sock,Mat img);
    	static cv::Mat frame_to_mat(const rs2::frame& f)
	{
		using namespace cv;
		using namespace rs2;

		auto vf = f.as<video_frame>();
		const int w = vf.get_width();
		const int h = vf.get_height();

		if (f.get_profile().format() == RS2_FORMAT_BGR8)
		{
			return Mat(Size(w, h), CV_8UC3, (void*)f.get_data(), Mat::AUTO_STEP);
		}
		else if (f.get_profile().format() == RS2_FORMAT_RGB8)
		{
			auto r_rgb = Mat(Size(w, h), CV_8UC3, (void*)f.get_data(), Mat::AUTO_STEP);
			Mat r_bgr;
			cv::cvtColor(r_rgb, r_bgr, COLOR_RGB2BGR);
			return r_bgr;
		}
		else if (f.get_profile().format() == RS2_FORMAT_Z16)
		{
			return Mat(Size(w, h), CV_16UC1, (void*)f.get_data(), Mat::AUTO_STEP);
		}
		else if (f.get_profile().format() == RS2_FORMAT_Y8)
		{
			return Mat(Size(w, h), CV_8UC1, (void*)f.get_data(), Mat::AUTO_STEP);
		}
		else if (f.get_profile().format() == RS2_FORMAT_DISPARITY32)
		{
			return Mat(Size(w, h), CV_32FC1, (void*)f.get_data(), Mat::AUTO_STEP);
		}

		throw std::runtime_error("Frame format is not supported yet!");
	}
        static cv::Mat depth_frame_to_meters(const rs2::depth_frame& f1)
    {
        
        cv::Mat dm = frame_to_mat(f1);
        // dm.convertTo(dm, CV_64F);
        // dm = dm * f1.get_units();
        // dm = dm / 1000.;
        return dm;
    }
};