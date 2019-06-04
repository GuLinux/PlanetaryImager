#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <libqhy/qhyccd.h>


int main(void)
{
    int num = 0;
    qhyccd_handle *camhandle;
    int ret;
    char id[32];
    char camtype[16];
    int found = 0;
    char ch = '0';

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
       
        InitQHYCCD(camhandle);

        while(1)
        {
            double tmp = GetQHYCCDParam(camhandle,CONTROL_CURTEMP);
            printf("current ccd temprature is:%f degree Celsius\n",tmp);
         

            //we have to wait one second to loop next control.
            sleep(1);

            ControlQHYCCDTemp(camhandle,-1.0);
        }

        //Stop cool
        SetQHYCCDParam(camhandle,CONTROL_MANULPWM,0);

    }
    else
    {
        printf("The camera is not QHYCCD or other error \n");
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
