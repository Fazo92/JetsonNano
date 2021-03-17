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
    int createServerSocket(int port);
    GpuMat dscGPU;
    vector<KeyPoint> kp;
    vector<float> dscVec;
	rs2::pipeline setRS(int fps);
    Mat getFrameRS(	rs2::pipeline pipe);
    void *serialTCP();
    void *sendFrameTCP();
    void *sendKeyPointsTCP();
    void *sendKeyPointsTCP2();
    void *detectFeatures();
    void *sendDescriptorTCP();
    void *sendDim();
    void *sendCudaDescriptorTCP();
    void *CudasendDim();
    void *sendFeatures();
    void *computeHomography();
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