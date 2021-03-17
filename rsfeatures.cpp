#include "rsfeatures.h"
#include "client.h"

void rsfeatures::getFrameTCP(){
    client cl;
    int sock=cl.createSocketTCP(10000);

     this->m_sock1=sock;
    int height = 480;
		int width = 640;
		int bytes = 0;
			// char buf[921600];
                const int dataSize = 921600;
    char* buf = new char[dataSize];
while (true)
	{
		Mat img = Mat::zeros(height, width, CV_8UC3);
		int imgSize;
		imgSize = img.total() * img.elemSize();
		memset(buf,0, imgSize);
		//Wait for client to send data
		for (int i = 0; i < imgSize; i += bytes)
			if ((bytes = recv(sock, buf + i, imgSize - i, 0)) == -1) cout << ("recv failed");
		int ptr = 0;
		for (int i = 0; i < img.rows; i++) {
			for (int j = 0; j < img.cols; j++) {
				img.at<Vec3b>(i, j) = Vec3b(buf[ptr + 0], buf[ptr + 1], buf[ptr + 2]);
				ptr = ptr + 3;

			}}
			if (bytes == -1)
		{
			cerr << "Error in recv().Quitting" << endl;
			break;
		}
		if (bytes == 0)
		{
			cout << "Client disconnected" << endl;
			break;
		}
		this->m_img1=img;
}
        close(sock);

}

rs2::pipeline rsfeatures::setRS(int fps){
		int width = m_width;
	int height = m_height;
	Size sz = Size(m_width, m_height);
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
void rsfeatures::getFrameRS(){

    rs2::pipeline pipe=setRS(15);
	while(true){
		rs2::frameset frames;
	frames = pipe.wait_for_frames();
			Size sz = Size(m_width, m_height);
		rs2::frame color_frame = frames.get_color_frame();
		Mat color(sz, CV_8UC3, (void*)color_frame.get_data(), Mat::AUTO_STEP);
		// imshow("as Socket",color);
		// waitKey(0);
		Mat imgRS=color.clone();
				this->m_img2= imgRS;

	}
}

void rsfeatures::getPointsOptFlow() {
	while(true){
        if(this->m_img1.empty()||this->m_img2.empty()) continue;
        Mat prev_img=this->m_img1;
        Mat next_img=this->m_img2;
        vector<Point2f> prev_pts,next_pts;
    int win_size = 10;
	// The first thing we need to do is get the features
	// we want to track.
	//
	cuda::GpuMat gpu_prev_img, gpu_next_img, gpu_cornersA, gpu_cornersB, gpu_features_found, gpu_err;

	cv::cvtColor(prev_img, prev_img, COLOR_BGR2GRAY);
	cv::cvtColor(next_img, next_img, COLOR_BGR2GRAY);

	gpu_prev_img.upload(prev_img);
	gpu_next_img.upload(next_img);
	Mat err;
	vector< cv::Point2f > cornersA, cornersB;
	const int MAX_CORNERS = 500;
	Ptr<cuda::CornersDetector> detector = cuda::createGoodFeaturesToTrackDetector(gpu_prev_img.type(),1000, 0.01, 5);
	detector->detect(gpu_prev_img, gpu_cornersA);
	
	Ptr<cuda::SparsePyrLKOpticalFlow> d_pyrLK_sparse = cuda::SparsePyrLKOpticalFlow::create(
		Size(win_size * 2 + 1, win_size * 2 + 1),5, 20);
	vector<uchar> features_found;
	
	
	d_pyrLK_sparse->calc(gpu_prev_img, gpu_next_img, gpu_cornersA, gpu_cornersB, gpu_features_found, gpu_err);
	gpu_cornersA.download(cornersA);
	gpu_cornersB.download(cornersB);
    gpu_features_found.download(features_found);
    gpu_err.download(err);

	for (int i = 0; i < cornersA.size(); i++)
	{
		if (features_found[i] == 1&& err.at<float>(i,0)<45.)
		{
			prev_pts.push_back(cornersA[i]);
			next_pts.push_back(cornersB[i]);
		}
	}

    this->m_prev_pts=prev_pts;
    this->m_next_pts=next_pts;
    }
}

void rsfeatures::getPointsOptFlowCPU(){
    while(true){
    if(this->m_img1.empty()||this->m_img2.empty()) continue;
        Mat prev_img=this->m_img1;
        Mat next_img=this->m_img2;
        cv::cvtColor(prev_img,prev_img,COLOR_BGR2GRAY);
        cv::cvtColor(next_img,next_img,COLOR_BGR2GRAY);

        vector<Point2f> prev_pts,next_pts;
        vector<Point2f> cornersA,cornersB;
    	Mat err;
	vector<uchar> features_found;

    int win_size = 10;
    	cv::goodFeaturesToTrack(
		prev_img,                         // Image to track
		cornersA,                     // Vector of detected corners (output)
		500,                  // Keep up to this many corners
		0.01,                         // Quality level (percent of maximum)
		5,                            // Min distance between corners
		cv::noArray(),                // Mask
		3,                            // Block size
		true,                        // true: Harris, false: Shi-Tomasi
		0.04                          // method specific parameter
	);

	cv::cornerSubPix(
		prev_img,                           // Input image
		cornersA,                       // Vector of corners (input and output)
		cv::Size(win_size, win_size),   // Half side length of search window
		cv::Size(-1, -1),               // Half side length of dead zone (-1=none)
		cv::TermCriteria(
			cv::TermCriteria::MAX_ITER | cv::TermCriteria::EPS,
			20,                         // Maximum number of iterations
			0.03                        // Minimum change per iteration
		)
	);

	cv::calcOpticalFlowPyrLK(
		prev_img,                         // Previous image
		next_img,                         // Next image
		cornersA,                     // Previous set of corners (from imgA)
		cornersB,                     // Next set of corners (from imgB)
		features_found,               // Output vector, each is 1 for tracked
		err,                // Output vector, lists errors (optional)
		cv::Size(win_size * 2 + 1, win_size * 2 + 1),  // Search window size
		5,                            // Maximum pyramid level to construct
		cv::TermCriteria(
			cv::TermCriteria::MAX_ITER | cv::TermCriteria::EPS,
			20,                         // Maximum number of iterations
			0.3                         // Minimum change per iteration
		)
	);

	for (int i = 0; i < cornersA.size(); i++)
	{
		if (features_found[i] == 1&& err.at<float>(i,0)<45.)
		{
			prev_pts.push_back(cornersA[i]);
			next_pts.push_back(cornersB[i]);
		}
	}
    this->m_prev_pts=prev_pts;
    this->m_next_pts=next_pts;
    }
}


void rsfeatures::cudaSurf(){

  while(true){

    SURF_CUDA surf;
    if(this->m_img2.empty()||this->m_img1.empty()) continue;
    // detecting keypoints & computing descriptors
    GpuMat keypoints1GPU, keypoints2GPU,img1,img2;
    GpuMat descriptors1GPU, descriptors2GPU;
    img1.upload(this->m_img1);
    img2.upload(this->m_img2);
    cuda::cvtColor(img1,img1,COLOR_BGR2GRAY);
    cuda::cvtColor(img2,img2,COLOR_BGR2GRAY);

    surf(img1, cuda::GpuMat(), keypoints1GPU, descriptors1GPU);
    surf(img2, cuda::GpuMat(), keypoints2GPU, descriptors2GPU);

     // matching descriptors
    Ptr<cv::cuda::DescriptorMatcher> matcher = cv::cuda::DescriptorMatcher::createBFMatcher(NORM_L2);
    vector<vector<DMatch>> matches;
    // matcher->match(descriptors1GPU, descriptors2GPU, matches);
	matcher->knnMatch(descriptors1GPU, descriptors2GPU, matches, 2);

    // downloading results
    vector<KeyPoint> keypoints1, keypoints2;
    vector<float> descriptors1, descriptors2;
    surf.downloadKeypoints(keypoints1GPU, keypoints1);
    surf.downloadKeypoints(keypoints2GPU, keypoints2);
    // surf.downloadDescriptors(descriptors1GPU, descriptors1);
    // surf.downloadDescriptors(descriptors2GPU, descriptors2);
	vector<DMatch> mt1;

	for (int i = 0; i < matches.size(); i++) {
		if (matches[i][0].distance < 0.7 * matches[i][1].distance) {
			mt1.push_back(matches[i][0]);
		}
	}
    	vector<Point2f> objtmp, scenetmp;
	for (int i = 0; i < mt1.size(); i++) {
	//if ((matches[i].queryIdx < kpObj.size()) && (matches[i].trainIdx < kpScn.size())) 
		scenetmp.push_back(keypoints1[mt1[i].queryIdx].pt);
		objtmp.push_back(keypoints2[mt1[i].trainIdx].pt);	
	}
    // cout << "scene size: " << scenetmp.size() << endl;
	// cout << "obj size: " << objtmp.size() << endl;
	
    this->m_prev_pts=scenetmp;
    this->m_next_pts=objtmp;
  }
}

void rsfeatures::surfcpu(){
    while(true){
    cuda::GpuMat imgGray1, imgGray2,gpu_dsc1,gpu_dsc2;
	cuda::GpuMat img1, img2;

    if(this->m_img1.empty()||this->m_img2.empty()) continue;
    	img1.upload(this->m_img1);
	img2.upload(this->m_img2);
	cuda::cvtColor(img1, imgGray1, COLOR_BGR2GRAY);
	cuda::cvtColor(img2, imgGray2, COLOR_BGR2GRAY);
	Mat cpu_gray1, cpu_gray2;
	imgGray1.download(cpu_gray1);
	imgGray2.download(cpu_gray2);

	Ptr<SURF> detector = SURF::create(400);
	Mat surfdescriptor1, surfdescriptor2;
    vector<KeyPoint> kp1,kp2; 

	detector->detectAndCompute(cpu_gray1, noArray(), kp1, surfdescriptor1);
	detector->detectAndCompute(cpu_gray2, noArray(), kp2, surfdescriptor2);
	vector<float> dsc1,dsc2;


	Ptr<cuda::DescriptorMatcher> matcherGPU;
	matcherGPU = cuda::DescriptorMatcher::createBFMatcher(NORM_L2);
	vector<vector<DMatch>> matches;
	vector<DMatch> gpu_matches;
	gpu_dsc1.upload(surfdescriptor1);
	gpu_dsc2.upload(surfdescriptor2);
	matcherGPU->knnMatch(gpu_dsc1, gpu_dsc2, matches, 2);

	vector<DMatch> mt1;



	for (int i = 0; i < matches.size(); i++) {
		if (matches[i][0].distance < 0.75 * matches[i][1].distance) {
			mt1.push_back(matches[i][0]);
		}
	}
    	vector<Point2f> objtmp, scenetmp;
	for (int i = 0; i < mt1.size(); i++) {
	//if ((matches[i].queryIdx < kpObj.size()) && (matches[i].trainIdx < kpScn.size())) 
		scenetmp.push_back(kp1[mt1[i].queryIdx].pt);
		objtmp.push_back(kp2[mt1[i].trainIdx].pt);	
	}
    // cout << "scene size: " << scenetmp.size() << endl;
	// cout << "obj size: " << objtmp.size() << endl;
	
    this->m_prev_pts=scenetmp;
    this->m_next_pts=objtmp;
	//cout << mt1.size() << endl;
    }
}

void rsfeatures::depthFrame(){
	rs2::pipeline pipe;
	rs2::colorizer color_map;
	pipe.start();
while(true){
    rs2::frameset data = pipe.wait_for_frames(); // Wait for next set of frames from the camera
    rs2::frame depth = data.get_depth_frame().apply_filter(color_map);

    // Query frame size (width and height)
    const int w = depth.as<rs2::video_frame>().get_width();
    const int h = depth.as<rs2::video_frame>().get_height();

    // Create OpenCV matrix of size (w,h) from the colorized depth data
    Mat image(Size(w, h), CV_8UC3, (void*)depth.get_data(), Mat::AUTO_STEP);
}
}

// void rsfeatures::detectObject() {

//     client cl;
//     int sock=cl.createSocketTCP(12000);
//     while(true){
//         if(this->dFrame.empty()||this->m_img2.empty()) continue;
//     Mat dephImage=this->dFrame;
//     Mat img=this->m_img2;
//     RNG rng(12345);
//     Mat hsv;
//     //convert RGB image into HSV image  
//     cvtColor(depthImage, hsv, COLOR_BGR2HSV);
//     //imshow("HSV",hsv);  
//     Mat binary, binary1, imgToProcess;
//     //get binary image  
//     inRange(hsv, Scalar(0, 85, 241), Scalar(18, 255, 255), binary);
//     // imshow("Binary",binary);
//     inRange(hsv, Scalar(171, 0, 0), Scalar(255, 255, 255), binary1);

//     //binary.copyTo(binary1);
//     //imshow("Binary1",binary1);
//     add(binary1, binary, imgToProcess, noArray(), 8);

//     //absdiff(binary1, binary, imgToProcess);
//     //imshow("Binary2", imgToProcess);


//     //find contours from binary image  
//     vector< vector<Point> > contours;
//     vector<Vec4i> hierarchy;
//     findContours(imgToProcess, contours, hierarchy, RETR_TREE, CHAIN_APPROX_SIMPLE, Point(0, 0)); //find contours  

//     vector<vector<Point> > contours_poly(contours.size());
//     vector<RotatedRect> minRect(contours.size());
//     vector<RotatedRect> minEllipse(contours.size());
//     vector<Rect> boundRect(contours.size());
//     vector<float>radius(contours.size());
//     vector<float>area(contours.size());
//     vector<Point2f>center(contours.size());

//     for (size_t i = 0; i < contours.size(); i++)
//     {
//         approxPolyDP(Mat(contours[i]), contours_poly[i], 3, true);
//         boundRect[i] = boundingRect(Mat(contours_poly[i]));
//         minEnclosingCircle(contours_poly[i], center[i], radius[i]);
//         area[i] = contourArea(Mat(contours_poly[i]));
//     }

//     int areaC = 0;
//     int areaC2 = 0;
//     int areaIdx = 0;
//     int areaIdx2 = 0;
//     Mat drawing = Mat::zeros(imgToProcess.size(), CV_8UC3);
//     for (int i = 0; i < contours.size(); i++)
//     {
//         Scalar color = Scalar(rng.uniform(0, 255), rng.uniform(0, 255), rng.uniform(0, 255));
//         drawContours(drawing, contours, i, color, 1, 8, vector<Vec4i>(), 0, Point());
//         ellipse(drawing, minEllipse[i], color, 2, 8);// ellipse
//         if (boundRect[i].area() > areaC) {
//             areaC = boundRect[i].area();
//             areaIdx = i;
//         }
//         if (boundRect[i].area() > areaC2 && boundRect[i].area() < areaC) {
//             areaC2 = boundRect[i].area();
//             areaIdx2 = i;

//         }

//         //rectangle(image, boundRect[i].tl(), boundRect[i].br(), color, 2, 8, 0);
//         //Point2f rect_points[4]; minRect[i].points( rect_points );
//          //for( int j = 0; j < 4; j++ )
//            //   line( drawing, rect_points[j], rect_points[(j+1)%4], color, 1, 8 );

//     }
//     rectangle(img, boundRect[areaIdx].tl(), boundRect[areaIdx].br(), (0, 0, 255), 2, 8, 0);
//     rectangle(img, boundRect[areaIdx2].tl(), boundRect[areaIdx2].br(), (0, 0, 255), 2, 8, 0);
//     this->detecedObject = img(Rect(boundRect[areaIdx]));
//     int imgSize=this->detecedObject.elemSize()*detecedObject.total();
//     this->rct = boundRect[areaIdx];
//     int size=rct.width*rct.height*sizeof(Rect);
//         int sendDobject= send(sock,rct.data,imgSize,0);

//     }

// }


void rsfeatures::detectCUDAFeatures(){
	client cl;
	int sock=cl.createSocketTCP(PORTDSC);
	int sockkpts=cl.createSocketTCP(PORTKEYPTS);

	 while(true){
    cuda::GpuMat imgGray1, imgGray2,gpu_dsc1,gpu_dsc2;
	cuda::GpuMat img1, img2;

    if(this->m_img2.empty()) continue;
    	// img1.upload(this->m_img1);
	img1.upload(this->m_img2);
	cuda::cvtColor(img1, imgGray1, COLOR_BGR2GRAY);
	// cuda::cvtColor(img2, imgGray2, COLOR_BGR2GRAY);
	Mat cpu_gray1, cpu_gray2;
	imgGray1.download(cpu_gray1);
	// imgGray2.download(cpu_gray2);

	Ptr<SURF> detector = SURF::create(40);
	Mat surfdescriptor1, surfdescriptor2;
    vector<KeyPoint> kp1; 

	detector->detectAndCompute(cpu_gray1, noArray(), kp1, surfdescriptor1);
	Mat dscTCP=receive_dsc(sock);
	vector<KeyPoint> kp2=recv_kpts(sockkpts); 
			cout << " kp size:"<<kp2.size() <<endl;
			cout << " dsc size:"<<dscTCP.size() <<endl;

	Ptr<cuda::DescriptorMatcher> gpu_matcher;
	gpu_matcher = cuda::DescriptorMatcher::createBFMatcher(NORM_L2);
	vector < vector<DMatch>> matches;
	vector<DMatch> gpu_matches;
	gpu_dsc1.upload(surfdescriptor1);
	gpu_dsc2.upload(dscTCP);
	gpu_matcher->knnMatch(gpu_dsc1, gpu_dsc2, matches, 2);
		for(int i=0;i<matches.size();i++){
			if(matches[i][0].distance<0.75*matches[i][1].distance){
				gpu_matches.push_back(matches[i][0]);
			}
		}
		sort(gpu_matches.begin(),gpu_matches.end());
	vector<Point2f> objtmp, scenetmp;

	for (int i = 0; i < gpu_matches.size(); i++) {
		if(gpu_matches[i].queryIdx>kp1.size()||gpu_matches[i].trainIdx>kp2.size()) continue; 
		objtmp.push_back(kp1[gpu_matches[i].queryIdx].pt);
		scenetmp.push_back(kp2[gpu_matches[i].trainIdx].pt);	
	}
    // cout << "scene size: " << scenetmp.size() << endl;
	// cout << "obj size: " << objtmp.size() << endl;
	
    this->m_prev_pts=scenetmp;
    this->m_next_pts=objtmp;	
	}
}

Mat rsfeatures::receive_dsc(int sock){
	int bytes=0;
	const int dscSize=64*10000*4;
	char *buf=new char[dscSize];
		memset(buf,0,dscSize);
			if((bytes=recv(sock,buf,dscSize,0))==-1) cout<<"dsc recv failed"<<endl;	
		int dsc_cols=64;

		int dsc_rows=bytes/(dsc_cols*4);
		float *p=(float*)buf;
		Mat dsc=Mat::zeros(dsc_rows,dsc_cols,CV_32FC1);
		for(int i=0;i<dsc_rows;i++){
			float* ptrdsc=dsc.ptr<float>(i);
			for( int j=0;j<dsc_cols;j++){
				ptrdsc[j]=*p;
				p++;
			}
		}
		string s="3dsc";
		send(sock,s.c_str(),100,0);
		return dsc;
	
}


vector<KeyPoint> rsfeatures::recv_kpts(int sock) {
	int bytes = 0;
	vector<KeyPoint> keyPoints;
	int kpSize = 1024 * 10 * sizeof(KeyPoint);
	char* buf = new char[kpSize];

	memset(buf,0, kpSize);
	if ((bytes = recv(sock, buf, kpSize, 0)) == -1) cout << ("recv kp failed");


	KeyPoint* p;
	for (p = (KeyPoint*)&buf[0]; p <= (KeyPoint*)&buf[bytes - 1]; p++) {
		keyPoints.push_back(*p);
	}

	string s = "3kp";
	int sendKpts = send(sock, s.c_str(), 10, 0);
	return keyPoints;
}

void rsfeatures::send_kpts(int sock,vector<KeyPoint> kpts){
	char *buf=new char[100];

	int kpts_size=kpts.size()*sizeof(KeyPoint);
	int send_bytes=send(sock,&kpts[0],kpts_size,0);
	memset(buf,0,100);
	int bytes=recv(sock,buf,sizeof(buf),0);
		cout << "bytes of kpts: " << kpts_size << endl;

}

void rsfeatures::send_dsc(int sock, Mat dsc){
	char *buf=new char[100];
	int dsc_size=dsc.total()*dsc.elemSize();
	int send_bytes=send(sock,dsc.data,dsc_size,0);
		memset(buf,0,100);

	int bytes=recv(sock,buf,sizeof(buf),0);

}
void rsfeatures::send_img(int sock,Mat img){
	char* buf = new char[10];
	int bytes=0;
	// vector<int> compressionsparam;
	// vector<uchar> encoded;

	// compressionsparam.push_back(IMWRITE_JPEG_QUALITY);
	// compressionsparam.push_back(80);
	// imencode(".jpg",img,encoded,compressionsparam);
	int imageSize=img.total()*img.elemSize();
	// int sendImg=send(sock,&encoded[0],encoded.size()*sizeof(encoded),0);
		int sendImg=send(sock,img.data,imageSize,0);
        cout<< "sendimg: " << sendImg<<endl;

	// memset(buf,0,10);
	// bytes=recv(sock,buf,10,0);
}


void rsfeatures::detectSURFCUDAFeatures(){


	 while(true){
    cuda::GpuMat imgGray1, imgGray2,gpu_dsc1,gpu_dsc2,gpu_kp1,gpu_kp2;
	cuda::GpuMat img1, img2;

    if(this->m_img2.empty()) continue;
    	// img1.upload(this->m_img1);
	img1.upload(this->m_img2);
	cuda::cvtColor(img1, imgGray1, COLOR_BGR2GRAY);
	// cuda::cvtColor(img2, imgGray2, COLOR_BGR2GRAY);
	Mat cpu_gray1, cpu_gray2;
	// imgGray1.download(cpu_gray1);
	// imgGray2.download(cpu_gray2);

	Mat surfdescriptor1, surfdescriptor2;
    vector<KeyPoint> kp1; 
	cv::Ptr<cuda::SURF_CUDA> surf=cv::cuda::SURF_CUDA::create(40,4,2,false);
	surf->detectWithDescriptors(imgGray1,cuda::GpuMat(),gpu_kp1,gpu_dsc1);
	surf->downloadKeypoints(gpu_kp1,kp1);
	gpu_dsc1.download(surfdescriptor1);
	this->m_dsc=surfdescriptor1;
	this->m_kpts=kp1;
	// send_dsc(sock,surfdescriptor1);
	// send_kpts(sockkpts,kp1);
	// Mat dscTCP=receive_dsc(sock);
	// vector<KeyPoint> kp2=recv_kpts(sockkpts); 


	// Ptr<cuda::DescriptorMatcher> gpu_matcher;
	// gpu_matcher = cuda::DescriptorMatcher::createBFMatcher(NORM_L2);
	// vector < vector<DMatch>> matches;
	// vector<DMatch> gpu_matches;
	// // gpu_dsc1.upload(surfdescriptor1);
	// gpu_dsc2.upload(dscTCP);

	// gpu_matcher->knnMatch(gpu_dsc1, gpu_dsc2, matches, 2);
	// 		sort(matches.begin(),matches.end());
	// 	for(int i=0;i<matches.size();i++){
	// 		if(matches[i][0].distance<0.75*matches[i][1].distance){
	// 			gpu_matches.push_back(matches[i][0]);
	// 		}
	// 	}
	// vector<Point2f> objtmp, scenetmp;
	// for (int i = 0; i < gpu_matches.size(); i++) {
	// 	if(gpu_matches[i].queryIdx>kp1.size()||gpu_matches[i].trainIdx>kp2.size()) continue; 
	// 	objtmp.push_back(kp1[gpu_matches[i].queryIdx].pt);
	// 	scenetmp.push_back(kp2[gpu_matches[i].trainIdx].pt);	
	// }
    // // cout << "scene size: " << scenetmp.size() << endl;
	// // cout << "obj size: " << objtmp.size() << endl;
	
    // this->m_prev_pts=scenetmp;
    // this->m_next_pts=objtmp;	
	}

}

void rsfeatures::warpImage(){
    client cl;
     int sock=cl.createSocketTCP(PORTFRAME);
	std::this_thread::sleep_for(std::chrono::milliseconds(100));
    while(true){ 
		if(m_prev_pts.empty()||m_next_pts.empty()) continue;
		Mat img=this->m_img2;

        // if(m_prev_pts.size()<4||m_next_pts.size()<4) {
		// 	 cout<< "   pts empty  " <<endl;

		// 	if(img.empty()) continue;
		// 	Mat img_alt;
		// 	cv::resize(img,img_alt,img.size()*2);
		// 	send_img(sock,img_alt); 
		// 			         cout<< "   pts empty  " <<endl;

		// 	continue;
		// }
        vector< Point2f > pts1 = this->m_prev_pts;
	    vector<Point2f > pts2 =this->m_next_pts;
        cuda::GpuMat gpu_img;
        gpu_img.upload(img);
        // Mat C=Mat::zeros(img.size()*2,img.type());
        cuda::GpuMat gpu_C(img.size()*2, CV_8UC3,Scalar::all(0));

        Mat H=findHomography(pts2,pts1,RANSAC);

		    if(H.empty()) continue;
        Mat M = H(Rect(0, 0, 2, 2));


        // if(determinant(M)>0&&determinant(M)<1){
        //     this->m_H=H;
        // } else {
		// 	if(!this->m_H.empty())
        //     H=this->m_H;
        // }
		//          cout<< determinant(M) <<endl;

		// 		    if(H.empty()) continue;

        // Mat roi(C, Rect(img.cols / 2, img.rows / 2, img.cols, img.rows));
        cuda::GpuMat roi(gpu_C, Rect(img.cols / 2, img.rows / 2, img.cols, img.rows));
        if(determinant(M)>1||determinant(M)<0){
			if(!this->m_H.empty())
			H=this->m_H;
		} 

        gpu_img.copyTo(roi);
        // if(determinant(H)<0) continue;
        //  cout<< determinant(M) <<endl;
		this->m_H=H;

        Mat warp_img;
        cuda::GpuMat gpu_warp;
        cuda::warpPerspective(gpu_C,gpu_warp,H,gpu_C.size(),INTER_CUBIC);
		cuda::resize(gpu_warp,gpu_warp,Size(640*2,480*2));
        gpu_warp.download(warp_img);

		send_img(sock,warp_img);
		// imshow("warp", warp_img);
		// if (waitKey(10000 / 30) >= 0) break;		

    }
    close(sock);
}

    void rsfeatures::send_fts(){
	client cl;
	int sock=cl.createSocketTCP(PORTDSC);
	int sockkpts=cl.createSocketTCP(PORTKEYPTS);
	int sockdim=cl.createSocketTCP(PORTDIM);

	char *buf=new char[100];

	while(true){
		if(this->m_kpts.empty()||this->m_dsc.empty()) continue;
	vector<KeyPoint> kpts;
	Mat dsc;
	kpts=this->m_kpts;
	dsc=this->m_dsc;
	int dsc_size=dsc.total()*dsc.elemSize();
	int kp_size=kpts.size()*sizeof(KeyPoint);

	send(sockdim,&dsc_size,sizeof(dsc_size),0);
		send(sockdim,&kp_size,sizeof(kp_size),0);

	int send_bytes=send(sock,dsc.data,dsc_size,0);
	int kpts_size=kpts.size()*sizeof(KeyPoint);
	int kp_sz=kpts.size();

		send(sockkpts,&kp_sz,sizeof(kpts.size()),0);

	int send_bytes_kpts=send(sockkpts,&kpts[0],kpts_size,0);
				cout << " dsc size:"<<dsc.size() <<endl;

			cout << " kp size:"<<kpts.size() <<endl;
	// 	memset(buf,0,100);

	// int bytes=recv(sock,buf,sizeof(buf),0);

	}
		close(sock);
		close(sockkpts);
	}

void rsfeatures::receive_fts(){
	client cl;
	int sock=cl.createSocketTCP(PORTDSC);
	int sockkpts=cl.createSocketTCP(PORTKEYPTS);
	int sockdim=cl.createSocketTCP(PORTDIM);
	while (true){
		int bytes = 0;
	int bytes_kp = 0;
	int buf_dsc;
	int kp_size;

	memset((char*)&buf_dsc, 0,sizeof(buf_dsc));
	memset((char*)&kp_size,0, sizeof(kp_size));


	recv(sockdim, (char*)&buf_dsc, sizeof(buf_dsc), 0);
	recv(sockdim, (char*)&kp_size, sizeof(kp_size), 0);

	//const int dscSize = 1024*1024*sizeof(float);
	const int dscSize = buf_dsc;
	cout << "buf_dsc: " << buf_dsc << endl;
	char* buf = new char[dscSize];
	memset(buf,0, dscSize);
	for (int i = 0; i < dscSize; i += bytes)
		if ((bytes = recv(sock, buf + i, dscSize - i, 0)) == -1) cout << "dsc recv failed" << endl;
	int dsc_cols = 64;
	int dsc_rows = buf_dsc / (dsc_cols * 4);
	float* p = (float*)buf;
	Mat dsc = Mat::zeros(dsc_rows, dsc_cols, CV_32FC1);
	for (int i = 0; i < dsc_rows; i++) {
		float* ptrdsc = dsc.ptr<float>(i);
		for (int j = 0; j < dsc_cols; j++) {
			ptrdsc[j] = *p;
			p++;
		}
	}

	///////////////////////////////////////////////////
	std::vector<KeyPoint> keyPoints;
	//const int kpSize = 1024 * 10 * sizeof(KeyPoint);
	const int kpSize = kp_size;

	char* buf_kp = new char[kpSize];

	memset(buf_kp,0, kpSize);
	for (int i = 0; i < kp_size; i += bytes_kp)
		if ((bytes_kp = recv(sockkpts, buf_kp + i, kpSize - i, 0)) == -1) cout << ("recv kp failed");

	KeyPoint* p_kp;
	for (p_kp = (KeyPoint*)&buf_kp[0]; p_kp <= (KeyPoint*)&buf_kp[bytes_kp - 1]; p_kp++) {
		keyPoints.push_back(*p_kp);
	}
	if (bytes_kp == -1)
	{
		cout << "Error in recv().Quitting" << endl;
	}
	cout << "dsc: " << dsc.size() << endl;
	cout << " kpts: " << keyPoints.size() << endl;

	this->m_kptsC=keyPoints;
	this->m_dscC=dsc;
	}
	close(sock);
	close(sockkpts);
	close(sockdim);
}
void rsfeatures::find_matches(){
	while (true){
		if(m_dsc.empty()||m_dscC.empty()) continue;
	Ptr<cuda::DescriptorMatcher> gpu_matcher;
	gpu_matcher = cuda::DescriptorMatcher::createBFMatcher(NORM_L2);
	vector < vector<DMatch>> matches;
	vector<DMatch> gpu_matches;
	Mat dsc_R=this->m_dsc;
	Mat dsc_C=this->m_dscC;
	vector<KeyPoint> kp_R,kp_C;
	kp_R=this->m_kpts;
	kp_C=this->m_kptsC;
	cuda::GpuMat gpu_dsc1,gpu_dsc2;
	gpu_dsc1.upload(dsc_R);
	gpu_dsc2.upload(dsc_C);

	gpu_matcher->knnMatch(gpu_dsc1, gpu_dsc2, matches, 2);
			sort(matches.begin(),matches.end());
		for(int i=0;i<matches.size();i++){
			if(matches[i][0].distance<0.75*matches[i][1].distance){
				gpu_matches.push_back(matches[i][0]);
			}
		}
		sort(gpu_matches.begin(),gpu_matches.end());

	vector<Point2f> objtmp, scenetmp;
	for (int i = 0; i < gpu_matches.size(); i++) {
		if(gpu_matches[i].queryIdx>kp_R.size()||gpu_matches[i].trainIdx>kp_C.size()) continue; 
		objtmp.push_back(kp_R[gpu_matches[i].queryIdx].pt);
		scenetmp.push_back(kp_C[gpu_matches[i].trainIdx].pt);	
	}
    // cout << "scene size: " << scenetmp[12].x << endl;
	// cout << "obj size: " << objtmp[12].x << endl;
	this->m_prev_pts=scenetmp;
    this->m_next_pts=objtmp;
	}
}