#include "sock.h"

void sock::initUDP(int portNumber,string ipAddress){
	
 this->sock= socket(AF_INET,SOCK_DGRAM,IPPROTO_UDP);
	if(sock==-1)
        {
                cout<<"Fehler Socket"<<endl;
        }

	
	this->server.sin_family= AF_INET;
	this->server.sin_port=htons(portNumber);
	this->server.sin_addr.s_addr = INADDR_ANY;
	// server.sin_addr.s_addr = inet_addr("192.168.137.1");
	int bin = ::bind(sock, (sockaddr*)&server, sizeof(server));

	this->serversend.sin_family= AF_INET;
	this->serversend.sin_port=htons(portNumber);
	this->serversend.sin_addr.s_addr = INADDR_ANY;
	inet_pton(AF_INET,ipAddress.c_str(),&this->serversend.sin_addr);

    tcp=false;

}

void sock::initTCP(int portNumber,string ipAddress)
{
	
 this->sock= socket(AF_INET,SOCK_STREAM,0);
	if(sock==-1)
        {
            cout<<"Fehler Socket"<<endl;
        }
	// string ipAddress = "192.168.137.1";
    sockaddr_in hint;
	hint.sin_family=AF_INET;
    hint.sin_port=htons(portNumber);
    inet_pton(AF_INET,ipAddress.c_str(),&hint.sin_addr);
	int connectRes =connect(sock,(sockaddr*)&hint,sizeof(hint));
    if (connectRes==-1)
        {
        cout<<"Fehler an Port: "<<portNumber<<endl;           
        } 
    else
        {
		cout<<"connected on port: "<< portNumber<<endl;
		}
        tcp=true;
}
void sock::send_img(cv::Mat img){
        vector <uchar> imgVec;
		vector < int > compression_params;
        compression_params.push_back(IMWRITE_JPEG_QUALITY);
        compression_params.push_back(80);

        imencode(".jpg", img, imgVec, compression_params);
        if(tcp==true){
            int vecSize=imgVec.size();
            send(sock,(char*)&vecSize,sizeof(int),0);
            send(sock,(char*)&imgVec[0],imgVec.size(),0);

        }
        else
        {
        int total_pack = 1 + (imgVec.size() - 1) / 65500;
		int ibuf[1];
		ibuf[0]=total_pack;
		sendto(sock,(char*)&ibuf,sizeof(int),0,(sockaddr *)&serversend,sizeof(serversend));

            for (int i = 0; i < total_pack; i++)
		sendto(sock,&imgVec[i * 65500],65500,0,(sockaddr *)&serversend,sizeof(serversend));
        }
}

void sock::send_depth_img(cv::Mat img){
        vector <uchar> imgVec;
		vector < int > compression_params;
        compression_params.push_back(IMWRITE_TIFF_XDPI);

        imencode(".tiff", img, imgVec, compression_params);
        if(tcp==true){
            int vecSize=imgVec.size();
            send(sock,(char*)&vecSize,sizeof(int),0);
            send(sock,(char*)&imgVec[0],imgVec.size(),0);

        }
        else
        {
        int total_pack = 1 + (imgVec.size() - 1) / 65500;
		int ibuf[1];
		ibuf[0]=total_pack;
		int sen=sendto(sock,(char*)&ibuf,sizeof(int),0,(sockaddr *)&serversend,sizeof(serversend));
            for (int i = 0; i < total_pack; i++)
		sendto(sock,&imgVec[i * 65500],65500,0,(sockaddr *)&serversend,sizeof(serversend));
        }
}

void sock::rcv_img(Mat &img) 
{
	// while (true) {
		int recvMsgSize; // Size of received message
		char buflen[sizeof(int)];
		if (tcp == true) {
			int bytesDim = 0;
			int bytes = 0;
			char buf2[4];
			int imgSize;
			memset(buf2,0, sizeof(buf2));

			//Wait for client to send data
			//for (int i = 0; i < sizeof(buf2); i += bytesDim)
				if ((bytesDim = recv(sock, buf2 , sizeof(buf2) , 0)) == -1) cout << "recv number failed" << endl;
			int* pt = (int*)buf2;
			imgSize = *pt;
			char* buf = new char[imgSize];
			memset(buf,0, imgSize);

			for (int i = 0; i < imgSize; i += bytes)
				if ((bytes = recv(sock, buf + i, imgSize - i, 0)) == -1) cout << ("recv failed");
			Mat rawData = Mat(1, imgSize, CV_8UC1, buf);
			Mat frame = imdecode(rawData, IMREAD_COLOR);
			delete[] buf;
			// imshow("frame", frame);
			// waitKey(1);

		}
		else {

			char buffer[65540]; // Buffer for echo string
    struct  sockaddr_in si_other;

                socklen_t slen=sizeof(si_other);
            memset(&si_other,0,sizeof(si_other));
            // memset(&server,0,sizeof(server));

			//ZeroMemory(&cl, sock1.getClientLength()); // Clear the client structure
			//ZeroMemory(buffer, 65540); // Clear the receive buffer

			do {
				recvMsgSize = recvfrom(this->sock, buffer, 65540, 0, (sockaddr *)&si_other,&slen);
} while (recvMsgSize > sizeof(int));

			long int total_pack = ((int*)buffer)[0];
			char* longbuf = new char[PACK_SIZE * total_pack];
			for (int i = 0; i < total_pack; i++) {
				recvMsgSize = recvfrom(this->sock, buffer, 65540, 0, (sockaddr *)&si_other,(socklen_t *)&slen);
				// if (recvMsgSize != PACK_SIZE) {
				// 	cerr << "Received unexpected size pack:" << recvMsgSize << endl;
				// 	continue;
				// }
				memcpy(&longbuf[i * PACK_SIZE], buffer, PACK_SIZE);
			}


			Mat rawData = Mat(1, PACK_SIZE * total_pack, 0, longbuf);
			Mat frame = imdecode(rawData, IMREAD_COLOR);
			 img = frame.clone();
			// if (frame.size().width == 0) {
			// 	cerr << "decode failure!" << endl;
			// 	continue;
			// }
			// namedWindow("recv", WINDOW_FREERATIO);
			// imshow("recv", frame);
			delete[] longbuf;

			// waitKey(1);
		}
	// }
}

bool sock::rcv_hindernis(){
	bool hindernis;
	int bytesBool;
	if (tcp == true) 
	{	char buf[sizeof(bool)];

		if ((bytesBool = recv(this->sock, buf , sizeof(bool) , 0)) == -1) cout << "recv bool failed" << endl;
			bool *b_ptr=(bool*)buf;
			hindernis=b_ptr;

	}
	else
	{

	char buf[sizeof(bool)];

    	struct  sockaddr_in si_other;
		socklen_t slen=sizeof(si_other);
            memset(&si_other,0,sizeof(si_other));
		 bytesBool = recvfrom(this->sock, buf, sizeof(bool), 0, (sockaddr *)&si_other,&slen);
			bool *b_ptr=(bool*)buf;
			hindernis=*b_ptr;
	}
	return hindernis;
}
void sock::closesock(){
	close(this->sock);
}


void sock::initMultiCastUDPrecv(int port, string localIP, string groupIP)
{



	int sockfd;
	struct sockaddr_in local;
	struct ip_mreq group;
	char databuf[1024];
	int datalen;
	struct sockaddr_in src_addr;    /* Used to receive (addr,port) of sender */


	if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
		perror("error creating socket \n");
	}
	else {
		printf("socket created \n");
	}

	int reuse = 1;
	if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, (char*)&reuse,
		sizeof(reuse)) == 1) {
		perror("error on setsockopt \n");
	}
	else {
		printf("sockopt'd successfully \n");
	}

	memset((char*)&local, 0, sizeof(local));
	local.sin_family = AF_INET;
	local.sin_port = htons(port);
	local.sin_addr.s_addr = INADDR_ANY;

	if (::bind(sockfd, (struct sockaddr*)&local, sizeof(local)) == -1) {
		perror("error on binding \n");
	}
	else {
		printf("bound \n");
	}

	group.imr_multiaddr.s_addr = inet_addr(groupIP.c_str());
	group.imr_interface.s_addr = inet_addr(localIP.c_str());
	if (setsockopt(sockfd, IPPROTO_IP, IP_ADD_MEMBERSHIP,
		(char*)&group, sizeof(group)) == -1) {
		perror("adding group error \n");
		close(sockfd);
		exit(1);
	}
	else {
		printf("adding multicast group \n");
	}
	this->sock = sockfd;
}