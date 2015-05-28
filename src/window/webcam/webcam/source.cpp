#include <opencv/highgui.h>
#include<opencv/cv.h>
int main() {
    IplImage *frame;
    CvCapture* capture = cvCaptureFromCAM(0);
 
	IplImage *dest = cvCreateImage(cvSize(960,540),IPL_DEPTH_8U, 3);

    cvNamedWindow("frontcamera",1);   

    while(capture) {
		#pragma omp parallel
        frame = cvQueryFrame(capture);
		cvFlip(frame,frame,0);
		cvFlip(frame,frame,1);
		cvResize(frame, dest, 4);

        cvShowImage("frontcamera",dest);
 
        if(cvWaitKey(33) >= 27)
            break;
    }
    cvReleaseCapture(&capture);
    cvDestroyWindow("frontcamera");
 
	
    return 0;
}
