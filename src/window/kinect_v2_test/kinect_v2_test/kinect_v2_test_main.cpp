// Kinect 2 Basic
// Kinect 2 Sample Source Code of Print Color and Depth using OpenCV
// http://www.locofield.com/3278
// Copyright 2014. LocoField. All rights reserved.
// 코딩 스타일에 따라 수정은 가능하지만 원본 출처를 밝혀주세요.
 
#include "stdafx.h"
 
 
BOOL bThreadLoop = TRUE;
BOOL bNormalize = FALSE;
 
 
template<class T>
void SafeKinectInterfaceRelease(T*& pInterface)
{
    if( pInterface != nullptr )
    {
        pInterface->Release();
        pInterface = nullptr;
    }
}
 
 
template<class T1, class T2>
void AcquireKinectFrame(T1*& pReader, T2*& pFrame)
{
    HRESULT hr = E_PENDING;
    T2* pTempFrame = nullptr;
 
    while( bThreadLoop && hr == E_PENDING )
    {
        hr = pReader->AcquireLatestFrame(&pTempFrame);
    }
 
    if( SUCCEEDED(hr) )
    {
        pFrame = pTempFrame;
    }
    else
    {
        pFrame = nullptr;
    }
}
 
 
void UpdateKinectColorFrame(IColorFrameReader* pColorFrameReader, Mat* matKinectColor)
{
    IColorFrame* pColorFrame = nullptr;
    IFrameDescription *pFrameDescription = nullptr;
    INT height;
    INT width;
    ColorImageFormat imageFormat = ColorImageFormat_None;
    UINT sizeBuffer = 0;
    BYTE* buffer = nullptr;
 
 
    // Init frame
    AcquireKinectFrame(pColorFrameReader, pColorFrame);
 
    if( pColorFrame != nullptr )
    {
        pColorFrame->get_FrameDescription(&pFrameDescription);
        pColorFrame->get_RawColorImageFormat(&imageFormat);
 
        pFrameDescription->get_Height(&height);
        pFrameDescription->get_Width(&width);
 
        *matKinectColor = Mat::zeros(height, width, CV_8UC4);
 
        SafeKinectInterfaceRelease(pColorFrame);
 
 
        // Start acquire loop
        while( bThreadLoop )
        {
            AcquireKinectFrame(pColorFrameReader, pColorFrame);
 
            if( pColorFrame != nullptr )
            {
                if( imageFormat == ColorImageFormat_Bgra )
                {
                    pColorFrame->AccessRawUnderlyingBuffer(&sizeBuffer, &buffer);
                    memcpy(matKinectColor->data, buffer, matKinectColor->dataend - matKinectColor->datastart);
 
                    if( buffer != nullptr )
                    {
                        memcpy(matKinectColor->data, buffer, matKinectColor->dataend - matKinectColor->datastart);
                        buffer = nullptr;
                    }
                }
                else
                {
                    pColorFrame->CopyConvertedFrameDataToArray(matKinectColor->dataend - matKinectColor->datastart, matKinectColor->data, ColorImageFormat_Bgra);
                }
 
                SafeKinectInterfaceRelease(pColorFrame);
            }
        }
    }
 
 
    SafeKinectInterfaceRelease(pFrameDescription);
    SafeKinectInterfaceRelease(pColorFrame);
}
 
 
void UpdateKinectDepthFrame(IDepthFrameReader* pDepthFrameReader, Mat* matKinectDepth)
{
    IDepthFrame* pDepthFrame = nullptr;
    IFrameDescription *pFrameDescription = nullptr;
    INT height;
    INT width;
    USHORT distanceDepthMin;
    USHORT distanceDepthMax;
    UINT sizeBuffer = 0;
    UINT16* buffer = nullptr;
 
 
    // Init frame
    AcquireKinectFrame(pDepthFrameReader, pDepthFrame);
 
    if( pDepthFrame != nullptr )
    {
        pDepthFrame->get_FrameDescription(&pFrameDescription);
        pDepthFrame->get_DepthMinReliableDistance(&distanceDepthMin);
        pDepthFrame->get_DepthMaxReliableDistance(&distanceDepthMax);
 
        pFrameDescription->get_Height(&height);
        pFrameDescription->get_Width(&width);
 
        *matKinectDepth = Mat::zeros(height, width, CV_16UC1);
 
        SafeKinectInterfaceRelease(pDepthFrame);
 
 
        // Start acquire loop
        while( bThreadLoop )
        {
            AcquireKinectFrame(pDepthFrameReader, pDepthFrame);
 
            if( pDepthFrame != nullptr )
            {
                pDepthFrame->AccessUnderlyingBuffer(&sizeBuffer, &buffer);
                memcpy(matKinectDepth->data, buffer, matKinectDepth->dataend - matKinectDepth->datastart);
 
                if( bNormalize )
                    normalize(*matKinectDepth, *matKinectDepth, numeric_limits<USHORT>::min(), numeric_limits<USHORT>::max(), NORM_MINMAX);
 
                SafeKinectInterfaceRelease(pDepthFrame);
            }
        }
    }
 
 
    SafeKinectInterfaceRelease(pFrameDescription);
    SafeKinectInterfaceRelease(pDepthFrame);
}
 
 
int _tmain(int argc, _TCHAR* argv[])
{
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
 
    HRESULT hr;
    IKinectSensor* pKinectSensor = nullptr;
    //IMultiSourceFrameReader
    IColorFrameReader* pColorFrameReader = nullptr;
    IDepthFrameReader* pDepthFrameReader = nullptr;
    IColorFrameSource* pColorFrameSource = nullptr;
    IDepthFrameSource* pDepthFrameSource = nullptr;
 
    Mat* matKinectColor = nullptr;
    Mat* matKinectDepth = nullptr;
 
 
    // Sensor init
    hr = GetDefaultKinectSensor(&pKinectSensor);
 
    if( FAILED(hr) )
    {
        cout << hr << endl;
        return 1;
    }
 
 
    hr = pKinectSensor->Open();
 
    if( FAILED(hr) )
    {
        cout << hr << endl;
        return 1;
    }
 
 
    // Color and depth frame init
    hr = pKinectSensor->get_ColorFrameSource(&pColorFrameSource);
    hr = pKinectSensor->get_DepthFrameSource(&pDepthFrameSource);
 
    if( pColorFrameSource != nullptr )
    {
        pColorFrameSource->OpenReader(&pColorFrameReader);
        SafeKinectInterfaceRelease(pColorFrameSource);
    }
 
    if( pDepthFrameSource != nullptr )
    {
        pDepthFrameSource->OpenReader(&pDepthFrameReader);
        SafeKinectInterfaceRelease(pDepthFrameSource);
    }
 
 
    // Acquire color and depth frame in each thread
    matKinectColor = new Mat;
    matKinectDepth = new Mat;
 
    thread t0(UpdateKinectColorFrame, pColorFrameReader, matKinectColor);
    thread t1(UpdateKinectDepthFrame, pDepthFrameReader, matKinectDepth);
 
    while( bThreadLoop )
    {
        if( !matKinectColor->empty() )
            imshow("Kinect Color", *matKinectColor);
 
        if( !matKinectDepth->empty() )
            imshow("Kinect Depth", *matKinectDepth);
 
        switch( waitKey(1) )
        {
        case 'q':
            bThreadLoop = FALSE;
            break;
 
        case 'n':
            if( bNormalize )
                bNormalize = FALSE;
            else
                bNormalize = TRUE;
            break;
        }
    }
 
    t0.join();
    t1.join();
 
 
    // Release
    SafeKinectInterfaceRelease(pColorFrameReader);
    SafeKinectInterfaceRelease(pDepthFrameReader);
    SafeKinectInterfaceRelease(pKinectSensor);
 
    if( matKinectColor != nullptr )
    {
        delete matKinectColor;
        matKinectColor = nullptr;
    }
 
    if( matKinectDepth != nullptr )
    {
        delete matKinectDepth;
        matKinectDepth = nullptr;
    }
 
 
    return 0;
}