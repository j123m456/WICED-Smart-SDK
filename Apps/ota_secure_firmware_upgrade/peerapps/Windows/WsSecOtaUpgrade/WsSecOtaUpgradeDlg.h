
// WsSecOtaUpgradeDlg.h : header file
//

#pragma once
#include "Win7Interface.h"
#include "Win8Interface.h"
#include "WsOtaDownloader.h"
#include "afxcmn.h"
#include "afxwin.h"

#define WM_CONNECTED    (WM_USER + 101)
#define WM_NOTIFIED     (WM_USER + 102)
#define WM_PROGRESS     (WM_USER + 103)

// CWsSecOtaUpgradeDlg dialog
class CWsSecOtaUpgradeDlg : public CDialogEx
{
// Construction
public:
	CWsSecOtaUpgradeDlg(LPBYTE pPatch, DWORD dwPatchSize, CWnd* pParent = NULL);	// standard constructor
	virtual ~CWsSecOtaUpgradeDlg();

// Dialog Data
	enum { IDD = IDD_WS_UPGRADE_DIALOG };

	BOOL m_bWin8;
//    BLUETOOTH_ADDRESS m_bth;
    HMODULE m_hLib;
    CBtInterface *m_btInterface;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support

private:
    int    numNotifies;
    HANDLE m_hDevice;
    CComboBox m_cbDevices;
    BOOL   GetDeviceList();
    int m_numDevices;
    WSDownloader *m_pDownloader;
    LPBYTE m_pPatch;
    DWORD m_dwPatchSize;
    BYTE m_errCode;

// Implementation
protected:
	HICON m_hIcon;


	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
    virtual void PostNcDestroy();
    LRESULT OnConnected(WPARAM bConnected, LPARAM lparam);
    LRESULT OnNotified(WPARAM op, LPARAM lparam);
    LRESULT OnProgress(WPARAM completed, LPARAM total);
public:
    afx_msg void OnBnClickedWriteChar2();
    afx_msg void OnBnClickedStart();
    CProgressCtrl m_Progress;
};
