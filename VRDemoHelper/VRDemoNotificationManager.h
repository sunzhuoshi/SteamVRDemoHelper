#pragma once
#include <windef.h>

#include <thread>
#include <log4cplus\thread\threads.h>

class VRDemoNotificationManager
{
public:
    VRDemoNotificationManager();
    ~VRDemoNotificationManager();
    static VRDemoNotificationManager& VRDemoNotificationManager::getInstance() {
        static VRDemoNotificationManager instance;
        return instance;
    }
    inline void init(HINSTANCE instance, HWND wnd) {
        m_instance = instance;
        m_wnd = wnd;
    }
    void addNotificationIcon();
    void deleteNotificationIcon();
    void modifyNotificationIcon(bool pause);
    inline void addNotificationInfo(int infoStringId) {
        this->addNotificationInfo(infoStringId, NOTIFICATION_INFO_DEFAULT_TIME_OUT);
    }
    void addNotificationInfo(int infoStringId, unsigned int timeOut);
    void deleteNotifcationInfo();
    void refreshNotificationArea();
private:
    HWND findNotificationBarWindow();

    HINSTANCE       m_instance;
    HWND            m_wnd;
    std::unique_ptr<std::thread> m_deleteInfoThread;
    log4cplus::thread::ManualResetEvent m_event;
    bool            m_cancelFlag;
    unsigned int    m_timeOut;
    
    static const unsigned int NOTIFICATION_INFO_DEFAULT_TIME_OUT = 2000;
};

