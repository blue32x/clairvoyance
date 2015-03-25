#include <Windows.h>
#include <NuiApi.h>
#include <iostream>

#include <opencv/cv.h>
#include <opencv/highgui.h>
#include <opencv/cxcore.h>

using namespace std;

#define DEPTH_WIDTH                   640
#define DEPTH_HEIGHT          480

RGBQUAD rgb[640*480];

void InitializeKinect();
int createDepthImage(HANDLE h, IplImage* Depth);
RGBQUAD Nui_ShortToQuad_Depth(USHORT s);


int main(void)
{
        HANDLE DepthStreamHandle;
        HANDLE nextDepthFrameEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

        HRESULT hr;
       
        InitializeKinect();


        IplImage *Depth = cvCreateImage(cvSize(DEPTH_WIDTH, DEPTH_HEIGHT), IPL_DEPTH_8U, 4);
        cvNamedWindow("Depth Image", CV_WINDOW_AUTOSIZE);

        hr = NuiImageStreamOpen(NUI_IMAGE_TYPE_DEPTH, NUI_IMAGE_RESOLUTION_640x480, 0, 2, nextDepthFrameEvent, &DepthStreamHandle);

        hr = NuiImageStreamSetImageFrameFlags(DepthStreamHandle, NUI_IMAGE_STREAM_FLAG_DISTINCT_OVERFLOW_DEPTH_VALUES);

        if(FAILED(hr))
        {
               cout<<"Could not open Depth Stream"<<endl;
               return hr;
        }

        while(1)
        {
               WaitForSingleObject(nextDepthFrameEvent, 1000);
               createDepthImage(DepthStreamHandle, Depth);

               if(cvWaitKey(10) == 0x001b)
               {
                       break;
               }
        }

        NuiShutdown();
        cvDestroyAllWindows();

        return 0;
}

void InitializeKinect()
{
        bool FailToConnect;

        do
        {
               HRESULT hr = NuiInitialize(NUI_INITIALIZE_FLAG_USES_DEPTH );

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
        BYTE I = 255- (BYTE)(256 * realDepth / (0x0fff));

//      cout<<"Real Depth : "<<realDepth<<endl;
//      cout<<"test Depth : "<<testRealDepth<<endl;

        RGBQUAD q;
        q.rgbRed = q.rgbBlue = q.rgbGreen = ~I;
        return q;
}