#ifdef VRDEMOCORE_EXPORTS
#define VRDEMOCORE_API __declspec(dllexport)
#else
#define VRDEMOCORE_API __declspec(dllimport)
#endif

#include "VRDemoArbiter.h"

extern VRDemoArbiter::Toggles toogles;

extern "C" VRDEMOCORE_API LRESULT WINAPI fnWndMsgProc(int nCode, WPARAM wParam, LPARAM lParam);

extern "C" VRDEMOCORE_API BOOL WINAPI fnInit(const CHAR *szRuleConfigFilePath, BOOL bTrace);

extern "C" VRDEMOCORE_API VOID WINAPI fnSetToggleValue(INT nIndex, BOOL nValue);
