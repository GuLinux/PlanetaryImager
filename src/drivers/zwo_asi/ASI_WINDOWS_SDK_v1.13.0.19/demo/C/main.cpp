#include "stdio.h"
#include "../OpenCV/include/highgui.h"
#include "../../include/asicamera.h"

#pragma comment(lib,"../OpenCV/lib/highgui.lib") 
#pragma comment(lib,"../OpenCV/lib/cxcore.lib") 
//#pragma comment(lib,"../../lib/ASICamera.lib") 


#define  MAX_CONTROL 7


void cvText(IplImage* img, const char* text, int x, int y)
{
	CvFont font;

	double hscale = 0.6;
	double vscale = 0.6;
	int linewidth = 2;
	cvInitFont(&font,CV_FONT_HERSHEY_SIMPLEX | CV_FONT_ITALIC,hscale,vscale,0,linewidth);

	CvScalar textColor =cvScalar(0,255,255);
	CvPoint textPos =cvPoint(x, y);

	cvPutText(img, text, textPos, &font,textColor);
}

void main()
{
	int width;
	char* bayer[] = {"RG","BG","GR","GB"};
	char* controls[MAX_CONTROL] = {"Exposure", "Gain", "Gamma", "WB_R", "WB_B", "Brightness", "USB Traffic"};

	int height;
	int i;
	bool bresult;

	int time1,time2;
	int count=0;

	char buf[128]={0};

	int CamNum=0;
	
	///long exposure, exp_min, exp_max, exp_step, exp_flag, exp_default;
	//long gain, gain_min, gain_max,gain_step, gain_flag, gain_default;

	IplImage *pRgb;


	int numDevices = getNumberOfConnectedCameras();





	if(numDevices < 0)
	{
		printf("no camera connected, press any key to exit\n");
		getchar();
		return;
	}
	else
		printf("attached cameras:\n");

	for(i = 0; i < numDevices; i++)
		printf("%d %s\n",i, getCameraModel(i));

	printf("\nselect one to privew\n");
	scanf("%d", &CamNum);

/*/->
	unsigned int cameraType;
	const char* currName;
	for ( i = 0; i < numDevices; i++ ) 
	{
		if (!( currName = getCameraModel ( i )))
		{    
			fprintf ( stderr, "ZW name[%d] = 0\n", i );
		} 
		else {
			cameraType = getCameraType ( i );
		}
		printf ( "ZW name[%d] = %s, type = %d\n", i, currName, cameraType );
	}
	//<-*/

	bresult = openCamera(CamNum);
	
	if(!bresult)
	{
		printf("OpenCamera error,check input,press any key to exit\n");
		getchar();
		return;
	}
	initCamera(); //Init camera after open

	printf("%s information\n",getCameraModel(CamNum));
	printf("resolution:%dX%d\n", getMaxWidth(),getMaxHeight());
	if(isColorCam())
		printf("Color Camera: bayer pattern:%s\n",bayer[getColorBayer()]);
	else
		printf("Mono camera\n");
	printf("sensor temperature:%02f\n", getSensorTemp());
	for( i = 0; i < MAX_CONTROL; i++)
	{
			if(isAvailable((Control_TYPE)i))
				printf("%s support:Yes\n", controls[i]);
			else
				printf("%s support:No\n", controls[i]);
	}


	printf("\nPlease input the width and height with one space，ie. 640 480\n");
	scanf("%d %d", &width, &height);

	while(!setImageFormat(width, height, 1, IMG_Y8))
	{
		printf("Set format error, please check the width and height\n ASI120's data size(width*height) must be integer multiple of 1024\n");
		printf("Please input the width and height again，ie. 640 480\n");
		scanf("%d %d", &width, &height);
	}
	printf("\nset image format success, start privew, press ESC to stop \n");
	cvNamedWindow("video", 1);

	pRgb=cvCreateImage(cvSize(getWidth(),getHeight()), IPL_DEPTH_8U, 1);

	setValue(CONTROL_EXPOSURE, 33*1000, true); //auto exposure

	startCapture(); //start privew

	time1 = GetTickCount();

	while(1)
	{

		getImageData((BYTE*)pRgb->imageData, pRgb->imageSize, -1);

		count++;
		time2 = GetTickCount();
		if(time2-time1 > 1000 )
		{
			sprintf_s(buf, "fps:%d dropped frames:%d",count, getDroppedFrames());

			count = 0;
			time1=GetTickCount();


		}
		cvText(pRgb, buf, 40,40 );

		//cvFlip(pRgb,NULL,1);//加上这句就水平翻转画面


		char c=cvWaitKey(1);
		switch(c)
		{
		case 27://按ESC退出
			goto END;
		}

		cvShowImage("video", pRgb);

	

	}
END:
	stopCapture();
	printf("over\n");
}






