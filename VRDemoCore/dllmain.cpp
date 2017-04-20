// dllmain.cpp : 定义 DLL 应用程序的入口点。

// TODO: fix the crash issue if it quits first when running together with Fraps
#include "stdafx.h"

#include <thread>

#include "OGET\OGraphicsAPIHookerFactory.h"

OIGraphicsAPIHooker* hooker = nullptr;

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
        }
        break;
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
        break;
	case DLL_PROCESS_DETACH:
        if (hooker) {
            hooker->unhook();
            delete hooker;
        }
		break;
	}
	return TRUE;
}

