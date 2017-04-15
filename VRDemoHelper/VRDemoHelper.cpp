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
#include "VRDemoSteamVRConfigurator.h"

#define MAX_LOADSTRING 100
#define LOG_PROPERTY_FILE "log4cplus.props"
#define SINGLE_INSTANCE_MUTEX_NAME "L4VRDemoHelperSingleInstanceMetux"
 
HINSTANCE hInst;                                
CHAR szTitle[MAX_LOADSTRING];                
CHAR szWindowClass[MAX_LOADSTRING];           

VRDemoTogglesWrapper togglesWrapper;

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

    if (!VRDemoSteamVRConfigurator::getInstance().init()) {
        togglesWrapper.setConfigurateVRNotification(FALSE);
        LOG4CPLUS_INFO(logger, "Failed to init SteamVR configurator, check if SteamVR is installed");
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

    LoadStringA(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringA(hInstance, IDC_VRDEMOHELPER, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_VRDEMOHELPER));

    MSG msg;

    LOG4CPLUS_INFO(logger, "VR Demo Helper started");

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
            case IDM_SHOW_FPS:
                togglesWrapper.toggleShowFPS();
                break;
            case IDM_MAXIMIZE_GAMES:
                togglesWrapper.toggleMaximizeGames();
                break;
            case IDM_IMPROVE_STEAM_VR:
                togglesWrapper.toggleImproveSteamVR();
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
            EndPaint(hWnd, &ps);
        }
        break;
    case WM_DESTROY:
        VRDemoSteamVRConfigurator::getInstance().restoreSettings();
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

            bool steamVRConfiguratorActive = VRDemoSteamVRConfigurator::getInstance().isActive();
            CheckMenuItem(hSubMenu, IDM_IMPROVE_STEAM_VR, MF_BYCOMMAND | (togglesWrapper.getImproveSteamVR() ? MF_CHECKED : MF_UNCHECKED));
            EnableMenuItem(hSubMenu, IDM_IMPROVE_STEAM_VR, MF_BYCOMMAND | (steamVRConfiguratorActive? MF_ENABLED: MF_DISABLED));

            CheckMenuItem(hSubMenu, IDM_SHOW_FPS, MF_BYCOMMAND | (togglesWrapper.getShowFPS() ? MF_CHECKED : MF_UNCHECKED));
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