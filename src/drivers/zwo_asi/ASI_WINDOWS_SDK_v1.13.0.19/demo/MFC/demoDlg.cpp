
// demoDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "demo.h"
#include "demoDlg.h"
#include <process.h>
//#include <vld.h>


#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define  UPDATE_MESSAGE 2010 //接受消息用的消息值
// 用于应用程序“关于”菜单项的 CAboutDlg 对话框

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// 对话框数据
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

// 实现
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
END_MESSAGE_MAP()


// CdemoDlg 对话框




CdemoDlg::CdemoDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CdemoDlg::IDD, pParent)
	, m_iWidth(0)
	, m_iHeight(0)
	, m_iStartX(0)
	, m_iStartY(0)
	, m_iROIWidth(0)
	, m_iROIHeight(0)
	, m_ctrlMin(0)
	, m_ctrlMax(0)
	, m_static_SliderPos(0)
	, m_fTemp(0)
	, m_ST4time(0)
	, m_strCameraType(_T(""))
	, m_bFlipY(FALSE)
	, m_bFlipX(FALSE)
	, m_static_SnapTime(_T(""))
	, m_edit_ctrl_value(_T(""))
	, m_check_SubDark(FALSE)
	, m_edit_id_in_flash(_T(""))
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	ConnectCamera.pRgb = NULL;
}

void CdemoDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_COMBO_CAMERAS, CComboCamera);
	DDX_Text(pDX, IDC_STATIC_WIDTH, m_iWidth);
	DDX_Text(pDX, IDC_STATIC_HEIGHT, m_iHeight);
	DDX_Text(pDX, IDC_EDIT_STARTX, m_iStartX);
	DDX_Text(pDX, IDC_EDIT_STARTY, m_iStartY);
	DDX_Text(pDX, IDC_EDIT_WIDTH, m_iROIWidth);
	DDX_Text(pDX, IDC_EDIT_HEIGHT, m_iROIHeight);
	DDX_Control(pDX, IDC_COMBO_IMGTYPE, m_ComboImgType);
	DDX_Control(pDX, IDC_COMBO_BIN, m_ComboBin);
	DDX_Control(pDX, IDC_COMBO_CTRL, m_comboCtrl);
	DDX_Text(pDX, IDC_STATIC_MIN, m_ctrlMin);
	DDX_Text(pDX, IDC_STATIC_MAX, m_ctrlMax);
	DDX_Control(pDX, IDC_SLIDER_CTRL, m_SliderCtrl);
	DDX_Text(pDX, IDC_STATIC_POS, m_static_SliderPos);
	DDX_Control(pDX, IDC_BUTTON_CLOSE, m_butClose);
	DDX_Control(pDX, IDC_BUTTON_OPEN, m_butOpen);
	DDX_Control(pDX, IDC_BUTTON_stop, m_butStop);
	DDX_Control(pDX, IDC_BUTTON_Start, m_butStart);
	DDX_Text(pDX, IDC_STATIC_TEMP, m_fTemp);
	DDX_Control(pDX, IDC_BUTTON_SETFORMAT, m_btnSetFormat);
	DDX_Control(pDX, IDC_BUTTON_STARTPOS, m_btnSetPostion);
	DDX_Text(pDX, IDC_EDIT_DELAY, m_ST4time);
	DDX_Control(pDX, IDC_CHECK_AUTO, m_ButAuto);
	DDX_Text(pDX, IDC_STATIC_CAMERATYPE, m_strCameraType);
	DDX_Check(pDX, IDC_CHECK_FLIPY, m_bFlipY);
	DDX_Check(pDX, IDC_CHECK_FLIPX, m_bFlipX);
	DDX_Control(pDX, IDC_BUTTON_NORTH, m_btNorth);
	DDX_Control(pDX, IDC_BUTTON_EAST, m_btEast);
	DDX_Control(pDX, IDC_BUTTON_SOUTH, m_btSouth);
	DDX_Control(pDX, IDC_BUTTON_West, m_btWest);
	DDX_Control(pDX, IDC_BUTTON_AUTOTEST, m_btAutoTest);
	DDX_Control(pDX, IDC_BUTTON_SNAP, m_btn_snap);
	DDX_Control(pDX, IDC_BUTTON_ABORTSNAP, m_btn_AbortSnap);
	DDX_Text(pDX, IDC_STATIC_SNAPTIME, m_static_SnapTime);
	DDX_Text(pDX, IDC_EDIT_CTRL_VALUE, m_edit_ctrl_value);
	DDX_Check(pDX, IDC_CHECK_SUBDARK, m_check_SubDark);
	DDX_Text(pDX, IDC_EDIT_ID_IN_FLASH, m_edit_id_in_flash);
	DDX_Control(pDX, IDC_BUTTON_GET_ID, m_btn_get_id);
	DDX_Control(pDX, IDC_BUTTON_WRITE_ID, m_btn_set_id);
}

BEGIN_MESSAGE_MAP(CdemoDlg, CDialog)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDC_BUTTON_SCAN, &CdemoDlg::OnBnClickedButtonScan)
	ON_CBN_SELCHANGE(IDC_COMBO_CAMERAS, &CdemoDlg::OnCbnSelchangeComboCameras)
	ON_BN_CLICKED(IDC_BUTTON_Start, &CdemoDlg::OnBnClickedButtonStart)
	ON_BN_CLICKED(IDC_BUTTON_stop, &CdemoDlg::OnBnClickedButtonstop)
	ON_BN_CLICKED(IDC_BUTTON_CLOSE, &CdemoDlg::OnBnClickedButtonClose)
	ON_CBN_SELCHANGE(IDC_COMBO_CTRL, &CdemoDlg::OnCbnSelchangeComboCtrl)
	ON_NOTIFY(NM_RELEASEDCAPTURE, IDC_SLIDER_CTRL, &CdemoDlg::OnNMReleasedcaptureSliderCtrl)
	ON_BN_CLICKED(IDC_BUTTON_STARTPOS, &CdemoDlg::OnBnClickedButtonStartpos)
	ON_BN_CLICKED(IDC_BUTTON_NORTH, &CdemoDlg::OnBnClickedButtonNorth)
	ON_BN_CLICKED(IDC_BUTTON_SOUTH, &CdemoDlg::OnBnClickedButtonSouth)
	ON_BN_CLICKED(IDC_BUTTON_West, &CdemoDlg::OnBnClickedButtonWest)
	ON_BN_CLICKED(IDC_BUTTON_EAST, &CdemoDlg::OnBnClickedButtonEast)
	ON_BN_CLICKED(IDC_CHECK_AUTO, &CdemoDlg::OnBnClickedCheckAuto)
	ON_BN_CLICKED(IDC_CHECK_FLIPX, &CdemoDlg::OnBnClickedCheckFlipx)
	ON_BN_CLICKED(IDC_CHECK_FLIPY, &CdemoDlg::OnBnClickedCheckFlipy)
	ON_BN_CLICKED(IDC_BUTTON_AUTOTEST, &CdemoDlg::OnBnClickedButtonAutotest)
	ON_BN_CLICKED(IDC_BUTTON_OPEN, &CdemoDlg::OnBnClickedButtonOpen)
	ON_BN_CLICKED(IDC_BUTTON_SETFORMAT, &CdemoDlg::OnBnClickedButtonSetformat)
	ON_BN_CLICKED(IDC_CHECK_SUBDARK, &CdemoDlg::OnBnClickedCheckSubDark)
	ON_WM_CREATE()
	ON_WM_TIMER()
	ON_BN_CLICKED(IDC_BUTTON_SNAP, &CdemoDlg::OnBnClickedButtonSnap)
	ON_BN_CLICKED(IDC_BUTTON_ABORTSNAP, &CdemoDlg::OnBnClickedButtonAbortsnap)
	ON_BN_CLICKED(IDC_BUTTON_GET_ID, &CdemoDlg::OnBnClickedButtonGetId)
	ON_BN_CLICKED(IDC_BUTTON_WRITE_ID, &CdemoDlg::OnBnClickedButtonWriteId)
	ON_WM_CLOSE()
END_MESSAGE_MAP()


// CdemoDlg 消息处理程序

BOOL CdemoDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// 将“关于...”菜单项添加到系统菜单中。

	// IDM_ABOUTBOX 必须在系统命令范围内。
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// 设置此对话框的图标。当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	// TODO: 在此添加额外的初始化代码

	OnBnClickedButtonScan();
	m_butClose.EnableWindow(FALSE);
	m_butStart.EnableWindow(FALSE);
	m_butStop.EnableWindow(FALSE);
	m_btnSetFormat.EnableWindow(FALSE);
	m_btnSetPostion.EnableWindow(FALSE);

	m_btNorth.EnableWindow(FALSE);
	m_btWest.EnableWindow(FALSE);
	m_btEast.EnableWindow(FALSE);
	m_btSouth.EnableWindow(FALSE);
	m_btAutoTest.EnableWindow(FALSE);

	m_ST4time = 100;

	m_ButAuto.EnableWindow(FALSE);


	m_btn_snap.EnableWindow(FALSE);
	m_btn_AbortSnap.EnableWindow(FALSE);

	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

void CdemoDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialog::OnSysCommand(nID, lParam);
	}
}

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CdemoDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 用于绘制的设备上下文

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 使图标在工作区矩形中居中
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 绘制图标
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

//当用户拖动最小化窗口时系统调用此函数取得光标
//显示。
HCURSOR CdemoDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}


void CdemoDlg::OnBnClickedButtonScan()
{

	// TODO: 在此添加控件通知处理程序代码
	CComboCamera.ResetContent();

	int cameras = getNumberOfConnectedCameras();
	for(int i = 0; i < cameras; i++)
	{
		CComboCamera.AddString(getCameraModel(i));
				
	}

	CComboCamera.SetCurSel(0);
}

void CdemoDlg::OnCbnSelchangeComboCameras()
{
	// TODO: 在此添加控件通知处理程序代码
	
}

void CdemoDlg::OnBnClickedButtonOpen()
{
	// TODO: 在此添加控件通知处理程序代码
	int i=0;
	i = CComboCamera.GetCurSel();
	if(i == -1)
		return;
	if(!openCamera(i))
	{
		MessageBox("reopen or open fail");
		return;
	}
	ConnectCamera.Status = opened;

	initCamera(); // need to init before operate
	m_iHeight = getMaxHeight();
	m_iWidth = getMaxWidth();
	m_iStartY = 0;
	m_iStartX = 0;
	m_iROIHeight = m_iHeight;
	m_iROIWidth = m_iWidth;
	m_ComboImgType.Clear();
	
	int count = 0;
	if(isImgTypeSupported(IMG_RAW8))
	{
		m_ComboImgType.AddString("RAW8");
		m_ImgTypeArray[count++] = IMG_RAW8;
	}
	if(isImgTypeSupported(IMG_RGB24))
	{
		m_ComboImgType.AddString("RGB24");
		m_ImgTypeArray[count++] = IMG_RGB24;
	}
	if(isImgTypeSupported(IMG_RAW16))
	{
		m_ComboImgType.AddString("RAW16");
		m_ImgTypeArray[count++] = IMG_RAW16;
	}
	if(isImgTypeSupported(IMG_Y8))
	{
		m_ComboImgType.AddString("Y8");
		m_ImgTypeArray[count++] = IMG_Y8;
	}
	m_ComboImgType.SetCurSel(0);

	count = 0;
	m_ComboBin.Clear();
	m_ComboBin.AddString("1");
	m_BinArray[count++] = 1;
	if(isBinSupported(2))
	{
		m_ComboBin.AddString("2");
		m_BinArray[count++] = 2;
	}
	if(isBinSupported(3))
	{
		m_ComboBin.AddString("3");
		m_BinArray[count++] = 3;
	}
	m_ComboBin.SetCurSel(0);

	int ctrl=0;
	if(isAvailable(CONTROL_GAIN))
	{
		m_comboCtrl.AddString("Gain");
		m_CtrlArray[ctrl++] = CONTROL_GAIN;
		
	}

	if(isAvailable(CONTROL_EXPOSURE))
	{
		m_comboCtrl.AddString("Exposure");
		m_CtrlArray[ctrl++] = CONTROL_EXPOSURE;
	}

	if(isAvailable(CONTROL_GAMMA))
	{
		m_comboCtrl.AddString("Gamma");
		m_CtrlArray[ctrl++] = CONTROL_GAMMA;
	}

	if(isAvailable(CONTROL_WB_R))
	{
		m_comboCtrl.AddString("WB_R");	
		m_CtrlArray[ctrl++] = CONTROL_WB_R;
	}

	if(isAvailable(CONTROL_WB_B))
	{
		m_comboCtrl.AddString("WB B");
		m_CtrlArray[ctrl++] = CONTROL_WB_B;
	}

	if(isAvailable(CONTROL_BRIGHTNESS))
	{
		m_comboCtrl.AddString("Brightness");
		m_CtrlArray[ctrl++] = CONTROL_BRIGHTNESS;
	}


	if(isAvailable(CONTROL_BANDWIDTHOVERLOAD))
	{
		m_comboCtrl.AddString("USB Limit");
		m_CtrlArray[ctrl++] = CONTROL_BANDWIDTHOVERLOAD;
	}
	if(isAvailable(CONTROL_OVERCLOCK))
	{
		m_comboCtrl.AddString("Over CLK");
		m_CtrlArray[ctrl++] = CONTROL_OVERCLOCK;

	}
	if(isAvailable(CONTROL_HARDWAREBIN))
	{
		m_comboCtrl.AddString("Hardware Bin");
		m_CtrlArray[ctrl++] = CONTROL_HARDWAREBIN;
	}
	if(isAvailable(CONTROL_HIGHSPEED))
	{
		m_comboCtrl.AddString("High speed mode");
		m_CtrlArray[ctrl++] = CONTROL_HIGHSPEED;
	}
	if(isAvailable(CONTROL_COOLERPOWERPERC))
	{
		m_comboCtrl.AddString("Cool Power Perc");
		m_CtrlArray[ctrl++] = CONTROL_COOLERPOWERPERC;
	}
	if(isAvailable(CONTROL_TARGETTEMP))
	{
		m_comboCtrl.AddString("Target Temp");
		m_CtrlArray[ctrl++] = CONTROL_TARGETTEMP;
	}
	if(isAvailable(CONTROL_COOLER_ON))
	{
		m_comboCtrl.AddString("Cooler On");
		m_CtrlArray[ctrl++] = CONTROL_COOLER_ON;
	}
	if(isAvailable(CONTROL_MONO_BIN))
	{
		m_comboCtrl.AddString("Mono Bin");
		m_CtrlArray[ctrl++] = CONTROL_MONO_BIN;
	}
	if(isAvailable(CONTROL_FAN_ON))
	{
		m_comboCtrl.AddString("Fan On");
		m_CtrlArray[ctrl++] = CONTROL_FAN_ON;
	}
	if(isAvailable(CONTROL_PATTERN_ADJUST))
	{
		m_comboCtrl.AddString("Pattern Adjust");
		m_CtrlArray[ctrl++] = CONTROL_PATTERN_ADJUST;
	}




	bool bAuto;
	m_fTemp = (float)getValue(CONTROL_TEMPERATURE, &bAuto)/10;
	
	char buf[128] = {0};
	if(isColorCam())
		sprintf_s(buf, "Color Camera");
	else
		sprintf_s(buf, "Mono Camera");

	if(isUSB3Camera())
		strcat_s(buf, " USB3.0Camera");
	else
		strcat_s(buf, " USB2.0Camera");

	if(isUSB3Host())
		strcat_s(buf, " USB3.0Host");
	else
		strcat_s(buf, " USB2.0Host");
	m_strCameraType = buf;

	m_comboCtrl.SetCurSel(0);
	OnCbnSelchangeComboCtrl();
	m_butOpen.EnableWindow(FALSE);
	m_butClose.EnableWindow(TRUE);

	m_butStart.EnableWindow(TRUE);
	m_butStop.EnableWindow(FALSE);


	m_btnSetFormat.EnableWindow(TRUE);
	m_btnSetPostion.EnableWindow(TRUE);

	m_btNorth.EnableWindow(TRUE);
	m_btWest.EnableWindow(TRUE);
	m_btEast.EnableWindow(TRUE);
	m_btSouth.EnableWindow(TRUE);
	m_btAutoTest.EnableWindow(TRUE);

 	m_btn_snap.EnableWindow(TRUE);
	m_btn_get_id.EnableWindow(TRUE);
	m_btn_set_id.EnableWindow(TRUE);



	GetMisc((bool*)&m_bFlipX, (bool*)&m_bFlipY);

	UpdateData(FALSE);
}
BOOL bworking=FALSE;
bool b_SanpWorking = FALSE;

void CdemoDlg::OnBnClickedButtonSetformat()
{
	// TODO: 在此添加控件通知处理程序代码
	UpdateData(TRUE);
	int i = m_ComboImgType.GetCurSel();
	if(i==-1)
		return;
	if(bworking || b_SanpWorking)
		return;

	if(!setImageFormat(m_iROIWidth, m_iROIHeight, m_BinArray[m_ComboBin.GetCurSel()], m_ImgTypeArray[i] ))
		MessageBox("Set format error\n");
	UpdateData(false);
}

inline void OutputDbgPrint(const char* strOutPutString, ...)
{
#ifdef _DEBUG
	char strBuf[128] = {0};
	sprintf_s(strBuf, "<%s> ", "demo");
	va_list vlArgs;
	va_start(vlArgs, strOutPutString);
	_vsnprintf((char*)(strBuf+strlen(strBuf)), sizeof(strBuf)-strlen(strBuf), strOutPutString, vlArgs);
	va_end(vlArgs);

	OutputDebugStringA(strBuf);
#endif
}

void cvText(IplImage* img, const char* text, int x, int y)
{
#ifdef WIN32
	CvFont font;

	double hscale = 0.6;
	double vscale = 0.6;
	int linewidth = 2;
	cvInitFont(&font,CV_FONT_HERSHEY_SIMPLEX | CV_FONT_ITALIC,hscale,vscale,0,linewidth);

	CvScalar textColor =cvScalar(0,255,255);
	CvPoint textPos =cvPoint(x, y);

	cvPutText(img, text, textPos, &font,textColor);
#endif
}


void Display(LPVOID params)
{
	CdemoDlg* dlg =(CdemoDlg*)params;
	cvNamedWindow("video", 1);
	while(bworking)
	{
		cvShowImage("video", dlg->ConnectCamera.pRgb);
		cvWaitKey(1);
	}
	cvDestroyWindow("video");
	_endthread();
}

void CaptureVideo(LPVOID params)
{
	CdemoDlg* dlg =(CdemoDlg*)params; 
	int time1,time2;
	time1 = GetTickCount();
	int iDropFrame;
	long lBuffSize;
	char buf[128]={0};
	int count=0;

	IplImage *pTempImg = NULL;
	int width, height;
	IMG_TYPE image_type;
	image_type = getImgType();
	
	width = getWidth();
	height = getHeight();
	switch(image_type)
	{
	case IMG_Y8:
	case IMG_RAW8:
		dlg->ConnectCamera.pRgb = cvCreateImage(cvSize(width,height), IPL_DEPTH_8U, 1);
		pTempImg = cvCreateImage(cvSize(width,height), IPL_DEPTH_8U, 1);
		lBuffSize = width*height;
		break;

	case IMG_RGB24:
		dlg->ConnectCamera.pRgb = cvCreateImage(cvSize(width,height), IPL_DEPTH_8U, 3);
		pTempImg = cvCreateImage(cvSize(width,height), IPL_DEPTH_8U, 3);
		lBuffSize = width*height*3;
		break;

	case IMG_RAW16:
		dlg->ConnectCamera.pRgb = cvCreateImage(cvSize(width,height), IPL_DEPTH_16U, 1);
		pTempImg = cvCreateImage(cvSize(width,height), IPL_DEPTH_16U, 1);
		lBuffSize = width*height*2;
		break;
	}
	ULONG expMs;
	bool bAuto = false;
	while(dlg->ConnectCamera.Status == dlg->capturing)
	{
		time2 = GetTickCount();
		if(time2-time1 > 1000 )
		{
			iDropFrame = getDroppedFrames();
			sprintf_s(buf, "fps:%d dropped frames:%d",count, iDropFrame);

			count = 0;
			time1=GetTickCount();


		}
		expMs = getValue(CONTROL_EXPOSURE, &bAuto)/1000;
		if(getImageData((BYTE*)pTempImg->imageData, pTempImg->imageSize, expMs*2))
		{
			count++;
			cvText(pTempImg, buf, 40,40 );
			memcpy((BYTE*)(dlg->ConnectCamera.pRgb->imageData), (BYTE*)pTempImg->imageData, lBuffSize);
		}
		dlg->SetDlgItemText(IDC_STATIC_FPS, buf);
		
	}
	sprintf_s(buf, "capture stop");
	dlg->SetDlgItemText(IDC_STATIC_FPS, buf);
	stopCapture();
	cvReleaseImage(&dlg->ConnectCamera.pRgb);
	cvReleaseImage(&pTempImg);

	_endthread();

}
void DisplayLongExp(LPVOID params)
{
	CdemoDlg* dlg =(CdemoDlg*)params;

	char buf[128]={0};

	CString WindowName, temp;
	WindowName = "snap";

	cvNamedWindow(WindowName, 1);

	IMG_TYPE image_type;
	switch( image_type = getImgType())
	{
	case IMG_Y8:
	case IMG_RAW8:
		dlg->ConnectCamera.pRgb=cvCreateImage(cvSize(getWidth(),getHeight()), IPL_DEPTH_8U, 1);
		break;

	case IMG_RGB24:
		dlg->ConnectCamera.pRgb=cvCreateImage(cvSize(getWidth(),getHeight()), IPL_DEPTH_8U, 3);
		break;

	case IMG_RAW16:
		dlg->ConnectCamera.pRgb=cvCreateImage(cvSize(getWidth(),getHeight()), IPL_DEPTH_16U, 1);
		break;
	}

	int iStrLen = 0, iTextX = 40, iTextY = 60;
	while(dlg->ConnectCamera.Status == dlg->snaping)
	{		
		sprintf_s(buf, "snap time: %d ms", dlg->ConnectCamera.iSnapTime);


	
			if(image_type != IMG_RGB24 && image_type != IMG_RAW16)
				{
					iStrLen = strlen(buf);
					CvRect rect = cvRect(iTextX, iTextY - 15, iStrLen* 11, 20);
					cvSetImageROI(dlg->ConnectCamera.pRgb , rect);
					cvSet(dlg->ConnectCamera.pRgb, CV_RGB(180, 180, 180)); //可以用cvSet()将图像填充成白色  
					cvResetImageROI(dlg->ConnectCamera.pRgb);
				}
		


		cvText(dlg->ConnectCamera.pRgb, buf, iTextX, iTextY);
		cvShowImage(WindowName, dlg->ConnectCamera.pRgb);
		cvWaitKey(1);
	}
	cvReleaseImage(&dlg->ConnectCamera.pRgb);
	stopCapture();
	cvDestroyWindow(WindowName);
	_endthread();
}

void CdemoDlg::OnBnClickedButtonStart()
{
	// TODO: 在此添加控件通知处理程序代码
	if(b_SanpWorking)
		return;
	startCapture();
	m_butStart.EnableWindow(FALSE);
	m_butStop.EnableWindow(TRUE);
 	m_btn_snap.EnableWindow(FALSE);
	m_btn_AbortSnap.EnableWindow(FALSE);
	m_btnSetFormat.EnableWindow(FALSE);
	UpdateData(FALSE);
	bworking=TRUE;
	ConnectCamera.Status = capturing; 

	ConnectCamera.Thr_Display = (HANDLE)_beginthread(Display, NULL, this);
	ConnectCamera.Thr_CapVideo = (HANDLE)_beginthread(CaptureVideo, NULL, this);
}


void CdemoDlg::OnBnClickedButtonstop()
{
	// TODO: 在此添加控件通知处理程序代码
	bworking = FALSE;
	m_butStart.EnableWindow(TRUE);
	m_butStop.EnableWindow(FALSE);
 	m_btn_snap.EnableWindow(TRUE);
	m_btn_AbortSnap.EnableWindow(FALSE);
	m_btnSetFormat.EnableWindow(TRUE);


	m_static_SnapTime = "";
	OnBnClickedButtonAbortsnap();
	
	if(ConnectCamera.Status == capturing)
	{
		ConnectCamera.Status = opened;
		WaitForSingleObject(ConnectCamera.Thr_CapVideo, 1000);
	}
	
	if(ConnectCamera.Status == capturing || ConnectCamera.Status == snaping )
	{	
		ConnectCamera.Status = opened;
		WaitForSingleObject(ConnectCamera.Thr_Display, 1000);
	}
	

	UpdateData(FALSE);
}

void CdemoDlg::OnBnClickedButtonClose()
{
	// TODO: 在此添加控件通知处理程序代码
	OnBnClickedButtonstop();//stop capture
	closeCamera();
	ConnectCamera.Status = opened;

	m_iHeight = 0;
	m_iWidth = 0;
	m_iStartY = 0;
	m_iStartX = 0;
	m_iROIHeight = 0;
	m_iROIWidth = 0;
	m_fTemp = 0;
	m_ComboImgType.Clear();

	m_ComboImgType.ResetContent( );



	m_ComboBin.Clear();
	m_ComboBin.ResetContent( );



	m_comboCtrl.Clear();
	m_comboCtrl.ResetContent( );


	m_butOpen.EnableWindow(TRUE);
	m_butClose.EnableWindow(FALSE);

	m_butStart.EnableWindow(FALSE);
	m_butStop.EnableWindow(FALSE);

	m_btnSetFormat.EnableWindow(FALSE);
	m_btnSetPostion.EnableWindow(FALSE);

	m_ButAuto.EnableWindow(FALSE);

	m_btNorth.EnableWindow(FALSE);
	m_btWest.EnableWindow(FALSE);
	m_btEast.EnableWindow(FALSE);
	m_btSouth.EnableWindow(FALSE);
	m_btAutoTest.EnableWindow(FALSE);

 	m_btn_snap.EnableWindow(FALSE);
	m_btn_AbortSnap.EnableWindow(FALSE);
	m_btn_get_id.EnableWindow(FALSE);
	m_btn_set_id.EnableWindow(FALSE);

	b_SanpWorking = false;
	m_static_SnapTime = "";
	if(ConnectCamera.pRgb)
		cvReleaseImage(&ConnectCamera.pRgb);

	UpdateData(FALSE);
}

void CdemoDlg::OnCbnSelchangeComboCtrl()
{
	// TODO: 在此添加控件通知处理程序代码
	int i = m_comboCtrl.GetCurSel();
	bool bAuto;

	m_ctrlMin = getMin(m_CtrlArray[i]);
	m_ctrlMax = getMax(m_CtrlArray[i]);
	if(m_CtrlArray[i] == CONTROL_EXPOSURE)
	{
		m_ctrlMin /= 1000;
		m_ctrlMax /= 1000;//us->ms
	}
	
	if(isAutoSupported(m_CtrlArray[i]))
	{
		m_ButAuto.EnableWindow(TRUE) ;
		((CEdit*)GetDlgItem(IDC_EDIT_CTRL_VALUE))->EnableWindow(true);
	}
	else
	{
		m_ButAuto.EnableWindow(FALSE) ;
		((CEdit*)GetDlgItem(IDC_EDIT_CTRL_VALUE))->EnableWindow(false);

	}
	
	int iValue;
	m_SliderCtrl.SetRange(m_ctrlMin, m_ctrlMax);
	iValue = getValue(m_CtrlArray[i], &bAuto);
	
	if(bAuto)
	{
		if(m_CtrlArray[i] != CONTROL_TARGETTEMP)
		{		
			m_SliderCtrl.EnableWindow(FALSE) ;
			((CEdit*)GetDlgItem(IDC_EDIT_CTRL_VALUE))->EnableWindow(false);
		}
		else
		{		
			m_SliderCtrl.EnableWindow(TRUE) ;
			((CEdit*)GetDlgItem(IDC_EDIT_CTRL_VALUE))->EnableWindow(TRUE);
		}
		m_ButAuto.SetCheck(TRUE);
	}
	else
	{
		if(m_CtrlArray[i] == CONTROL_COOLERPOWERPERC)
		{		
			m_SliderCtrl.EnableWindow(FALSE) ;
			((CEdit*)GetDlgItem(IDC_EDIT_CTRL_VALUE))->EnableWindow(false);
		}
		else
		{
			m_SliderCtrl.EnableWindow(true) ;
			((CEdit*)GetDlgItem(IDC_EDIT_CTRL_VALUE))->EnableWindow(true);
		}
		
		m_ButAuto.SetCheck(FALSE);
	}

	m_static_SliderPos = iValue;
	if(m_CtrlArray[i] == CONTROL_EXPOSURE)
	{
		m_static_SliderPos = iValue/1000;
		m_edit_ctrl_value.Format("%dms", m_static_SliderPos);
	}
	else
		m_edit_ctrl_value.Format("%d", m_static_SliderPos);

	m_SliderCtrl.SetPos(m_static_SliderPos);
	
	if(m_CtrlArray[i] == CONTROL_EXPOSURE&&m_static_SliderPos < (m_ctrlMax - m_ctrlMin)/280)//小于最小移动值时不移动
		m_SliderCtrl.SetPos((m_ctrlMax - m_ctrlMin)/280);
	m_SliderCtrl.UpdateWindow();
	UpdateData(FALSE);

}

void CdemoDlg::OnNMReleasedcaptureSliderCtrl(NMHDR *pNMHDR, LRESULT *pResult)
{
	// TODO: 在此添加控件通知处理程序代码

	*pResult = 0;


	m_static_SliderPos = m_SliderCtrl.GetPos();

	if(BST_UNCHECKED == m_ButAuto.GetCheck() || m_CtrlArray[m_comboCtrl.GetCurSel()] == CONTROL_TARGETTEMP)//不自动时才能调整滑动条
	{
		if(m_CtrlArray[m_comboCtrl.GetCurSel()] == CONTROL_EXPOSURE)
		{
			m_edit_ctrl_value.Format("%dms", m_static_SliderPos);
			setValue(m_CtrlArray[m_comboCtrl.GetCurSel()], m_static_SliderPos*1000, false);
		}
		else
		{
			m_edit_ctrl_value.Format("%d", m_static_SliderPos);
			setValue(m_CtrlArray[m_comboCtrl.GetCurSel()], m_static_SliderPos, false);
		}
	}	
	UpdateData(FALSE);

}



void CdemoDlg::OnBnClickedButtonStartpos()
{
	// TODO: 在此添加控件通知处理程序代码
	UpdateData(TRUE);
	setStartPos(m_iStartX, m_iStartY);
}


void CdemoDlg::OnBnClickedButtonNorth()
{
	// TODO: 在此添加控件通知处理程序代码
	UpdateData(TRUE);
	pulseGuide(guideNorth, m_ST4time);
}

void CdemoDlg::OnBnClickedButtonSouth()
{
	// TODO: 在此添加控件通知处理程序代码
	UpdateData(TRUE);
	pulseGuide(guideSouth, m_ST4time);
}


void CdemoDlg::OnBnClickedButtonWest()
{
	// TODO: 在此添加控件通知处理程序代码
	UpdateData(TRUE);
	pulseGuide(guideWest, m_ST4time);
}

void CdemoDlg::OnBnClickedButtonEast()
{
	// TODO: 在此添加控件通知处理程序代码
	UpdateData(TRUE);
	pulseGuide(guideEast, m_ST4time);
}

void CdemoDlg::OnBnClickedCheckAuto()
{
	// TODO: 在此添加控件通知处理程序代码
	if(BST_CHECKED == m_ButAuto.GetCheck())
	{
		
		if(m_CtrlArray[m_comboCtrl.GetCurSel()] == CONTROL_EXPOSURE)
			setValue(m_CtrlArray[m_comboCtrl.GetCurSel()], m_static_SliderPos*1000, true);
		else
			setValue(m_CtrlArray[m_comboCtrl.GetCurSel()], m_static_SliderPos, true);

		if(m_CtrlArray[m_comboCtrl.GetCurSel()] != CONTROL_TARGETTEMP)
		{
			m_SliderCtrl.EnableWindow(false);
			((CEdit*)GetDlgItem(IDC_EDIT_CTRL_VALUE))->EnableWindow(false);
		}
		
	}
	else
	{
		UpdateData(TRUE);
		bool bAuto; 
		int i = m_comboCtrl.GetCurSel();
		if(m_CtrlArray[m_comboCtrl.GetCurSel()] == CONTROL_EXPOSURE)
		{
			m_static_SliderPos = getValue(m_CtrlArray[i], &bAuto)/1000;
			m_edit_ctrl_value.Format("%dms", m_static_SliderPos);
		}
		else
		{
			m_static_SliderPos = getValue(m_CtrlArray[i], &bAuto);
			m_edit_ctrl_value.Format("%d", m_static_SliderPos);
		}
		m_SliderCtrl.SetPos(m_static_SliderPos);//
		
		m_SliderCtrl.UpdateWindow();
		UpdateData(FALSE);
		
		if(m_CtrlArray[m_comboCtrl.GetCurSel()] == CONTROL_EXPOSURE)
			setValue(m_CtrlArray[m_comboCtrl.GetCurSel()], m_static_SliderPos*1000, false);
		else
			setValue(m_CtrlArray[m_comboCtrl.GetCurSel()], m_static_SliderPos, false);
		m_SliderCtrl.EnableWindow(true);
		((CEdit*)GetDlgItem(IDC_EDIT_CTRL_VALUE))->EnableWindow(true);

	}
}

void CdemoDlg::OnBnClickedCheckFlipx()
{
	// TODO: 在此添加控件通知处理程序代码
	UpdateData(TRUE);
	SetMisc(m_bFlipX, m_bFlipY);
}

void CdemoDlg::OnBnClickedCheckFlipy()
{
	// TODO: 在此添加控件通知处理程序代码
	UpdateData(TRUE);
	SetMisc(m_bFlipX, m_bFlipY);
}

UINT AutoTest(LPVOID params)
{
	pulseGuide(guideNorth, 100);
	pulseGuide(guideWest, 100);
	pulseGuide(guideSouth, 100);
	pulseGuide(guideEast, 100);
	return 0;
}
void CdemoDlg::OnBnClickedButtonAutotest()
{
	// TODO: 在此添加控件通知处理程序代码
	CWinThread * t1 = AfxBeginThread(AutoTest,this);
}

void CdemoDlg::OnBnClickedCheckSubDark()
{
	// TODO: 在此添加控件通知处理程序代码
	UpdateData(true);
	if ( m_check_SubDark)
	{

		char *FilePath;
		CString temp;
		  CFileDialog dlg(TRUE);///TRUE为OPEN对话框，FALSE为SAVE AS对话框  
		  if(dlg.DoModal()==IDOK) 
		  {
			  temp=dlg.GetPathName();
			  FilePath = (LPSTR)(LPCTSTR)temp;
			  EnableDarkSubtract(FilePath);
		  }
		  else
		  {
			  m_check_SubDark = false;
			 
		  }
	}
	else
	{
		DisableDarkSubtract();
		m_check_SubDark = false;
	}
	UpdateData(false);
}

int CdemoDlg::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CDialog::OnCreate(lpCreateStruct) == -1)
		return -1;

	// TODO:  在此添加您专用的创建代码
	SetTimer(1, 1000, NULL);
	return 0;
}

void CdemoDlg::OnTimer(UINT_PTR nIDEvent)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	
	int i = m_comboCtrl.GetCurSel();
	if(m_CtrlArray[i] == CONTROL_TARGETTEMP)
		return;
	
	bool bAuto= false;
	if(nIDEvent == 1 && ConnectCamera.Status != closed )
	{
		UpdateData(true);
		if(BST_CHECKED == m_ButAuto.GetCheck() || m_CtrlArray[i] == CONTROL_COOLERPOWERPERC)
		{		
			int iValue;			
			
			iValue = getValue(m_CtrlArray[i], &bAuto);

			m_static_SliderPos = iValue;
			if(m_CtrlArray[i] == CONTROL_EXPOSURE)
			{
				m_static_SliderPos = iValue/1000;
				m_edit_ctrl_value.Format("%dms", m_static_SliderPos);
			}
			else
				m_edit_ctrl_value.Format("%d", m_static_SliderPos);

			m_SliderCtrl.SetPos(m_static_SliderPos);
		}
		
		m_fTemp = (float)getValue(CONTROL_TEMPERATURE, &bAuto)/10;
		
		UpdateData(false);
	}

	
	CDialog::OnTimer(nIDEvent);
}


void SnapThr(LPVOID params)
{
	CdemoDlg* dlg =(CdemoDlg*)params; 

	long exp = 0;
	bool bAuto;
	exp = getValue(CONTROL_EXPOSURE, &bAuto);
 	exp/=1000;//ms
	if( exp>=2000 )
	{
		dlg->ConnectCamera.bSnap = false;//反的 false为开始
	}
	else
		dlg->m_static_SnapTime = "";

	int time = GetTickCount(), deltaMs, OldDeltaMs = 0;
	startExposure();//start exposure

	EXPOSURE_STATUS status;
	CString cs;

	do 
	{
		status = getExpStatus();
		if(!dlg->ConnectCamera.bSnap)  
		{
			deltaMs = GetTickCount() - time;
			deltaMs/=100;
			if( deltaMs!= OldDeltaMs)
			{
				OldDeltaMs = deltaMs;
				cs.Format("%.1fsec", (float)deltaMs/10);
			//	dlg->m_static_SnapTime = cs;
			//	SendMessage(dlg->m_hWnd, WM_MY_MSG, WM_UPDATEUISTATE, FALSE);
				dlg->SetDlgItemText(IDC_STATIC_SNAPTIME, cs);

			}
		}
		Sleep(1);
	} while (status == EXP_WORKING);

	char buf[64];
	if(status == EXP_FAILED)
	{		
		MessageBox(dlg->m_hWnd,"Snap Image failed!", "error", MB_OK);
	}
	else if (status == EXP_SUCCESS)
	{	
		dlg->ConnectCamera.iSnapTime = GetTickCount() - time;	
		sprintf_s(buf , "snap cost time: %d ms", dlg->ConnectCamera.iSnapTime);
		//	SetDlgItemText(IDC_STATIC_FPS, buf);

		getImageAfterExp((BYTE*)dlg->ConnectCamera.pRgb->imageData, dlg->ConnectCamera.pRgb->imageSize);

	}
	dlg->ConnectCamera.bSnap = true;//反的 false为开始 true为停止
	dlg->m_btn_snap.EnableWindow(true);
	dlg->m_btn_AbortSnap.EnableWindow(FALSE);

	_endthread();
}

void CdemoDlg::OnBnClickedButtonSnap()
{
	// TODO: 在此添加控件通知处理程序代码
	if(!(ConnectCamera.Status == capturing || ConnectCamera.Status == snaping))
	{

		ConnectCamera.Status = snaping;

		ConnectCamera.Thr_Display = (HANDLE)_beginthread(DisplayLongExp, NULL, this);

		m_butStart.EnableWindow(false);
	}
	if(!(ConnectCamera.Status == snaping) )
		return;

	m_btn_snap.EnableWindow(FALSE);
	CString cs;

	m_butStop.EnableWindow(TRUE);
	m_btn_AbortSnap.EnableWindow(TRUE);
	m_btnSetFormat.EnableWindow(FALSE);


	ConnectCamera.Thr_Snap = (HANDLE)_beginthread(SnapThr, NULL, this);//开始snap线程


}

void CdemoDlg::OnBnClickedButtonAbortsnap()
{
	// TODO: 在此添加控件通知处理程序代码
	if(ConnectCamera.Status == snaping)
	{
		m_btn_AbortSnap.EnableWindow(FALSE);
		stopExposure();
		WaitForSingleObject(ConnectCamera.Thr_Snap, 1000);

	}
}


BOOL CdemoDlg::PreTranslateMessage(MSG* pMsg)
{
	// TODO: 在此添加专用代码和/或调用基类
	if(pMsg->message == WM_KEYDOWN)
	{
		switch(pMsg->wParam) 
		{
		case VK_RETURN: 
			if( GetDlgItem(IDC_EDIT_CTRL_VALUE) == GetFocus() )
			{
				UpdateData(true);
			
				long lValue;
				lValue = m_static_SliderPos = atol(m_edit_ctrl_value);
				m_SliderCtrl.SetPos(m_static_SliderPos);
				if(m_CtrlArray[m_comboCtrl.GetCurSel()] == CONTROL_EXPOSURE)
					lValue*=1000;
				setValue(m_CtrlArray[m_comboCtrl.GetCurSel()], lValue, false);
				UpdateData(false);
			}
			return TRUE;

		}
	}
	return CDialog::PreTranslateMessage(pMsg);
}

void CdemoDlg::OnBnClickedButtonGetId()
{
	// TODO: 在此添加控件通知处理程序代码
	ASIID ASI_id = {0};
	m_edit_id_in_flash.Empty();
	GetID(&ASI_id);
	for (int i = 0; i < 8; i++)
	{
		m_edit_id_in_flash += ASI_id.id[i];
	}
	UpdateData(false);
}

void CdemoDlg::OnBnClickedButtonWriteId()
{
	// TODO: 在此添加控件通知处理程序代码
	ASIID ASI_id;
	UpdateData(true);
	if(m_edit_id_in_flash.GetLength()!=8)
	{
		AfxMessageBox("id length must be 8");
		return;
	}
	for (int i = 0; i < 8; i++)
	{
		ASI_id.id[i] = m_edit_id_in_flash.GetAt(i);
	}
	if(SetID( ASI_id))
		AfxMessageBox("set ID success");
	else
		AfxMessageBox("set ID fail");

}

void CdemoDlg::OnClose()
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	OnBnClickedButtonClose();
	CDialog::OnClose();
}
