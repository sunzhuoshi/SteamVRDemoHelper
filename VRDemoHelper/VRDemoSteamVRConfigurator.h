#pragma once

#include <map>
#include <string>

#include "jsoncpp\json\json-forwards.h"

#include "util\l4Singleton.hpp"

class VRDemoSteamVRConfigurator : public l4Singleton<VRDemoSteamVRConfigurator>
{
public:
    typedef std::map<std::string, Json::Value> PROPERTY_MAP;
    VRDemoSteamVRConfigurator();
    ~VRDemoSteamVRConfigurator();
    bool init();
    bool isActive() { return m_inited; };
    // it may require restarting of SteamVR to take effect,
    // we'll pop a promote message box. 
    bool applySettings();
    // it may require restarting of SteamVR to take effect,.
    // but we will not pop a promote message box, try not to distube users
    bool restoreSettings();
    static const std::string SETTINGS_FILE_NAME;
    static const std::string SETTINGS_FILE_OLD_POSTFIX;
    static const std::string SETTINGS_FILE_RELATIVE_PATH;
    static const std::string SETTINGS_SECTION_NAME;

    static const std::string STEAM_VR_FILE_RELATIVE_PATH;
    static const std::string STEAM_VR_PROCESS_NAME;
    static const std::string STEAM_VR_REG_KEY;
private:
    bool loadConfigurations();
    void cleanObject(Json::Value &object);
    void restartSteamVR(DWORD processID);
    DWORD findSteamVRProcessID();
    Json::Value getSettingsItemValue(Json::Value &root, const std::string path);
    bool setSettingItemValue(Json::Value &root, const std::string path, const Json::Value& value);
    bool saveJsonToFile(const Json::Value &root, const std::string& filePath);

    bool m_inited = false;
    std::string m_settingFilePath;
    std::string m_steamVRFilePath;
    // NOTE: to simplify, "." is used for path delimiter
    // it will fail, if a key name in the path contain "." 
    PROPERTY_MAP m_targetSettings;
    PROPERTY_MAP m_oldSettings;
};

