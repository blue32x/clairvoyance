#include"myheader.h"
#include"mykinect.h"
#include"visualprocessing.h"

//Kinect sdk pointer Release를 위한 함수
//Interface로 구성되어 어떤 함수가 와도 처리가 가능
template<class Interface>
inline void SafeRelease( Interface *& pInterfaceToRelease )
{
	if( pInterfaceToRelease != NULL ){
		pInterfaceToRelease->Release();
		pInterfaceToRelease = NULL;
	}
}

static int DrawX,DrawY;
/*******************************************************************************
			Kinect for Windows SDK v2 의 테이터 취득의 흐름

			Sensor -> Source -> Reader -> Frame -> Data

********************************************************************************/


//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////이부분만 고치면 이미지 처리가 된다//////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
void ErrorHandling(char * message)
{
	std::cerr<<message<<std::endl;
	exit(1);
}





//color + depth 영상 출력 
//http://www.buildinsider.net/small/kinectv2cpp/02

//영상의 Frame을 출력하기 위한 전역변수
long nmrTotalFrames = 0;

//kinect에서 가장 가까운 물체까지의 거리를 나타내줄 전역변수
int myMinDepth = 4500;

DWORD startTime = 0;
//main functio start
int main(void)
{
	//FPS를 표시해 줄 Font 및 문자열 버퍼 초기화
	char strBuffer[64] = {0,}; //화면에 출력할 스트링을 저장할 버퍼
	startTime = timeGetTime(); //시작을 나타낸다.
	
	//Sensor를 얻을 수 있다.
	///////////////////////////////////////////////////////////////////////////////////
	IKinectSensor * pSensor=NULL;  //Kinect v2 대우를 위한 Sensor인터페이스.
	IColorFrameSource * pColorSource=NULL;
	IDepthFrameSource * pDepthSource=NULL;
	IColorFrameReader *pColorReader=NULL;
	IDepthFrameReader * pDepthReader=NULL;
	HRESULT hResult = S_OK;
	HRESULT depthResult =S_OK;

	HANDLE hMapFile2;
	LPCTSTR pBuf2;
	char buffer2[BUF_SIZE];

	//
	// Open Named Shared Memory
	//
	hMapFile2 = OpenFileMapping(
		FILE_MAP_ALL_ACCESS,   // read/write access
		FALSE,                 // do not inherit the name
		TEXT("ModeSig"));      // name of mapping object

	if (hMapFile2 == NULL)
	{
		_tprintf(TEXT("Could not open hMapFile2 mapping object (%d).\n"),
			GetLastError());
		return 1;
	}

	hMapFile3 = OpenFileMapping(
		FILE_MAP_ALL_ACCESS,   // read/write access
		FALSE,                 // do not inherit the name
		TEXT("StopSig"));      // name of mapping object

	if (hMapFile3 == NULL)
	{
		_tprintf(TEXT("Could not open hMapFile3 mapping object (%d).\n"),
			GetLastError());
		return 1;
	}

	pBuf3 = (LPTSTR) MapViewOfFile(hMapFile3,   // handle to map object
		FILE_MAP_ALL_ACCESS, // read/write permission
		0,
		0,
		BUF_SIZE);

	if (pBuf3 == NULL)
	{
		_tprintf(TEXT("Could not map view of hMapFile3 (%d).\n"),
			GetLastError());

		CloseHandle(hMapFile3);
		return 1;
	}
	
	mykinect _kinect(pSensor);
	pSensor=_kinect.initialize();
	pColorSource=_kinect.setColorSource(pColorSource);
	pDepthSource=_kinect.setDepthSource(pDepthSource);
	pColorReader=_kinect.setColorReader(pColorSource,pColorReader);
	pDepthReader=_kinect.setDepthReader(pDepthSource,pDepthReader);
	
	//Frame ~ Data
	//color
	////////////////////////////////////////////////////////////////////////////////////
	
	unsigned int colorBufferSize = colorWidth * colorHeight * 4 * sizeof(unsigned char); //color 이미지의 테이터 크기.

	//Color 이미지를 처리하기 위해 OpenCV의 cv::Mat형을 준비한다.
	//bufferMat 원시 이미지 테이터 colorMat는 리사이즈 한 화상 데이터를 취급힌다.
	//CV_8UC4는 부호없는 8bit정수가 4channel 나란히 1화소를 표현하는 데이터 형식이다.
	cv::Mat colorBufferMat(colorHeight, colorWidth, CV_8UC4); 
	cv::Mat colorMat(colorHeight/2, colorWidth/2, CV_8UC4);
	
	cv::namedWindow("Output");
	/////////////////////////////////////////////////////////////////////////////////////



	//Frame ~ Data
	//depth
	//////////////////////////////////////////////////////////////////////////////////////
	
	//int depthWidth = 512;
	//int depthHeight= 424;
	unsigned int depthBufferSize = depthWidth * depthHeight * sizeof(unsigned short);

	//Depth 데이터를 처리하기 위해 OpendCV의 cv::Mat 형을 준비한다.
	//bufferMat는 16bit 원시 Depth 데이터 depthMat는 이미지로 표기하기 위해 8bit의 범위에 담는 Depth데이터를 취급한다.
	//"CV_16UC1"은 부호없는 16bit 정수 (16U)이 1channel (C1) 나란히 1 화소를 표현하는 데이터 형식이다. 
	//"CV_8UC1"은 부호없는 8bit 정수 (8U)를 표현하는 데이터 형식이다.
	cv :: Mat depthBufferMat(depthHeight, depthWidth, CV_16UC1);
	cv :: Mat depthMat(depthHeight, depthWidth, CV_8UC1);
	
	
	//////////////////////////////////////////////////////////////////////////////////////


	//get Coordinate Mapper
	///////////////////////////////////////////////////////////////////////////////////////
	ICoordinateMapper * pCoordinateMapper;
	hResult = pSensor-> get_CoordinateMapper(&pCoordinateMapper);
	if(FAILED(hResult))
	{
		std::cerr << "Error : IKinectSensor::get_CoordinateMapper()" << std::endl;
		return -1;
	}
	//color에 depth를 표현하기 위한 메트릭스
	cv::Mat colorCoordinateMapperMat(colorHeight/SPEEDBOOST,colorWidth/SPEEDBOOST,CV_8UC4);


	
	///////////////////////////////////////////////////////////////////////////////////////


	unsigned short minDepth, maxDepth;

	pDepthSource->get_DepthMinReliableDistance(&minDepth);
	pDepthSource->get_DepthMaxReliableDistance(&maxDepth);

	//std::cout << minDepth << " " << maxDepth << std::endl;

	char mode_detec;
	char mode = NORMALIMAGE;
	int bling_var = 0;
	//Frame을 생성하고 color와 depth image를 출력한다.
	///////////////////////////////////////////////////////////////////////////////////////
	int loopCount = 0;
	while(1)
	{
		loopCount++;
		//가장 가까운 거리를 구하기 위해 myMinDepth를 초기화해 준다.
		

		//colorFrame
		IColorFrame * pColorFrame = nullptr; //color 이미지를 얻기 위한 Frame 인터페이스
		hResult = pColorReader -> AcquireLatestFrame(&pColorFrame); //Reader에서 최신 Frame을 얻을 수 있다.
		if(SUCCEEDED(hResult))
		{
			//Frame에서 Color이미지를 얻을 수 있다.
			//기본 형식은 YUY2(=밝기와 색상 차이로 표현하는 형식)이지만, 다루기 쉬운 BGRA로 변환하여 얻을 수 있다.
			hResult = pColorFrame -> CopyConvertedFrameDataToArray(colorBufferSize, reinterpret_cast<BYTE*> (colorBufferMat.data),ColorImageFormat_Bgra);

			if(SUCCEEDED(hResult))
			{
				cv::resize(colorBufferMat,colorMat,cv::Size(),0.5,0.5); //가로 세로 각각 절반 크기(960x540)로 축소한다.
			}
		}


		//depthFrame
		IDepthFrame * pDepthFrame = nullptr;
		depthResult = pDepthReader -> AcquireLatestFrame(&pDepthFrame);
		if(SUCCEEDED(depthResult))
		{
			//Frame에서 Depth데이터를 검색한다.
			//Depth 데이터가 저장된 배열의 포인터를 얻을 수 있다. 여기서 Depth 데이터를 시각화 하기 위한 변환처리에 편리한 cv::Mat 형으로 받고있다.
			hResult = pDepthFrame -> AccessUnderlyingBuffer(&depthBufferSize, reinterpret_cast<UINT16**> (&depthBufferMat.data));

			myMinDepth = 4500;
			for(int y=0; y < depthHeight; y++)
			{
				for(int x = 0; x < depthWidth; x++)
				{
					if(myMinDepth > depthBufferMat.at<UINT16>(y,x) && depthBufferMat.at<UINT16>(y,x) != 0  )
					 {
						 myMinDepth= depthBufferMat.at<UINT16>(y,x);
						
					 }
				}
			}

			if(SUCCEEDED(hResult))
			{
				//depthBufferMat.convertTo(depthMat,CV_8U, -255.0f / 8000.0f, 255.0f); //Depth데이터를 이미지로 표시하기 위해 16bit에서 8bit로 변환한다.
				depthBufferMat.convertTo(depthMat,CV_8U, -255.0f / 8000.0f, 255.0f);
			}
		}



		//Mapping Frame	


		//color에 depth를 표현
		DepthSpacePoint depthSpacePoints[colorHeight][colorWidth];
		if(SUCCEEDED(depthResult))
		{
			hResult = pCoordinateMapper->MapColorFrameToDepthSpace(depthWidth * depthHeight, reinterpret_cast<UINT16 *>(depthBufferMat.data), colorWidth * colorHeight, &depthSpacePoints[0][0]);
		}
		if(SUCCEEDED(hResult))
		{
			colorCoordinateMapperMat = cv::Scalar(0,0,0,0);
		
			
			for(int y = 0; y < colorHeight; y+=SPEEDBOOST)
			{
				for(int x = 0; x < colorWidth; x+=SPEEDBOOST)
				{
					DepthSpacePoint dPoint = depthSpacePoints[y][x];

					int depthX = static_cast<int>(dPoint.X);
					int depthY = static_cast<int>(dPoint.Y);

					colorCoordinateMapperMat.at<cv::Vec4b>(y/SPEEDBOOST,x/SPEEDBOOST) = colorBufferMat.at<cv::Vec4b>(y,x);
					
					
				}

			}
			
		}
		/*
		else
		{
		//	std::cerr << "Error : IKinectSensor::missinbg frame" << std::endl;
		}
		*/

		/*
		#define NORMALIMAGE 0
		#define FINDNEAR 1
		#define GRAIMAGE 2
		#define BLINGBLING 3
		*/

		//모드를 선택한다.
		//영상 출력중에 입력키를 받는다.
		cv::waitKey(1); //fps 가 너무 빨라지면 450 default 값으로 틴다

		/*
		//mode_detec = cv::waitKey( 1);

		//받은 입력mode_detec이 효과를 원하는 키일 경우
		//mode에 값을 넣어준다.
		if(mode_detec== VK_ESCAPE)
		{
			break;
		}
		else if(mode_detec == NORMALIMAGE)
		{
			mode= NORMALIMAGE; //0을 누를 경우 영상처리 되지 않은 이미지 출력
		}
		else if(mode_detec == GRAIMAGE )
		{
			mode = GRAIMAGE; //2를 누를 경우 gradation image 출력
		}
		else if(mode_detec == BLINGBLING)
		{
			mode = BLINGBLING; // 3을 누를 경우 깜빡이는 image 출력
		}
		else if(mode_detec ==REDPOINT)
		{
			mode = REDPOINT; // 4를 누를 경우 붉은 색으로 등고선 표시
		}
		else if(mode_detec ==FINDNEAR)
		{
			mode = FINDNEAR; // 1을 누르면 가까운 경우 붉은 색으로 표시
		}
		*/

		///// Receive mode selection from joystick process
		if(loopCount == 10)
		{
			loopCount = 0;

			pBuf2 = (LPTSTR) MapViewOfFile(hMapFile2, // handle to map object
				FILE_MAP_ALL_ACCESS,  // read/write permission
				0,
				0,
				BUF_SIZE);

			if (pBuf2 == NULL)
			{
				_tprintf(TEXT("Could not map view of hMapFile2 (%d).\n"),
					GetLastError());

				CloseHandle(hMapFile2);
				return 1;
			}

			// convert LPCTSTR pBuf to char [] buf
			WideCharToMultiByte(CP_ACP, 0, pBuf2, BUF_SIZE, buffer2, BUF_SIZE, NULL, NULL);

			if(buffer2[0] == '0')
			{
				mode = NORMALIMAGE;
			}
			else if(buffer2[0] == '2')
			{
				mode = FINDNEAR;
			}
			else if(buffer2[0] == '4')
			{
				mode = GRAIMAGE;
			}
			else if(buffer2[0] == '6')
			{
				mode = BLINGBLING;
			}

			//printf("%c, %c\n", buffer2[0], mode);
			//printf("current mode : %c\n", mode);

			UnmapViewOfFile(pBuf2);
		}

		//만약 mode에서 선택영상을 받는 경우
		//선택된 mode에 따라 영상처리 함수를 실행시킨다.
		//mode=BLINGBLING;
		if(mode == NORMALIMAGE)
		{
			//처리 과정 X
			//calibration_image_processing_all(colorCoordinateMapperMat, depthSpacePoints, depthBufferMat);

			//Write buffer3 for kinect process
			buffer3[0] = '0';
			CopyMemory((PVOID)pBuf3, buffer3, sizeof(buffer3));
		}
		else if(mode == REDPOINT && SUCCEEDED(depthResult))
		{
			//calibration_image_processing_red(colorCoordinateMapperMat, depthSpacePoints, depthBufferMat);
			calibration_image_processing_all(colorCoordinateMapperMat, depthSpacePoints, depthBufferMat);
			CopyMemory((PVOID)pBuf3, buffer3, sizeof(buffer3));
			buffer3[0]='0';
		}
		else if(mode == FINDNEAR && SUCCEEDED(depthResult))
		{
			calibration_image_processing_near(colorCoordinateMapperMat, depthSpacePoints, depthBufferMat);
			CopyMemory((PVOID)pBuf3, buffer3, sizeof(buffer3));
			buffer3[0]='0';
		}
		else if(mode == BLINGBLING && SUCCEEDED(depthResult))
		{
			bling_var += 1;
			if(bling_var > 100)
			{
				bling_var = 0;
			}
			calibration_image_processing_bling(colorCoordinateMapperMat, depthSpacePoints, depthBufferMat, bling_var);
			CopyMemory((PVOID)pBuf3, buffer3, sizeof(buffer3));
			buffer3[0]='0';
		}
		else if(mode == GRAIMAGE && SUCCEEDED(depthResult))
		{
			calibration_image_processing_gra(colorCoordinateMapperMat, depthSpacePoints, depthBufferMat);
			CopyMemory((PVOID)pBuf3, buffer3, sizeof(buffer3));
			buffer3[0]='0';
		}
		//영상처리를 위한 함수
		

		//resize가 필요한 경우를 위한 코드
		//resize가 필요 없을 경우에는 이 코드부분을 지우고
		//colorCoordinateMapperMat을 출력 시켜주면 된다.
		cv::resize(colorCoordinateMapperMat,colorMat,cv::Size(),1,1);
		
	
		//좌우 반전을 통해 사용자가 주차할 경우 
		//헷갈리지 않게 해준다.
		cv::flip(colorMat,colorMat,1); 


		nmrTotalFrames++;
		SafeRelease(pDepthFrame); 
		SafeRelease(pColorFrame); //Frame을 풀어놓는다.
		//내부 버퍼가 해제되어 다음 데이터를 검색 할 수있는 산태가 된다.
		//Show window
		float fps=(float)((nmrTotalFrames*1000.0)/(timeGetTime()-startTime));
		cv::Point mypoint;
		mypoint.x=10;
		mypoint.y=40;
		//itoa(fps,strBuffer,10);
		sprintf_s(strBuffer,"%.2lf fps %d cm",fps,myMinDepth/10);
		//sprintf_s(strBuffer,"%.2lf fps",fps);
		//fps를 이미지 버퍼에 출력한다.
		cv::putText(colorMat,strBuffer,mypoint,2,1.2,cv::Scalar::all(255));
		//cv::circle(colorMat,cv::Point(DrawX,DrawY),10,cv::Scalar::all(255),2);

		SafeRelease(pDepthFrame); 
		SafeRelease(pColorFrame); //Frame을 풀어놓는다.
		//내부 버퍼가 해제되어 다음 데이터를 검색 할 수있는 산태가 된다.

		//Show window
		cv::imshow("Output", colorMat);
		cv::imshow("Depth", depthMat);
		//cv::imshow("CoordinateMapper",colorCoordinateMapperMat);
		
	}
	////////////////////////////////////////////////////////////////////////////////////

	UnmapViewOfFile(pBuf2);
	UnmapViewOfFile(pBuf3);
	CloseHandle(hMapFile2);
	CloseHandle(hMapFile3);
	SafeRelease( pColorSource );
	SafeRelease( pColorReader );
	SafeRelease( pDepthSource );
	SafeRelease( pDepthReader );
	SafeRelease( pCoordinateMapper);
	//kinect sensor close
	if( pSensor )
	{
		pSensor->Close();
	}

	//kinect sensor pointer release
	SafeRelease( pSensor );

	cv::destroyAllWindows(); //생성한 3개의 윈도우를 닫아준다.


	return 0;
}