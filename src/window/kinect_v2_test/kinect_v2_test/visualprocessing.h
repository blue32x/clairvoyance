#include"myheader.h"

HANDLE hMapFile3;
LPCTSTR pBuf3;
char buffer3[BUF_SIZE] = "0";

void calibration_image_processing_all(cv::Mat colorCoordinateMapperMat, DepthSpacePoint depthSpacePoints[][colorWidth], cv::Mat depthMat)
{
	double colorR, colorG, colorB;
	float depth_var;
	//#pragma omp parallel for
	for(int y = 0; y < colorHeight; y+=SPEEDBOOST)
	{
		for(int x = 0; x < colorWidth; x+=SPEEDBOOST)
		{
			DepthSpacePoint dPoint = depthSpacePoints[y][x]; //depthmap ��ǥ�� �޾ƿ´�.
			int depthX = static_cast<int>(dPoint.X); //depthX�� depthmap ��ǥ x���� �����Ѵ�.
			int depthY = static_cast<int>(dPoint.Y); //depthY�� depthmap ��ǥ y���� �����Ѵ�.
			if(depthX >=0 && depthX < depthWidth && depthY >= 0 && depthY < depthHeight)
			{
				//color RGB�� �޾ƿ´�.
				colorB = colorCoordinateMapperMat.at<cv::Vec4b>(y/SPEEDBOOST,x/SPEEDBOOST)[0];
				colorG = colorCoordinateMapperMat.at<cv::Vec4b>(y/SPEEDBOOST,x/SPEEDBOOST)[1];
				colorR = colorCoordinateMapperMat.at<cv::Vec4b>(y/SPEEDBOOST,x/SPEEDBOOST)[2];
				depth_var =depthMat.at<UINT16>(depthY,depthX) ;
				//if(depth_var>999) depth_var=depth_var/10.0;
				colorB = colorB / (depth_var/1000.0);
				colorG = colorG / (depth_var/1000.0);
				colorR = colorR / (depth_var/1000.0);
				if(colorB >= 255) colorB = 254;
				if(colorG >= 255) colorG = 254;
				if(colorR >= 255)colorR = 254;

				colorCoordinateMapperMat.at<cv::Vec4b>(y/SPEEDBOOST,x/SPEEDBOOST)[0] = colorB;
				colorCoordinateMapperMat.at<cv::Vec4b>(y/SPEEDBOOST,x/SPEEDBOOST)[1] = colorG;
				colorCoordinateMapperMat.at<cv::Vec4b>(y/SPEEDBOOST,x/SPEEDBOOST)[2] = colorR;

			}
			if(depth_var<600 && depth_var >=500)
			{
				buffer3[0] = '1';
			}
			/*
			else{
			///// Send mode signal to kinect process
			buffer3[0] = '0';

			Write buffer3 for kinect process
			CopyMemory((PVOID)pBuf3, buffer3, sizeof(buffer3));
			}
			*/
		}
	}
}

//����� ���� ���, ��ο� ���� ��Ӱ� ��� ó���� ���ش�.
//4.5m ���� 50cm �� ��⸦ ����� ������ ���� ��� ó���Ѵ�.
void calibration_image_processing_gra(cv::Mat colorCoordinateMapperMat, DepthSpacePoint depthSpacePoints[][colorWidth], cv::Mat depthMat)
{
	double colorR, colorG, colorB;
	int depth_var;
	//#pragma omp parallel for
	for(int y = 0; y < colorHeight; y+=SPEEDBOOST)

	{
		for(int x = 0; x < colorWidth; x+=SPEEDBOOST)
		{
			DepthSpacePoint dPoint = depthSpacePoints[y][x]; //depthmap ��ǥ�� �޾ƿ´�.
			int depthX = static_cast<int>(dPoint.X); //depthX�� depthmap ��ǥ x���� �����Ѵ�.
			int depthY = static_cast<int>(dPoint.Y); //depthY�� depthmap ��ǥ y���� �����Ѵ�.
			if(depthX >=0 && depthX < depthWidth && depthY >= 0 && depthY < depthHeight)
			{
				//color RGB�� �޾ƿ´�.
				colorB = colorCoordinateMapperMat.at<cv::Vec4b>(y/SPEEDBOOST,x/SPEEDBOOST)[0];
				colorG = colorCoordinateMapperMat.at<cv::Vec4b>(y/SPEEDBOOST,x/SPEEDBOOST)[1];
				colorR = colorCoordinateMapperMat.at<cv::Vec4b>(y/SPEEDBOOST,x/SPEEDBOOST)[2];
				depth_var =depthMat.at<UINT16>(depthY,depthX) ;

				if(depth_var != 0 || depth_var != 4500)
				{
					//depth�� 4 ~ 3.5m�϶��� ���
					if(depth_var  <  4000 && depth_var >= 3500)
					{
						colorB = colorB * 0.5;
						colorG = colorG * 0.5;
						colorR = colorR * 0.5;
						if(colorB >= 255) colorB = 254;
						if(colorG >= 255) colorG = 254;
						if(colorR >= 255)colorR = 254;
					}
					//depth�� 3.5 ~ 3m �϶��� ���
					else if (depth_var < 3500 && depth_var >= 3000)
					{
						colorB = colorB * 0.7;
						colorG = colorG * 0.7;
						colorR = colorR * 0.7;
						if(colorB >= 255) colorB = 254;
						if(colorG >= 255) colorG = 254;
						if(colorR >= 255)colorR = 254;
					}
					//depth�� 3 ~ 2.5m �϶��� ���
					else if (depth_var < 3000 && depth_var >= 2500)
					{
						colorB = colorB * 0.9;
						colorG = colorG * 0.9;
						colorR = colorR * 0.9;
						if(colorB >= 255) colorB = 254;
						if(colorG >= 255) colorG = 254;
						if(colorR >= 255)colorR = 254;
					}
					//depth�� 2.5 ~ 2m �϶��� ���
					else if (depth_var < 2500 && depth_var >= 2000)
					{
						colorB = colorB * 1.2;
						colorG = colorG * 1.2;
						colorR = colorR * 1.2;
						if(colorB >= 255) colorB = 254;
						if(colorG >= 255) colorG = 254;
						if(colorR >= 255)colorR = 254;
					}
					//depth�� 2 ~ 1.5m �϶��� ���
					else if (depth_var < 2000 && depth_var >= 1500)
					{
						colorB = colorB * 1.7;
						colorG = colorG * 1.7;
						colorR = colorR * 1.7;
						if(colorB >= 255) colorB = 254;
						if(colorG >= 255) colorG = 254;
						if(colorR >= 255)colorR = 254;
					}
					//depth�� 1.5 ~ 1m �϶��� ���
					else if (depth_var < 1500 && depth_var >= 1000)
					{
						colorB = colorB * 2;
						colorG = colorG * 2;
						colorR = colorR * 2;
						if(colorB >= 255) colorB = 254;
						if(colorG >= 255) colorG = 254;
						if(colorR >= 255)colorR = 254;
					}
					//depth�� 1 ~ 0.5m �϶��� ���
					else if (depth_var < 1000 && depth_var >= 500)
					{
						colorB = colorB * 2.3;
						colorG = colorG * 2.3;
						colorR = colorR * 2.3;
						if(colorB >= 255) colorB = 254;
						if(colorG >= 255) colorG = 254;
						if(colorR >= 255)colorR = 254;
					}
					if(depth_var<600 && depth_var >=500){
						buffer3[0] = '1';
					}
					/*
					else{
						///// Send mode signal to kinect process
						buffer3[0] = '0';

						Write buffer3 for kinect process
						CopyMemory((PVOID)pBuf3, buffer3, sizeof(buffer3));
					}
					*/
				}

				colorCoordinateMapperMat.at<cv::Vec4b>(y/SPEEDBOOST,x/SPEEDBOOST)[0] = colorB;
				colorCoordinateMapperMat.at<cv::Vec4b>(y/SPEEDBOOST,x/SPEEDBOOST)[1] = colorG;
				colorCoordinateMapperMat.at<cv::Vec4b>(y/SPEEDBOOST,x/SPEEDBOOST)[2] = colorR;

			}
		}
	}

}

//������ ������ ��� ó���� ���ش�.
void calibration_image_processing_red(cv::Mat colorCoordinateMapperMat, DepthSpacePoint depthSpacePoints[][colorWidth], cv::Mat depthMat)
{
	//#pragma omp parallel for
	for(int y = 0; y < colorHeight; y+=SPEEDBOOST)
	{
		for(int x = 0; x < colorWidth; x+=SPEEDBOOST)
		{
			DepthSpacePoint dPoint = depthSpacePoints[y][x]; //depthmap ��ǥ�� �޾ƿ´�.
			int depthX = static_cast<int>(dPoint.X); //depthX�� depthmap ��ǥ x���� �����Ѵ�.
			int depthY = static_cast<int>(dPoint.Y); //depthY�� depthmap ��ǥ y���� �����Ѵ�.
			if(depthX >=0 && depthX < depthWidth && depthY >= 0 && depthY < depthHeight)
			{
				if(depthMat.at<UINT16>(depthY,depthX) % 100 < 10 )
				{
					colorCoordinateMapperMat.at<cv::Vec4b>(y/SPEEDBOOST,x/SPEEDBOOST)[2] = 175;
				}
			}
			//if(depth_var<530 && depth_var >=500) CopyMemory((PVOID)lpMapping,isStop,sizeof(char));
		}
	}
}

//���� ����� ��ü�� ���������� ǥ�����ش�.
//1m ~ 0.5m�϶� ����ó���� ���ش�.
//1m���� 0.5m�� ��������� �������� �� ��������.
void calibration_image_processing_near(cv::Mat colorCoordinateMapperMat, DepthSpacePoint depthSpacePoints[][colorWidth], cv::Mat depthMat)
{
	int colorR;
	int depth_var;

	//#pragma omp parallel for private(colorR)
	for(int y = 0; y < colorHeight; y+=SPEEDBOOST)
	{
		for(int x = 0; x < colorWidth; x+=SPEEDBOOST)
		{
			DepthSpacePoint dPoint = depthSpacePoints[y][x]; //depthmap ��ǥ�� �޾ƿ´�.
			int depthX = static_cast<int>(dPoint.X); //depthX�� depthmap ��ǥ x���� �����Ѵ�.
			int depthY = static_cast<int>(dPoint.Y); //depthY�� depthmap ��ǥ y���� �����Ѵ�.
			if(depthX >=0 && depthX < depthWidth && depthY >= 0 && depthY < depthHeight)
			{
				depth_var = depthMat.at<UINT16>(depthY,depthX);
				if(depth_var != 0 || depth_var != 4500) // ������ ����
				{
					colorR = colorCoordinateMapperMat.at<cv::Vec4b>(y/SPEEDBOOST,x/SPEEDBOOST)[2];
					//kinect�� ��ü�� �Ÿ��� 1m ������ ��� ����ó���� ���ش�.
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
					if(depth_var<600 && depth_var >=500){
						buffer3[0] = '1';
					}
					/*
					else{
						///// Send mode signal to kinect process
						buffer3[0] = '0';

						Write buffer3 for kinect process
						CopyMemory((PVOID)pBuf3, buffer3, sizeof(buffer3));
					}
					*/
				}
			}
		}
	}
}

//���� ����� ��ü�� ��� ��Ӱ� �����̴� ȿ���� �ִ´�.
void calibration_image_processing_bling(cv::Mat colorCoordinateMapperMat, DepthSpacePoint depthSpacePoints[][colorWidth], cv::Mat depthMat,int bling_var)
{
	int colorR;
	int depth_var;
	//std::cout<<buffer3<<std::endl;
	for(int y = 0; y < colorHeight; y+=SPEEDBOOST)
	{
		for(int x = 0; x < colorWidth; x+=SPEEDBOOST)
		{
			DepthSpacePoint dPoint = depthSpacePoints[y][x]; //depthmap ��ǥ�� �޾ƿ´�.
			int depthX = static_cast<int>(dPoint.X); //depthX�� depthmap ��ǥ x���� �����Ѵ�.
			int depthY = static_cast<int>(dPoint.Y); //depthY�� depthmap ��ǥ y���� �����Ѵ�.
			if(depthX >=30 && depthX < depthWidth-30 && depthY >= 60 && depthY < depthHeight-60)
			{
				depth_var = depthMat.at<UINT16>(depthY,depthX);
				if(depth_var != 0 || depth_var != 4500) // ������ ����
				{
					colorR = colorCoordinateMapperMat.at<cv::Vec4b>(y/SPEEDBOOST,x/SPEEDBOOST)[2];
					//kinect�� ��ü�� �Ÿ��� 1m ������ ��� ����ó���� ���ش�.
					if( depth_var > 1300 && depth_var <= 1500)
					{
						if(bling_var % 32 == 0)
						{
							colorR = colorCoordinateMapperMat.at<cv::Vec4b>(y/SPEEDBOOST,x/SPEEDBOOST)[2];
							colorR += 10;
							if(colorR >= 255) colorR = 254;
							colorCoordinateMapperMat.at<cv::Vec4b>(y/SPEEDBOOST,x/SPEEDBOOST)[2] = colorR;
						}
					}
					else if( depth_var > 1100 && depth_var <= 1300)
					{
						if(bling_var % 16 == 0)
						{
							colorR = colorCoordinateMapperMat.at<cv::Vec4b>(y/SPEEDBOOST,x/SPEEDBOOST)[2];
							colorR += 30;
							if(colorR >= 255) colorR = 254;
							colorCoordinateMapperMat.at<cv::Vec4b>(y/SPEEDBOOST,x/SPEEDBOOST)[2] = colorR;
						}
					}
					else if( depth_var > 900 && depth_var <= 1100)
					{
						if(bling_var % 8 == 0)
						{
							colorR = colorCoordinateMapperMat.at<cv::Vec4b>(y/SPEEDBOOST,x/SPEEDBOOST)[2];
							colorR += 50;
							if(colorR >= 255) colorR = 254;
							colorCoordinateMapperMat.at<cv::Vec4b>(y/SPEEDBOOST,x/SPEEDBOOST)[2] = colorR;
						}
					}
					else if( depth_var > 700 && depth_var <= 900)
					{
						if(bling_var % 4 == 0)
						{
							colorR = colorCoordinateMapperMat.at<cv::Vec4b>(y/SPEEDBOOST,x/SPEEDBOOST)[2];
							colorR += 70;
							if(colorR >= 255) colorR = 254;
							colorCoordinateMapperMat.at<cv::Vec4b>(y/SPEEDBOOST,x/SPEEDBOOST)[2] = colorR;
						}
					}
					else if( depth_var > 500 && depth_var <= 700)
					{
						if(bling_var % 2 == 0)
						{
							colorR = colorCoordinateMapperMat.at<cv::Vec4b>(y/SPEEDBOOST,x/SPEEDBOOST)[2];
							colorR += 90;
							if(colorR >= 255) colorR = 254;
							colorCoordinateMapperMat.at<cv::Vec4b>(y/SPEEDBOOST,x/SPEEDBOOST)[2] = colorR;

						}
					}
					if(depth_var<600 && depth_var >=500){
						buffer3[0] = '1';
					}
					/*
					else{
						///// Send mode signal to kinect process
						buffer3[0] = '0';

						Write buffer3 for kinect process
						CopyMemory((PVOID)pBuf3, buffer3, sizeof(buffer3));
					}
					*/
				}
			}
		}
	}
}
//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////