#include <client.h>

pthread_mutex_t mu=PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t muImg=PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t muSend=PTHREAD_MUTEX_INITIALIZER;
struct UDPstruct {
	struct sockaddr_in server;
	int sock;
};

rs2::pipeline client::setRS(int fps){
		int width = 640;
	int height = 480;
	Size sz = Size(640, 480);
	rs2::pipeline pipe;
	rs2::context ctx;
	rs2::config cfg;
	std::vector<std::string> serials;
	for (auto&& dev : ctx.query_devices()) {
		serials.push_back(dev.get_info(RS2_CAMERA_INFO_SERIAL_NUMBER));
		cout << dev.get_info(RS2_CAMERA_INFO_SERIAL_NUMBER) << endl;
	}
	cfg.enable_device(serials[0]);
	cfg.enable_stream(RS2_STREAM_COLOR, width, height, RS2_FORMAT_BGR8, fps);
	pipe.start(cfg);

	rs2::frameset frames;
	for (int i = 0; i < fps; i++)
	{
		frames = pipe.wait_for_frames();
	}
	return pipe;
}
Mat client::getFrameRS(rs2::pipeline pipe){


	// while(true){
		rs2::frameset frames;
	frames = pipe.wait_for_frames();
			Size sz = Size(640, 480);
		rs2::frame color_frame = frames.get_color_frame();
		Mat color(sz, CV_8UC3, (void*)color_frame.get_data(), Mat::AUTO_STEP);
		// imshow("as Socket",color);
		// waitKey(0);
		Mat imgRS=color.clone();
				return imgRS;

	// }
}
UDPstruct createSocketUDP(int portNumber)
{
	UDPstruct udp;
	 udp.sock= socket(AF_INET,SOCK_DGRAM,0);
	if(udp.sock==-1)
        {
                cout<<"Fehler Socket"<<endl;
        }

	
	udp.server.sin_family= AF_INET;
	udp.server.sin_port=htons(portNumber);
	// udp.server.sin_addr.s_addr = INADDR_ANY; 
	// inet_pton(AF_INET,"192.168.137.1",&udp.server.sin_addr);
	if(inet_aton("192.168.100.1",&udp.server.sin_addr)==0){
		cout << "Fehler"<< endl;
	}
		// int connectRes =connect(sock,(sockaddr*)&server,
        // sizeof(server));
        // if (connectRes==-1)
        // {
        // cout<<"Fehler"<<endl;           
        // }
		return udp;
}
void *client::serialTCP()
{
	int sock=createSocketTCP(54000);
	int sock2=createSocketTCP(53000);
	int sock3=createSocketTCP(52000);
	while(true) {
	

	
	rs2::pipeline pipe =setRS(15);
	
// 	Ptr<cv::SIFT> sift=cv::SIFT::create();
	Ptr<SURF> detector = SURF::create(40);
// 	cuda::SURF_CUDA surf;
// 	cuda::GpuMat img1,keypoints1GPU;
// 	cuda::GpuMat descriptors1GPU;
// 	cv::Ptr<cv::ORB> detectororb = cv::ORB::create(5000);
// 	cv::Ptr<cv::DescriptorExtractor> extractor = cv::ORB::create();
	cv::Mat img_descriptors;
	Mat dsc;
	int bytes=0;
	char buf[1000];
	while(true){
		Mat image=getFrameRS(pipe);
	// 		img1.upload(image);
		std::vector<cv::KeyPoint> img_keypoints;

// 		//sift detector###############################################################
		detector->detectAndCompute(image,noArray(),img_keypoints,dsc);
		int dSize=dsc.total()*dsc.elemSize();
		send(sock,&(dSize),sizeof(dSize),0);
		int kpsize=img_keypoints.size()*28;
		send(sock,&kpsize,sizeof(kpsize),0);

		// int descriptorSize=img_descriptors.total()*img_descriptors.elemSize();
		int imageSize = image.total()*image.elemSize();
		// int sendRes=send(sock,image.data,imageSize,0);
		this->kp=img_keypoints;
			int sendKp=send(sock2,&img_keypoints[0],28*img_keypoints.size(),0);

			int dscSize=dsc.elemSize()*dsc.total();
			int senddsc=send(sock3,dsc.data,dscSize,0);
			memset(buf,0,1000);
			bytes=recv(sock3,buf,1000,0);
		

		// if(sendRes==-1)
		// {
		// 	cout<<"Could not send to server"<<endl;
		// 	continue;
		// }

			}
}
	// close(sock);
	close(sock2);
	close(sock3);

}

// void client::serialUDP()
// {
// 	int sock;
// 	sockaddr_in serv;
// 	// UDPstruct frames = createSocketUDP(54000);
// 	// sock=frames.sock;
// 	// server=frames.server;



// 	serv.sin_family= AF_INET;
// 	serv.sin_port=htons(54000);
// 	serv.sin_addr.s_addr = INADDR_ANY; 
//  	// serv.sin_addr.s_addr = inet_addr("192.168.137.1");

// 	string ipaddr="192.168.137.1";
// 	// server.sin_addr.s_addr = INADDR_ANY; 
// //  server.sin_addr.s_addr = inet_addr("192.168.137.1");
// 	// udp.server.sin_addr.s_addr = INADDR_ANY; 
// 	inet_pton(AF_INET,ipaddr.c_str(),&serv.sin_addr);
// 			 sock= socket(AF_INET,SOCK_DGRAM,IPPROTO_UDP);
// 				 	int connectRes =connect(sock,(sockaddr*)&serv,
//         sizeof(serv));
//         if (connectRes==-1)
//         {
//         cout<<"Fehler"<<endl;           
//         }

// 	if(sock==-1)
//         {
//                 cout<<"Fehler Socket"<<endl;
//         }
// 	// if(inet_aton("192.168.137.1",&server.sin_addr)==0){
// 		// cout << 	ipaddr.c_str()<< endl;
// 	// }

// 	rs2::context ctx;   
// 	rs2::rates_printer printer;
// 	rs2::colorizer color_map;
// 	std::vector<std::string>  serials;
// 	for (auto&& dev : ctx.query_devices()) {
// 		serials.push_back(dev.get_info(RS2_CAMERA_INFO_SERIAL_NUMBER));
// 		cout << dev.get_info(RS2_CAMERA_INFO_SERIAL_NUMBER) << endl;
// 	}
// 	vector<rs2::config> cfg;
// 	for (int i = 0; i < serials.size(); i++) {
// 		rs2::config cfgtemp;
// 		cfgtemp.enable_device(serials[i]);
// 		cfgtemp.enable_stream(RS2_STREAM_COLOR,
// 		 640,480,
// 		 RS2_FORMAT_BGR8, 15);
// 		cfg.push_back(cfgtemp);
// 	}
// 	rs2::pipeline pipe1(ctx);
// 	std::map<std::string, rs2::colorizer> colorizers;
// 	pipe1.start(cfg[0]);
// 	rs2::frameset frames1;
// 	vector<rs2::frameset> frame;

// 	while(true){
// 		frames1=pipe1.wait_for_frames();
// 		rs2::frame color_frame1 = frames1.get_color_frame();
// 		cv::Mat image(cv::Size(640,480), CV_8UC3, (void*)color_frame1.get_data(), Mat::AUTO_STEP);
// 		std::vector<cv::KeyPoint> img_keypoints;

// 		int imageSize = image.total()*image.elemSize();
// 		//  fgets((char*)image.data,imageSize,stdin);

// 		// int sendRes=sendto(sock,image.data,imageSize,0,(sockaddr*)&serv,sizeof(serv));
// 			int sendImg=send(sock,image.data,imageSize,0);

// 		if(sendImg==-1)
// 		{
// 			cout<<"Could not send to server"<<endl;
// 			continue;
// 		}

// 			}

// 	close(sock);
// }

int client::createSocketTCP(int port){
	int sock= socket(AF_INET,SOCK_STREAM,0);
	 if(sock==-1)
        {
                cout<<"Fehler Socket"<<endl;
                return 1;
        }
	string ipAddress = "192.168.100.166";
        sockaddr_in hint;
	  hint.sin_family=AF_INET;
        hint.sin_port=htons(port);

        inet_pton(AF_INET,ipAddress.c_str(),&hint.sin_addr);
	int connectRes =connect(sock,(sockaddr*)&hint,
        sizeof(hint));
        if (connectRes==-1)
        {
        cout<<"Fehler"<<endl;           
        return 1;
        }
	return sock;

}



 void * client::sendFrameTCP()
{	
    int sock=createSocketTCP(54000);
	this->m_sock1=sock;
	Mat image;
	int bytes1=0;
	int bytes2=0;
	int bytes3=0;
	char buf[1000];
        while(true){
			if(this->framesend==false) continue;
			image=this->img.clone();
            int imageSize = image.total()*image.elemSize();
	        // pthread_mutex_lock(&muSend);
			int sendImg=send(sock,image.data,imageSize,0);
			// pthread_mutex_unlock(&muSend);
			this->framesend=false;
            if(sendImg==-1)
	        	{
		        	cout<<"Could not send Frame to server"<<endl;
		        	continue;
	        	}  
            
        }             
close(sock);
return NULL;
}

 void * client::sendKeyPointsTCP()
{		
		std::this_thread::sleep_for(std::chrono::milliseconds(4000));
        int sock=createSocketTCP(53000);
		this->m_sock2=sock;
        // Ptr<cv::SIFT> sift=cv::SIFT::create();
		Ptr<SURF> detector = SURF::create(400);
		Mat dsc;
		vector<float> descriptors;
		Mat imggray,imgclone;
		char buf[1000];
		int bytes1=0;
		int bytes2=0;
		int bytes3=0;
		while(true){

			if(this->kpsend==false) continue;
			        std::vector<cv::KeyPoint> keypoints=this->kp;

			//  detector->detectAndCompute(this->img,noArray(),keypoints,dsc);
			//  this->m_dsc=dsc;

			if (keypoints.size() > 0)
		{
			cout<<"keypoints Size: "<<keypoints.size()<<endl;
	
			int sendKeyPoints = send(sock, &keypoints[0], 28 * keypoints.size(), 0);
			this->kpsend=false;
			if (sendKeyPoints == -1)
			{
				cout << "Could not send Keypoints to server" << endl;
				continue;
			}

		}

			// memset(buf,0,1000);
			// bytes2=recv(m_sock2,buf,1000,0);
			this->kpsend=false;

			}	
		
	close(sock);
	return NULL;
}

void * client:: sendKeyPointsTCP2()
{		
		std::this_thread::sleep_for(std::chrono::milliseconds(2000));
        int sock=createSocketTCP(53000);
		Mat imgGray1;
		cuda::SURF_CUDA surf;
		cuda::GpuMat kpGPU,imgGPU,dsc;
        std::vector<cv::KeyPoint> keypoints;
		vector<float> descriptors;
		Mat imggray,imgclone;
		while(true){
			cvtColor(this->img, imgGray1, COLOR_BGR2GRAY);
			imgGPU.upload(imgGray1);
			// surf.detect(imgGPU, cuda::GpuMat(), kpGPU,this->dscGPU);
			surf(imgGPU, cuda::GpuMat(), kpGPU,this->dscGPU);

			surf.downloadKeypoints(kpGPU, keypoints);
			if (keypoints.size() > 0)
		{
			int sendKeyPoints = send(sock, &keypoints[0], sizeof(KeyPoint) * keypoints.size(), 0);
			if (sendKeyPoints == -1)
			{
				cout << "Could not send Keypoints to server" << endl;
				continue;
			}
		}
			}	
		
	close(sock);
	return NULL;	
}

void * client::detectFeatures(){
	rs2::pipeline pipe=setRS(30);

	Ptr<SURF> detector = SURF::create(400);
	vector<cv::KeyPoint> keypoints;
	Mat dsc1;
	while(true){
		this->img=getFrameRS(pipe);
		if(this->img.empty()) continue;
		detector->detectAndCompute(this->img,noArray(),keypoints,dsc1);
		if (this->dscsend != false || this->kpsend != false || this->framesend != false) continue;
		this->kp=keypoints;
		this->m_dsc=dsc1;

		this->dscsend=true;
		this->kpsend=true;
		this->framesend=true;
	}
}

void * client::sendDescriptorTCP()
{
	std::this_thread::sleep_for(std::chrono::milliseconds(2000));
	vector<cv::KeyPoint> keypoints;
	int sock=createSocketTCP(52000);
	// int sock2=createSocketTCP(80000);
	this->m_sock3=sock;
	Ptr<SURF> detector = SURF::create(400);
	std::vector< float > descriptors;
	int bytes1=0;
	int bytes2=0;
	int bytes3=0;
	char buf[1000];
	while(true)
	{
			if(!this->m_dsc.empty()){
			if(this->dscsend==false) continue;
			Mat dsc=this->m_dsc.clone();
			// cout<<"dsc Size: "<<dsc.size()<<endl;
			int dscVecSize=dsc.total()*dsc.elemSize();
			// int senddscRow=send(sock2,(char*)&dsc.rows,4,0);
			// int senddscCol=send(sock2,(char*)&dsc.cols,4,0);


			int senddsc=send(sock,(char*)dsc.data,dscVecSize,0);
			memset(buf,0,1000);
			bytes3=recv(m_sock3,buf,1000,0);
			// dsc.release();

			this->dscsend=false;
				if(senddsc==-1)
				{
					cout<<"Could not send descriptor"<<endl;
					continue;
				}




			}else {
				continue;
			}

		}
	close(sock);
	return NULL;

}
void *client::sendFeatures(){
	std::this_thread::sleep_for(std::chrono::milliseconds(4000));
    int sock=createSocketTCP(53000);
	int sock1=createSocketTCP(52000);
	int sock2=createSocketTCP(80000);
			Ptr<SURF> detector = SURF::create(400);
        std::vector<cv::KeyPoint> keypoints;
		Mat dsc;
		Mat imggray,imgclone;
		char buf[1000];
		int bytes=0;
		while(true){
			 detector->detectAndCompute(this->img,noArray(),keypoints,dsc);

			if (keypoints.size() > 0)
		{
			int sendKeyPoints = send(sock, &keypoints[0], sizeof(KeyPoint) * keypoints.size(), 0);
			if (sendKeyPoints == -1)
			{
				cout << "Could not send Keypoints to server" << endl;
				continue;
			}
	
				int dscVecSize=dsc.total()*dsc.elemSize();
				int senddscRow=send(sock2,(char*)&dsc.rows,4,0);
				int senddscCol=send(sock2,(char*)&dsc.cols,4,0);
				int senddsc=send(sock1,dsc.data,dscVecSize,0);
				memset(buf,0,1000);
				bytes=recv(sock1,buf,1000,0);
			

		}
			}	
		
	close(sock);
	close(sock1);
	close(sock2);
	return NULL;
}

void * client::sendCudaDescriptorTCP()
{
	std::this_thread::sleep_for(std::chrono::milliseconds(2000));
	int sock=createSocketTCP(52000);
	int sock2=createSocketTCP(80000);

	std::vector< float > dscVec;
	cuda::SURF_CUDA surf;
	Mat imggray;
	GpuMat dscCuda,kpGPU,img1;
	int gpusize;
	while(true)
	{	
		cvtColor(this->img,imggray,COLOR_BGR2GRAY);	
		img1.upload(imggray);
		// surf.detectWithDescriptors(img1,cuda::GpuMat(),kpGPU,dscCuda);
		dscCuda=this->dscGPU;
		surf.downloadDescriptors(dscCuda,dscVec);


		if(dscVec.empty()!=true)
		{
			int dscVecSize=sizeof(dscVec)+(sizeof(float)*dscVec.size());
			int senddscRow=send(sock2,(char*)&dscCuda.rows,4,0);
			int senddscCol=send(sock2,(char*)&dscCuda.cols,4,0);
			int senddsc=send(sock,&dscVec[0],dscVecSize,0);
		Mat dscCpu=Mat::zeros(dscCuda.rows, dscCuda.cols, CV_32FC1);
		int cnt=0;
		for (int i = 0; i < dscCuda.rows; i++) {
			for (int j = 0; j < dscCuda.cols; j++) {
				dscCpu.at<float>(i, j) = dscVec[cnt];
				cnt ++;

			}
		}

		// 		imshow("S", dscCpu);
		// if (waitKey(10) == 27) break;
			if(senddsc==-1)
			{
				cout<<"Could not send descriptor"<<endl;
				continue;
			}
		}

	}


	close(sock);
	close(sock2);

	return NULL;

}

void * client::CudasendDim()
{
	std::this_thread::sleep_for(std::chrono::milliseconds(4000));
	int sock=createSocketTCP(51000);	
	int dscSize;
	int kpSize;
	while(true)
    {
        dscSize=this->dscSize;
		kpSize=this->kp.size();
		int sendKpSize=send(sock,(int*)&kpSize,sizeof(kpSize),0);
		int senddscSize=send(sock,(int*)&dscSize,sizeof(dscSize),0);

    }
	close(sock);
	return NULL;

}

void * client::sendDim()
{
	std::this_thread::sleep_for(std::chrono::milliseconds(3000));
	int sock=createSocketTCP(51000);	
	int dscRows;
	int dscCols;
	while(true)
    {
        dscRows=this->dscRow;
        dscCols=this->dscCol;
        int sendrow=send(sock,(char*)&dscRows,sizeof(dscRows),0);
	    int sendcol=send(sock,(char*)&dscCols,sizeof(dscCols),0);
    }
	close(sock);
	return NULL;

}

void * client::computeHomography()
{
	int port=8000;
	int port2=8000+1500;
	int sock=createSocketTCP(port);
	int sock2=createSocketTCP(port2);
	// int sock3=createSocketTCP(1002);

	int bytes=0;
	int bytes2=0;
	char buf[1024*1024];
	int bufSize=1024*1024;
	char buf2[1024*1024*2];
	int bufSize2=1024*1024*2;
	setsockopt(sock, SOL_SOCKET, SO_RCVBUF, &bufSize, sizeof(bufSize));

		std::this_thread::sleep_for(std::chrono::milliseconds(4000));

Mat G=Mat::zeros(3,3,CV_64FC1);
	while(true){
				memset(	buf,0,bufSize);
		// for(int i=0;i<bufSize;i+=(bytes*sizeof(DMatch))){
		if((bytes=recv(sock,buf,bufSize,0))==-1) break;
				// }
				cout<<"bztes : "<<bytes<<endl;
		vector<DMatch> matches;
		vector<KeyPoint> kp2=this->kp;

	// 	// cout<<"Bytes: "<<bytes<<endl;
	 	DMatch *p=(DMatch*)buf;

	 	for(p=(DMatch*)&buf[0];p<(DMatch*)&buf[bytes];p++){
	 		matches.push_back(*p);
	 	}
	 			cout<<"mt size : "<<matches.size()<<endl;

	


		memset(buf2,0,bufSize2);

		// for(int i =0;i<bufSize2;i+=(bytes2*sizeof(KeyPoint)))
		if((bytes2=recv(sock2,buf2,bufSize2,0))==-1) cout <<"recv kp failed"<<endl;
		
		vector<KeyPoint> keypoints;
		KeyPoint *k=(KeyPoint*)buf2;

		for(k=(KeyPoint*)&buf2[0];k<(KeyPoint*)&buf2[bytes2];k++){
			keypoints.push_back(*k);
		}
		cout<<"keypoints: "<<keypoints.size()<<endl;
	// 	if(kp2.size()==0||matches.size()==0){
	// 		cout<<"size of Kp or Matches is 0"<<endl;
	// 		int sendHomo=send(sock3,G.data,72,0);
	// 		continue;
	// 	}
		vector<Point2f> obj,scene;
		for(int i=0;i<matches.size();i++){
			scene.push_back(kp2[matches[i].queryIdx].pt);
			obj.push_back(keypoints[matches[i].trainIdx].pt);

		}
		Mat H=findHomography(obj,scene,RANSAC,1.5);
		cout<<"Homo "<<H<<endl;
		int sizeHomo=H.elemSize()*H.total();
		if(sizeHomo==0){
			cout<<"size is 0"<<endl;
			 H=G.clone();
			int sendHomo=send(sock2,H.data,72,0);
		} else {
		int sendHomo=send(sock2,H.data,sizeHomo,0);
				cout<<"homo: "<<H<<endl;
		if(sendHomo==-1) continue;
		}
	
}
close(sock);
}
int client::createServerSocket(int port){
	int listening=socket(AF_INET,SOCK_STREAM,0);
	if(listening==-1) cerr<<"Can't create Socket"<<endl;
	sockaddr_in hint;
	hint.sin_family=AF_INET;
	hint.sin_port=htons(port);
	inet_pton(AF_INET,"0.0.0.0",&hint.sin_addr);
	bind(listening,(sockaddr*)&hint,sizeof(hint));
	listen(listening,SOMAXCONN);
	sockaddr_in client;
	socklen_t clientSize=sizeof(client);
	int clientSocket=accept(listening,(sockaddr*)&client,&clientSize);
	char host[NI_MAXHOST];
	char service[NI_MAXSERV];
	memset(host,0,NI_MAXHOST);
	memset(service,0,NI_MAXSERV);
	 if (getnameinfo((sockaddr*)&client, sizeof(client), host, NI_MAXHOST, service, NI_MAXSERV, 0) == 0)
    {
        cout << host << " connected on port " << service << endl;
    }
    else
    {
        inet_ntop(AF_INET, &client.sin_addr, host, NI_MAXHOST);
        cout << host << " connected on port " << ntohs(client.sin_port) << endl;
    }
	return listening;

    // Close listening socket
    close(listening);
	 
}

