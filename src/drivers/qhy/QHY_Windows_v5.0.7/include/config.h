#ifndef __CONFIG_H__
#define __CONFIG_H__

/*
0, 5, 0, 7
QHYCCD  (Beijing) Technology Co., Ltd.

system_profiler SPUSBDataType

/lib/firmware/qhy
*/


//Module compilation switch part
#undef 	GIGAESUPPORT
#undef 	AuxImage
#undef 	WIN_98_DDK





#if defined (_WIN32)
#define QHYCCD_OPENCV_SUPPORT
#define CALLBACK_MODE_SUPPORT	0
#define WINDOWS_PTHREAD_SUPPORT	1
#define CYUSB_MODE_SUPPORT  	1
#define WINUSB_MODE_SUPPORT  	1
#else
#undef  QHYCCD_OPENCV_SUPPORT
#define CALLBACK_MODE_SUPPORT	0
#define WINDOWS_PTHREAD_SUPPORT	0
#define LIBUSB_MODE_SUPPORT  	1
#endif



#endif

