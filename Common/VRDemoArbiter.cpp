#include "stdafx.h"
#include "VRDemoArbiter.h"

#include <string.h>
#include <sstream>

#include "util\l4util.h"
#include "VRDemoConfigurator.h"

const std::string VRDemoArbiter::SECTION_IGNORE_LIST = "A.IgnoreList";
const std::string VRDemoArbiter::SECTION_PREFIX_IMPROVE_STEAM_VR = "A.SteamVR";
const std::string VRDemoArbiter::SECTION_PREFIX_MAXIMIZE_GAMES = "A.Game";
const std::string VRDemoArbiter::SECTION_PREFIX_ALL = "A.";    // A stands for Arbiter


VRDemoArbiter::TokenMap VRDemoArbiter::s_ruleTypeTokenMap = {
    {VRDemoArbiter::RT_MESSAGE, "MESSAGE"},
    {VRDemoArbiter::RT_POLL, "POLL"},
    {VRDemoArbiter::RT_UNKNOWN, "UNKNOWN"}
};

VRDemoArbiter::TokenMap VRDemoArbiter::s_ruleMessageTokenMap = {
    {HCBT_ACTIVATE, "ACTIVATE"},
    {HCBT_CREATEWND, "CREATE"},
    {VRDemoArbiter::RM_UNKNOWN, "UNKNOWN"}
};

VRDemoArbiter::TokenMap VRDemoArbiter::s_ruleActionTokenMap = {
	{SW_MAXIMIZE, "MAX"},
	{SW_MINIMIZE, "MIN"},
	{SW_HIDE, "HIDE"},
	{WM_CLOSE, "CLOSE"},
	{VRDemoArbiter::RA_FULL, "FULL"},
    {VRDemoArbiter::RA_UNKNOWN, "UNKNOWN"}
};

bool VRDemoArbiter::arbitrate(RuleType type, int message, HWND wnd)
{
    bool result = false;

    if (!m_toggles->m_pause) {
        char className[MAX_PATH];
        RuleAction action = RA_UNKNOWN;
        DWORD processId = 0;

        GetWindowThreadProcessId(wnd, &processId);

        std::string processName = l4util::getProcessNameWithWindow(wnd);

        // if we can't get process name, then we can do less...
        if (!ifIgnore(processName) && processName.size()) {
            RealGetWindowClassA(wnd, className, MAX_PATH);

            RuleItemMap::const_iterator it = m_ruleItemMap.begin();
            while (it != m_ruleItemMap.end()) {
                BOOL okToGo = FALSE;
                if (l4util::keyStartWith(it->first, VRDemoArbiter::SECTION_PREFIX_MAXIMIZE_GAMES)) {
                    okToGo = m_toggles->m_maximizeGames;
                }
                else if (l4util::keyStartWith(it->first, VRDemoArbiter::SECTION_PREFIX_IMPROVE_STEAM_VR)) {
                    okToGo = m_toggles->m_improveSteamVR;
                }
                else {
                    okToGo = TRUE;
                }
                if (okToGo && type == it->second.m_type &&
                    0 == it->second.m_className.compare(className) &&
                    it->second.getMessage() == message) {
                    action = it->second.m_action;
                    break;
                }
                ++it;
            }
            if (it != m_ruleItemMap.end()) {
                performAction(wnd, it->second);
                result = true;
            }
        }
    }
    return result;
}

bool VRDemoArbiter::hasRuleWithType(RuleType ruleType) const
{
    bool result = false;
    for (const auto& it: m_ruleItemMap) {
        if (it.second.m_type == ruleType) {
            result = true;
            break;
        }
    }
    return result;
}



bool VRDemoArbiter::ifIgnore(const std::string &processName)
{
	bool result = false;

	for (const auto& it : m_ignoredProcessNameList) {
		if (0 == _stricmp(processName.c_str(), it.c_str())) {
			result = true;
			break;
		}
	}
	return result;
}

void VRDemoArbiter::performAction(HWND wnd, const RuleItem &ruleItem)
{
    switch (ruleItem.m_action) {
    case RA_FULL:
        performFullScreenAction(wnd, ruleItem);
        break;
    case RA_CLOSE:
        PostMessage(wnd, WM_CLOSE, 0, 0);
        break;
    case RA_HIDE:
    case RA_MAX:
    case RA_MIN:
        // show window commands
        ShowWindow(wnd, ruleItem.m_action);
        break;
    default:
        break;
    }
}


void VRDemoArbiter::performFullScreenAction(HWND wnd, const RuleItem &ruleItem)
{
	HWND desktopWindow = GetDesktopWindow();
	RECT desktopRect, windowRect;

	GetWindowRect(desktopWindow, &desktopRect);
	GetWindowRect(wnd, &windowRect);

	if (windowRect.left > desktopRect.left ||
		windowRect.right < desktopRect.right ||
		windowRect.top < desktopRect.top ||
		windowRect.bottom < desktopRect.bottom) {
		PostMessageA(wnd, WM_KEYDOWN, VK_MENU, 0);				// Post WM_KEYDOWN for Unreal games
		// TODO: check if char code or repeat is necessary
		PostMessageA(wnd, WM_SYSKEYDOWN, VK_RETURN, 1 << 29 | 0x001C0001);  // ALT down | char code = 1C | repeat = 1
		PostMessageA(wnd, WM_KEYUP, VK_MENU, 0);				// Post WM_KEYUP for Unreal games  
	}
}

int VRDemoArbiter::parseValue(const std::string &token, const TokenMap &tokenMap)
{
	int result = UNKNOWN_TOKEN;
	std::string trimmedToken = (token);

	for (const auto& it : tokenMap) {
		if (0 == _stricmp(it.second.c_str(), trimmedToken.c_str())) {
			result = it.first;
			break;
		}
	}
	return result;
}

bool VRDemoArbiter::init(const Toggles& toggles)
{
    bool result = false;

    for (auto sectionIt : VRDemoConfigurator::getInstance().getSections()) {
        if (l4util::keyStartWith(sectionIt.first, SECTION_PREFIX_ALL)) {
            if (l4util::matchKey(sectionIt.first, VRDemoArbiter::SECTION_IGNORE_LIST)) {
                for (auto propertyIt : sectionIt.second) {
                    m_ignoredProcessNameList.push_back(propertyIt.second);
                }
            }
            else {
                RuleItem ruleItem;
                ruleItem.m_ruleName = sectionIt.first;
                for (auto propertyIt : sectionIt.second) {
                    if (l4util::matchKey(propertyIt.first, "ClassName")) {
                        ruleItem.m_className = propertyIt.second;
                    }
                    else if (l4util::matchKey(propertyIt.first, "Type")) {
                        ruleItem.m_type = (RuleType)parseValue(propertyIt.second, VRDemoArbiter::s_ruleTypeTokenMap);
                    }
                    else if (l4util::matchKey(propertyIt.first, "Message")) {
                        ruleItem.setMessage((RuleMessage)parseValue(propertyIt.second, VRDemoArbiter::s_ruleMessageTokenMap));
                    }
                    else if (l4util::matchKey(propertyIt.first, "Action")) {
                        ruleItem.m_action = (RuleAction)parseValue(propertyIt.second, VRDemoArbiter::s_ruleActionTokenMap);
                    }
                }
                if (ruleItem.isValid()) {
                    m_ruleItemMap[sectionIt.first] = ruleItem;
                }
            }
        }
    }       
    result = !m_ruleItemMap.empty();
    if (result) {
        m_toggles = &toggles;
    }
	return result;
}

