#pragma once

#include "VRDemoArbiter.h"
#include "VRDemoEventDispatcher.h"
#include "VRDemoNotificationManager.h"
#include "VRDemoSteamVRConfigurator.h"

class VRDemoTogglesWrapper {
public:
    inline bool getPause() { return 0 != m_toggles.m_pause; };
    inline bool getMaximmizeGames() { return 0 != m_toggles.m_maximizeGames; };
    inline bool getImproveSteamVR() { return 0 != m_toggles.m_improveSteamVR; };
    inline bool getShowFPS() { return 0 != m_toggles.m_showFPS; };
    inline void setValue(INT toggleIndex, BOOL value) {
        if (VRDemoArbiter::TI_MIN <= toggleIndex && VRDemoArbiter::TI_MAX >= toggleIndex) {
            BOOL oldValue = m_toggles.m_values[toggleIndex];

            m_toggles.m_values[toggleIndex] = value;
            if (oldValue != value) {
                VRDemoEventDispatcher::getInstance().dispatchEvent(
                    VRDemoEventDispatcher::EV_TOGGLE_VALUE_CHANGED,
                    toggleIndex,
                    m_toggles.m_values[toggleIndex]
                );
            }
        }
    }
    inline void setConfigurateVRNotification(BOOL value) {
        setValue(VRDemoArbiter::TI_IMPROVE_STEAM_VR, value);
    }
    inline INT toggleValue(INT toggleIndex) {
        BOOL newValue = -1;
        if (VRDemoArbiter::TI_MIN <= toggleIndex && VRDemoArbiter::TI_MAX >= toggleIndex) {
            newValue = !m_toggles.m_values[toggleIndex];
            setValue(toggleIndex, newValue);
            VRDemoEventDispatcher::getInstance().dispatchEvent(
                VRDemoEventDispatcher::EV_TOGGLE_VALUE_CHANGED,
                toggleIndex,
                m_toggles.m_values[toggleIndex]
            );
        }
        return newValue;
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
    inline void toggleImproveSteamVR() {
        INT improveSteamVR =  toggleValue(VRDemoArbiter::TI_IMPROVE_STEAM_VR);

        switch (improveSteamVR) {
        case TRUE:
            VRDemoSteamVRConfigurator::getInstance().applySettings();
            break;
        case FALSE:
            // We'll not restore settings file in running,
            // only restore it when it quits normally.
            //VRDemoSteamVRConfigurator::getInstance().restoreSettings();
            break;
        default:
            break;
        }
    }
    inline void toggleShowFPS() {
        toggleValue(VRDemoArbiter::TI_SHOW_FPS);
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

