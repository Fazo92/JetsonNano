#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <sys/time.h>
#include <errno.h>
#include <iostream>
#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>
#include <fstream>
//#define PACK_SIZE 50000 
#define PACK_SIZE 65500 

#define HOSTIP "192.168.100.34"

#define ENCODE_QUALITY 80
#define BUFLEN_UDP 65540
using namespace std;
using namespace cv;

class sock {
    public:
int sock;
sockaddr_in server,serversend;
	
void initUDP(int portNumber,string ipAddress);
void initTCP(int portNumber,string ipAdress);
void send_img(cv::Mat img);
void send_depth_img(cv::Mat img);
void rcv_img(Mat &img);
bool rcv_hindernis();
void initMultiCastUDPrecv(int port, string localIP, string groupIP);

void closesock();
bool tcp=false;
bool serv;
};