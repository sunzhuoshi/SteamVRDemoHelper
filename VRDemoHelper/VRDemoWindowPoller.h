#pragma once
#include <log4cplus\thread\threads.h>

#include "VRDemoEventDispatcher.h"
#include "VRDemoArbiter.h"

/*
  Rule type "POLL" owner
*/
class VRDemoWindowPoller : public log4cplus::thread::AbstractThread
{
public:
    typedef log4cplus::helpers::SharedObjectPtr<VRDemoWindowPoller> VRDemoWindowPollerPtr;
    VRDemoWindowPoller();
    ~VRDemoWindowPoller();
    bool init(const VRDemoArbiter::Toggles& toggles);
    void run();
    void stop();
    static const int POLL_INTERVAL = 1000; // ms
private:
    static BOOL CALLBACK enumWindowsProc(HWND wnd, LPARAM param);
    BOOL realEnumChildProc(HWND wnd);
    bool m_runFlag;
    const VRDemoArbiter::Toggles *m_toggles;
    log4cplus::thread::ManualResetEvent m_pollEvent;
    log4cplus::thread::ManualResetEvent m_initEvent;
};

