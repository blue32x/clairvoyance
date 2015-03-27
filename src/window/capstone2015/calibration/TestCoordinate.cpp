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



RGBQUAD depthPixel[DEPTH_WIDTH*DEPTH_HEIGHT];

void initializeKinect();
void createRGBImage(HANDLE h,IplImage *color);
void createDepthImage(HANDLE h,IplImage *depth);
void createSumImage(HANDLE h);
void processDepth(HANDLE colorStreamHandle, HANDLE depthStreamHandle);

Mat depthImage;
Mat colorImage;
Mat depthColor;

int _tmain(int argc,_TCHAR argv[])
{
	HANDLE depthStreamHandle;
	HANDLE colorStreamHandle;
	HANDLE nextDepthFrameEvent = CreateEvent(NULL,TRUE,FALSE,NULL);
	HANDLE nextColorFrameEvent = CreateEvent(NULL,TRUE,FALSE,NULL);
	HRESULT hr;

	IplImage *color = cvCreateImage(cvSize(COLOR_WIDTH,COLOR_HEIGHT),IPL_DEPTH_8U,3);
	IplImage *depth = cvCreateImage(cvSize(DEPTH_WIDTH,DEPTH_HEIGHT),IPL_DEPTH_8U,3);
	IplImage *sum = cvCreateImage(cvSize(COLOR_WIDTH,COLOR_HEIGHT),IPL_DEPTH_8U,3);

	depthImage = Mat(Size(DEPTH_WIDTH,DEPTH_HEIGHT),CV_8UC1);
	colorImage = Mat(Size(COLOR_WIDTH,COLOR_HEIGHT),CV_8UC3);
	depthColor = Mat(Size(COLOR_WIDTH,COLOR_HEIGHT),CV_8UC3);

	cvNamedWindow("color",CV_WINDOW_AUTOSIZE);
	cvNamedWindow("depth",CV_WINDOW_AUTOSIZE);
	cvNamedWindow("sum",CV_WINDOW_AUTOSIZE);

	initializeKinect();

	hr = NuiImageStreamOpen(NUI_IMAGE_TYPE_COLOR,NUI_IMAGE_RESOLUTION_640x480,0,2,nextColorFrameEvent,&colorStreamHandle);
	if(FAILED(hr))
	{
		printf("Could not open ImageStream\n");
		return hr;
	}

	hr = NuiImageStreamOpen(NUI_IMAGE_TYPE_DEPTH_AND_PLAYER_INDEX,NUI_IMAGE_RESOLUTION_640x480,0,2,nextDepthFrameEvent,&depthStreamHandle);
	NuiImageStreamSetImageFrameFlags(depthStreamHandle,NUI_IMAGE_STREAM_FLAG_DISTINCT_OVERFLOW_DEPTH_VALUES);

	if(FAILED(hr))
	{
		printf("Could not open DepthStream\n");
		return hr;
	}

	while(1)
	{
		processDepth(colorStreamHandle,depthStreamHandle);

		if(cvWaitKey(10) == 0x001b)
		{
			break;
		}
	}

	NuiShutdown();

	cvReleaseImageHeader(&depth);
	cvReleaseImageHeader(&color);
	cvReleaseImage(&sum);
	cvDestroyAllWindows();

	return 0;
}

void initializeKinect()
{
	bool FailToConnect;

	do
	{
		HRESULT hr = NuiInitialize(NUI_INITIALIZE_FLAG_USES_DEPTH_AND_PLAYER_INDEX | NUI_INITIALIZE_FLAG_USES_COLOR);

		if(FAILED(hr))
		{
			system("cls");
			printf("\nFailed to Connect!\n\n");
			FailToConnect = true;
			system("PAUSE");
		}
		else
		{
			printf("\nConnection Established!\n\n");
			FailToConnect = false;
		}
	}
	while(FailToConnect);
}

void processDepth(HANDLE colorStreamHandle, HANDLE depthStreamHandle)
{
	HRESULT hr;
	const NUI_IMAGE_FRAME *depthFrame, *colorFrame;

	hr = NuiImageStreamGetNextFrame(depthStreamHandle,0,&depthFrame);
	if(FAILED(hr)) return;
	hr = NuiImageStreamGetNextFrame(colorStreamHandle,20,&colorFrame);
	if(FAILED(hr)) return;

	INuiFrameTexture *depthTex = depthFrame->pFrameTexture;
	INuiFrameTexture *colorTex = colorFrame->pFrameTexture;

	NUI_LOCKED_RECT lockedRectDepth;
	NUI_LOCKED_RECT lockedRectColor;

	depthTex->LockRect(0,&lockedRectDepth,NULL,0);
	colorTex->LockRect(0,&lockedRectColor,NULL,0);

	if(lockedRectDepth.Pitch != 0 && lockedRectColor.Pitch != 0)
	{
		const USHORT *depthBufferRun = (const USHORT*)lockedRectDepth.pBits;
		const USHORT *depthBufferEnd = (const USHORT*)depthBufferRun + (DEPTH_WIDTH * DEPTH_HEIGHT);
		const BYTE *colorBufferRun = (const BYTE*)lockedRectColor.pBits;
		const BYTE *colorBufferEnd = (const BYTE*)colorBufferRun + (COLOR_WIDTH * COLOR_HEIGHT * 4);

		memcpy(depthPixel,colorBufferRun,COLOR_WIDTH * COLOR_HEIGHT * sizeof(RGBQUAD));

		int count = 0;
		int x,y;

		while(depthBufferRun < depthBufferEnd)
		{
			USHORT depth = NuiDepthPixelToDepth(*depthBufferRun);
			BYTE intensity = 256 - static_cast<BYTE>(((float)depth/50)*256);

			x = count % DEPTH_WIDTH;
			y = floor((float)count / (float)DEPTH_WIDTH);

			depthImage.at<uchar>(y,x) = intensity;

			LONG colorInDepthX;
			LONG colorInDepthY;

			

			NuiImageGetColorPixelCoordinatesFromDepthPixelAtResolution(NUI_IMAGE_RESOLUTION_640x480,NUI_IMAGE_RESOLUTION_640x480,NULL,x,y,*depthBufferRun,&colorInDepthX,&colorInDepthY);

			if(colorInDepthX >= 0 && colorInDepthX < DEPTH_WIDTH && colorInDepthY >= 0 && colorInDepthY <DEPTH_HEIGHT)
			{
				RGBQUAD &ColorPixel = depthPixel[colorInDepthX + colorInDepthY * COLOR_WIDTH];
				LONG colorIndex = colorInDepthX + colorInDepthY * COLOR_WIDTH * 3;


				depthColor.at<Vec3b>(y,x) = Vec3b(ColorPixel.rgbBlue,ColorPixel.rgbGreen,ColorPixel.rgbRed);
			}
			else
			{
				depthColor.at<Vec3b>(y,x) = Vec3b(0,0,0);
			}

			RGBQUAD &ColorPixel = depthPixel[count];
			colorImage.at<Vec3b>(y,x) = Vec3b(ColorPixel.rgbBlue,ColorPixel.rgbGreen,ColorPixel.rgbRed);

			count++;
			depthBufferRun++;
		}
		imshow("color",colorImage);
		imshow("depth",depthImage);
		imshow("sum",depthColor);
	}
	depthTex->UnlockRect(0);
	colorTex->UnlockRect(0);
	NuiImageStreamReleaseFrame(depthStreamHandle,depthFrame);
	NuiImageStreamReleaseFrame(colorStreamHandle,colorFrame);
}


RGBQUAD Nui_ShortToQuad_Depth(USHORT s)
{
	USHORT depthValue = (s&0xfff8) >> 3;
	BYTE l = 255-(BYTE)(256*depthValue /(0x0fff));

	RGBQUAD q;
	if(NuiDepthPixelToDepth(depthValue)<200)
		q.rgbBlue = q.rgbGreen = q.rgbRed = ~l;
	else
		q.rgbRed = q.rgbBlue = q.rgbGreen = NULL;
	return q;
}

void createRGBImage(HANDLE h,IplImage *color)
{
	const NUI_IMAGE_FRAME *pImageFrame = NULL;

	HRESULT hr = NuiImageStreamGetNextFrame(h,1000, &pImageFrame);

	if(FAILED(hr))
	{
		printf("Create RGB Image Failed\n");
	}

	INuiFrameTexture *pTexture = pImageFrame->pFrameTexture;
	NUI_LOCKED_RECT LockedRect;
	pTexture->LockRect(0,&LockedRect,NULL,0);

	if(LockedRect.Pitch != 0)
	{
		BYTE * pBuffer = (BYTE*)LockedRect.pBits;
		cvSetData(color, pBuffer, LockedRect.Pitch);
		cvShowImage("color", color);
	}

	NuiImageStreamReleaseFrame(h, pImageFrame);
}

void createDepthImage(HANDLE h,IplImage *depth)
{
	const NUI_IMAGE_FRAME *pImageFrame = NULL;

	HRESULT hr = NuiImageStreamGetNextFrame(h,1000,&pImageFrame);

	if(FAILED(hr))
	{
		printf("Create DepthImage Failed\n");
	}

	INuiFrameTexture *pTexture = pImageFrame->pFrameTexture;
	NUI_LOCKED_RECT LockedRect;

	pTexture->LockRect(0,&LockedRect,NULL,0);

	if(LockedRect.Pitch != 0)
	{
		BYTE * pBuffer = (BYTE*)LockedRect.pBits;
		RGBQUAD *rgbrun = depthPixel;
		USHORT *pBufferRun = (USHORT*) pBuffer;

		for( int y = 0 ; y < DEPTH_HEIGHT ; y++ )
		{
			for( int x = 0 ; x < DEPTH_WIDTH ; x++ )
			{
				RGBQUAD quad = Nui_ShortToQuad_Depth( *pBufferRun );
				pBufferRun++;
				*rgbrun = quad;
				rgbrun++;
			}
		}
		cvSetData(depth, (BYTE*) depthPixel, depth->widthStep);
		cvShowImage("depth", depth);
	}
	NuiImageStreamReleaseFrame(h, pImageFrame);
}