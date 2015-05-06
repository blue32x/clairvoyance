
//헤더부분
#include "stdafx.h" //stdafx 헤더파일 참조

//safeRelease함수를 정의????
//쓰임새는 main함수에서 살펴봐야할듯
template<class Interface>
inline void SafeRelease( Interface *& pInterfaceToRelease )
{
	if( pInterfaceToRelease != NULL ){
		pInterfaceToRelease->Release();
		pInterfaceToRelease = NULL;
	}
}


/*******************************************************************************
			Kinect for Windows SDK v2 의 테이터 취득의 흐름

			Sensor -> Source -> Reader -> Frame -> Data

********************************************************************************/


#define depthWidth 512
#define depthHeight 424
#define colorWidth 1920
#define colorHeight 1080


//coordinate 이미지 영상을 줄여주는 변수
//1로 바꿔주면 원본, 1이상이면 영상의 크기를 줄여주어 
//속도를 빠르게 해준다.
//추후에 멀티쓰레드 혹은 분산처리로 해결 가능성을 보인다.
#define SPEEDBOOST 3


//color + depth 영상 출력 
//http://www.buildinsider.net/small/kinectv2cpp/02
int main()
{


	double color_R, color_G, color_B;
	double ycbcr_Y, ycbcr_Cb, ycbcr_Cr;

	//깜빡임 효과를 줄 변수
	int blingbling = 0;

	//Sensor를 얻을 수 있다.
	///////////////////////////////////////////////////////////////////////////////////
	IKinectSensor * pSensor;  //Kinect v2 대우를 위한 Sensor인터페이스.
	HRESULT hResult = S_OK;
	hResult = GetDefaultKinectSensor(&pSensor);  //기본 Sensor를 얻을 수 있다.
	if(FAILED(hResult))
	{
		std :: cerr << "Error : GetDEfaultKinectSensor" << std ::endl;
		return -1;
	}
	hResult = pSensor ->Open(); //Sensor를 연다.
	if(FAILED(hResult))
	{
		std :: cerr << "Error : IKinectSensor :: opne()" << std :: endl;
	}
	////////////////////////////////////////////////////////////////////////////////////


	//Sensor에서 Source를 가져온다.
	////////////////////////////////////////////////////////////////////////////////////
	IColorFrameSource * pColorSource; //color 구조를 위한 Source 인터페이스
	hResult = pSensor -> get_ColorFrameSource(&pColorSource); //Sensor에서 Source를 가져온다.
	if(FAILED(hResult)) //pColorSource에 kinect v2의 color 정보가 저장된 상태이다.
	{
		std::cerr << "Error : IKinectSensor :: get_ColorFrameSource()" << std:: endl;
		return -1;
	}

	IDepthFrameSource * pDepthSource; //depth 구조를 위한 Source 인터페이스
	hResult = pSensor -> get_DepthFrameSource(&pDepthSource);
	if(FAILED(hResult))
	{
		std::cerr<<"Error :IKinectSensor::get_DepthFrameSource()"<<std::endl;
		return -1;
	}
	////////////////////////////////////////////////////////////////////////////////////


	//Source에서 Reader를 연다.
	////////////////////////////////////////////////////////////////////////////////////
	IColorFrameReader *pColorReader; //Color 구조를 위한 Rader 인터페이스.
	hResult = pColorSource -> OpenReader( & pColorReader); //Source에서 Reader를 연다.
	if(FAILED(hResult))
	{
		std :: cerr << "Error : IColorFrameSource :: OpenReader()" << std :: endl;
		return -1;
	}
	IDepthFrameReader * pDepthReader; //depth 구조를 위한 Reader 인터페이스
	hResult = pDepthSource -> OpenReader( & pDepthReader); //Source에서 Reader를 연다.
	if(FAILED(hResult))
	{
		std::cerr << "Error : IDepthFrameSource :: OpenReader()"<<std::endl;
		return -1;
	}
	////////////////////////////////////////////////////////////////////////////////////


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



	//Frame을 생성하고 color와 depth image를 출력한다.
	///////////////////////////////////////////////////////////////////////////////////////
	while(1)
	{
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
		hResult = pDepthReader -> AcquireLatestFrame(&pDepthFrame);
		if(SUCCEEDED(hResult))
		{
			//Frame에서 Depth데이터를 검색한다.
			//Depth 데이터가 저장된 배열의 포인터를 얻을 수 있다. 여기서 Depth 데이터를 시각화 하기 위한 변환처리에 편리한 cv::Mat 형으로 받고있다.
			hResult = pDepthFrame -> AccessUnderlyingBuffer(&depthBufferSize, reinterpret_cast<UINT16**> (&depthBufferMat.data));
			
			if(SUCCEEDED(hResult))
			{
				depthBufferMat.convertTo(depthMat,CV_8U, -255.0f / 8000.0f, 255.0f); //Depth데이터를 이미지로 표시하기 위해 16bit에서 8bit로 변환한다.
			}
		}



		//Mapping Frame	
		//color에 depth를 표현
		DepthSpacePoint depthSpacePoints[colorHeight][colorWidth];
		hResult = pCoordinateMapper->MapColorFrameToDepthSpace(depthWidth * depthHeight, reinterpret_cast<UINT16 *>(depthBufferMat.data), colorWidth * colorHeight, &depthSpacePoints[0][0]);
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



		//blingbling
		//color 이미지 영상에 깜빡이는 효과를 주기 위하여 blingbling 변수 컨트롤
		if(blingbling < 12)
		{
			blingbling++;
		}
		else
		{
			blingbling /= 13;
		}


		///his이미지 변환을 통한 color 이미지 처리
		
		if(blingbling < 6)
		{
			for(int y = 0; y < colorHeight; y+=SPEEDBOOST)
			{
				for(int x = 0; x < colorWidth; x+=SPEEDBOOST)
				{
					DepthSpacePoint dPoint = depthSpacePoints[y][x];
					int depthX = static_cast<int>(dPoint.X);
					int depthY = static_cast<int>(dPoint.Y);
					
					if(depthX >=0 && depthX < depthWidth && depthY >= 0 && depthY < depthHeight)
					{
	
						if(depthMat.at<uchar>(depthY,depthX)  >= 235)
						{
							
							
							//colorCoordinateMapperMat.at<cv::Vec4b>(y/SPEEDBOOST,x/SPEEDBOOST)[2] = 250;
							color_B = colorCoordinateMapperMat.at<cv::Vec4b>(y/SPEEDBOOST,x/SPEEDBOOST)[0] ;
							color_G = colorCoordinateMapperMat.at<cv::Vec4b>(y/SPEEDBOOST,x/SPEEDBOOST)[1] ;
							color_R = colorCoordinateMapperMat.at<cv::Vec4b>(y/SPEEDBOOST,x/SPEEDBOOST)[2] ;

							ycbcr_Y = (299*color_R + 587*color_G + 114*color_B)/1000;
							ycbcr_Cb = 0.5643*(color_B - ycbcr_Y) + 128;
							ycbcr_Cr = 0.7132*(color_R - ycbcr_Y) + 128;

							ycbcr_Y += 30;

							color_R = (1000*ycbcr_Y + 1402*(ycbcr_Cr-128))/1000;
							color_G = (1000*ycbcr_Y - 714*(ycbcr_Cr-128) - 334*(ycbcr_Cb-128))/1000;
							color_B = (1000*ycbcr_Y + 1772*(ycbcr_Cb-128))/1000;

							colorCoordinateMapperMat.at<cv::Vec4b>(y/SPEEDBOOST,x/SPEEDBOOST)[0] = (uchar)color_B;
							colorCoordinateMapperMat.at<cv::Vec4b>(y/SPEEDBOOST,x/SPEEDBOOST)[1] = (uchar)color_G;
							colorCoordinateMapperMat.at<cv::Vec4b>(y/SPEEDBOOST,x/SPEEDBOOST)[2] = (uchar)color_R;
							
						}

					
					}//if(depthX >=0 && depthX < depthWidth && depthY >= 0 && depthY < depthHeight)
				}//for(int x = 0; x < colorWidth; x+=SPEEDBOOST)
			}//for(int y = 0; y < colorHeight; y+=SPEEDBOOST)
		}//if(blingbling < 6)
		




		cv::resize(colorCoordinateMapperMat,colorMat,cv::Size(),1,1);
		
	

		SafeRelease(pDepthFrame); 
		SafeRelease(pColorFrame); //Frame을 풀어놓는다.
		//내부 버퍼가 해제되어 다음 데이터를 검색 할 수있는 산태가 된다.

		//Show window
		cv::imshow("Output", colorMat);
		cv::imshow("Depth", depthMat);
		//cv::imshow("CoordinateMapper",colorCoordinateMapperMat);

		//break;

		if( cv::waitKey( 30 ) == VK_ESCAPE )
		{
			break;
		}
	}
	////////////////////////////////////////////////////////////////////////////////////

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

