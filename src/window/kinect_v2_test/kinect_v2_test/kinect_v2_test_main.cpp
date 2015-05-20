/******************************************************************************************************

								국민대학교
								컴퓨터공학부
								천리안(clairvoyance)
								Kinect v2의 depth map을 활용한 자동차 후방 영상 개선
								지도교수 : 한재일
								조장 : 최승혁
								조원 : 신태섭 박성우 신동호 박민욱 최성현
								version 2.74

******************************************************************************************************/

//opencv header
#include<cv.h>
#include<highgui.h>


//kinect header
#include<Kinect.h>

//.c header
#include<stdio.h>

#include<mmsystem.h>

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


/*******************************************************************************
			Kinect for Windows SDK v2 의 테이터 취득의 흐름

			Sensor -> Source -> Reader -> Frame -> Data

********************************************************************************/


#define depthWidth 512 //Kinect v2 depth map width
#define depthHeight 424 //Kinect v2 depth map height
#define colorWidth 1920 //Kinect v2 color camera width
#define colorHeight 1080 //Kinect v2 color camera height

//coordinate 이미지 영상을 줄여주는 변수
//1로 바꿔주면 원본, 1이상이면 영상의 크기를 줄여주어 
//속도를 빠르게 해준다.
//추후에 멀티쓰레드 혹은 분산처리로 해결 가능성을 보인다.
#define SPEEDBOOST 3

#define NORMALIMAGE '0'
#define FINDNEAR '2'
#define GRAIMAGE '1'
#define BLINGBLING '3'
#define REDPOINT '4'

//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////이부분만 고치면 이미지 처리가 된다//////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////

//가까운 것은 밝게, 어두운 것은 어둡게 등고선 처리를 해준다.
//4.5m 부터 50cm 씩 밝기를 가까워 질수록 점점 밝게 처리한다.
void calibration_image_processing_gra(cv::Mat colorCoordinateMapperMat, DepthSpacePoint depthSpacePoints[][colorWidth], cv::Mat depthMat)
{
	double colorR, colorG, colorB;
	int depth_var;

	for(int y = 0; y < colorHeight; y+=SPEEDBOOST)
	{
		for(int x = 0; x < colorWidth; x+=SPEEDBOOST)
		{
			DepthSpacePoint dPoint = depthSpacePoints[y][x]; //depthmap 좌표를 받아온다.
			int depthX = static_cast<int>(dPoint.X); //depthX에 depthmap 좌표 x값을 저장한다.
			int depthY = static_cast<int>(dPoint.Y); //depthY에 depthmap 좌표 y값을 저장한다.
			if(depthX >=0 && depthX < depthWidth && depthY >= 0 && depthY < depthHeight)
			{
				//color RGB를 받아온다.
				colorB = colorCoordinateMapperMat.at<cv::Vec4b>(y/SPEEDBOOST,x/SPEEDBOOST)[0];
				colorG = colorCoordinateMapperMat.at<cv::Vec4b>(y/SPEEDBOOST,x/SPEEDBOOST)[1];
				colorR = colorCoordinateMapperMat.at<cv::Vec4b>(y/SPEEDBOOST,x/SPEEDBOOST)[2];
				depth_var =depthMat.at<UINT16>(depthY,depthX) ;

				if(depth_var != 0 || depth_var != 4500)
				{
					//depth가 4 ~ 3.5m일때의 밝기
					if(depth_var  <  4000 && depth_var >= 3500)
					{
						colorB = colorB * 1.1;
						colorG = colorG * 1.1;
						colorR = colorR * 1.1;
						if(colorB >= 255) colorB = 254;
						if(colorG >= 255) colorG = 254;
						if(colorR >= 255)colorR = 254;
					}
					//depth가 3.5 ~ 3m 일때의 밝기
					else if (depth_var < 3500 && depth_var >= 3000)
					{
						colorB = colorB * 1.2;
						colorG = colorG * 1.2;
						colorR = colorR * 1.2;
						if(colorB >= 255) colorB = 254;
						if(colorG >= 255) colorG = 254;
						if(colorR >= 255)colorR = 254;
					}
					//depth가 3 ~ 2.5m 일때의 밝기
					else if (depth_var < 3000 && depth_var >= 2500)
					{
						colorB = colorB * 1.3;
						colorG = colorG * 1.3;
						colorR = colorR * 1.3;
						if(colorB >= 255) colorB = 254;
						if(colorG >= 255) colorG = 254;
						if(colorR >= 255)colorR = 254;
					}
					//depth가 2.5 ~ 2m 일때의 밝기
					else if (depth_var < 2500 && depth_var >= 2000)
					{
						colorB = colorB * 1.5;
						colorG = colorG * 1.5;
						colorR = colorR * 1.5;
						if(colorB >= 255) colorB = 254;
						if(colorG >= 255) colorG = 254;
						if(colorR >= 255)colorR = 254;
					}
					//depth가 2 ~ 1.5m 일때의 밝기
					else if (depth_var < 2000 && depth_var >= 1500)
					{
						colorB = colorB * 1.7;
						colorG = colorG * 1.7;
						colorR = colorR * 1.7;
						if(colorB >= 255) colorB = 254;
						if(colorG >= 255) colorG = 254;
						if(colorR >= 255)colorR = 254;
					}
					//depth가 1.5 ~ 1m 일때의 밝기
					else if (depth_var < 1500 && depth_var >= 1000)
					{
						colorB = colorB * 2;
						colorG = colorG * 2;
						colorR = colorR * 2;
						if(colorB >= 255) colorB = 254;
						if(colorG >= 255) colorG = 254;
						if(colorR >= 255)colorR = 254;
					}
					//depth가 1 ~ 0.5m 일때의 밝기
					else if (depth_var < 1000 && depth_var >= 500)
					{
						colorB = colorB * 2.3;
						colorG = colorG * 2.3;
						colorR = colorR * 2.3;
						if(colorB >= 255) colorB = 254;
						if(colorG >= 255) colorG = 254;
						if(colorR >= 255)colorR = 254;
					}
				}

				colorCoordinateMapperMat.at<cv::Vec4b>(y/SPEEDBOOST,x/SPEEDBOOST)[0] = colorB;
				colorCoordinateMapperMat.at<cv::Vec4b>(y/SPEEDBOOST,x/SPEEDBOOST)[1] = colorG;
				colorCoordinateMapperMat.at<cv::Vec4b>(y/SPEEDBOOST,x/SPEEDBOOST)[2] = colorR;
				
			}
		}
	}
	
}

//붉은색 선으로 등고선 처리를 해준다.
void calibration_image_processing_red(cv::Mat colorCoordinateMapperMat, DepthSpacePoint depthSpacePoints[][colorWidth], cv::Mat depthMat)
{
	for(int y = 0; y < colorHeight; y+=SPEEDBOOST)
	{
		for(int x = 0; x < colorWidth; x+=SPEEDBOOST)
		{
			DepthSpacePoint dPoint = depthSpacePoints[y][x]; //depthmap 좌표를 받아온다.
			int depthX = static_cast<int>(dPoint.X); //depthX에 depthmap 좌표 x값을 저장한다.
			int depthY = static_cast<int>(dPoint.Y); //depthY에 depthmap 좌표 y값을 저장한다.
			if(depthX >=0 && depthX < depthWidth && depthY >= 0 && depthY < depthHeight)
			{
				if(depthMat.at<UINT16>(depthY,depthX) % 100 < 10 )
				{
					colorCoordinateMapperMat.at<cv::Vec4b>(y/SPEEDBOOST,x/SPEEDBOOST)[2] = 175;
				}
			}
		}
	}
}

//가장 가까운 물체를 붉은색으로 표시해준다.
//1m ~ 0.5m일때 영상처리를 해준다.
//1m에서 0.5m로 가까워지면 붉은색이 더 진해진다.
void calibration_image_processing_near(cv::Mat colorCoordinateMapperMat, DepthSpacePoint depthSpacePoints[][colorWidth], cv::Mat depthMat)
{
	int colorR;
	int depth_var;

	for(int y = 0; y < colorHeight; y+=SPEEDBOOST)
	{
		for(int x = 0; x < colorWidth; x+=SPEEDBOOST)
		{
			DepthSpacePoint dPoint = depthSpacePoints[y][x]; //depthmap 좌표를 받아온다.
			int depthX = static_cast<int>(dPoint.X); //depthX에 depthmap 좌표 x값을 저장한다.
			int depthY = static_cast<int>(dPoint.Y); //depthY에 depthmap 좌표 y값을 저장한다.
			if(depthX >=0 && depthX < depthWidth && depthY >= 0 && depthY < depthHeight)
			{
				depth_var = depthMat.at<UINT16>(depthY,depthX);
				if(depth_var != 0 || depth_var != 4500) // 노이즈 제거
				{
					colorR = colorCoordinateMapperMat.at<cv::Vec4b>(y/SPEEDBOOST,x/SPEEDBOOST)[2];
					//kinect와 물체의 거리가 1m 이하일 경우 영상처리를 해준다.
					if( depth_var > 900 && depth_var <= 1000)
					{
						colorR = colorCoordinateMapperMat.at<cv::Vec4b>(y/SPEEDBOOST,x/SPEEDBOOST)[2];
						colorR += 10;
						if(colorR >= 255) colorR = 254;
						colorCoordinateMapperMat.at<cv::Vec4b>(y/SPEEDBOOST,x/SPEEDBOOST)[2] = colorR;
					}
					else if( depth_var > 800 && depth_var <= 900)
					{
						colorR = colorCoordinateMapperMat.at<cv::Vec4b>(y/SPEEDBOOST,x/SPEEDBOOST)[2];
						colorR += 30;
						if(colorR >= 255) colorR = 254;
						colorCoordinateMapperMat.at<cv::Vec4b>(y/SPEEDBOOST,x/SPEEDBOOST)[2] = colorR;
					}
					else if( depth_var > 700 && depth_var <= 800)
					{
						colorR = colorCoordinateMapperMat.at<cv::Vec4b>(y/SPEEDBOOST,x/SPEEDBOOST)[2];
						colorR += 50;
						if(colorR >= 255) colorR = 254;
						colorCoordinateMapperMat.at<cv::Vec4b>(y/SPEEDBOOST,x/SPEEDBOOST)[2] = colorR;
					}
					else if( depth_var > 600 && depth_var <= 700)
					{
						colorR = colorCoordinateMapperMat.at<cv::Vec4b>(y/SPEEDBOOST,x/SPEEDBOOST)[2];
						colorR += 70;
						if(colorR >= 255) colorR = 254;
						colorCoordinateMapperMat.at<cv::Vec4b>(y/SPEEDBOOST,x/SPEEDBOOST)[2] = colorR;
					}
					else if( depth_var > 500 && depth_var <= 600)
					{
						colorR = colorCoordinateMapperMat.at<cv::Vec4b>(y/SPEEDBOOST,x/SPEEDBOOST)[2];
						colorR += 90;
						if(colorR >= 255) colorR = 254;
						colorCoordinateMapperMat.at<cv::Vec4b>(y/SPEEDBOOST,x/SPEEDBOOST)[2] = colorR;
					}
				}
			}
		}
	}
}

//가장 가까운 물체가 밝고 어둡고 깜빡이는 효과를 넣는다.
void calibration_image_processing_bling(cv::Mat colorCoordinateMapperMat, DepthSpacePoint depthSpacePoints[][colorWidth], cv::Mat depthMat,int bling_var)
{
	int colorR;
	int depth_var;

	for(int y = 0; y < colorHeight; y+=SPEEDBOOST)
	{
		for(int x = 0; x < colorWidth; x+=SPEEDBOOST)
		{
			DepthSpacePoint dPoint = depthSpacePoints[y][x]; //depthmap 좌표를 받아온다.
			int depthX = static_cast<int>(dPoint.X); //depthX에 depthmap 좌표 x값을 저장한다.
			int depthY = static_cast<int>(dPoint.Y); //depthY에 depthmap 좌표 y값을 저장한다.
			if(depthX >=0 && depthX < depthWidth && depthY >= 0 && depthY < depthHeight)
			{
				depth_var = depthMat.at<UINT16>(depthY,depthX);
				if(depth_var != 0 || depth_var != 4500) // 노이즈 제거
				{
					colorR = colorCoordinateMapperMat.at<cv::Vec4b>(y/SPEEDBOOST,x/SPEEDBOOST)[2];
					//kinect와 물체의 거리가 1m 이하일 경우 영상처리를 해준다.
					if( depth_var > 900 && depth_var <= 1000)
					{
						if(bling_var % 10 == 0)
						{
							colorR = colorCoordinateMapperMat.at<cv::Vec4b>(y/SPEEDBOOST,x/SPEEDBOOST)[2];
							colorR += 10;
							if(colorR >= 255) colorR = 254;
							colorCoordinateMapperMat.at<cv::Vec4b>(y/SPEEDBOOST,x/SPEEDBOOST)[2] = colorR;
						}
					}
					else if( depth_var > 800 && depth_var <= 900)
					{
						if(bling_var % 8 == 0)
						{
							colorR = colorCoordinateMapperMat.at<cv::Vec4b>(y/SPEEDBOOST,x/SPEEDBOOST)[2];
							colorR += 30;
							if(colorR >= 255) colorR = 254;
							colorCoordinateMapperMat.at<cv::Vec4b>(y/SPEEDBOOST,x/SPEEDBOOST)[2] = colorR;
						}
					}
					else if( depth_var > 700 && depth_var <= 800)
					{
						if(bling_var % 6 == 0)
						{
							colorR = colorCoordinateMapperMat.at<cv::Vec4b>(y/SPEEDBOOST,x/SPEEDBOOST)[2];
							colorR += 50;
							if(colorR >= 255) colorR = 254;
							colorCoordinateMapperMat.at<cv::Vec4b>(y/SPEEDBOOST,x/SPEEDBOOST)[2] = colorR;
						}
					}
					else if( depth_var > 600 && depth_var <= 700)
					{
						if(bling_var % 4 == 0)
						{
							colorR = colorCoordinateMapperMat.at<cv::Vec4b>(y/SPEEDBOOST,x/SPEEDBOOST)[2];
							colorR += 70;
							if(colorR >= 255) colorR = 254;
							colorCoordinateMapperMat.at<cv::Vec4b>(y/SPEEDBOOST,x/SPEEDBOOST)[2] = colorR;
						}
					}
					else if( depth_var > 500 && depth_var <= 600)
					{
						if(bling_var % 2 == 0)
						{
							colorR = colorCoordinateMapperMat.at<cv::Vec4b>(y/SPEEDBOOST,x/SPEEDBOOST)[2];
							colorR += 90;
							if(colorR >= 255) colorR = 254;
							colorCoordinateMapperMat.at<cv::Vec4b>(y/SPEEDBOOST,x/SPEEDBOOST)[2] = colorR;
						}
					}
				}
			}
		}
	}
}
//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////





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

	std::cout << minDepth << " " << maxDepth << std::endl;

	char mode_detec;
	char mode=FINDNEAR;
	int bling_var = 0;
	//Frame을 생성하고 color와 depth image를 출력한다.
	///////////////////////////////////////////////////////////////////////////////////////



	while(1)
	{

		//가장 가까운 거리를 구하기 위해 myMinDepth를 초기화해 준다.
		myMinDepth = 4500;

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

			for(int y=0; y < depthHeight; y++)
			{
				for(int x = 0; x < depthWidth; x++)
				{
					if(myMinDepth > depthBufferMat.at<UINT16>(y,x) &&depthBufferMat.at<UINT16>(y,x) != 0 )
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
		

		/*
		#define NORMALIMAGE 0
		#define FINDNEAR 1
		#define GRAIMAGE 2
		#define BLINGBLING 3
		*/

		//모드를 선택한다.
		//영상 출력중에 입력키를 받는다.
		mode_detec = cv::waitKey( 1 );

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
		else if(mode_detec == GRAIMAGE)
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

		//만약 mode에서 선택영상을 받는 경우
		//선택된 mode에 따라 영상처리 함수를 실행시킨다.
		if(mode == NORMALIMAGE)
		{
			//처리 과정 X
		}
		else if(mode == REDPOINT)
		{
			calibration_image_processing_red(colorCoordinateMapperMat, depthSpacePoints, depthBufferMat);
		}
		else if(mode == FINDNEAR)
		{
			calibration_image_processing_near(colorCoordinateMapperMat, depthSpacePoints, depthBufferMat);
		}
		else if(mode == BLINGBLING)
		{
			bling_var += 1;
			if(bling_var > 100)
			{
				bling_var = 0;
			}
			
			calibration_image_processing_bling(colorCoordinateMapperMat, depthSpacePoints, depthBufferMat, bling_var);
			
		}
		else if(mode == GRAIMAGE)
		{
			calibration_image_processing_gra(colorCoordinateMapperMat, depthSpacePoints, depthBufferMat);
		}
		//영상처리를 위한 함수
		

		//resize가 필요한 경우를 위한 코드
		//resize가 필요 없을 경우에는 이 코드부분을 지우고
		//colorCoordinateMapperMat을 출력 시켜주면 된다.
		cv::resize(colorCoordinateMapperMat,colorMat,cv::Size(),1,1);
		
	

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




		SafeRelease(pDepthFrame); 
		SafeRelease(pColorFrame); //Frame을 풀어놓는다.
		//내부 버퍼가 해제되어 다음 데이터를 검색 할 수있는 산태가 된다.

		//Show window
		cv::imshow("Output", colorMat);
		cv::imshow("Depth", depthMat);
		//cv::imshow("CoordinateMapper",colorCoordinateMapperMat);

		
		
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