#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "qhyccd.h"

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



int main(int argc,char *argv[])
{
  int ret = QHYCCD_ERROR;
  char path[] = "/lib/firmware/qhy";

  if ((argc < 2)||((strcmp(argv[1],"file"))&&(strcmp(argv[1],"memory"))))
  {
    printf("usage: %s  memory|file\n",argv[0]);
    return -1;
  }

  SDKVersion();

  ret = InitQHYCCDResource();
  if(ret == QHYCCD_SUCCESS)
  {
    printf("Init SDK success!\n");
  }
  else
  {
    printf("Init SDK failed!\n");
  }

  if (!strcmp(argv[1],"file"))
  {
    ret = OSXInitQHYCCDFirmware(path);
  }
  else if (!strcmp(argv[1],"memory"))
  {
    ret = OSXInitQHYCCDFirmwareArray();
  }

  if(ret == QHYCCD_SUCCESS)
  {
    printf("Download firmware successed!\n");
  }
  else
  {
    printf("Download firmware failed!\n");
  }

  return 0;
}
