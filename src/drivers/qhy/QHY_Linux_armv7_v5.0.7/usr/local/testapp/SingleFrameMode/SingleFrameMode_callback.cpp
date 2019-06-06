
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "qhyccd.h"

#define	COMMAND_QUEUE_PUSH		1
#define	COMMAND_QUEUE_POP		2

#define	DEMO_MAXDEVICES			6
#define	MESSAGE_SIZS				1000



typedef struct _CameraParam 
{
    qhyccd_handle *pHandle;
    char CamId[CAMERA_ID_LENGTH];
    bool CamCapable[CONTROL_MAX_ID];
    QHYCamMinMaxStepValue CapParam[CONTROL_MAX_ID];	
}CameraParam;

typedef struct _Camera 
{
    CameraParam CamParam[DEMO_MAXDEVICES];	
    int CamNum;	
}Camera;


typedef struct _CmdQueue 
{
	int push;
	int pop;
	int count;
	int tatol;
	int *Id;
	int *msg;
	
#if defined (_WIN32)
  CRITICAL_SECTION mutex;
#else
  pthread_mutex_t mutex;
#endif
	
}CmdQueue;


class CmdQueue
{
public:
  CmdQueue(int size)
  {
          CQueue = new CmdQueue;
          CQueue->tatol = size;
          CQueue->msg = new int[size];
          CQueue->Id = new int[size];
  		
  #if defined (_WIN32)
      InitializeCriticalSection(&CQueue->mutex);
  #else
      pthread_mutex_init(&CQueue->mutex, NULL);
  #endif
  		
  	return CQueue;
  }  	
  ~CmdQueue()
  {
  #if defined (_WIN32)
      DeleteCriticalSection(&CQueue->mutex);
  #else
      pthread_mutex_destroy(&CQueue->mutex);
  #endif
  	if (NULL != CQueue->msg)
  	{
            delete CQueue->msg;
  	} 
  	if (NULL != CQueue->Id)
  	{
            delete CQueue->Id;
  	}	
  	if (NULL != CQueue)
  	{
            delete CQueue;
  	}
  }  	



unsigned int CmdQueueProc(int *Id,int *msg,int flag)
{
    Lock();
    if (COMMAND_QUEUE_PUSH == flag)
    {
      CQueue->msg[CQueue->push] = *msg;
      CQueue->Id[CQueue->push] = *Id;
      CQueue->push = (CQueue->push + 1)%MESSAGE_SIZS;
      CQueue->count ++;
    }
    else if (COMMAND_QUEUE_POP == flag)
    {
      *msg = CQueue->msg[CQueue->pop];
      *Id = CQueue->Id[CQueue->pop];    
      CQueue->pop = (CQueue->pop + 1)%MESSAGE_SIZS;
      CQueue->count --;
    }
	
    Unlock();
    return QHYCCD_SUCCESS;
}

unsigned int CmdQueuePush(int *Id,int *msg)
{
	return CmdQueueProc(CQueue,COMMAND_QUEUE_PUSH);		
}

unsigned int CmdQueuePop(int *Id,int *msg)
{
       if (CQueue->count > 0)
       	{
	   return CmdQueueProc(CQueue,COMMAND_QUEUE_POP);		
       	}

       return QHYCCD_ERROR;
}

private:
  void Lock()
  {
#if defined (_WIN32)
    EnterCriticalSection(&CQueue->mutex);
#else
    pthread_mutex_lock(&CQueue->mutex);
#endif
  }

  void Unlock()
  {
#if defined (_WIN32)
    LeaveCriticalSection(&CQueue->mutex);
#else
    pthread_mutex_unlock(&CQueue->mutex);
#endif
  }
	
  CmdQueue *CQueue;

};



void SDKVersion()
{
  unsigned int  YMDS[4];
  unsigned char sVersion[80];

  memset ((char *)sVersion,0x00,sizeof(sVersion));
  GetQHYCCDSDKVersion(&YMDS[0],&YMDS[1],&YMDS[2],&YMDS[3]);

  if ((YMDS[1] < 10)&&(YMDS[2] < 10))
  {
    sprintf((char *)sVersion,"V20%d0%d0%d_%d\n",YMDS[0],YMDS[1],YMDS[2],YMDS[3]	);
  }
  else if ((YMDS[1] < 10)&&(YMDS[2] > 10))
  {
    sprintf((char *)sVersion,"V20%d0%d%d_%d\n",YMDS[0],YMDS[1],YMDS[2],YMDS[3]	);
  }
  else if ((YMDS[1] > 10)&&(YMDS[2] < 10))
  {
    sprintf((char *)sVersion,"V20%d%d0%d_%d\n",YMDS[0],YMDS[1],YMDS[2],YMDS[3]	);
  }
  else
  {
    sprintf((char *)sVersion,"V20%d%d%d_%d\n",YMDS[0],YMDS[1],YMDS[2],YMDS[3]	);
  }

  fprintf(stderr,"QHYCCD SDK Version: %s\n", sVersion);
}


void FirmWareVersion(qhyccd_handle *h)
{
  int i = 0;
  unsigned char fwv[32],FWInfo[256];
  unsigned int ret;
  memset (FWInfo,0x00,sizeof(FWInfo));
  ret = GetQHYCCDFWVersion(h,fwv);
  if(ret == QHYCCD_SUCCESS)
  {
    if((fwv[0] >> 4) <= 9)
    {

      sprintf((char *)FWInfo,"Firmware version:20%d_%d_%d\n",((fwv[0] >> 4) + 0x10),
              (fwv[0]&~0xf0),fwv[1]);

    }
    else
    {

      sprintf((char *)FWInfo,"Firmware version:20%d_%d_%d\n",(fwv[0] >> 4),
              (fwv[0]&~0xf0),fwv[1]);

    }
  }
  else
  {
    sprintf((char *)FWInfo,"Firmware version:Not Found!\n");
  }
  fprintf(stderr,"%s\n", FWInfo);

}


Camera gCamera;
CmdQueue *gCmdQueue = new CmdQueue(MESSAGE_SIZS);


static unsigned int   QHYCCDProcCallBack(void  *pHandle, unsigned int message, 
	unsigned int wParam, unsigned int lParam) 
{
    int i;
    qhyccd_handle  *handle = NULL;	
    if (pHandle != NULL)
    {
       handle = (qhyccd_handle  *)pHandle;
    }
    switch (message) 
    {
        case SEND_CAMERA_ID: 
	 {
           char *p = (char *)wParam;
           int Id,msg;		   
           gCamera.CamNum = lParam;
           for (i = 0;i < lParam;i ++)
            {
                memcpy(gCamera.CamParam[i].CamId,p,CAMERA_ID_LENGTH);
		 p += CAMERA_ID_LENGTH;				
            }
            		
            msg = SEND_CAMERA_ID;
            Id = -1;
            gCmdQueue->CmdQueuePush(&Id,&msg);			
            return QHYCCD_SUCCESS;
        }
        case SEND_CAMERA_CAPABLE: 
	 {
            bool *p = (bool *)wParam;	
            i = qhyccd_handle2index(handle);	
            if (i >= 0)
            {
            }
            return QHYCCD_SUCCESS;
        }	
        case SEND_CAMERA_MAXMINSTEP: 
	 {
            return QHYCCD_SUCCESS;
        }		
    }

    return QHYCCD_ERROR;
	
}

int qhyccd_handle2index(qhyccd_handle *h)
{
  int index = -1;
  int i;
  for (i = 0; i < DEMO_MAXDEVICES; ++i)
  {
    if (h == gCamera.CamParam[i].pHandle)
    {
      index = i;
      break;
    }
  }
  return index;
}

void Init()
{
    int i,j;
    gCamera.CamNum = -1;

    for (i = 0;i < DEMO_MAXDEVICES ; i ++)
    {
        gCamera.CamParam[i].pHandle = NULL;
        memset(gCamera.CamParam[i].CamId,0x00,CAMERA_ID_LENGTH);
        for (j = 0;j < DEMO_MAXDEVICES ; j ++)	
        {
           gCamera.CamParam[i].CamCapable[j] = false;
           gCamera.CamParam[i].CapParam[j].name = NULL;
           gCamera.CamParam[i].CapParam[j].max = 0.0;
           gCamera.CamParam[i].CapParam[j].min = 0.0;
           gCamera.CamParam[i].CapParam[j].step = 0.0;
        }
    }

}

int main(int argc, char *argv[])
{
  int Id, msg;
  int i;
  unsigned int ret;

  Init();
  SDKVersion();

  // init SDK
  unsigned int retVal = InitQHYCCDResource();
  if (QHYCCD_SUCCESS == retVal)
  {
    printf("SDK resources initialized.\n");
  }
  else
  {
    printf("Cannot initialize SDK resources, error: %d\n", retVal);
    return 1;
  }

  // scan cameras
  gCamera.CamNum = ScanQHYCCD();
  if (gCamera.CamNum > 0)
  {
    printf("Number of QHYCCD cameras found: %d \n", camCount);
  }
  else
  {
    printf("No QHYCCD camera found, please check USB or power.\n");
  }


  while(true)
  {
     #if defined (_WIN32)
       Sleep(500);
     #else
       usleep(500000);
     #endif     
     Id = -1;
     msg = -1;
     if (gCamera.CamNum <= 0)
     {
        gCamera.CamNum = ScanQHYCCD();
	 continue;
     }
     if (QHYCCD_ERROR == gCmdQueue->CmdQueuePop(&Id, &msg))
     {
	 continue;
     }  
	 
     switch (msg) 
     {
         case SEND_CAMERA_ID: 
 	 {
	     for (i = 0;i < gCamera.CamNum;i ++)
	     {
                gCamera.CamParam[i].pHandle = 
					OpenQHYCCD(gCamera.CamParam[i].CamId);
                if(gCamera.CamParam[i].pHandle != NULL)
                {
                  printf("Open QHYCCD success!\n");
                }
                else
                {
                  printf("Open QHYCCD failed \n");
                }	

                ret = SetQHYCCDStreamMode(gCamera.CamParam[i].pHandle,SINGLE_MODE);				
                if(QHYCCD_SUCCESS == ret)
                {
                  printf("Set QHYCCD Single stream mode success!\n");
                }
                else
                {
                  printf("Set QHYCCD Single stream mode failed \n");
                }	
	     }
            break;
         }
         case SEND_CAMERA_CAPABLE: 
 	 {
            break;
         }	
         case SEND_CAMERA_MAXMINSTEP: 
 	 {
            break;
         }		
     }     
  }
  // release sdk resources
  retVal = ReleaseQHYCCDResource();
  if (QHYCCD_SUCCESS == retVal)
  {
    printf("SDK resources released.\n");
  }
  else
  {
    printf("Cannot release SDK resources, error %d.\n", retVal);
    return 1;
  }

  return 0;
}
