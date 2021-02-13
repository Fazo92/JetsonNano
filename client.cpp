#include <client.h>

pthread_mutex_t mu=PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t muImg=PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t muSend=PTHREAD_MUTEX_INITIALIZER;


void client::serialTCP()
{
	int sock=createSocketTCP(54000);
	int sock2=createSocketTCP(53000);
	int sock3=createSocketTCP(52000);

	while(true) {
	rs2::context ctx;   
	rs2::rates_printer printer;
	rs2::colorizer color_map;
	std::vector<std::string>  serials;
	for (auto&& dev : ctx.query_devices()) {
		serials.push_back(dev.get_info(RS2_CAMERA_INFO_SERIAL_NUMBER));
		cout << dev.get_info(RS2_CAMERA_INFO_SERIAL_NUMBER) << endl;
	}
	vector<rs2::config> cfg;
	for (int i = 0; i < serials.size(); i++) {
		rs2::config cfgtemp;
		cfgtemp.enable_device(serials[i]);
		cfgtemp.enable_stream(RS2_STREAM_COLOR,
		 640,480,
		 RS2_FORMAT_BGR8, 15);
		cfg.push_back(cfgtemp);
	}
	rs2::pipeline pipe1(ctx);
	std::map<std::string, rs2::colorizer> colorizers;
	pipe1.start(cfg[0]);
	rs2::frameset frames1;
	vector<rs2::frameset> frame;
// 	Ptr<cv::SIFT> sift=cv::SIFT::create();
	Ptr<SURF> detector = SURF::create(400);
// 	cuda::SURF_CUDA surf;
// 	cuda::GpuMat img1,keypoints1GPU;
// 	cuda::GpuMat descriptors1GPU;
// 	cv::Ptr<cv::ORB> detectororb = cv::ORB::create(5000);
// 	cv::Ptr<cv::DescriptorExtractor> extractor = cv::ORB::create();
	cv::Mat img_descriptors;
	vector<float> dscVec;
	while(true){
		frames1=pipe1.wait_for_frames();
		rs2::frame color_frame1 = frames1.get_color_frame();
		cv::Mat image(cv::Size(640,480), CV_8UC3, (void*)color_frame1.get_data(), Mat::AUTO_STEP);
// 		img1.upload(image);
		std::vector<cv::KeyPoint> img_keypoints;

// 		//sift detector###############################################################
		detector->detectAndCompute(image,noArray(),img_keypoints,dscVec);
		// int descriptorSize=img_descriptors.total()*img_descriptors.elemSize();
		int imageSize = image.total()*image.elemSize();
		int sendRes=send(sock,image.data,imageSize,0);

		if(img_keypoints.size()>0)
		{
			int sendKp=send(sock2,&img_keypoints[0],29*img_keypoints.size(),0);
		}

		if(dscVec.empty()!=true)
		{
			int senddsc=send(sock3,&dscVec[0],dscVec.size(),0);
		}

		if(sendRes==-1)
		{
			cout<<"Could not send to server"<<endl;
			continue;
		}

			}
}
	close(sock);
	close(sock2);
	close(sock3);

}

int client::createSocketTCP(int port){
	int sock= socket(AF_INET,SOCK_STREAM,0);
	 if(sock==-1)
        {
                cout<<"Fehler Socket"<<endl;
                return 1;
        }
	string ipAddress = "192.168.137.1";
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

void createSocketUDP(int portNumber,sockaddr_in &server,int &sock)
{
	 sock= socket(AF_INET,SOCK_DGRAM,0);
	if(sock==-1)
        {
                cout<<"Fehler Socket"<<endl;
        }

	
	server.sin_family= AF_INET;
	server.sin_port=htons(portNumber);
	inet_pton(AF_INET,"192.168.137.1",&server.sin_addr);
		// int connectRes =connect(sock,(sockaddr*)&server,
        // sizeof(server));
        // if (connectRes==-1)
        // {
        // cout<<"Fehler"<<endl;           
        // }
}

 void * client::sendFrameTCP()
{	
    int sock=createSocketTCP(54000);

	while(true) {
	rs2::context ctx;       
	rs2::rates_printer printer;
	rs2::colorizer color_map;
	std::vector<std::string>  serials;
	for (auto&& dev : ctx.query_devices()) {
		serials.push_back(dev.get_info(RS2_CAMERA_INFO_SERIAL_NUMBER));
		cout << dev.get_info(RS2_CAMERA_INFO_SERIAL_NUMBER) << endl;
	}
        rs2::config cfg;
        cfg.enable_device(serials[0]);
	cfg.enable_stream(RS2_STREAM_COLOR,
	640,480,
	RS2_FORMAT_BGR8, 15);
	rs2::pipeline pipe1(ctx);
	std::map<std::string, rs2::colorizer> colorizers;
	pipe1.start(cfg);
	rs2::frameset frames1;
	vector<rs2::frameset> frame;
        while(true){
	        frames1=pipe1.wait_for_frames();
		rs2::frame color_frame1 = frames1.get_color_frame();
		cv::Mat image(cv::Size(640,480), CV_8UC3, (void*)color_frame1.get_data(), Mat::AUTO_STEP);
                this->img=image.clone();
                 int imageSize = image.total()*image.elemSize();
	        pthread_mutex_lock(&muSend);
			int sendImg=send(sock,image.data,imageSize,0);
			pthread_mutex_unlock(&muSend);

                if(sendImg==-1)
	        {
		        cout<<"Could not send Frame to server"<<endl;
		        continue;
	        }  
            
        }       

}
        
close(sock);
return NULL;
}

 void * client::sendKeyPointsTCP()
{		
		std::this_thread::sleep_for(std::chrono::milliseconds(2000));
        int sock=createSocketTCP(53000);
        // Ptr<cv::SIFT> sift=cv::SIFT::create();
		Ptr<SURF> detector = SURF::create(400);
        std::vector<cv::KeyPoint> keypoints;
		Mat dsc;
		vector<float> descriptors;
		Mat imggray,imgclone;
		while(true){
			 detector->detectAndCompute(this->img,noArray(),keypoints,dsc);
			 this->m_dsc=dsc;
			if (keypoints.size() > 0)
		{
			int sendKeyPoints = send(sock, &keypoints[0], 29 * keypoints.size(), 0);
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
			int sendKeyPoints = send(sock, &keypoints[0], 28 * keypoints.size(), 0);
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

void * client::sendDescriptorTCP()
{
	std::this_thread::sleep_for(std::chrono::milliseconds(2000));
	vector<cv::KeyPoint> keypoints;
	int sock=createSocketTCP(52000);
	int sock2=createSocketTCP(80000);
	// Ptr<SURF> surf=SURF::create();
	std::vector< float > descriptors;
	Mat dsc;
	while(true)
	{
			dsc=this->m_dsc;
			if(!dsc.empty()){
			int dscVecSize=dsc.total()*dsc.elemSize();
			int senddscRow=send(sock2,(char*)&dsc.rows,4,0);
			int senddscCol=send(sock2,(char*)&dsc.cols,4,0);
			int senddsc=send(sock,dsc.data,dscVecSize,0);
				if(senddsc==-1)
				{
					cout<<"Could not send descriptor"<<endl;
					continue;
				}
				// cout << dsc.at<float>(20,20) << endl;

				// imshow("DSC",dsc);
				// if(waitKey(10)==27) break;
			}else {
				continue;
			}

		}
	close(sock);
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