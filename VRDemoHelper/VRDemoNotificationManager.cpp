#include "stdafx.h"
#include "VRDemoNotificationManager.h"

#include <CommCtrl.h>
#include <shellapi.h>
#include "VRDemoHelper.h"

// we need commctrl v6 for LoadIconMetric()
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")
#pragma comment(lib, "comctl32.lib")


// Use a guid to uniquely identify our icon
#ifdef _DEBUG
class __declspec(uuid("BC719626-7CD0-4FF2-B9B4-6D821515C9E8")) HelperIcon;
#else
class __declspec(uuid("BC719626-7CD0-4FF2-B9B4-6D821515C9E9")) HelperIcon;
#endif 


VRDemoNotificationManager::VRDemoNotificationManager()
{
}


VRDemoNotificationManager::~VRDemoNotificationManager()
{
    if (m_deleteInfoThread.get() && m_deleteInfoThread->joinable()) {
        m_deleteInfoThread->join();
    }
}

void VRDemoNotificationManager::addNotificationIcon()
{
    NOTIFYICONDATA nid = { sizeof(nid) };

    // we should try to delete the icon first, it may not delete it when crashing
    this->deleteNotificationIcon();  

    nid.hWnd = m_wnd;
    // add the icon, setting the icon, tooltip, and callback message.
    // the icon will be identified with the GUID
    nid.uFlags = NIF_ICON | NIF_TIP | NIF_MESSAGE | NIF_SHOWTIP | NIF_GUID;
    nid.guidItem = __uuidof(HelperIcon);
    nid.uCallbackMessage = WMAPP_NOTIFYCALLBACK;
    LoadIconMetric(m_instance, MAKEINTRESOURCEW(IDI_NOTIFICATIONICON), LIM_SMALL, &nid.hIcon);
    LoadString(m_instance, IDS_TOOLTIP, nid.szTip, ARRAYSIZE(nid.szTip));
    Shell_NotifyIcon(NIM_ADD, &nid);

    // NOTIFYICON_VERSION_4 is prefered
    nid.uVersion = NOTIFYICON_VERSION_4;
    Shell_NotifyIcon(NIM_SETVERSION, &nid);
}

void VRDemoNotificationManager::deleteNotificationIcon()
{
    NOTIFYICONDATA nid = { sizeof(nid) };
    nid.uFlags = NIF_GUID;
    nid.guidItem = __uuidof(HelperIcon);
    Shell_NotifyIcon(NIM_DELETE, &nid);
}

void VRDemoNotificationManager::modifyNotificationIcon(bool pause)
{
    NOTIFYICONDATA nid = { sizeof(nid) };
    int iconId = IDI_NOTIFICATIONICON;

    if (pause) {
        iconId = IDI_NOTIFICATIONICON_PAUSED;
    }
    nid.hWnd = m_wnd;
    nid.uFlags = NIF_ICON | NIF_GUID;
    nid.guidItem = __uuidof(HelperIcon);
    LoadIconMetric(m_instance, MAKEINTRESOURCEW(iconId), LIM_SMALL, &nid.hIcon);
    Shell_NotifyIcon(NIM_MODIFY, &nid);
}


void VRDemoNotificationManager::addNotificationInfo(int infoStringId, unsigned int timeOut)
{
    NOTIFYICONDATA nid = { sizeof(nid) };

    this->deleteNotifcationInfo();
    nid.hWnd = m_wnd;
    nid.uFlags = NIF_INFO;
    if (LoadString(m_instance, infoStringId, nid.szInfo, ARRAYSIZE(nid.szInfo))) {
        Shell_NotifyIcon(NIM_ADD, &nid);
        if (m_deleteInfoThread.get() && m_deleteInfoThread->joinable()) {
            m_cancelFlag = true;
            m_event.signal();
            m_deleteInfoThread->join();
            m_cancelFlag = false;
        }
        m_timeOut = timeOut;
        m_deleteInfoThread.reset(new std::thread([this]() {
            m_event.timed_wait(m_timeOut);
            if (!m_cancelFlag) {
                deleteNotifcationInfo();
            }
        }));
    }
}

void VRDemoNotificationManager::deleteNotifcationInfo()
{
    NOTIFYICONDATA nid = { sizeof(nid) };
    nid.hWnd = m_wnd;
    nid.uFlags = NIF_INFO;
    Shell_NotifyIcon(NIM_DELETE, &nid);
}

void VRDemoNotificationManager::refreshNotificationArea() {
    HWND wnd = findNotificationBarWindow();
    if (wnd) {
        tagRECT rect;
        GetClientRect(wnd, &rect);

        for (LONG x = 0; x < rect.right; x += 5) {
            for (LONG y = 0; y < rect.bottom; y += 5) {
                PostMessage(
                    wnd,
                    WM_MOUSEMOVE,
                    0,
                    (y << 16) + x
                );
            }
        }
    }
}

HWND VRDemoNotificationManager::findNotificationBarWindow()
{
    HWND result = NULL;

    // use spy++ to get window hierarchy of the notification tool bar
    // NOTE: only tested on windows 8.1
    HWND trayWnd = FindWindow("Shell_TrayWnd", "");
    if (trayWnd) {
        HWND notifyWnd = FindWindowEx(trayWnd, NULL, "TrayNotifyWnd", "");
        if (notifyWnd) {
            HWND sysPagerWnd = FindWindowEx(notifyWnd, NULL, "SysPager", "");
            if (sysPagerWnd) {
                result = FindWindowEx(sysPagerWnd, NULL, "ToolbarWindow32", NULL);
            }
        }
    }
    return result;
}