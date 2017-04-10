﻿// VRDemoHelper.cpp : 定义应用程序的入口点。
//

// NOTE: DO NOT add anything except comments before "stdafx.h", it will be omitted without any warning...
#include "stdafx.h"

#include "VRDemoHelper.h"

#include <sstream>
#include <windows.h>
#include <shellapi.h>
#include <CommCtrl.h>

#include <log4cplus/log4cplus.h>
#include <log4cplus/logger.h>
#include <log4cplus/loggingmacros.h>
#include <log4cplus/configurator.h>
#include <log4cplus/initializer.h>
#include <log4cplus/socketappender.h>

#include "util/l4util.h"
#include "argcargv.h"
#include "VRDemoConfigurator.h"
#include "VRDemoWindowPoller.h"
#include "VRDemoCoreWrapper.h"
#include "VRDemoNotificationManager.h"
#include "VRDemoHotKeyManager.h"

#define MAX_LOADSTRING 100
#define LOG_PROPERTY_FILE "log4cplus.props"
#define SINGLE_INSTANCE_MUTEX_NAME "L4VRDemoHelperSingleInstanceMetux"

// 全局变量: 
HINSTANCE hInst;                                // 当前实例
CHAR szTitle[MAX_LOADSTRING];                  // 标题栏文本
CHAR szWindowClass[MAX_LOADSTRING];            // 主窗口类名

VRDemoTogglesWrapper togglesWrapper;

// 此代码模块中包含的函数的前向声明: 
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);

VOID ShowContextMenu(HWND hwnd, POINT pt);
BOOL IsAbleToRun();
VOID InitLogConfiguration();

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

	// init log first in main
	log4cplus::Initializer initializer;

    InitLogConfiguration();

    log4cplus::Logger logger = log4cplus::Logger::getRoot();

	if (!IsAbleToRun())
	{
		return FALSE;
	}

	LOG4CPLUS_INFO(logger, "VR Demo Helper is starting");

    if (!VRDemoConfigurator::getInstance().init(
            l4util::getFileFullPath(VRDemoConfigurator::FILE_SETTINGS)
        )
    ) {  
        VR_DEMO_ALERT_IS(IDS_CAPTION_ERROR, "Failed to init configurator,\ncheck the log for detail.");
        return FALSE;
    }


    VRDemoCoreWrapper::VRDemoCoreWrapperPtr coreWrapper(new VRDemoCoreWrapper());
    if (!coreWrapper->init()) {
        VR_DEMO_ALERT_IS(IDS_CAPTION_ERROR, "Failed to init core module,\ncheck the log for detail.");
        return FALSE;
    }

    VRDemoWindowPoller::VRDemoWindowPollerPtr poller(new VRDemoWindowPoller());
    if (!poller->init(togglesWrapper.getToggles())) {
        LOG4CPLUS_ERROR(logger, "Failed to init window poller");
        return FALSE;
    }

    // 初始化全局字符串
    LoadStringA(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringA(hInstance, IDC_VRDEMOHELPER, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // 执行应用程序初始化: 
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_VRDEMOHELPER));

    MSG msg;

    LOG4CPLUS_INFO(logger, "VR Demo Helper started");

    // 主消息循环: 
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    // stop muse be called, or poller thread will access data freed
    poller->stop();

	LOG4CPLUS_INFO(logger, "VR Demo Helper exited");
    return (int) msg.wParam;
}



//
//  函数: MyRegisterClass()
//
//  目的: 注册窗口类。
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEX wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCEA(IDI_VRDEMOHELPER));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = MAKEINTRESOURCEA(IDC_VRDEMOHELPER);
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCEA(IDI_NOTIFICATIONICON));

    return RegisterClassExA(&wcex);
}

//
//   函数: InitInstance(HINSTANCE, int)
//
//   目的: 保存实例句柄并创建主窗口
//
//   注释: 
//
//        在此函数中，我们在全局变量中保存实例句柄并
//        创建和显示主程序窗口。
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   hInst = hInstance; // 将实例句柄存储在全局变量中

   // set hWndParent to HWD_MESSAGE to hide window(receive messages only)
   HWND hWnd = CreateWindowA(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, HWND_MESSAGE, nullptr, hInstance, nullptr);

   if (!hWnd)
   {
      return FALSE;
   }

   VRDemoHotKeyManager hotKeyManager;
   hotKeyManager.configurate(hWnd);

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return TRUE;
}

//
//  函数: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  目的:    处理主窗口的消息。
//
//  WM_COMMAND  - 处理应用程序菜单
//  WM_PAINT    - 绘制主窗口
//  WM_DESTROY  - 发送退出消息并返回
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
	case WM_CREATE:
		{
            VRDemoNotificationManager::getInstance().init(hInst, hWnd);
            VRDemoNotificationManager::getInstance().addNotificationIcon();
            VRDemoNotificationManager::getInstance().addNotificationInfo(IDS_NOTIFICATION_STARTED);
        }
		break;
    case WM_COMMAND:
        {
            int wmId = LOWORD(wParam);
            // 分析菜单选择: 
            switch (wmId)
            {
            case IDM_ABOUT:
                DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
                break;
            case IDM_EXIT:
                DestroyWindow(hWnd);
                break;
            case IDM_PAUSE:
            {
                togglesWrapper.togglePause();
            }
                break;
            case IDM_SHOW_FPS_OVERLAP:
                MessageBoxA(hWnd, "Coming soon...", "Info", MB_OK);
                break;
            case IDM_MAXIMIZE_GAMES:
                togglesWrapper.toggleMaximizeGames();
                break;
            case IDM_HIDE_STEAM_VR_NOTIFICATION:
                togglesWrapper.toggleHideSteamVrNotification();
                break;
            default:
                return DefWindowProc(hWnd, message, wParam, lParam);
            }
        }
        break;
    case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps);
            // TODO: 在此处添加使用 hdc 的任何绘图代码...
            EndPaint(hWnd, &ps);
        }
        break;
    case WM_DESTROY:
        VRDemoNotificationManager::getInstance().deleteNotificationIcon();
        PostQuitMessage(0);
        break;
	case WMAPP_NOTIFYCALLBACK:
		switch (LOWORD(lParam))
		{
		case WM_CONTEXTMENU:
			{
				POINT const pt = { LOWORD(wParam), HIWORD(wParam) };
				ShowContextMenu(hWnd, pt);
			}
			break;
		default:
			break;
		}
		break;
    case WM_HOTKEY:
        switch (HIWORD(lParam)) {
        case VK_F8:
            togglesWrapper.togglePause();
            break;
        }
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

// “关于”框的消息处理程序。
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
        break;
    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}

VOID ShowContextMenu(HWND hwnd, POINT pt)
{
	HMENU hMenu = LoadMenu(hInst, MAKEINTRESOURCE(IDC_CONTEXTMENU));
	if (hMenu)
	{
		HMENU hSubMenu = GetSubMenu(hMenu, 0);
		if (hSubMenu)
		{
			// our window must be foreground before calling TrackPopupMenu or the menu will not disappear when the user clicks away
			SetForegroundWindow(hwnd);

			// respect menu drop alignment
			UINT uFlags = TPM_RIGHTBUTTON;
			if (GetSystemMetrics(SM_MENUDROPALIGNMENT) != 0)
			{
				uFlags |= TPM_RIGHTALIGN;
			}
			else
			{
				uFlags |= TPM_LEFTALIGN;
			}
            CheckMenuItem(hSubMenu, IDM_PAUSE, MF_BYCOMMAND | (togglesWrapper.getPause() ? MF_CHECKED : MF_UNCHECKED));
            CheckMenuItem(hSubMenu, IDM_MAXIMIZE_GAMES, MF_BYCOMMAND | (togglesWrapper.getMaximmizeGames() ? MF_CHECKED : MF_UNCHECKED));
            CheckMenuItem(hSubMenu, IDM_HIDE_STEAM_VR_NOTIFICATION, MF_BYCOMMAND | (togglesWrapper.getHideSteamVrNotification() ? MF_CHECKED : MF_UNCHECKED));
            TrackPopupMenuEx(hSubMenu, uFlags, pt.x, pt.y, hwnd, NULL);
		}
		DestroyMenu(hMenu);
	}
}

BOOL IsAbleToRun()
{
	BOOL bResult = FALSE;
	std::ostringstream errorMsg;
    log4cplus::Logger logger = log4cplus::Logger::getRoot();

	HANDLE hMutex = CreateMutexA(NULL, TRUE, SINGLE_INSTANCE_MUTEX_NAME);
	if (hMutex)
	{
		if (GetLastError() == ERROR_ALREADY_EXISTS) 
		{
			CloseHandle(hMutex);
			errorMsg << l4util::loadString(IDC_VRDEMOHELPER) << " is already running";
			LOG4CPLUS_ERROR(logger, errorMsg.str());
			MessageBox(NULL, 
				errorMsg.str().c_str(), 
				l4util::loadString(IDS_CAPTION_ERROR).c_str(), 
				MB_OK
			);
			// TODO: send a notification message?
		}
		else 
		{
			bResult = TRUE;
		}
	}
	else 
	{
		errorMsg << "Failed to create single instance lock, error code: " << GetLastError();
		LOG4CPLUS_ERROR(logger, errorMsg.str());;
		MessageBox(NULL, 
			errorMsg.str().c_str(), 
			l4util::loadString(IDS_CAPTION_ERROR).c_str(), 
			MB_OK
		);
	}
	return bResult;
}

VOID InitLogConfiguration()
{
    std::ostringstream defaultProps;

    defaultProps << "log4cplus.rootLogger = DEBUG, FILE" << std::endl;
    defaultProps << "log4cplus.appender.FILE = log4cplus::RollingFileAppender" << std::endl;
    defaultProps << "log4cplus.appender.FILE.MaxFileSize = 100MB" << std::endl;
    defaultProps << "log4cplus.appender.FILE.MaxBackupIndex = 10" << std::endl;
    defaultProps << "log4cplus.appender.FILE.File = helper.log" << std::endl;
    defaultProps << "log4cplus.appender.FILE.layout = log4cplus::PatternLayout" << std::endl;
    defaultProps << "log4cplus.appender.FILE.layout.ConversionPattern = [%-5p %d{%y-%m-%d %H:%M:%S}] %m%n%n" << std::endl;

    log4cplus::PropertyConfigurator defaultConfigutator(std::istringstream(defaultProps.str()));
    defaultConfigutator.configure();
}