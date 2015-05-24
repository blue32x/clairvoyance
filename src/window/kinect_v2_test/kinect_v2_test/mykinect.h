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
	hResult = GetDefaultKinectSensor(&pSensor);  //�⺻ Sensor�� ���� �� �ִ�.
	if(FAILED(hResult))
	{
		std :: cerr << "Error : GetDEfaultKinectSensor" << std ::endl;
	}
	hResult = pSensor ->Open(); //Sensor�� ����.
	if(FAILED(hResult))
	{
		std :: cerr << "Error : IKinectSensor :: opne()" << std :: endl;
	}
	////////////////////////////////////////////////////////////////////////////////////
	return pSensor;
}
IColorFrameSource * mykinect::setColorSource(IColorFrameSource *color)
{
	//Sensor���� Source�� �����´�.
	////////////////////////////////////////////////////////////////////////////////////
	 //color ������ ���� Source �������̽�
	hResult = pSensor -> get_ColorFrameSource(&color); //Sensor���� Source�� �����´�.
	if(FAILED(hResult)) //pColorSource�� kinect v2�� color ������ ����� �����̴�.
	{
		std::cerr << "Error : IKinectSensor :: get_ColorFrameSource()" << std:: endl;
	}
	return color;
}
IDepthFrameSource *mykinect::setDepthSource(IDepthFrameSource *depth)
{
	//depth ������ ���� Source �������̽�
	hResult = pSensor -> get_DepthFrameSource(&depth);
	if(FAILED(hResult))
	{
		std::cerr<<"Error :IKinectSensor::get_DepthFrameSource()"<<std::endl;
	}
	return depth;
}
IColorFrameReader *mykinect::setColorReader(IColorFrameSource *color,IColorFrameReader *pColorReader)
{
		hResult = color -> OpenReader( & pColorReader); //Source���� Reader�� ����.
	if(FAILED(hResult))
	{
		std :: cerr << "Error : IColorFrameSource :: OpenReader()" << std :: endl;
	}
	return pColorReader;
}
IDepthFrameReader *mykinect::setDepthReader(IDepthFrameSource *depth,IDepthFrameReader * pDepthReader)
{
		hResult = depth -> OpenReader( & pDepthReader); //Source���� Reader�� ����.
	if(FAILED(hResult))
	{
		std::cerr << "Error : IDepthFrameSource :: OpenReader()"<<std::endl;
	}
	return pDepthReader;
}