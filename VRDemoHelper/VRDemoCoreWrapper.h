#pragma once
#include <VRDemoEventDispatcher.h>
#include <log4cplus\helpers\pointer.h>

class VRDemoCoreWrapper : public IVRDemoEventListener
{
public:   
    typedef log4cplus::helpers::SharedObjectPtr<VRDemoCoreWrapper> VRDemoCoreWrapperPtr;
    VRDemoCoreWrapper();
    ~VRDemoCoreWrapper();
    bool init();
    void handleEvent(int event, unsigned long long param1, unsigned long long param2);
private:
    typedef BOOL(*InitFuncPtr)(const CHAR *);

    typedef VOID(*SetToggleValueFuncPtr)(INT, BOOL);

    HINSTANCE m_dll;
    HHOOK m_hook;

    SetToggleValueFuncPtr m_setToggleValueFunc;

    static const std::string FILE_HOOK_DLL;
    static const std::string FILE_SETTINGS;
    static const std::string FUNCTION_INIT;
    static const std::string FUNCTION_HOOK_PROC;
    static const std::string FUNCTION_SET_TOGGLE_VALUE;
};

