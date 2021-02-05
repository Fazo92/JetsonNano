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
		 	cout<<"Deskriptoren: "<< dscVec[5]<<endl;
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
		cuda::GpuMat kpGPU,imgGPU;
        std::vector<cv::KeyPoint> keypoints;
		vector<float> descriptors;
		Mat imggray,imgclone;
		while(true){
		imgclone=this->img;	
		cvtColor(imgclone,imggray,COLOR_BGR2GRAY);
		imgGPU.upload(imggray);
		if (imgGPU.empty())
		{
		cout << "Error!!: Upload is not succes" << endl;
		}
	
		// mu.lock();
		pthread_mutex_lock(&mu);
		this->surfCUDA(imgGPU, cuda::GpuMat(), kpGPU, this->dscGPU);
		// sift->detect(this->img,keypoints,noArray());
        // detector->detect(this->img,keypoints,noArray());
		this->surfCUDA.downloadKeypoints(kpGPU,keypoints);
		this->kp=keypoints;
		cout<<"keypoints.pt.x: "<<keypoints[0].pt.x<<endl;
		pthread_mutex_unlock(&mu);
		// mu.unlock();
		for(int i=0; i<keypoints.size();i++)
			{
			int sendKpX=send(sock,(char*)&keypoints[i].pt.x,4,0);
			int sendKpY=send(sock,(char*)&keypoints[i].pt.y,4,0);
			int sendKpSize=send(sock,(char*)&keypoints[i].size,4,0);
			if(sendKpX==-1||sendKpY==-1||sendKpSize==-1)
	        {
		        cout<<"Could not send Keypoints to server"<<endl;
		        continue;
	        }  
			}	
			// cout<<keypoints.size()<<endl;
		}
	close(sock);
	return NULL;
}

void * client::sendKeyPointsTCP2()
{		
		std::this_thread::sleep_for(std::chrono::milliseconds(2000));
        int sock=createSocketTCP(53000);
        Ptr<SURF> detector=SURF::create(400);
        std::vector<cv::KeyPoint> keypoints;
		vector<float> descriptors;
		Mat imgclone,dsc,imgK;
		while(true){
			imgclone=this->img;
			pthread_mutex_lock(&mu);
        detector->detectAndCompute(imgclone,noArray(),keypoints,dsc);
		this->kp=keypoints;
		this->dscVec=descriptors;
		pthread_mutex_unlock(&mu);
		// for(int i=0; i<keypoints.size();i++)
		// 	{
						if(keypoints.size()>0)	{
			cout<<(char*)keypoints.data()<<endl;
			pthread_mutex_lock(&muSend);
			int sendKpX=send(sock,(char*)keypoints.data(),keypoints.size(),0);
			pthread_mutex_unlock(&muSend);

			// int sendKpY=send(sock,(char*)&keypoints[i].pt.y,4,0);
			// int sendKpSize=send(sock,(char*)&keypoints[i].size,4,0);

			// if(sendKpX==-1||sendKpY==-1||sendKpSize==-1)
	        // {
		    //     cout<<"Could not send Keypoints to server"<<endl;
		    //     continue;
	        // }  
			// }
			cout << "kp.pt.x: " << keypoints[0].pt.x << endl;


			cout<<keypoints.size()<<endl;

			}
			
		}
	close(sock);
	return NULL;
}

void * client::sendDescriptorTCP()
{
	std::this_thread::sleep_for(std::chrono::milliseconds(2000));
	Mat dsc;
	vector<cv::KeyPoint> keypoints;
	int sock=createSocketTCP(52000);
	// Ptr<SURF> surf=SURF::create();
	std::vector< float > descriptors;
	while(true)
	{
		keypoints=this->kp;
		this->surfCUDA.downloadDescriptors(this->dscGPU,descriptors);
		// surf->compute(this->img,keypoints,dsc);
		// if(dsc.empty()!=true)
		// {
		// 				imshow("Deskriptor",dsc);
		// 	if(waitKey(1000/20)>=0){
		// 		break;
			
		// 	}
			int dscSize=dsc.total()*dsc.elemSize();
			// this->dscCol=dsc.cols;
			// this->dscRow=dsc.rows;
			// dsc=dsc.reshape(0,1);
			int senddsc=send(sock,dsc.data,dscSize,0);
			if(senddsc==-1)
			{
				cout<<"Could not send descriptor"<<endl;
				continue;
			}

		}
	// }
	close(sock);
	return NULL;

}

void * client::sendCudaDescriptorTCP()
{
	std::this_thread::sleep_for(std::chrono::milliseconds(2000));
	int sock=createSocketTCP(52000);
	std::vector< float > descriptors;
	cuda::SURF_CUDA surfC;

	GpuMat dscCuda;
	while(true)
	{		
		dscCuda=this->dscGPU;
		if(!dscCuda.empty()){
		// mu.lock();
		pthread_mutex_lock(&mu);
		this->surfCUDA.downloadDescriptors(dscCuda,descriptors);
		pthread_mutex_unlock(&mu);
		// mu.unlock();

		this->dscSize=(int)descriptors.size();
		int senddsc=send(sock,(float*)descriptors.data(),(int)descriptors.size(),0);
		if(senddsc==-1)
			{
				cout<<"Could not send descriptor"<<endl;
				continue;
			}

		}
		}


	close(sock);
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