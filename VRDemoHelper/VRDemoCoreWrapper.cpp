#include "stdafx.h"
#include "VRDemoCoreWrapper.h"

#include <log4cplus\log4cplus.h>

#include "util\l4util.h"
#include "VRDemoArbiter.h"

#ifdef _WIN64
const std::string VRDemoCoreWrapper::FILE_HOOK_DLL = "VRDemoCore.dll"; // TODO: rename to "VRDemoCore_x64.dll"
#else
const std::string VRDemoCoreWrapper::FILE_HOOK_DLL = "VRDemoCore_x86.dll";
#endif
const std::string VRDemoCoreWrapper::FILE_SETTINGS = "settings.ini";
const std::string VRDemoCoreWrapper::FUNCTION_INIT = "fnInit";
const std::string VRDemoCoreWrapper::FUNCTION_HOOK_PROC = "fnWndMsgProc";
const std::string VRDemoCoreWrapper::FUNCTION_SET_TOGGLE_VALUE = "fnSetToggleValue";

VRDemoCoreWrapper::VRDemoCoreWrapper():
    m_dll(nullptr),
    m_hook(nullptr),
    m_setToggleValueFunc(nullptr)
{
}


VRDemoCoreWrapper::~VRDemoCoreWrapper()
{
    if (m_hook) {
        UnhookWindowsHookEx(m_hook);
    }
    if (m_dll) {
        FreeLibrary(m_dll);
    }
}

bool VRDemoCoreWrapper::init(BOOL trace)
{
    bool result = false;
    log4cplus::Logger logger = log4cplus::Logger::getInstance(VR_DEMO_LOGGER_CLIENT);

    m_dll = LoadLibrary(l4util::getFileFullPath(FILE_HOOK_DLL).c_str());
    if (m_dll) {
        InitFuncPtr initFunc = (InitFuncPtr)GetProcAddress(m_dll, FUNCTION_INIT.c_str());
        HOOKPROC hookProc = (HOOKPROC)GetProcAddress(m_dll, FUNCTION_HOOK_PROC.c_str());
        SetToggleValueFuncPtr setToggleValueFunc = (SetToggleValueFuncPtr)GetProcAddress(m_dll, FUNCTION_SET_TOGGLE_VALUE.c_str());

        if (initFunc && hookProc && setToggleValueFunc) {
            if (initFunc(l4util::getFileFullPath(FILE_SETTINGS).c_str(), trace)) {
                m_hook = SetWindowsHookEx(
                    WH_CBT,
                    hookProc,
                    m_dll,
                    0
                );
                if (m_hook) {
                    m_setToggleValueFunc = setToggleValueFunc;

                    VRDemoEventDispatcher::getInstance().addEventListener(
                        VRDemoEventDispatcher::EV_TOGGLE_VALUE_CHANGED,
                        VRDemoEventDispatcher::VRDemoEventListenerPtr(this)
                    );
                    result = true;
                }
                else {
                    LOG4CPLUS_ERROR(logger, "Failed to hook, errno: " << GetLastError());
                }
            }
            else {
                LOG4CPLUS_ERROR(logger, "Failed to init in core module");
            }
        }
        else {
            LOG4CPLUS_ERROR(logger, "Invalid core exports, check your dll version");
        }
    }
    return result;
}

void VRDemoCoreWrapper::handleEvent(int event, unsigned long long param1, unsigned long long param2)
{
    BOOL value = 0 != param2;
    switch (event) {
    case VRDemoEventDispatcher::EV_TOGGLE_VALUE_CHANGED:
        m_setToggleValueFunc(static_cast<int>(param1), value);
        break;
    case VRDemoEventDispatcher::EV_SHOW_FPS_CHANGED:
        m_setShowFPSFunc(value);
        break;
    default:
        break;
    }
}
