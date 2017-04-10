#pragma once
#include <sstream>
#include <string>
#include <map>
#include <list>

#include <log4cplus\log4cplus.h>

class VRDemoArbiter
{
public:
    enum ToggleIndex {
        TI_PAUSE = 0,
        TI_MAXIMIZE_GAMES = 1,
        TI_HIDE_STEAM_VR_NOTIFICATION = 2,
        TI_SHOW_FPS = 3
    };
    static const int TI_MIN = TI_PAUSE;
    static const int TI_MAX = TI_SHOW_FPS;
    union Toggles {
        struct {
            BOOL m_pause;
            BOOL m_maximizeGames;
            BOOL m_hideSteamVrNotifcation;
            BOOL m_showFPS;
        };
        BOOL m_values[TI_MAX+1];
    };
    
	static const int UNKNOWN_TOKEN = -1;
	enum RuleMessage {
		RM_UNKNOWN = UNKNOWN_TOKEN,
		RM_ACTIVATE = HCBT_CREATEWND,
		RM_CREATE = HCBT_ACTIVATE
	};
	enum RuleAction {
		RA_UNKNOWN = UNKNOWN_TOKEN,
		RA_MAX = SW_MAXIMIZE,
		RA_MIN = SW_MINIMIZE,
		RA_HIDE = SW_HIDE,
		RA_FULL = SW_MAX + 1,
		RA_CLOSE = WM_CLOSE,
	};
    enum RuleType {
        RT_UNKNOWN = UNKNOWN_TOKEN,
        RT_MESSAGE,
        RT_POLL
    };
	struct RuleItem {
		std::string m_ruleName;
		std::string m_className;
        RuleType    m_type;
		RuleAction  m_action;
    private:
        RuleMessage m_message;
    public:
		RuleItem() : m_message(RM_UNKNOWN),
			m_action(RA_UNKNOWN),
            m_type(RT_UNKNOWN)
		{};
        RuleMessage getMessage() const { return m_message; };
        void setMessage(RuleMessage message) {
            m_message = message;
            if (RM_UNKNOWN != m_message && RT_UNKNOWN == m_type) {
                m_type = RT_MESSAGE;
            }
        }
		bool isValid() {
            bool valid = false;

            if (!m_ruleName.empty()) {
                switch (m_type) {
                    case RT_POLL:
                        if (!m_className.empty() && RA_UNKNOWN != m_action) {
                            valid = true;
                        }
                        break;
                    case RT_MESSAGE:
                        if (!m_className.empty() && RA_UNKNOWN != m_action && RM_UNKNOWN != m_message) {
                            valid = true;
                        }
                        break;
                    default:
                        break;
                }
            }
            return valid;
        };
        std::string toString() const {
            std::ostringstream buf;
            buf << "{ name: " << m_ruleName <<
                ", className: " << m_className <<
                ", type: " << s_ruleTypeTokenMap[m_type] <<
                ", message: " << s_ruleMessageTokenMap[m_message] <<
                ", action: " << s_ruleActionTokenMap[m_action];
            return buf.str();
        };
	};
private:
    typedef std::map<int, std::string> TokenMap;
    typedef std::list<std::string> NameList;
    typedef std::map<std::string, RuleItem> RuleItemMap;
public:
    VRDemoArbiter::VRDemoArbiter() :
        m_toggles(nullptr),
        m_trace(false) {
    };
    VRDemoArbiter::~VRDemoArbiter() {};
    static VRDemoArbiter& VRDemoArbiter::getInstance() {
        static VRDemoArbiter instance;
        return instance;
    }
    bool init(const std::string &loggerName, const Toggles &toggles, bool trace);
    bool arbitrate(RuleType type, int message, HWND wnd);
    inline bool hasRuleWithTypePoll() const {
        return hasRuleWithType(RT_POLL);
    }
    inline bool hasRuleWithTypeMessage() const {
        return hasRuleWithType(RT_MESSAGE);
    }
    bool hasRuleWithType(RuleType ruleType) const;
    static const std::string SECTION_PREFIX_MAXIMIZE_GAMES;
    static const std::string SECTION_PREFIX_HIDE_STEAM_VR_NOTIFICATION;
    static const std::string SECTION_PREFIX_ALL;
private:
	bool ifIgnore(const std::string &processName);

    void performAction(HWND wnd, const RuleItem &ruleItems);
	void performFullScreenAction(HWND wnd, const RuleItem &ruleItem);
    void performShowWindowAction(HWND wnd, const RuleItem &ruleItem);

    int parseValue(const std::string &token, const TokenMap &tokenMap);

    static TokenMap s_ruleTypeTokenMap;
    static TokenMap s_ruleMessageTokenMap;
    static TokenMap s_ruleActionTokenMap;
    static const std::string SECTION_IGNORE_LIST;

    NameList m_ignoredProcessNameList; // TODO: use set
    RuleItemMap m_ruleItemMap;
    log4cplus::Logger m_logger;

    const Toggles *m_toggles;
    bool m_trace;
};

