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
#include <opencv2/opencv.hpp>   // Include OpenCV API
#include <vector>
#include <opencv2/features2d/features2d.hpp>
#include <opencv2/xfeatures2d.hpp>
#include <opencv2/xfeatures2d/cuda.hpp>
#include <opencv2/cudafeatures2d.hpp>
#include <chrono>
#include <thread>
#include "pthread.h"
#include <chrono>
#include <mutex>


using namespace cv::xfeatures2d;
using namespace std;
using namespace cv;
using namespace cuda;

class client{
    public:
    int createSocketTCP(int port);
    // void createSocketUDP(int portNumber,sockaddr_in &server,int &sock);
    Mat img,m_dsc;
    GpuMat dscGPU;
    vector<KeyPoint> kp;
    vector<float> dscVec;
    void serialTCP();
    void *sendFrameTCP();
    void *sendKeyPointsTCP();
    void *sendKeyPointsTCP2();

    void *sendDescriptorTCP();
    void *sendDim();
    void *sendCudaDescriptorTCP();
    void *CudasendDim();
    int dscRow;
    int dscCol;
    int dscSize;
    cuda::SURF_CUDA surfCUDA;

};