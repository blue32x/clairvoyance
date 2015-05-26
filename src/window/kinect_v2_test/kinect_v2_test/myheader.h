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

#include<omp.h>

//interprocess comnunication
#include<Windows.h>
#include<conio.h>
#include<tchar.h>


#define depthWidth 512 //Kinect v2 depth map width
#define depthHeight 424 //Kinect v2 depth map height
#define colorWidth 1920 //Kinect v2 color camera width
#define colorHeight 1080 //Kinect v2 color camera height

//coordinate 이미지 영상을 줄여주는 변수
//1로 바꿔주면 원본, 1이상이면 영상의 크기를 줄여주어 
//속도를 빠르게 해준다.
//추후에 멀티쓰레드 혹은 분산처리로 해결 가능성을 보인다.
#define SPEEDBOOST 2

#define NORMALIMAGE '0'
#define FINDNEAR '2'
#define GRAIMAGE '1'
#define BLINGBLING '3'
#define REDPOINT '4'

#define BUF_SIZE 64