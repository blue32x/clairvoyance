#include"main.h"

int main()
{
	//IplImage *srcImage;

   // srcImage = cvLoadImage("C:/Users/choiseunghyuk/Desktop/ssibal/20150121_090933.jpg", CV_LOAD_IMAGE_GRAYSCALE);

 //IplImage *image = cvLoadImage("C:/Users/choiseunghyuk/Desktop/ssibal/20150121_090933.jpg");
 
    //cvShowImage("Test",image);
   // cvWaitKey(0);
 
    //cvReleaseImage(&image);
VideoCapture capture(CV_CAP_OPENNI); // or CV_CAP_OPENNI
for(;;)
{
    Mat depthMap;
    Mat rgbImage;

    capture.grab();

    capture.retrieve( depthMap, CV_CAP_OPENNI_DEPTH_MAP );
    capture.retrieve( rgbImage, CV_CAP_OPENNI_BGR_IMAGE );

	imshow("depthmap",depthMap);
	imshow("WINDOW",rgbImage);
    if( waitKey( 30 ) >= 0 )
        break;
}

	return 0;
}