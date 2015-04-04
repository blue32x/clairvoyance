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

//main 함수
int main(/* int argc, _TCHAR* argv[] */) //_tmain은 유니코드가 정의되어 있지 않으면 main으로 확인되고, 유니코드가 정의된 경우에 wmain으로 확인됨
{
	cv::setUseOptimized( true );


	// Sensor 변수 선언
	IKinectSensor* pSensor; //kinect Sensor 구조체 포인터 변수 선언
	//IKinectSensor 구조체 변수에 대한 분석이 필요할듯
	HRESULT hResult = S_OK; //error check를 위한 HRESULT 변수, long type
	GetDefaultKinectSensor( &pSensor ); //sensor 초기화
	pSensor->Open(); //sensor를 연다
	

	// Source
	//color와 depth source를 받아온다.
	IColorFrameSource* pColorSource;
	pSensor->get_ColorFrameSource( &pColorSource );
	IDepthFrameSource* pDepthSource;
	pSensor->get_DepthFrameSource( &pDepthSource );
	

	// Reader
	//color와 depth source를 읽는다.
	IColorFrameReader* pColorReader;
	pColorSource->OpenReader( &pColorReader );
	IDepthFrameReader* pDepthReader;
	pDepthSource->OpenReader( &pDepthReader );
	

	// Description
	// color정보 분석
	IFrameDescription* pColorDescription;
	pColorSource->get_FrameDescription( &pColorDescription );
	//depth정보 분석
	IFrameDescription* pDepthDescription;
	pDepthSource->get_FrameDescription( &pDepthDescription );



	//color image 출력을 위하여 변수선언
	int colorWidth = 0; //color width를 받아올 변수
	int colorHeight = 0; //color height를 받아올 변수
	pColorDescription->get_Width( &colorWidth ); // 1920(v2의 color 이미지는 1920픽셀의 너비를 갖는다.)
	pColorDescription->get_Height( &colorHeight ); // 1080(v2의 color 이미지는 1080픽셀의 높이를 갖는다.)
	unsigned int colorBufferSize = colorWidth * colorHeight * 4 * sizeof( unsigned char ); //color buffer size 정의???

	cv::Mat colorBufferMat( colorHeight, colorWidth, CV_8UC4 ); //color buffer matrix를 생성한다.
	cv::Mat colorMat( colorHeight / 2, colorWidth / 2, CV_8UC4 ); //mat 구조체에 대하여 분석이 필요함.
	cv::namedWindow( "Color" ); //color 윈도우 생성

	



	//depth image 출력을 위하여 변수선언
	int depthWidth = 0; //depth width를 받아올 변수
	int depthHeight = 0; //depth hegith를 받아올 변수
	pDepthDescription->get_Width( &depthWidth ); // 512 (v2의 depth 이미지는 512픽셀의 너비를 갖는다.)
	pDepthDescription->get_Height( &depthHeight ); // 424 (v2의 depth 이미지는 424픽셀의 높이를 갖는다.)
	unsigned int depthBufferSize = depthWidth * depthHeight * sizeof( unsigned short ); // depth buffer size 정의 color buffer와 다른값을 갖는다??

	cv::Mat depthBufferMat( depthHeight, depthWidth, CV_16UC1 ); //depth buffer matrix를 생성
	cv::Mat depthMat( depthHeight, depthWidth, CV_8UC1 );
	cv::namedWindow( "Depth" ); //depth 윈도우 생성




	// Coordinate Mapper
	ICoordinateMapper* pCoordinateMapper; //coordinate Map 포인터 변수
	pSensor->get_CoordinateMapper( &pCoordinateMapper ); //sensor에서 coordinate map을 갖고온다.

	cv::Mat coordinateMapperMat( depthHeight, depthWidth, CV_8UC4 ); //depth 영상이 더 작기때문에 depth map을 기준으로 coordinate map matrix 생성
	cv::namedWindow( "CoordinateMapper" ); //캘리브래이션한 정보를 담고있는 영상을 출력할 윈도우를 생성한다.

	unsigned short minDepth, maxDepth;
	pDepthSource->get_DepthMinReliableDistance( &minDepth );
	pDepthSource->get_DepthMaxReliableDistance( &maxDepth );




	//생성한 3개의 frame에 영상을 출력해준다.
	while( 1 ){
		
		// Color Frame
		IColorFrame* pColorFrame = nullptr;
		hResult = pColorReader->AcquireLatestFrame( &pColorFrame );
		if( SUCCEEDED( hResult ) ){
			hResult = pColorFrame->CopyConvertedFrameDataToArray( colorBufferSize, reinterpret_cast<BYTE*>( colorBufferMat.data ), ColorImageFormat::ColorImageFormat_Bgra );
			if( SUCCEEDED( hResult ) ){
				cv::resize( colorBufferMat, colorMat, cv::Size(), 0.5, 0.5 );
			}
		}
		//SafeRelease( pColorFrame );

		// Depth Frame
		IDepthFrame* pDepthFrame = nullptr;
		hResult = pDepthReader->AcquireLatestFrame( &pDepthFrame );
		if( SUCCEEDED( hResult ) ){
			hResult = pDepthFrame->AccessUnderlyingBuffer( &depthBufferSize, reinterpret_cast<UINT16**>( &depthBufferMat.data ) );
			if( SUCCEEDED( hResult ) ){
				depthBufferMat.convertTo( depthMat, CV_8U, -255.0f / 8000.0f, 255.0f );
			}
		}
		//SafeRelease( pDepthFrame );
		
		// Mapping (Depth to Color)
		if( SUCCEEDED( hResult ) )
		{
			std::vector<ColorSpacePoint> colorSpacePoints( depthWidth * depthHeight );
			hResult = pCoordinateMapper->MapDepthFrameToColorSpace( depthWidth * depthHeight, reinterpret_cast<UINT16*>( depthBufferMat.data ), depthWidth * depthHeight, &colorSpacePoints[0] );
			if( SUCCEEDED( hResult ) )
			{
				coordinateMapperMat = cv::Scalar( 0, 0, 0, 0 );
				for( int y = 0; y < depthHeight; y++ )
				{
					for( int x = 0; x < depthWidth; x++ )
					{
						unsigned int index = y * depthWidth + x;
						ColorSpacePoint point = colorSpacePoints[index];
						int colorX = static_cast<int>( std::floor( point.X + 1  ) );
						int colorY = static_cast<int>( std::floor( point.Y + 0.5 ) );
						unsigned short depth = depthBufferMat.at<unsigned short>( y, x );
						if( ( colorX >= 0 ) && ( colorX < colorWidth ) && ( colorY >= 0 ) && ( colorY < colorHeight ) && ( depth >= minDepth ) && ( depth <= maxDepth ) )
						{
							coordinateMapperMat.at<cv::Vec4b>( y, x ) = colorBufferMat.at<cv::Vec4b>( colorY, colorX );
						}
					}
				}
			}
		}

		SafeRelease( pColorFrame );
		SafeRelease( pDepthFrame );

		cv::imshow( "Color", colorMat );
		cv::imshow( "Depth", depthMat );
		cv::imshow( "CoordinateMapper", coordinateMapperMat );

		if( cv::waitKey( 30 ) == VK_ESCAPE ){
			break;
		}
	}

	//safeRlease함수를 통하여 release해준다.
	SafeRelease( pColorSource );
	SafeRelease( pDepthSource );
	SafeRelease( pColorReader );
	SafeRelease( pDepthReader );
	SafeRelease( pColorDescription );
	SafeRelease( pDepthDescription );
	SafeRelease( pCoordinateMapper );

	//kinect sensor close
	if( pSensor ){
		pSensor->Close();
	}

	//kinect sensor pointer release
	SafeRelease( pSensor );

	cv::destroyAllWindows(); //생성한 3개의 윈도우를 닫아준다.

	return 0; //main 함수 반환
}
