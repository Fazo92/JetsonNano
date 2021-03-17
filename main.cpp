#include "client.h"
#include "rsfeatures.h"
#include <sys/resource.h>

#pragma comment(linker,"/STACK:1024*1024*2")
#pragma comment(linker, "/HEAP:1024*1024*2")


typedef void * (*THREADFUNCPTR)(void *);

int main()
{
	 const rlim_t kStackSize = 16 * 1024 * 1024;   // min stack size = 16 MB
    struct rlimit rl;
    int result;

    result = getrlimit(RLIMIT_STACK, &rl);
    if (result == 0)
    {
        if (rl.rlim_cur < kStackSize)
        {
            rl.rlim_cur = kStackSize;
            result = setrlimit(RLIMIT_STACK, &rl);
            if (result != 0)
            {
                fprintf(stderr, "setrlimit returned result = %d\n", result);
            }
        }
    }
		client cls;
		rsfeatures f;
		// cls.serialTCP();

	client * cl =new client();
	rsfeatures *rsf=new rsfeatures();
	pthread_t pt1;
	pthread_t pt2;
	pthread_t pt3,pt4,pt5,pt6;

	int err1=pthread_create(&pt1,NULL,(THREADFUNCPTR) &rsfeatures::getFrameRS,rsf);
	int err2=pthread_create(&pt2,NULL,(THREADFUNCPTR) &rsfeatures::detectSURFCUDAFeatures,rsf);
	int err3=pthread_create(&pt3,NULL,(THREADFUNCPTR) &rsfeatures::receive_fts,rsf);
	int err4=pthread_create(&pt4,NULL,(THREADFUNCPTR) &rsfeatures::find_matches,rsf);
	int err5=pthread_create(&pt5,NULL,(THREADFUNCPTR) &rsfeatures::warpImage,rsf);

	// int err2=pthread_create(&pt2,NULL,(THREADFUNCPTR) &rsfeatures::getFrameRS,rsf);
	// int err3=pthread_create(&pt3,NULL,(THREADFUNCPTR) &rsfeatures::surfcpu,rsf);
	// int err4=pthread_create(&pt4,NULL,(THREADFUNCPTR) &rsfeatures::warpImage,rsf);
	// int err5=pthread_create(&pt5,NULL,(THREADFUNCPTR) &rsfeatures::depthFrame,rsf);
	// int err6=pthread_create(&pt6,NULL,(THREADFUNCPTR) &rsfeatures::detectObject,rsf);

	// int err1=pthread_create(&pt1,NULL,(THREADFUNCPTR) &client::serialTCP,cl);
	// int err2=pthread_create(&pt2,NULL,(THREADFUNCPTR) &client::computeHomography,cl);
	// pthread_t pt3;
	// pthread_t pt4;
	// pthread_t pt5;

	// int err1=pthread_create(&pt1,NULL,(THREADFUNCPTR) &client::sendFrameTCP,cl);
	// int err2=pthread_create(&pt2,NULL,(THREADFUNCPTR) &client::sendKeyPointsTCP,cl);
	// int err3=pthread_create(&pt3,NULL,(THREADFUNCPTR) &client::sendDescriptorTCP,cl);
	// int err4=pthread_create(&pt4,NULL,(THREADFUNCPTR) &client::detectFeatures,cl);
	// // int err5=pthread_create(&pt5,NULL,(THREADFUNCPTR) &client::computeHomography,cl);

	err1=pthread_join(pt1,NULL);
	err2=pthread_join(pt2,NULL);
	err3=pthread_join(pt3,NULL);
	err4=pthread_join(pt4,NULL);
	err5=pthread_join(pt5,NULL);

	pthread_exit(NULL);
	delete(rsf);

	return 0;

}
