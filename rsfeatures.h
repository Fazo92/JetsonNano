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
#include <opencv2/cudacodec.hpp>
#include <opencv2/video/tracking.hpp>

#define PORTFRAME 20000
#define PORTDSC 21000
#define PORTKEYPTS 22000
#define PORTDIM 23000


using namespace cv::xfeatures2d;
using namespace std;
using namespace cv;
class rsfeatures{
    public:
    int m_width=640;
    int m_height=480;
     void getFrameTCP();
    Mat m_img1,m_img2,m_img3;
    void getPointsOptFlow();
    void getPointsOptFlowCPU();
    void cudaSurf();
    rs2::pipeline setRS(int fps);
    void getFrameRS();
    vector<Point2f> m_prev_pts, m_next_pts;
    void warpImage();
    int m_sock1;
    void surfcpu();
    Mat m_H=Mat::zeros(3,3,CV_64F);
    Mat dFrame;
    void depthFrame();
    void detectObject();
    Mat detecedObject;
    Rect rct;
    void detectCUDAFeatures();
    Mat receive_dsc(int sock);
    vector<KeyPoint> recv_kpts(int sock);
    void send_img(int sock,Mat img);
    void send_kpts(int sock,vector<KeyPoint> kpts);
    void send_dsc(int sock,Mat dsc);
    void send_fts();
    void receive_fts();
    void detectSURFCUDAFeatures();
    void find_matches();
    double number_H=0;
    Mat m_dsc;
    vector<KeyPoint> m_kpts;
    Mat m_dscC;
    vector<KeyPoint> m_kptsC;

};