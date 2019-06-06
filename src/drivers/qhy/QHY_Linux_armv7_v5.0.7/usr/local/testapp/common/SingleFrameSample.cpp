
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <libqhy/qhyccd.h>

int main(int argc,char *argv[])
{
    int num = 0;
    qhyccd_handle *camhandle = NULL;
    int ret = QHYCCD_ERROR;
    char id[32];
    int found = 0;
    unsigned int w,h,bpp,channels;
    unsigned char *ImgData;
    int camtime = 100000,camgain = 0,camoffset = 140,camspeed = 0,cambinx = 1,cambiny = 1;

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
            printf("Open QHYCCD fail \n");
            goto failure;
        }

        ret = SetQHYCCDStreamMode(camhandle,0);
        if(ret == QHYCCD_SUCCESS)
        {
            printf("SetQHYCCDStreamMode success!\n");
        }
        else
        {
            printf("SetQHYCCDStreamMode code:%d\n",ret);
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
        
        ret = IsQHYCCDControlAvailable(camhandle,CAM_COLOR);
        if(ret == BAYER_GB || ret == BAYER_GR || ret == BAYER_BG || ret == BAYER_RG)
        {
            printf("This is a Color Cam\n");
            printf("even this is a color camera, in Single Frame mode THE SDK ONLY SUPPORT RAW OUTPUT.So please do not set SetQHYCCDDebayerOnOff() to true;");
			
            //SetQHYCCDDebayerOnOff(camhandle,true);
            //SetQHYCCDParam(camhandle,CONTROL_WBR,20);
            //SetQHYCCDParam(camhandle,CONTROL_WBG,20);
            //SetQHYCCDParam(camhandle,CONTROL_WBB,20);
        }        

        ret = IsQHYCCDControlAvailable(camhandle,CONTROL_USBTRAFFIC);
        if(ret == QHYCCD_SUCCESS)
        {
            ret = SetQHYCCDParam(camhandle,CONTROL_USBTRAFFIC,30);
            if(ret != QHYCCD_SUCCESS)
            {
                printf("SetQHYCCDParam CONTROL_USBTRAFFIC failed\n");
                getchar();
                return 1;
            }
        }

        ret = IsQHYCCDControlAvailable(camhandle,CONTROL_GAIN);
        if(ret == QHYCCD_SUCCESS)
        {
            ret = SetQHYCCDParam(camhandle,CONTROL_GAIN,30);
            if(ret != QHYCCD_SUCCESS)
            {
                printf("SetQHYCCDParam CONTROL_GAIN failed\n");
                getchar();
                return 1;
            }
        }

        ret = IsQHYCCDControlAvailable(camhandle,CONTROL_OFFSET);
        if(ret == QHYCCD_SUCCESS)
        {
            ret = SetQHYCCDParam(camhandle,CONTROL_OFFSET,140);
            if(ret != QHYCCD_SUCCESS)
            {
                printf("SetQHYCCDParam CONTROL_GAIN failed\n");
                getchar();
                return 1;
            }
        }

        ret = SetQHYCCDParam(camhandle,CONTROL_EXPOSURE,20000);//170000000);
        if(ret != QHYCCD_SUCCESS)
        {
            printf("SetQHYCCDParam CONTROL_EXPOSURE failed\n");
            getchar();
            return 1;
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
        
        ret = SetQHYCCDBinMode(camhandle,cambinx,cambiny);
        if(ret == QHYCCD_SUCCESS)
        {
            printf("SetQHYCCDBinMode success!\n");
        }
        else
        {
            printf("SetQHYCCDBinMode fail\n");
            goto failure;
        }
        
       ret = IsQHYCCDControlAvailable(camhandle,CONTROL_TRANSFERBIT);
        if(ret == QHYCCD_SUCCESS)
        {
            ret = SetQHYCCDBitsMode(camhandle,16);
            if(ret != QHYCCD_SUCCESS)
            {
                printf("SetQHYCCDParam CONTROL_GAIN failed\n");
                getchar();
                return 1;
            }
        }

        ret = ExpQHYCCDSingleFrame(camhandle);
        if( ret != QHYCCD_ERROR )
        {
            printf("ExpQHYCCDSingleFrame success!\n");
            if( ret != QHYCCD_READ_DIRECTLY )
            {
                sleep(100);
            } 
        }
        else
        {
            printf("ExpQHYCCDSingleFrame fail\n");
            goto failure;
        }
        
        uint32_t length = GetQHYCCDMemLength(camhandle);
        
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

        ret = GetQHYCCDSingleFrame(camhandle,&w,&h,&bpp,&channels,ImgData);
        if(ret == QHYCCD_SUCCESS)
        {
            printf("GetQHYCCDSingleFrame succeess! \n");
            //to do anything you like:
                 
        }
        else
        {
            printf("GetQHYCCDSingleFrame fail:%d\n",ret);
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
            printf("CancelQHYCCDExposingAndReadout fail\n");
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
    return 1;
}
