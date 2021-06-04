#include "frame.h"

void frame::compute_features_orb()
{
    cuda::GpuMat imggray;
    cuda::cvtColor(m_img_gpu,imggray,COLOR_BGR2GRAY);
    orb->detectAndCompute(imggray,cuda::GpuMat(),this->kp,this->dsc);
}

void frame::compute_features_surf()
{
    cuda::GpuMat imggray,kp_gpu;
    cuda::cvtColor(m_img_gpu,imggray,COLOR_BGR2GRAY);
    this->surf->detectWithDescriptors(imggray,cuda::GpuMat(),kp_gpu,this->dsc);
    this->surf->downloadKeypoints(kp_gpu,this->kp);
}

void frame::compute_features_sift()
{
    Mat imggray;
	Ptr<SIFT> sift = SIFT::create(5000);
	cv::cvtColor(m_img, imggray, COLOR_BGR2GRAY);
	Mat dsc;
	sift->detectAndCompute(imggray, noArray(), this->kp, dsc);
    this->dsc.upload(dsc);
}


void frame::getPointsOptFlow(Mat prev_img)
{

    int win_size = 18;

    cuda::GpuMat gpu_prev_img,gpu_prev_img_gray,gpu_next_img, gpu_cornersA, gpu_cornersB, gpu_features_found, gpu_err;
    gpu_prev_img.upload(prev_img);
    cuda::cvtColor(gpu_prev_img, gpu_prev_img_gray, COLOR_BGR2GRAY);
    cuda::cvtColor(m_img_gpu, gpu_next_img, COLOR_BGR2GRAY);
    Mat err;
    std::vector<cv::Point2f> cornersA, cornersB;
    const int MAX_CORNERS = 4000;
    Ptr<cuda::CornersDetector> detector = cuda::createGoodFeaturesToTrackDetector(gpu_prev_img_gray.type(), 4000, 0.01, 5);
    detector->detect(gpu_prev_img_gray, gpu_cornersA);

    Ptr<cuda::SparsePyrLKOpticalFlow> d_pyrLK_sparse = cuda::SparsePyrLKOpticalFlow::create(
        Size(win_size * 2 + 1, win_size * 2 + 1), 5, 20);
    std::vector<uchar> features_found;

    d_pyrLK_sparse->calc(gpu_prev_img_gray, gpu_next_img, gpu_cornersA, gpu_cornersB, gpu_features_found, gpu_err);
    gpu_cornersA.download(cornersA);
    gpu_cornersB.download(cornersB);
    gpu_features_found.download(features_found);
    gpu_err.download(err);
    for( int i=0;i<cornersA.size();i++)
    {
        if((int)features_found[i]==1){
            prev_pts.push_back(cornersA[i]);
            next_pts.push_back(cornersB[i]);

        }
    }
}