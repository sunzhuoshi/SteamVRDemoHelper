// SteamVRDemoCore.cpp : 定义 DLL 应用程序的导出函数。
//

#include "stdafx.h"
#include "SteamVRDemoCore.h"


// 这是导出变量的一个示例
STEAMVRDEMOCORE_API int nActive=0;

/*
#pragma data_seg(".shared")
HHOOK hHook = NULL;
#pragma data_seg()
#pragma comment(linker,"/section:.shared,rws")
*/

// 这是导出函数的一个示例。
STEAMVRDEMOCORE_API LRESULT WINAPI fnWndMsgProc(int nCode, WPARAM wParam, LPARAM lParam)
{
	TCHAR tmp[MAX_PATH];

	switch (nCode) 
	{
	case HCBT_CREATEWND:
	{
		HWND wnd = (HWND)wParam;
		CBT_CREATEWND *createWndParam = (CBT_CREATEWND *)lParam;
		GetWindowText(wnd, tmp, MAX_PATH);
		SendMessage(wnd, SW_MAXIMIZE, 0, 0);
	}
		break;
	case HCBT_ACTIVATE: 
	{
		HWND wnd = (HWND)wParam;
		CBTACTIVATESTRUCT *activeParam = (CBTACTIVATESTRUCT *)lParam;
		GetWindowText(wnd, tmp, MAX_PATH);
		ShowWindow(wnd, SW_MAXIMIZE);
	}
		break;
	default:
		break;
	}
    return CallNextHookEx(NULL, nCode, wParam, lParam);
}
