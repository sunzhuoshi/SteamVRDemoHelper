// dllmain.cpp : 定义 DLL 应用程序的入口点。

// TODO: fix the crash issue if it quits first when running together with Fraps
#include "stdafx.h"

#include <thread>

#include "OGET\OGraphicsAPIHookerFactory.h"
#include "VRDemoCore.h"

OIGraphicsAPIHooker* hooker = nullptr;
// hook CallWndProc to receive toggle benchmark message from VRDemoHelper
HHOOK callWndHook = NULL;

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
					 )
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
        hooker = OGraphicsAPIHookerFactory::createHooker();
        if (hooker) {
            hooker->hook();
            callWndHook = SetWindowsHookExA(WH_CALLWNDPROC, CallWndProc, NULL, GetCurrentThreadId());
        }
        break;
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
        break;
	case DLL_PROCESS_DETACH:
        if (callWndHook) {
            UnhookWindowsHookEx(callWndHook);
            callWndHook = NULL;
        }
        if (hooker) {
            hooker->unhook();
            delete hooker;
            hooker = nullptr;
        }
		break;
	}
	return TRUE;
}

