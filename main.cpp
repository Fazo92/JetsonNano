#include "frame.h"
#include "rsfeatures.h"
#include <sys/resource.h>

typedef void * (*THREADFUNCPTR)(void *);
int main()
{   
	rsfeatures *rsf=new rsfeatures();

	pthread_t pt1,pt2,pt3,pt4;
	///////////////////Falls optischer Pfad verwendet werden möchte, errOpt einkommentieren 
	/////////////////// und err2 und err3 auskommentieren
	int err1=pthread_create(&pt1,NULL,(THREADFUNCPTR) &rsfeatures::getdepth_and_colorFrame,rsf);
	
	int err2=pthread_create(&pt2,NULL,(THREADFUNCPTR) &rsfeatures::compute_fts,rsf);
	int err3=pthread_create(&pt3,NULL,(THREADFUNCPTR) &rsfeatures::homography,rsf);
	
	// int errOpt=pthread_create(&pt3,NULL,(THREADFUNCPTR) &rsfeatures::optFlow,rsf);

	int err4=pthread_create(&pt4,NULL,(THREADFUNCPTR) &rsfeatures::send_warped_img,rsf);
	

	err1=pthread_join(pt1,NULL);
	err2=pthread_join(pt2,NULL);
	err3=pthread_join(pt3,NULL);
	// errOpt=pthread_join(pt3,NULL);
	err4=pthread_join(pt4,NULL);
	pthread_exit(NULL);
	delete(rsf);
	return 0;

}
