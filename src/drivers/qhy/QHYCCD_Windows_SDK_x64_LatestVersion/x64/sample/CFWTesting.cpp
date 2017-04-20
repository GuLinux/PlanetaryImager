// CFWTesting.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "qhyccd.h"
#include "time.h"

int _tmain(int argc, _TCHAR* argv[])
{
	int num = 0;
    qhyccd_handle *camhandle;
    int ret = QHYCCD_ERROR;
    char id[32];
    int found = 0;

    double min, max, step;
	int cfwSlots = 0;
	char ch = '0';
	int checktimes = 0;
	char currentpos[64];

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
        
		ret = IsQHYCCDControlAvailable(camhandle,CONTROL_CFWPORT);
        if(ret == QHYCCD_SUCCESS)
        {
			printf("Camera has CFW Port\n");

			ret = IsQHYCCDCFWPlugged(camhandle);
			if(ret == QHYCCD_SUCCESS)
			{
                ret = GetQHYCCDParam(camhandle, CONTROL_CFWSLOTSNUM);
                if(ret == QHYCCD_ERROR)
				{
					printf("GetQHYCCDParam CONTROL_CFWSLOTSNUM Failed!\n");
                    goto failure;
				}
				else
				{
                    cfwSlots = ret; 

					while(ch != 'e' && ch != 'E')
					{
						printf("Please enter the command you want to control:\n");
						printf("There is %d hole CFW, Choice is 0 - %d\n", cfwSlots, cfwSlots-1);
						printf("If you want to exit,enter e or E\n");
						scanf("%c",&ch);
                        getchar();

						if(ch >= '0' && ch <= (cfwSlots-1 + 0x30))
						{
							ret = IsQHYCCDCFWPlugged(camhandle);
							if(ret != QHYCCD_SUCCESS)
							{
								printf("CFW is Unplugged, please check the cable and try again.\n\n");
							}
							else
							{
								ret = SendOrder2QHYCCDCFW(camhandle, &ch, 1);
								if(ret != QHYCCD_SUCCESS)
								{
									printf("Control the color filter wheel failure \n");
									goto failure;
								}
								else
								{
									checktimes = 0;

									while(checktimes < 90)
									{
										ret = GetQHYCCDCFWStatus(camhandle,currentpos);
										if(ret == QHYCCD_SUCCESS)
										{
											printf("currentpos = %c ",currentpos[0]);
											if(ch == currentpos[0])
											{
												printf(" yes we got the right return\n\n");
												break;
											}
										}
										checktimes++;
										Sleep(100);
									}
								}
							}
						}
						else
						{
                             printf("out of range - please chose 0-%d \n\n", cfwSlots-1);
						}
					}
				}
			}
			else
			{
				printf("no CFW plug in to the port \n");
				getchar();
			}			
		}
		else
		{
			printf("Camera do not have CFW Port\n");
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

