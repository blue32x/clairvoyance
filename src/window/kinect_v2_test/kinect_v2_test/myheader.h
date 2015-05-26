/******************************************************************************************************

								���δ��б�
								��ǻ�Ͱ��к�
								õ����(clairvoyance)
								Kinect v2�� depth map�� Ȱ���� �ڵ��� �Ĺ� ���� ����
								�������� : ������
								���� : �ֽ���
								���� : ���¼� �ڼ��� �ŵ�ȣ �ڹο� �ּ���
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

#include<omp.h>

//interprocess comnunication
#include<Windows.h>
#include<conio.h>
#include<tchar.h>


#define depthWidth 512 //Kinect v2 depth map width
#define depthHeight 424 //Kinect v2 depth map height
#define colorWidth 1920 //Kinect v2 color camera width
#define colorHeight 1080 //Kinect v2 color camera height

//coordinate �̹��� ������ �ٿ��ִ� ����
//1�� �ٲ��ָ� ����, 1�̻��̸� ������ ũ�⸦ �ٿ��־� 
//�ӵ��� ������ ���ش�.
//���Ŀ� ��Ƽ������ Ȥ�� �л�ó���� �ذ� ���ɼ��� ���δ�.
#define SPEEDBOOST 2

#define NORMALIMAGE '0'
#define FINDNEAR '2'
#define GRAIMAGE '1'
#define BLINGBLING '3'
#define REDPOINT '4'

#define BUF_SIZE 64