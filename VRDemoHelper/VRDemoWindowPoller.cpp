#include "stdafx.h"
#include "VRDemoWindowPoller.h"

#include <log4cplus\log4cplus.h>
#include <Windows.h>
#include "VRDemoArbiter.h"
#include "util\l4util.h"

VRDemoWindowPoller::VRDemoWindowPoller() :
    m_runFlag(true)
{
    start();
}


VRDemoWindowPoller::~VRDemoWindowPoller()
{
}

bool VRDemoWindowPoller::init(const VRDemoArbiter::Toggles& toggles)
{
    bool result = false;

    if (VRDemoArbiter::getInstance().init(toggles)) {
        m_toggles = &toggles;
        m_initEvent.signal();
        result = true;
    }
    return result;
}

void VRDemoWindowPoller::run()
{
    m_initEvent.wait();
    while (m_runFlag)
    {
        m_pollEvent.timed_wait(POLL_INTERVAL);
        if (m_runFlag && !m_toggles->m_pause) {
            EnumWindows(&VRDemoWindowPoller::enumWindowsProc, (LPARAM)this);
        }
    }
}

void VRDemoWindowPoller::stop()
{
    m_runFlag = false;
    m_initEvent.signal();
    m_pollEvent.signal();
    this->join();
}

BOOL CALLBACK VRDemoWindowPoller::enumWindowsProc(HWND wnd, LPARAM param)
{
    VRDemoWindowPoller *poller = (VRDemoWindowPoller *)param;
    return poller->realEnumChildProc(wnd);
}

BOOL VRDemoWindowPoller::realEnumChildProc(HWND wnd)
{
    BOOL cont = FALSE;
    if (m_runFlag) {
        DWORD processId;

        GetWindowThreadProcessId(wnd, &processId);
        // handle other processes only
        if (GetCurrentProcessId() != processId) {
            VRDemoArbiter::getInstance().arbitrate(VRDemoArbiter::RT_POLL, VRDemoArbiter::RM_UNKNOWN, wnd);
        }
        cont = TRUE;
    }
    return cont;
}

