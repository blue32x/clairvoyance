#include"myheader.h"
class mykinect
{
private:
	IKinectSensor * pSensor;
	HRESULT hResult;

public:
	mykinect(IKinectSensor *mysensor);
	IKinectSensor* initialize();
	IColorFrameSource *setColorSource(IColorFrameSource *color);
	IDepthFrameSource *setDepthSource(IDepthFrameSource *depth);
	IColorFrameReader *setColorReader(IColorFrameSource *color,IColorFrameReader *pColorReader);
	IDepthFrameReader *setDepthReader(IDepthFrameSource *depth,IDepthFrameReader * pDepthReader);
};

mykinect::mykinect(IKinectSensor *mysensor)
{
	pSensor=mysensor;
	hResult=S_OK;
}
IKinectSensor* mykinect::initialize()
{
	hResult = GetDefaultKinectSensor(&pSensor);  //기본 Sensor를 얻을 수 있다.
	if(FAILED(hResult))
	{
		std :: cerr << "Error : GetDEfaultKinectSensor" << std ::endl;
	}
	hResult = pSensor ->Open(); //Sensor를 연다.
	if(FAILED(hResult))
	{
		std :: cerr << "Error : IKinectSensor :: opne()" << std :: endl;
	}
	////////////////////////////////////////////////////////////////////////////////////
	return pSensor;
}
IColorFrameSource * mykinect::setColorSource(IColorFrameSource *color)
{
	//Sensor에서 Source를 가져온다.
	////////////////////////////////////////////////////////////////////////////////////
	 //color 구조를 위한 Source 인터페이스
	hResult = pSensor -> get_ColorFrameSource(&color); //Sensor에서 Source를 가져온다.
	if(FAILED(hResult)) //pColorSource에 kinect v2의 color 정보가 저장된 상태이다.
	{
		std::cerr << "Error : IKinectSensor :: get_ColorFrameSource()" << std:: endl;
	}
	return color;
}
IDepthFrameSource *mykinect::setDepthSource(IDepthFrameSource *depth)
{
	//depth 구조를 위한 Source 인터페이스
	hResult = pSensor -> get_DepthFrameSource(&depth);
	if(FAILED(hResult))
	{
		std::cerr<<"Error :IKinectSensor::get_DepthFrameSource()"<<std::endl;
	}
	return depth;
}
IColorFrameReader *mykinect::setColorReader(IColorFrameSource *color,IColorFrameReader *pColorReader)
{
		hResult = color -> OpenReader( & pColorReader); //Source에서 Reader를 연다.
	if(FAILED(hResult))
	{
		std :: cerr << "Error : IColorFrameSource :: OpenReader()" << std :: endl;
	}
	return pColorReader;
}
IDepthFrameReader *mykinect::setDepthReader(IDepthFrameSource *depth,IDepthFrameReader * pDepthReader)
{
		hResult = depth -> OpenReader( & pDepthReader); //Source에서 Reader를 연다.
	if(FAILED(hResult))
	{
		std::cerr << "Error : IDepthFrameSource :: OpenReader()"<<std::endl;
	}
	return pDepthReader;
}