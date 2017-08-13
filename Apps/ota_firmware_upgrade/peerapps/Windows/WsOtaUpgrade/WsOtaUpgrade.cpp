
// WsOtaUpgrade.cpp : Defines the class behaviors for the application.
//
#include "stdafx.h"
#include "btwleapis.h"
#include "WsOtaUpgrade.h"
#include "WsOtaUpgradeDlg.h"
#include "BTFullLibPath.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

HMODULE hLib;

GUID guidSvcWSUpgrade;
GUID guidCharWSUpgradeControlPoint;
GUID guidCharWSUpgradeData;

// CWsOtaUpgradeApp

BEGIN_MESSAGE_MAP(CWsOtaUpgradeApp, CWinApp)
	ON_COMMAND(ID_HELP, &CWinApp::OnHelp)
END_MESSAGE_MAP()


// CWsOtaUpgradeApp construction

CWsOtaUpgradeApp::CWsOtaUpgradeApp()
{
}


// The one and only CWsOtaUpgradeApp object

CWsOtaUpgradeApp theApp;

BOOL IsOSWin8()
{
	BOOL bIsWin8 = FALSE;

	OSVERSIONINFOEX osvi;
	DWORDLONG dwlConditionMask = 0;

	// Initialize the OSVERSIONINFOEX structure.
	ZeroMemory(&osvi, sizeof(OSVERSIONINFOEX));
	osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
	osvi.dwMajorVersion = 6;
	osvi.dwMinorVersion = 2;
	osvi.wServicePackMajor = 0;

	// Initialize the condition mask.
	VER_SET_CONDITION( dwlConditionMask, VER_MAJORVERSION, VER_GREATER_EQUAL );
	VER_SET_CONDITION( dwlConditionMask, VER_MINORVERSION, VER_GREATER_EQUAL );
	VER_SET_CONDITION( dwlConditionMask, VER_SERVICEPACKMAJOR, VER_GREATER_EQUAL );

	// Perform the test.
	if(VerifyVersionInfo(&osvi, VER_MAJORVERSION | VER_MINORVERSION | VER_SERVICEPACKMAJOR, dwlConditionMask))
	{
		bIsWin8 = TRUE;
	}

	return bIsWin8;
}

BOOL IsOSWin7()
{
	BOOL bIsWinSeven = FALSE;

	OSVERSIONINFOEX osvi;
	DWORDLONG dwlConditionMask = 0;

	// Initialize the OSVERSIONINFOEX structure.
	ZeroMemory(&osvi, sizeof(OSVERSIONINFOEX));
	osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
	osvi.dwMajorVersion = 6;
	osvi.dwMinorVersion = 1;
	osvi.wServicePackMajor = 0;

	// Initialize the condition mask.
	VER_SET_CONDITION( dwlConditionMask, VER_MAJORVERSION, VER_GREATER_EQUAL );
	VER_SET_CONDITION( dwlConditionMask, VER_MINORVERSION, VER_GREATER_EQUAL );
	VER_SET_CONDITION( dwlConditionMask, VER_SERVICEPACKMAJOR, VER_GREATER_EQUAL );

	// Perform the test.
	if(VerifyVersionInfo(&osvi, VER_MAJORVERSION | VER_MINORVERSION | VER_SERVICEPACKMAJOR, dwlConditionMask))
	{
		bIsWinSeven = TRUE;
	}

	return bIsWinSeven;
}

void BtwGuidFromGuid(GUID *pOut, const GUID *pIn)
{
    pOut->Data1 = (pIn->Data4[4] << 24) + (pIn->Data4[5] << 16) + (pIn->Data4[6] << 8) + pIn->Data4[7];
    pOut->Data2 = (pIn->Data4[2] << 8) + pIn->Data4[3];
    pOut->Data3 = (pIn->Data4[0] << 8) + pIn->Data4[1];
    pOut->Data4[0] = (pIn->Data3      ) & 0xff;
    pOut->Data4[1] = (pIn->Data3 >> 8 ) & 0xff;
    pOut->Data4[2] = (pIn->Data2      ) & 0xff;
    pOut->Data4[3] = (pIn->Data2 >> 8 ) & 0xff;
    pOut->Data4[4] = (pIn->Data1      ) & 0xff;
    pOut->Data4[5] = (pIn->Data1 >> 8 ) & 0xff;
    pOut->Data4[6] = (pIn->Data1 >> 16) & 0xff;
    pOut->Data4[7] = (pIn->Data1 >> 24) & 0xff;
}

// CWsOtaUpgradeApp initialization

BOOL CWsOtaUpgradeApp::InitInstance()
{
	// InitCommonControlsEx() is required on Windows XP if an application
	// manifest specifies use of ComCtl32.dll version 6 or later to enable
	// visual styles.  Otherwise, any window creation will fail.
	INITCOMMONCONTROLSEX InitCtrls;
	InitCtrls.dwSize = sizeof(InitCtrls);
	// Set this to include all the common control classes you want to use
	// in your application.
	InitCtrls.dwICC = ICC_WIN95_CLASSES;
	InitCommonControlsEx(&InitCtrls);

	CWinApp::InitInstance();

    CCommandLineInfo cmdInfo;
	ParseCommandLine(cmdInfo);

    char filename[MAX_PATH];
    WideCharToMultiByte(CP_ACP, 0, cmdInfo.m_strFileName, -1, filename, sizeof (filename), NULL, 0);

    FILE *fPatch;
    if (fopen_s(&fPatch, filename, "rb"))
    {
		MessageBox(NULL, L"Failed to open the patch file", L"Error", MB_OK);
        return (FALSE);
    }
    // read private key
    fseek(fPatch, 0, SEEK_END);
    DWORD fileSize = ftell(fPatch);
    rewind(fPatch);
    LPBYTE pPatch = (LPBYTE)new BYTE[fileSize];

    DWORD readBytes = (DWORD)fread(pPatch, 1, fileSize, fPatch);
    fclose(fPatch);

	CWsOtaUpgradeDlg dlg(pPatch, fileSize);

	// Lets check if BT is plugged in
	HANDLE hRadio = INVALID_HANDLE_VALUE;
	BLUETOOTH_FIND_RADIO_PARAMS params = {sizeof(BLUETOOTH_FIND_RADIO_PARAMS)};
	HBLUETOOTH_RADIO_FIND hf = BluetoothFindFirstRadio(&params, &hRadio);
	if (hf == NULL)
	{
		MessageBox(NULL, L"Bluetooth radio is not present or not functioning.  Please plug in the dongle and try again.", L"Error", MB_OK);
		delete[] pPatch;
		return FALSE;
	}
    CloseHandle(hRadio);
    BluetoothFindRadioClose(hf);

	// This application supports Microsoft on Win8 or BTW on Win7.
	if (IsOSWin8())
	{
		dlg.m_bWin8 = TRUE;
        if ((hLib = LoadLibrary(L"BluetoothApis.dll")) == NULL)
		{
			MessageBox(NULL, L"Failed to load BluetoothAPIs library", L"Error", MB_OK);
			delete[] pPatch;
			return FALSE;
		}
        guidSvcWSUpgrade                = GUID_WS_UPGRADE_SERVICE;
        guidCharWSUpgradeControlPoint   = GUID_WS_UPGRADE_CHARACTERISTIC_CONTROL_POINT;
        guidCharWSUpgradeData           = GUID_WS_UPGRADE_CHARACTERISTIC_DATA;
	}
	else if (IsOSWin7())
	{
		dlg.m_bWin8 = FALSE;

		TCHAR BtDevFullPath[MAX_PATH + 1] = { '\0' };
		CBTFullLibPath LibPath;
		LibPath.GetFullInstallPathOf(L"BTWLeApi.Dll", BtDevFullPath, MAX_PATH);
		if ((hLib = LoadLibrary(BtDevFullPath)) == NULL)
		{
			MessageBox(NULL, L"Broadcom Blueototh profile pack for Windows (BTW) has to be installed", L"Error", MB_OK);
			delete[] pPatch;
			return FALSE;
		}
        BtwGuidFromGuid(&guidSvcWSUpgrade, &GUID_WS_UPGRADE_SERVICE);
        BtwGuidFromGuid(&guidCharWSUpgradeControlPoint, &GUID_WS_UPGRADE_CHARACTERISTIC_CONTROL_POINT);
        BtwGuidFromGuid(&guidCharWSUpgradeData, &GUID_WS_UPGRADE_CHARACTERISTIC_DATA);
	}
	else
	{
		MessageBox(NULL, L"This application can run on Windows 8 or on Windows 7 with Broadcom Bluetooth profile pack for Windows (BTW) installed", L"Error", MB_OK);
		delete[] pPatch;
		return FALSE;
	}

	// Activate "Windows Native" visual manager for enabling themes in MFC controls
	CMFCVisualManager::SetDefaultManager(RUNTIME_CLASS(CMFCVisualManagerWindows));

	// Standard initialization
	// If you are not using these features and wish to reduce the size
	// of your final executable, you should remove from the following
	// the specific initialization routines you do not need
	// Change the registry key under which our settings are stored
	SetRegistryKey(_T("Broadcom"));

	m_pMainWnd = &dlg;
	INT_PTR nResponse = dlg.DoModal();
	if (nResponse == IDOK)
	{
	}
	else if (nResponse == IDCANCEL)
	{
	}
	else if (nResponse == -1)
	{
		TRACE(traceAppMsg, 0, "Warning: dialog creation failed, so application is terminating unexpectedly.\n");
		TRACE(traceAppMsg, 0, "Warning: if you are using MFC controls on the dialog, you cannot #define _AFX_NO_MFC_CONTROLS_IN_DIALOGS.\n");
	}
    if (hLib != NULL)
        FreeLibrary(hLib);

    delete[] pPatch;

	// Since the dialog has been closed, return FALSE so that we exit the
	//  application, rather than start the application's message pump.
	return FALSE;
}


void ods(char * fmt_str, ...)
{
	char buf[1000] = {0};
	va_list marker = NULL;

	va_start (marker, fmt_str );
	vsnprintf_s (buf, sizeof(buf), _TRUNCATE, fmt_str, marker );
	va_end  ( marker );

    strcat_s(buf, sizeof(buf), "\n");
	OutputDebugString(CA2W(buf));	
}

void BdaToString (PWCHAR buffer, BLUETOOTH_ADDRESS *btha)
{
	WCHAR c;
	for (int i = 0; i < 6; i++)
	{
		c = btha->rgBytes[5 - i] >> 4;
		buffer[2 * i    ] = c < 10 ? c + '0' : c + 'A' - 10;
		c = btha->rgBytes[5 - i] & 0x0f;
		buffer[2 * i + 1] = c < 10 ? c + '0' : c + 'A' - 10;
	}
}


void UuidToString(LPWSTR buffer, size_t buffer_size, GUID *uuid)
{
	// Example {00001101-0000-1000-8000-00805F9B34FB}
    _swprintf_p(buffer, buffer_size, L"{%08X-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X}", uuid->Data1, uuid->Data2,
			uuid->Data3, uuid->Data4[0], uuid->Data4[1], uuid->Data4[2], uuid->Data4[3],
			uuid->Data4[4], uuid->Data4[5], uuid->Data4[6], uuid->Data4[7]);
}
