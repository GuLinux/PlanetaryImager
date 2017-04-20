// ControlCooler.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "qhyccd.h"
#include "time.h"
#include <conio.h>

int _tmain(int argc, _TCHAR* argv[])
{
	int num = 0;
    qhyccd_handle *camhandle;
    int ret;
    char id[32];
	int found = 0;
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
        ret = SetQHYCCDStreamMode(camhandle,1);
    

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
       
		ret = IsQHYCCDControlAvailable(camhandle,CONTROL_COOLER);
        if(ret == QHYCCD_SUCCESS)
        {
			double min, max, step;
			ret = GetQHYCCDParamMinMaxStep(camhandle, CONTROL_COOLER, &min, &max, &step);
			if(ret == QHYCCD_SUCCESS)
			{
                printf("target temprature range:[%f,%f]\n",min, max);
			}
			ret = GetQHYCCDParamMinMaxStep(camhandle, CONTROL_MANULPWM, &min, &max, &step);
			if(ret == QHYCCD_SUCCESS)
			{
                printf("cooler Power range:[%f,%f]\n\n",min, max);
			}

			double curTemp;
             
#if 1
            //*************control temperature automaticlly.*************/
			double targetTemp = -5.0;

			while(run)
			{
				if(kbhit())
				{
					run = false;
				}

				//get current temperature
				curTemp = GetQHYCCDParam(camhandle,CONTROL_CURTEMP);
				printf("current ccd temprature is:%f degree Celsius\n\n",curTemp);


				//we have to wait one second to loop next control.
				Sleep(1000);
				ret = SetQHYCCDParam(camhandle, CONTROL_COOLER, targetTemp);   
				if(ret == QHYCCD_SUCCESS)
				{
					printf("set target temperature:%f degree Celsius\n",targetTemp);
				}
			}
#else
			/*************Control cooler manual*********/
            // for example, set camera cooler work as power 50
            ret = SetQHYCCDParam(camhandle, CONTROL_MANULPWM, 50);
			if(ret == QHYCCD_SUCCESS)
		    {
                while(run)
			    {
				    if(kbhit())
					{
						run = false;
					}
					//get current temperature
					curTemp = GetQHYCCDParam(camhandle,CONTROL_CURTEMP);
					printf("current ccd temprature is:%f degree Celsius\n\n",curTemp);

					Sleep(1000);
				}
			}
#endif
		} 
		else
		{
			printf("camera do not support cooler\n");
			getchar();
		}
    }
    else
    {
        printf("The camera is not QHYCCD or other error \n");
        goto failure;
    }
    
    if(camhandle)
    {
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

