// VRDemoCore.cpp : 定义 DLL 应用程序的导出函数。
//

#include "stdafx.h"
#include "VRDemoCore.h"

#include <log4cplus/log4cplus.h>

#include "util/l4util.h"
#include "VRDemoArbiter.h"
#include "VRDemoConfigurator.h"

#define HELPER_NAME "VRDemoHelper"				// used to tell if we're in helper process

BOOL bIsHelperProcess = FALSE;

// All instances share data segment
#pragma data_seg(".shared")
VRDEMOCORE_API CHAR szConfigFilePath[MAX_PATH] = "";
VRDEMOCORE_API BOOL bTrace = FALSE;
VRDEMOCORE_API VRDemoArbiter::Toggles toggles = { FALSE, TRUE, TRUE, TRUE };
#pragma data_seg()
#pragma comment(linker,"/section:.shared,rws")

BOOL fnIsHelperProcess()
{
	BOOL result = FALSE;
	CHAR szFileName[MAX_PATH];
	if (0 < GetModuleFileNameA(NULL, szFileName, MAX_PATH))
	{
		if (strstr(szFileName, HELPER_NAME))
		{
			result = TRUE;
		}
	}
	//result = false; // debug hack, it will cause crash
	return result;
}

void fnInitLog()
{
	if (bTrace) {
		log4cplus::SharedAppenderPtr append_1(new log4cplus::SocketAppender("127.0.0.1", 8888, "127.0.0.1"));
		append_1->setName(LOG4CPLUS_TEXT("First"));
		log4cplus::Logger::getInstance(VR_DEMO_LOGGER_SERVER).addAppender(append_1);
	}
}

void fnDelayInit()
{
	bIsHelperProcess = fnIsHelperProcess();
	// We do nothing when loaded by helper process(not necessary, either),
	// or it will cause file dead lock, for client and server will try to use the same SERVER file apender 
	if (!bIsHelperProcess) {
		fnInitLog();
		log4cplus::Logger logger = log4cplus::Logger::getInstance(VR_DEMO_LOGGER_SERVER);

        if (VRDemoConfigurator::getInstance().init(szConfigFilePath, VR_DEMO_LOGGER_SERVER)) {
            if (!VRDemoArbiter::getInstance().init(
                VR_DEMO_LOGGER_SERVER,
                toggles, 0 != bTrace)) 
            {
                LOG4CPLUS_ERROR(logger, "Failed to init VR Demo Arbiter");
            }
        }
        else {
            LOG4CPLUS_ERROR(logger, "Failed to init VR Demo Configurator, file path: " << szConfigFilePath);
        }
	}
}

// 这是导出函数的一个示例。
VRDEMOCORE_API LRESULT WINAPI fnWndMsgProc(int nCode, WPARAM wParam, LPARAM lParam)
{
	static BOOL bDelayInited = false;

	if (!bDelayInited) {
		fnDelayInit();
		bDelayInited = TRUE;
	}

	// we don't handle hook messages in helper process
	if (!bIsHelperProcess && !toggles.m_pause)
	{
		switch (nCode)
		{
		case HCBT_CREATEWND:
		case HCBT_ACTIVATE:
		{
			VRDemoArbiter::getInstance().arbitrate(VRDemoArbiter::RT_MESSAGE, nCode, (HWND)wParam);
		}
		break;
		default:
			break;
		}
	}
    return CallNextHookEx(NULL, nCode, wParam, lParam);
}

VRDEMOCORE_API BOOL WINAPI fnInit(const CHAR *szConfigFilePath_, BOOL bTrace_)
{
	strcpy_s(szConfigFilePath, MAX_PATH, szConfigFilePath_);
	// TODO: local config file content into share data segment
	bTrace = bTrace_;
	return TRUE;
}

VRDEMOCORE_API VOID WINAPI fnSetToggleValue(INT nIndex, BOOL nValue)
{
    if (VRDemoArbiter::TI_MIN <= nIndex && VRDemoArbiter::TI_MAX >= nIndex) {
        toggles.m_values[nIndex] = nValue;
    }
}

VRDEMOCORE_API VOID WINAPI fnSetShowFPS(BOOL nShowFPS)
{
    toggles.m_showFPS = nShowFPS;
}

