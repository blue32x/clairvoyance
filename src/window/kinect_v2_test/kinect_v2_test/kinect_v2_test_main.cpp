
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
/*
//http://www.buildinsider.net/small/kinectv2cpp/02
int main()
{
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
	int colorWidth = 1920; //image의 크기
	int colorHeight = 1080;
	unsigned int colorBufferSize = colorWidth * colorHeight * 4 * sizeof(unsigned char); //color 이미지의 테이터 크기.

	//Color 이미지를 처리하기 위해 OpenCV의 cv::Mat형을 준비한다.
	//bufferMat 원시 이미지 테이터 colorMat는 리사이즈 한 화상 데이터를 취급힌다.
	//CV_8UC4는 부호없는 8bit정수가 4channel 나란히 1화소를 표현하는 데이터 형식이다.
	cv::Mat colorBufferMat(colorHeight, colorWidth, CV_8UC4); 
	
	/////////////////////////////////////////////////////////////////////////////////////



	//Frame ~ Data
	//depth
	//////////////////////////////////////////////////////////////////////////////////////
	
	int depthWidth = 512;
	int depthHeight= 424;
	unsigned int depthBufferSize = depthWidth * depthHeight * sizeof(unsigned short);

	//Depth 데이터를 처리하기 위해 OpendCV의 cv::Mat 형을 준비한다.
	//bufferMat는 16bit 원시 Depth 데이터 depthMat는 이미지로 표기하기 위해 8bit의 범위에 담는 Depth데이터를 취급한다.
	//"CV_16UC1"은 부호없는 16bit 정수 (16U)이 1channel (C1) 나란히 1 화소를 표현하는 데이터 형식이다. 
	//"CV_8UC1"은 부호없는 8bit 정수 (8U)를 표현하는 데이터 형식이다.
	cv :: Mat depthBufferMat(depthHeight, depthWidth, CV_16UC1);
	
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

	cv::Mat coordinateMapperMat(depthHeight,depthWidth,CV_8UC4);
	cv::namedWindow("CoordinateMapper");
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
			pColorFrame -> CopyConvertedFrameDataToArray(colorBufferSize, reinterpret_cast<BYTE*> (colorBufferMat.data),ColorImageFormat_Bgra);
		}


		//depthFrame
		IDepthFrame * pDepthFrame = nullptr;
		hResult = pDepthReader -> AcquireLatestFrame(&pDepthFrame);
		if(SUCCEEDED(hResult))
		{
			//Frame에서 Depth데이터를 검색한다.
			//Depth 데이터가 저장된 배열의 포인터를 얻을 수 있다. 여기서 Depth 데이터를 시각화 하기 위한 변환처리에 편리한 cv::Mat 형으로 받고있다.
			pDepthFrame -> AccessUnderlyingBuffer(&depthBufferSize, reinterpret_cast<UINT16**> (&depthBufferMat.data));
			
		}



		//Mapping (Depth to Color)
		std::vector<ColorSpacePoint> colorSpacePoints(depthWidth * depthHeight);
		hResult = pCoordinateMapper->MapDepthFrameToColorSpace(depthWidth * depthHeight, reinterpret_cast<UINT16 *>( depthBufferMat.data ), depthWidth * depthHeight, &colorSpacePoints[0] );
		if(SUCCEEDED(hResult))
		{
			coordinateMapperMat = cv::Scalar(0,0,0,0);
			for(int y = 0; y < depthHeight; y++)
			{
				for(int x = 0; x < depthWidth; x++)
				{
					unsigned int index = y * depthWidth + x;
					ColorSpacePoint point = colorSpacePoints[index];

					// round down to the nearest pixel
					int colorX = static_cast<int>(std::floor(point.X + 0.5));
					int colorY = static_cast<int>(std::floor(point.Y + 0.5));
					unsigned short depth = depthBufferMat.at<unsigned short>(y,x);
					

					// make sure the pixel is part of the image
					if( ( colorX >= 0 ) && ( colorX < colorWidth ) && ( colorY >= 0 ) && ( colorY < colorHeight ))// && ( depth >= minDepth ) && ( depth <= maxDepth ) )
					{
						coordinateMapperMat.at<cv::Vec4b>( y, x ) = colorBufferMat.at<cv::Vec4b>( colorY, colorX );
					}
					

				}
			}
		}





		SafeRelease(pDepthFrame); 
		SafeRelease(pColorFrame); //Frame을 풀어놓는다.
		//내부 버퍼가 해제되어 다음 데이터를 검색 할 수있는 산태가 된다.

		//Show window
		
		cv::imshow("CoordinateMapper",coordinateMapperMat);
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

*/


//color + depth 영상 출력 
//http://www.buildinsider.net/small/kinectv2cpp/02
int main()
{
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
	int colorWidth =1920; //image의 크기
	int colorHeight = 1080;
	unsigned int colorBufferSize = colorWidth * colorHeight * 4 * sizeof(unsigned char); //color 이미지의 테이터 크기.

	//Color 이미지를 처리하기 위해 OpenCV의 cv::Mat형을 준비한다.
	//bufferMat 원시 이미지 테이터 colorMat는 리사이즈 한 화상 데이터를 취급힌다.
	//CV_8UC4는 부호없는 8bit정수가 4channel 나란히 1화소를 표현하는 데이터 형식이다.
	cv::Mat colorBufferMat(colorHeight, colorWidth, CV_8UC4); 
	cv::Mat colorMat(colorHeight/2, colorWidth/2, CV_8UC4);
	
	cv::namedWindow("Color");
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
	cv :: namedWindow("Depth");
	
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

	cv::Mat coordinateMapperMat(depthHeight,depthWidth,CV_8UC4);
	cv::namedWindow("CoordinateMapper");
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
		//  using arry
		ColorSpacePoint colorSpacePoints[depthHeight][depthWidth];
		hResult = pCoordinateMapper->MapDepthFrameToColorSpace(depthWidth * depthHeight, reinterpret_cast<UINT16 *>(depthBufferMat.data), depthWidth * depthHeight, &colorSpacePoints[0][0]);
		if(SUCCEEDED(hResult))
		{
			coordinateMapperMat = cv::Scalar(0,0,0,0);
			for(int y = 0; y < depthHeight; y++)
			{
				for(int x = 0; x < depthWidth; x++)
				{
					ColorSpacePoint point = colorSpacePoints[y][x];

					int colorX = static_cast<int>(std::floor(point.X+0.5));
					int colorY = static_cast<int>(std::floor(point.Y+0.5));
					unsigned short depth = depthBufferMat.at<unsigned short>(y,x);

					if( (colorX >= 0) && ( colorX < colorWidth) && ( colorY >= 0) && (colorY < colorHeight) && (depth >= minDepth) && ( depth <= maxDepth))
					{
						coordinateMapperMat.at<cv::Vec4b>(y,x) = colorBufferMat.at<cv::Vec4b>(colorY,colorX);
					}
				}
			}
		}
		




		SafeRelease(pDepthFrame); 
		SafeRelease(pColorFrame); //Frame을 풀어놓는다.
		//내부 버퍼가 해제되어 다음 데이터를 검색 할 수있는 산태가 된다.

		//Show window
		cv::imshow("Color", colorMat);
		cv::imshow("Depth", depthMat);
		cv::imshow("CoordinateMapper",coordinateMapperMat);
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

