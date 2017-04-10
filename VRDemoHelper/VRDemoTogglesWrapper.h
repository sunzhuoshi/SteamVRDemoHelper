#pragma once

#include "VRDemoArbiter.h"
#include "VRDemoEventDispatcher.h"
#include "VRDemoNotificationManager.h"

class VRDemoTogglesWrapper {
public:
    inline bool getPause() { return 0 != m_toggles.m_pause; };
    inline bool getMaximmizeGames() { return 0 != m_toggles.m_maximizeGames; };
    inline bool getHideSteamVrNotification() { return 0 != m_toggles.m_hideSteamVrNotifcation; };
    inline bool getShowFPS() { return 0 != m_toggles.m_showFPS; };
    inline void toggleValue(INT toggleIndex) {
        if (VRDemoArbiter::TI_MIN <= toggleIndex && VRDemoArbiter::TI_MAX >= toggleIndex) {
            m_toggles.m_values[toggleIndex] = !m_toggles.m_values[toggleIndex];
            VRDemoEventDispatcher::getInstance().dispatchEvent(
                VRDemoEventDispatcher::EV_TOGGLE_VALUE_CHANGED,
                toggleIndex,
                m_toggles.m_values[toggleIndex]
            );
        }
    }
    inline void togglePause() {
        toggleValue(VRDemoArbiter::TI_PAUSE);
        // quick hack here, no event listener used
        VRDemoNotificationManager::getInstance().modifyNotificationIcon(
            0 != m_toggles.m_pause
        );
    }
    inline void toggleMaximizeGames() {
        toggleValue(VRDemoArbiter::TI_MAXIMIZE_GAMES);
    };
    inline void toggleHideSteamVrNotification() {
        toggleValue(VRDemoArbiter::TI_HIDE_STEAM_VR_NOTIFICATION);
    }
    inline void toggleShowFPS() {
        toggleValue(VRDemoArbiter::TI_SHOW_FPS);
    }
    inline void toggleShowFPS() {
        m_toggles.m_showFPS = !m_toggles.m_showFPS;
        VRDemoEventDispatcher::getInstance().dispatchEvent(
            VRDemoEventDispatcher::EV_SHOW_FPS_CHANGED,
            m_toggles.m_showFPS
        );
    }
    VRDemoTogglesWrapper() :
        m_toggles({ FALSE, TRUE, TRUE, TRUE }) {
    }
    const VRDemoArbiter::Toggles& getToggles() {
        return m_toggles;
    }
private:
    VRDemoArbiter::Toggles m_toggles;
};

