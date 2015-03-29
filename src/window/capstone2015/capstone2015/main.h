#include <opencv/highgui.h>
#include <opencv/cxcore.h>
#include <opencv/cv.h>

#include <stdio.h>
#include <Windows.h>
#include <tchar.h>

#include <NuiApi.h>

using namespace cv;

#define COLOR_WIDTH		640
#define COLOR_HEIGHT	480
#define DEPTH_WIDTH		640
#define DEPTH_HEIGHT	480

void initializeKinect();
void createRGBImage(HANDLE h,IplImage *color);
void createDepthImage(HANDLE h,IplImage *depth);
void createSumImage(HANDLE h);
void processDepth(HANDLE colorStreamHandle, HANDLE depthStreamHandle);


RGBQUAD depthPixel[DEPTH_WIDTH*DEPTH_HEIGHT];


Mat depthImage;
Mat colorImage;
Mat depthColor;