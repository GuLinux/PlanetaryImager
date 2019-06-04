#include <stdio.h>
#include <libqhy/qhyccd.h>
#include <unistd.h>

int main(void)
{
    char path[1024];
    
    getcwd(path,1024);

    /*
     before you run this sample,please specify the path your firmware folder.the firmware folder must name "firmware".it is hardcode to my sdk.

     ex:
     OSXInitQHYCCDFirmware(./firmware/);
     */

    OSXInitQHYCCDFirmware(path);
}
