// SingleFrame.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "qhyccd.h"
#include <conio.h>

int _tmain(int argc, _TCHAR* argv[])
{
    int num = 0;
    qhyccd_handle *camhandle;
    int ret;
    char id[32];
    char camtype[16];
    int found = 0;
    unsigned int w,h,bpp,channels;
    unsigned char *ImgData;
    int camtime = 50000,camgain = 0,camspeed = 0,cambinx = 1,cambiny = 1;
	unsigned int cambits = 8, usbtraffic = 30;
	bool run = true;

    ret = InitQHYCCDResource();
    if(ret == QHYCCD_SUCCESS)
    {
        printf("Init SDK success!\n");
    }
    else
    {
        goto failure;
    }
    num = ScanQHYCCD();
    if(num > 0)
    {
        printf("Yes!Found QHYCCD,the num is %d \n",num);
    }
    else
    {
        printf("Not Found QHYCCD,please check the usblink or the power\n");
        goto failure;
    }

    for(int i = 0;i < num;i++)
    {
        ret = GetQHYCCDId(i,id);
        if(ret == QHYCCD_SUCCESS)
        {
            printf("connected to the first camera from the list,id is %s\n",id);
            found = 1;
            break;
        }
    }

    if(found == 1)
    {
        camhandle = OpenQHYCCD(id);
        if(camhandle != NULL)
        {
            printf("Open QHYCCD success!\n");
        }
        else
        {
            printf("Open QHYCCD failed \n");
            goto failure;
        }

		ret = SetQHYCCDStreamMode(camhandle, 0);  //set stream mode ,let camera work as single frame mode
		if(ret == QHYCCD_SUCCESS)
		{
			printf("SetQHYCCDStreamMode success!\n");
		}
		else
		{
			goto failure;
		}
    

        ret = InitQHYCCD(camhandle);
        if(ret == QHYCCD_SUCCESS)
        {
            printf("Init QHYCCD success!\n");
        }
        else
        {
            printf("Init QHYCCD fail code:%d\n",ret);
            goto failure;
        }
        
        double chipw,chiph,pixelw,pixelh;
        ret = GetQHYCCDChipInfo(camhandle,&chipw,&chiph,&w,&h,&pixelw,&pixelh,&bpp);
        if(ret == QHYCCD_SUCCESS)
        {
            printf("GetQHYCCDChipInfo success!\n");
            printf("CCD/CMOS chip information:\n");
            printf("Chip width %3f mm,Chip height %3f mm\n",chipw,chiph);
            printf("Chip pixel width %3f um,Chip pixel height %3f um\n",pixelw,pixelh);
            printf("Chip Max Resolution is %d x %d,depth is %d\n",w,h,bpp);
        }
        else
        {
            printf("GetQHYCCDChipInfo fail\n");
            goto failure;
        }
        
        ret = IsQHYCCDControlAvailable(camhandle,CONTROL_TRANSFERBIT);
        if(ret == QHYCCD_SUCCESS)
        {
            ret = SetQHYCCDBitsMode(camhandle, cambits);
            if(ret != QHYCCD_SUCCESS)
            {
                printf("SetQHYCCDBitsMode failed\n");
                goto failure;
            }
			else
                printf("SetQHYCCDBitsMode %d \n", cambits);
        }    

		ret = IsQHYCCDControlAvailable(camhandle,CONTROL_USBTRAFFIC);
        if(ret == QHYCCD_SUCCESS)
        {
            ret = SetQHYCCDParam(camhandle, CONTROL_USBTRAFFIC, usbtraffic);
            if(ret != QHYCCD_SUCCESS)
            {
                printf("SetQHYCCDParam CONTROL_USBTRAFFIC failed\n");
                goto failure;
            }
			else
                printf("SetQHYCCDParam CONTROL_USBTRAFFIC %d \n", usbtraffic);
        }

		ret = IsQHYCCDControlAvailable(camhandle,CONTROL_SPEED);
        if(ret == QHYCCD_SUCCESS)
        {
            ret = SetQHYCCDParam(camhandle, CONTROL_SPEED, camspeed);
            if(ret != QHYCCD_SUCCESS)
            {
                printf("SetQHYCCDParam CONTROL_SPEED failed\n");
                goto failure;
            }
			else
                printf("SetQHYCCDParam CONTROL_SPEED %d \n", camspeed);
        }

		ret = IsQHYCCDControlAvailable(camhandle,CONTROL_EXPOSURE);
        if(ret == QHYCCD_SUCCESS)
        {
            ret = SetQHYCCDParam(camhandle, CONTROL_EXPOSURE, camtime);
            if(ret != QHYCCD_SUCCESS)
            {
                printf("SetQHYCCDParam CONTROL_EXPOSURE failed\n");
                goto failure;
            }
			else
				printf("SetQHYCCDParam CONTROL_EXPOSURE %d \n", camtime);
        }

		ret = IsQHYCCDControlAvailable(camhandle,CONTROL_GAIN);
        if(ret == QHYCCD_SUCCESS)
        {
            ret = SetQHYCCDParam(camhandle, CONTROL_GAIN, camgain);
            if(ret != QHYCCD_SUCCESS)
            {
                printf("SetQHYCCDParam CONTROL_GAIN failed\n");
                goto failure;
            }
			else
				printf("SetQHYCCDParam CONTROL_GAIN %d \n", camgain);
        }

        ret = SetQHYCCDResolution(camhandle,0,0,w,h);
        if(ret == QHYCCD_SUCCESS)
        {
            printf("SetQHYCCDResolution success!\n");
        }
        else
        {
            printf("SetQHYCCDResolution fail\n");
            goto failure;
        }

        int length = GetQHYCCDMemLength(camhandle);
        
        if(length > 0)
        {
            ImgData = (unsigned char *)malloc(length);
            memset(ImgData,0,length);
        }
        else
        {
            printf("Get the min memory space length failure \n");
            goto failure;
        }

		while(run)
        {
			if (kbhit())
			{
				run = false;  // quit while
			}

			ret = ExpQHYCCDSingleFrame(camhandle);       //expose one frame.
			if(ret == QHYCCD_READ_DIRECTLY)
			{
				printf("ExpQHYCCDSingleFrame QHYCCD_READ_DIRECTLY!\n");
			}
			else if(ret == QHYCCD_SUCCESS)
			{
				printf("ExpQHYCCDSingleFrame, wait...\n");
				Sleep(200);
			}
			else
			{
				printf("ExpQHYCCDSingleFrame failure!\n");
				goto failure;
			}


            ret = GetQHYCCDSingleFrame(camhandle,&w,&h,&bpp,&channels,ImgData);   //get one frame
            if(ret == QHYCCD_SUCCESS)
            {
				printf("GetQHYCCDSingleFrame success\n");
                //get one frame success...
				//to do as you want...
            }

        }
        delete(ImgData); 
	}
    else
    {
        printf("The camera is not QHYCCD or other error \n");
        goto failure;
    }

	if(camhandle)
    {
        ret = CancelQHYCCDExposingAndReadout(camhandle);
		if(ret == QHYCCD_SUCCESS)
		{
			printf("CancelQHYCCDExposingAndReadout success!\n");
		}
		else
		{
			goto failure;
		}

        ret = CloseQHYCCD(camhandle);
        if(ret == QHYCCD_SUCCESS)
        {
            printf("Close QHYCCD success!\n");
        }
        else
        {
            goto failure;
        }
    }

    ret = ReleaseQHYCCDResource();
    if(ret == QHYCCD_SUCCESS)
    {
        printf("Rlease SDK Resource  success!\n");
    }
    else
    {
        goto failure;
    }

    return 0;

failure:
    printf("some fatal error happened\n");
	getchar();
    return 1;
}

