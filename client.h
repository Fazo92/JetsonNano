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
#include <netinet/in.h> 



using namespace cv::xfeatures2d;
using namespace std;
using namespace cv;
using namespace cuda;

class client{
    public:
    void serialUDP();
    int createSocketTCP(int port);
    // void createSocketUDP(int portNumber,sockaddr_in &server,int &sock);
    Mat img,m_dsc;

    GpuMat dscGPU;
    vector<KeyPoint> kp;
    vector<float> dscVec;
	rs2::pipeline setRS();
    Mat getFrameRS(	rs2::pipeline pipe);
    void serialTCP();
    void *sendFrameTCP();
    void *sendKeyPointsTCP();
    void *sendKeyPointsTCP2();
    void *detectFeatures();
    void *sendDescriptorTCP();
    void *sendDim();
    void *sendCudaDescriptorTCP();
    void *CudasendDim();
    void *sendFeatures();
    int dscRow;
    int dscCol;
    int dscSize;
    cuda::SURF_CUDA surfCUDA;
    private:
    int m_sock1,m_sock2,m_sock3;
    bool dscsend=false;
    bool kpsend=false;
    bool framesend=false;

};