
#include"main.h"



int _tmain(int argc,_TCHAR argv[])
{
	HANDLE depthStreamHandle;
	HANDLE colorStreamHandle;
	HANDLE nextDepthFrameEvent = CreateEvent(NULL,TRUE,FALSE,NULL);
	HANDLE nextColorFrameEvent = CreateEvent(NULL,TRUE,FALSE,NULL);
	HRESULT hr;
	IplImage *color = cvCreateImage(cvSize(COLOR_WIDTH,COLOR_HEIGHT),IPL_DEPTH_8U,4);
	IplImage *depth = cvCreateImage(cvSize(DEPTH_WIDTH,DEPTH_HEIGHT),IPL_DEPTH_8U,4);
	IplImage *sum = cvCreateImage(cvSize(COLOR_WIDTH,COLOR_HEIGHT),IPL_DEPTH_8U,4);

	depthImage = Mat(Size(DEPTH_WIDTH,DEPTH_HEIGHT),CV_8UC1);
	colorImage = Mat(Size(COLOR_WIDTH,COLOR_HEIGHT),CV_8UC3);
	depthColor = Mat(Size(COLOR_WIDTH,COLOR_HEIGHT),CV_8UC3);

	cvNamedWindow("color",CV_WINDOW_AUTOSIZE);
	cvNamedWindow("depth",CV_WINDOW_AUTOSIZE);
	cvNamedWindow("sum",CV_WINDOW_AUTOSIZE);
	initializeKinect();

	hr = NuiImageStreamOpen(NUI_IMAGE_TYPE_COLOR,NUI_IMAGE_RESOLUTION_640x480,0,2,nextColorFrameEvent,&colorStreamHandle); //image stream open
	if(FAILED(hr))
	{
		printf("Could not open ImageStream\n");
		return hr;
	}

	//hr = NuiImageStreamOpen(NUI_IMAGE_TYPE_DEPTH_AND_PLAYER_INDEX,NUI_IMAGE_RESOLUTION_640x480,0,2,nextDepthFrameEvent,&depthStreamHandle); //depth stream open
	hr = NuiImageStreamOpen(NUI_IMAGE_TYPE_DEPTH_AND_PLAYER_INDEX,NUI_IMAGE_RESOLUTION_640x480,0,2,nextDepthFrameEvent,&depthStreamHandle);
	//NuiImageStreamSetImageFrameFlags(depthStreamHandle,NUI_IMAGE_DEPTH_MINIMUM_NEAR_MODE);
	NuiImageStreamSetImageFrameFlags(depthStreamHandle,NUI_IMAGE_STREAM_FLAG_DISTINCT_OVERFLOW_DEPTH_VALUES); //가

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
		//	USHORT realDepth = (depth&0xfff8) >> 3;
		//	BYTE intensity = 255 - static_cast<BYTE>(((float)depth/50)*256);
			BYTE intensity = 255 - static_cast<BYTE>(depth*8 / 256); //이 위치의  depth를 나누는 값을 적당히 수정하면 우리가 원하는 모양새의 뎁스를 구할수 있다...csh


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


//아래의 함수들은 사용하지 않는 함수 지만 함수의 내용을 참고 할수 있기 때문에 남겨둠...csh
/*
RGBQUAD Nui_ShortToQuad_Depth(USHORT s)
{
	 USHORT realDepth = (s&0xfff8) >> 3;
          //    USHORT testRealDepth = NuiDepthPixelToDepth(s);
       
        // Convert depth info into an intensity for display
        BYTE b = 255 - static_cast<BYTE>(256 * realDepth / 0x0fff);

          //    std::cout<<"Real Depth : "<<realDepth<<std::endl;
          //    std::cout<<"test Depth : "<<testRealDepth<<std::endl;
       
        RGBQUAD q;
        q.rgbRed = q.rgbBlue = q.rgbGreen = ~b;
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
*/