#include "rsfeatures.h"
#include "frame.h"

void rsfeatures::getdepth_and_colorFrame()
{
	rs2::pipeline pipe;
	rs2::colorizer color_map;
	rs2::config cfg;
	rs2::frameset frames;

		//Add desired streams to configuration
		cfg.enable_stream(RS2_STREAM_COLOR, m_width, m_height, RS2_FORMAT_BGR8, 30);
		pipe.start(cfg);

		for (int i = 0; i < 30; i++)
		{
			frames = pipe.wait_for_frames();
		}

	while (true)
	{

		frames = pipe.wait_for_frames();
		pthread_mutex_lock(&mu_img);
		this->m_img = frame_to_mat(frames.get_color_frame());
		pthread_mutex_unlock(&mu_img);

		if (m_img.empty())
			cout << " Image is empty" << endl;
	}
}
void rsfeatures::compute_fts(){
	sock socket1,socket2;
	socket1.initMultiCastUDPrecv(PORTMULTICASTIMG, IPADRESS, GROUPIPIMG);
	socket2.initMultiCastUDPrecv(PORTHINDERNIS, IPADRESS, GROUPIP);

	while(true){
		Mat img_ref;
		////////////-------Überprüfen ob Basiskamera verdeckt ist.-------////////////
		////////////-------Falls ja, wird die boolsche Membervariable "hindernis" auf true gesetzt.-------////////////
		this->hindernis = socket2.rcv_hindernis();
		////////////------solange Basiskamera verdeckt ist, führe nachfolgende Schleife aus-------////////////
			while(this->hindernis==true)
			{
				if(this->m_img.empty()) continue;
				////////////-------Bild der angeschlossenen Realsense zum Zeitpunkt
				////////////-------der Sichteinschränkung speichern.-------////////////
				pthread_mutex_lock(&mu_img);
				frame f(this->m_img.clone());
				pthread_mutex_unlock(&mu_img);

				f.compute_features_orb();
				pthread_mutex_lock(&mu_dsc_ref);
				this->m_gpu_dsc_ref=f.dsc.clone();
				this->m_kp_ref=f.kp;
				pthread_mutex_unlock(&mu_dsc_ref);
					while(this->hindernis==true)
					{
						//////////-------status der Boolschen Variable überprüfen-----///
						this->hindernis = socket2.rcv_hindernis();
						//////////-------nachfolgende Bilder speichern und Bildmerkmale detektieren--------////////////

						pthread_mutex_lock(&mu_img);
						frame f_next(this->m_img.clone());
						pthread_mutex_unlock(&mu_img);

						f_next.compute_features_orb();
						pthread_mutex_lock(&mu_dsc);
						this->m_gpu_dsc=f_next.dsc.clone();
						this->m_kp=f_next.kp;
						pthread_mutex_unlock(&mu_dsc);
					}

			}
		////////////-------Bilder vom Host empfangen-------////////////////		
		try{
		socket1.rcv_img(img_ref);
		}catch(exception &e) { continue;};
		if(img_ref.empty()) continue;
			frame f_ref(img_ref);
		if(m_img.empty()) continue;
		pthread_mutex_lock(&mu_img);
			frame f(this->m_img.clone());
		pthread_mutex_unlock(&mu_img);
		////////////-------Bildmerkmalserkennung-------////////////////		
		////////////-------für ORB computes_features_orb-------////////////////		
		////////////-------für SURF computes_features_surf-------////////////////		
		////////////-------für SIFT computes_features_sift-------////////////////		

		f.compute_features_orb();

		f_ref.compute_features_orb();

		////////////-------speichern der Features in Membervariablen-------////////////////		
		pthread_mutex_lock(&mu_dsc_ref);
		this->m_gpu_dsc_ref=f_ref.dsc.clone();
		this->m_kp_ref=f_ref.kp;
		pthread_mutex_unlock(&mu_dsc_ref);
		pthread_mutex_lock(&mu_dsc);
		this->m_gpu_dsc=f.dsc.clone();
		this->m_kp=f.kp;
		pthread_mutex_unlock(&mu_dsc);

	}
}
void rsfeatures::homography(){
	Mat K;
	while(true)
	{
	cuda::GpuMat gpu_dsc1,gpu_dsc2;
	Ptr<cuda::DescriptorMatcher> gpu_matcher;
	if(this->m_gpu_dsc_ref.empty()||this->m_gpu_dsc.empty()) continue;
	vector<KeyPoint> kp_ref, kp;

	pthread_mutex_lock(&mu_dsc_ref);
	gpu_dsc1 = this->m_gpu_dsc_ref.clone();
	kp_ref=this->m_kp_ref;
	pthread_mutex_unlock(&mu_dsc_ref);
	pthread_mutex_lock(&mu_dsc);
	gpu_dsc2 = m_gpu_dsc.clone();
	kp=this->m_kp;
	pthread_mutex_unlock(&mu_dsc);
	vector<vector<DMatch>> matches;
	vector<DMatch> good_matches;
	////////////-------falls SURF oder SIFT-> Float Deskriptoren ->NORM_L2-------////////////////		
	////////////-------falls ORB-> binäre Deskriptoren->NORM_HAMMING-------////////////////		

	if(gpu_dsc2.type()==5)
	gpu_matcher = cuda::DescriptorMatcher::createBFMatcher(NORM_L2);
	else gpu_matcher = cuda::DescriptorMatcher::createBFMatcher(NORM_HAMMING);

	gpu_matcher->knnMatch(gpu_dsc1, gpu_dsc2, matches, 2);
	sort(matches.begin(), matches.end());
	for (int i = 0; i < matches.size(); i++)
	{
		if (matches[i][0].distance < 0.75 * matches[i][1].distance)
		{
			good_matches.push_back(matches[i][0]);
		}
	}
	vector<Point2f> obj, scene;
	for (int i = 0; i < good_matches.size(); i++)
	{
		obj.push_back(kp_ref[good_matches[i].queryIdx].pt);
		scene.push_back(kp[good_matches[i].trainIdx].pt);
	}
	if(obj.size()<4) continue;

	////////////-------hier Berechnung der Homographie-------////////////////		

	Mat H=findHomography(scene,obj,RANSAC,3.0);
	if(H.empty()) continue;
	if(determinant(H)<0.5||determinant(H)>1.3) continue;
	////////////-------accumulateWeighted mittelt über vergangene x Homographien-------////////////////		
	////////////-------bei 0.01 z.b über die letzten 100 Homographien-------////////////////		

	if(this->hindernis==false){
	pthread_mutex_lock(&mu_H);
	accumulateWeighted(H,this->m_H,0.01);
	K=this->m_H.clone();
	pthread_mutex_unlock(&mu_H);
	} 
	else
	{
		////////////-------wenn Sichtfeldkamera verdeckt ist,-------////////////////		
		////////////-------wird eine Verkettung der homographie durchgeführt-------////////////////		

		pthread_mutex_lock(&mu_H);
		if(K.empty()) K=this->m_H.clone();
		this->m_H=K*H;
		accumulateWeighted(this->m_H,this->m_H,0.01);
		pthread_mutex_unlock(&mu_H);

	}
	
	}
	
}


void rsfeatures::send_warped_img()
{
	////////////-------Einfachen Socket initialisieren-------////////////////
	sock socket1;
	socket1.initUDP(PORTFRAME, HOSTIP);
	while(true)
	{
		cuda::GpuMat gpu_img;
		if(this->m_img.empty()) continue;
		pthread_mutex_lock(&mu_img);
		gpu_img.upload(this->m_img.clone());
		pthread_mutex_unlock(&mu_img);

		pthread_mutex_lock(&mu_H);
		Mat H=this->m_H.clone();
		pthread_mutex_unlock(&mu_H);
		///////////-------leeres Bild erzeugen und Bild der Realsense mittig positionieren-------////////////////
		Mat C = Mat::zeros(gpu_img.size() * 2, gpu_img.type());
		cuda::GpuMat gpu_C,gpu_warp;
		gpu_C.upload(C);
		cuda::GpuMat roi(gpu_C, Rect(gpu_img.cols / 2, gpu_img.rows / 2, gpu_img.cols, gpu_img.rows));
		gpu_img.copyTo(roi);
		///////////-------Bild transformieren-------////////////////
		cuda::warpPerspective(gpu_C, gpu_warp, H, gpu_C.size(), INTER_LINEAR);
		Mat warp_img;
		///////////-------Bild auf CPU übertragen und senden an Host-------////////////////
		gpu_warp.download(warp_img);
		socket1.send_img(warp_img);

	}
}

void rsfeatures::send_homography(int sock, Mat img)
{
	int imageSize = img.total() * img.elemSize();
	int sendImg = send(sock, img.data, imageSize, 0);
}

double rsfeatures::calcError(vector<Point2f> scene, vector<Point2f> obj, Mat H)
{
	float ex=0;
	float ey=0;
	float e=0;
	H=H.clone();

	for(int i=0;i<obj.size();i++)
	{
		Point2f pt;
		pt.x=(H.at<double>(0,0)*scene[i].x+H.at<double>(0,1)*scene[i].y+H.at<double>(0,2))/(H.at<double>(2,0)*scene[i].x+H.at<double>(2,1)*scene[i].y+H.at<double>(2,2));
		pt.y=(H.at<double>(1,0)*scene[i].x+H.at<double>(1,1)*scene[i].y+H.at<double>(1,2))/(H.at<double>(2,0)*scene[i].x+H.at<double>(2,1)*scene[i].y+H.at<double>(2,2));
		ex+=abs(pt.x-obj[i].x);
		ey+=abs(pt.y-obj[i].y);

	}
	e=(ex+ey)/(2*obj.size());
	return e;
}

void rsfeatures::optFlow()
{
	sock socket1;
	socket1.initMultiCastUDPrecv(PORTMULTICASTIMG, IPADRESS, GROUPIPIMG);

	while (true)
	{
		if (this->m_img.empty())
			continue;
		pthread_mutex_lock(&mu_img);
		frame f(this->m_img.clone());
		pthread_mutex_unlock(&mu_img);
		Mat img_ref;
		try
		{
			socket1.rcv_img(img_ref);
		}
		catch (exception &e)
		{
			continue;
		};
		if(img_ref.empty()) continue;
		f.getPointsOptFlow(img_ref);
		Mat H = findHomography(f.prev_pts, f.next_pts, RANSAC, 3.0);
		if (H.empty())
			continue;
		if (determinant(H) < 0.5 || determinant(H) > 1.3)
			continue;
		pthread_mutex_lock(&mu_H);
		accumulateWeighted(H, this->m_H, 0.01);
		pthread_mutex_unlock(&mu_H);

	}
}