
// demoDlg.h : 头文件
//

#pragma once
#include "afxwin.h"
#include "afxcmn.h"
#include "opencv/highgui.h"
#include "../../include/ASICamera.h"
// CdemoDlg 对话框
class CdemoDlg : public CDialog
{
// 构造
public:
	CdemoDlg(CWnd* pParent = NULL);	// 标准构造函数

// 对话框数据
	enum { IDD = IDD_DEMO_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持


// 实现
protected:
	HICON m_hIcon;

	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	CComboBox CComboCamera;
	afx_msg void OnBnClickedButtonScan();
	afx_msg void OnCbnSelchangeComboCameras();
	int m_iWidth;
	int m_iHeight;
	int m_iStartX;
	int m_iStartY;
	int m_iROIWidth;
	int m_iROIHeight;
	CComboBox m_ComboImgType;
	CComboBox m_ComboBin;
	afx_msg void OnBnClickedButtonStart();
	afx_msg void OnBnClickedButtonstop();
	afx_msg void OnBnClickedButtonClose();
	CComboBox m_comboCtrl;
	afx_msg void OnCbnSelchangeComboCtrl();
	int m_ctrlMin;
	int m_ctrlMax;
	afx_msg void OnNMReleasedcaptureSliderCtrl(NMHDR *pNMHDR, LRESULT *pResult);
	CSliderCtrl m_SliderCtrl;
	Control_TYPE m_CtrlArray[64];
	IMG_TYPE m_ImgTypeArray[64];
	int m_BinArray[64];
	int m_static_SliderPos;
	CButton m_butOpen;
	CButton m_butClose;
	CButton m_butStop;
	CButton m_butStart;
	afx_msg void OnBnClickedButtonStartpos();
	float m_fTemp;
	CButton m_btnSetFormat;
	CButton m_btnSetPostion;

	int m_ST4time;
	afx_msg void OnBnClickedButtonNorth();
	afx_msg void OnBnClickedButtonSouth();
	afx_msg void OnBnClickedButtonWest();
	afx_msg void OnBnClickedButtonEast();
	CButton m_ButAuto;
	afx_msg void OnBnClickedCheckAuto();
	CString m_strCameraType;
	BOOL m_bFlipY;
	BOOL m_bFlipX;
	afx_msg void OnBnClickedCheckFlipx();
	afx_msg void OnBnClickedCheckFlipy();
	CButton m_btNorth;
	CButton m_btEast;
	CButton m_btSouth;
	CButton m_btWest;
	afx_msg void OnBnClickedButtonAutotest();
	CButton m_btAutoTest;
	afx_msg void OnBnClickedButtonOpen();
	afx_msg void OnBnClickedButtonSetformat();
	afx_msg void OnBnClickedCheckSubDark();
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnTimer(UINT_PTR nIDEvent);

	afx_msg void OnBnClickedButtonSnap();
	enum CamStatus {
		closed = 0,
		opened,
		capturing,
		snaping
	};
	struct	ConnectedCam{
		HANDLE Thr_Display, Thr_Snap, Thr_CapVideo;
		CamStatus Status;
		IplImage *pRgb;
		int iSnapTime;
		bool bSnap;
		float iSnapSecond;
	};
	ConnectedCam ConnectCamera;
	CButton m_btn_snap;
	CButton m_btn_AbortSnap;
	CString m_static_SnapTime;
	afx_msg void OnBnClickedButtonAbortsnap();
	CString m_edit_ctrl_value;
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	BOOL m_check_SubDark;
	CString m_edit_id_in_flash;
	afx_msg void OnBnClickedButtonGetId();
	afx_msg void OnBnClickedButtonWriteId();
	CButton m_btn_get_id;
	CButton m_btn_set_id;
	afx_msg void OnClose();
};
