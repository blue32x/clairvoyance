/********color + depth 영상**************************************/

#include <Windows.h>
#include <NuiApi.h>
#include <iostream>

#include <opencv/cv.h>
#include <opencv/highgui.h>
#include <opencv/cxcore.h>

using namespace std;

#define COLOR_WIDTH                   640
#define COLOR_HEIGHT                  480

#define DEPTH_WIDTH                    640
#define DEPTH_HEIGHT                  480

RGBQUAD rgb[640*480];

void InitializeKinect();
int createRGBImage(HANDLE h, IplImage* Color);
int createDepthImage(HANDLE h, IplImage* Depth);
RGBQUAD Nui_ShortToQuad_Depth(USHORT s);

int main(void)
{
        HANDLE colorStreamHandle;
        HANDLE DepthStreamHandle;
        HANDLE nextColorFrameEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
        HANDLE nextDepthFrameEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

        HRESULT hr;

        InitializeKinect();


        IplImage *Color = cvCreateImage(cvSize(COLOR_WIDTH, COLOR_HEIGHT), IPL_DEPTH_8U, 4);
        IplImage *Depth = cvCreateImage(cvSize(DEPTH_WIDTH, DEPTH_HEIGHT), IPL_DEPTH_8U, 4);
        cvNamedWindow("Color Image", CV_WINDOW_AUTOSIZE);
        cvNamedWindow("Depth Image", CV_WINDOW_AUTOSIZE);

        hr = NuiImageStreamOpen(NUI_IMAGE_TYPE_COLOR, NUI_IMAGE_RESOLUTION_640x480, 0, 2, nextColorFrameEvent, &colorStreamHandle);
        if(FAILED(hr))
        {
               cout<<"Could not open Image Stream"<<endl;
               return hr;
        }

        hr = NuiImageStreamOpen(NUI_IMAGE_TYPE_DEPTH_AND_PLAYER_INDEX, NUI_IMAGE_RESOLUTION_640x480, 0, 2, nextDepthFrameEvent, &DepthStreamHandle);
        if(FAILED(hr))
        {
               cout<<"Could not open Depth Image Stream"<<endl;
               return hr;
        }

        while(1)
        {
               WaitForSingleObject(nextColorFrameEvent, 1000);
               createRGBImage(colorStreamHandle, Color);
               WaitForSingleObject(nextDepthFrameEvent, 0);
               createDepthImage(DepthStreamHandle, Depth);

               if(cvWaitKey(10) == 0x001b)
               {
                       break;
               }
        }
        cvReleaseImageHeader(&Depth);
        cvReleaseImageHeader(&Color);
        NuiShutdown();
        cvDestroyAllWindows();

        return 0;
}

void InitializeKinect()
{
        bool FailToConnect;

        do
        {
               HRESULT hr = NuiInitialize(NUI_INITIALIZE_FLAG_USES_COLOR | NUI_INITIALIZE_FLAG_USES_DEPTH_AND_PLAYER_INDEX );

               if (FAILED(hr))
               {
                       system("cls");
                       cout<<"Failed to connect!"<<endl<<endl;
                       FailToConnect = true;
                       system("PAUSE");
               }
               else
               {
                       cout<<"Connection Established!"<<endl<<endl;
                       FailToConnect = false;
               }

        }
        while (FailToConnect);
}

int createRGBImage(HANDLE h, IplImage* Color)
{
        const NUI_IMAGE_FRAME *pImageFrame = NULL;
        HRESULT hr = NuiImageStreamGetNextFrame(h, 1000, &pImageFrame);

        if(FAILED(hr))
        {
               cout<<"Create RGB Image Failed"<<endl;
               return -1;
        }

        INuiFrameTexture *pTexture = pImageFrame->pFrameTexture;
        NUI_LOCKED_RECT LockedRect;
        pTexture->LockRect(0, &LockedRect, NULL, 0);

        if(LockedRect.Pitch != 0)
        {
               BYTE *pBuffer = (BYTE*)LockedRect.pBits;
               cvSetData(Color, pBuffer, LockedRect.Pitch);
               cvShowImage("Color Image", Color);
        }

        NuiImageStreamReleaseFrame(h, pImageFrame);

        return 0;
}

int createDepthImage(HANDLE h, IplImage* Depth)
{
        const NUI_IMAGE_FRAME *pImageFrame = NULL;
        HRESULT hr = NuiImageStreamGetNextFrame(h, 1000, &pImageFrame);

        if(FAILED(hr))
        {
               cout<<"Create Depth Image Failed"<<endl;
               return -1;
        }

        INuiFrameTexture *pTexture = pImageFrame->pFrameTexture;
        NUI_LOCKED_RECT LockedRect;
        pTexture->LockRect(0, &LockedRect, NULL, 0);

        if(LockedRect.Pitch != 0)
        {
               BYTE *pBuffer = (BYTE*)LockedRect.pBits;
               RGBQUAD *rgbrun = rgb;
               USHORT *pBufferRun = (USHORT*) pBuffer;

               for(int y = 0; y < DEPTH_HEIGHT; y++)
               {
                       for(int x = 0; x < DEPTH_WIDTH; x++)
                       {
                              RGBQUAD quad = Nui_ShortToQuad_Depth(*pBufferRun);
                              pBufferRun++;
                              *rgbrun = quad;
                              rgbrun++;
                       }
               }

               cvSetData(Depth, (BYTE*)rgb, Depth->widthStep);
               cvShowImage("Depth Image", Depth);
        }

        NuiImageStreamReleaseFrame(h, pImageFrame);

        return 0;
}

RGBQUAD Nui_ShortToQuad_Depth(USHORT s)
{
        USHORT realDepth = (s&0xfff8) >> 3;
        //      USHORT testRealDepth = NuiDepthPixelToDepth(s);
       
        // Convert depth info into an intensity for display
        BYTE b = 255 - static_cast<BYTE>(256 * realDepth / 0x0fff);

        //      cout<<"Real Depth : "<<realDepth<<endl;
        //      cout<<"test Depth : "<<testRealDepth<<endl;
       
        RGBQUAD q;
        q.rgbRed = q.rgbBlue = q.rgbGreen = ~b;
        return q;
}
/********color + depth 영상**************************************/