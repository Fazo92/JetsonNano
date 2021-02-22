#include "client.h"


typedef void * (*THREADFUNCPTR)(void *);

int main()
{
		// client cl;
		
		// cl.serialTCP();


	client * cl =new client();
	pthread_t pt1;
	pthread_t pt2;
	pthread_t pt3;
	pthread_t pt4;
	// pthread_t pt4;
	int err1=pthread_create(&pt1,NULL,(THREADFUNCPTR) &client::sendFrameTCP,cl);
	// int err2=pthread_create(&pt2,NULL,(THREADFUNCPTR) &client::sendFeatures,cl);
	int err2=pthread_create(&pt2,NULL,(THREADFUNCPTR) &client::sendKeyPointsTCP,cl);
	int err3=pthread_create(&pt3,NULL,(THREADFUNCPTR) &client::sendDescriptorTCP,cl);
	int err4=pthread_create(&pt4,NULL,(THREADFUNCPTR) &client::detectFeatures,cl);

	err1=pthread_join(pt1,NULL);
	// err2=pthread_join(pt2,NULL);
	err3=pthread_join(pt3,NULL);
	err4=pthread_join(pt4,NULL);

	pthread_exit(NULL);
	// delete(cl);

	return 0;

}
