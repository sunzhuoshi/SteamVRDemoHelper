#include "stdafx.h"
#include "VRDemoSteamVRConfigurator.h"

#include <list>
#include <fstream>
#include <vector>
#include <Psapi.h>

#include "log4cplus\log4cplus.h"
#include "jsoncpp\json\json.h"
#include "util\l4util.h"
#include "VRDemoDefine.h"
#include "VRDemoNotificationManager.h"
#include "VRDemoTogglesWrapper.h"
#include "VRDemoConfigurator.h"

extern VRDemoTogglesWrapper togglesWrapper;

const std::string VRDemoSteamVRConfigurator::SETTINGS_FILE_NAME = "steamvr.vrsettings";
const std::string VRDemoSteamVRConfigurator::SETTINGS_FILE_OLD_POSTFIX = ".old";
const std::string VRDemoSteamVRConfigurator::SETTINGS_FILE_RELATIVE_PATH = "config\\" + VRDemoSteamVRConfigurator::SETTINGS_FILE_NAME;
const std::string VRDemoSteamVRConfigurator::SETTINGS_SECTION_NAME = "SteamVRSettings";

const std::string VRDemoSteamVRConfigurator::STEAM_VR_FILE_RELATIVE_PATH = "steamapps\\common\\SteamVR\\tools\\bin\\win32\\vrmonitor.exe";
const std::string VRDemoSteamVRConfigurator::STEAM_VR_PROCESS_NAME = "vrmonitor.exe";
const std::string VRDemoSteamVRConfigurator::STEAM_VR_REG_KEY = "vrmonitor\\Shell\\Open\\Command";

VRDemoSteamVRConfigurator::VRDemoSteamVRConfigurator()
{
}


VRDemoSteamVRConfigurator::~VRDemoSteamVRConfigurator()
{
}

bool VRDemoSteamVRConfigurator::init()
{
    DWORD bufSize;
    LSTATUS ret = RegGetValueA(HKEY_CLASSES_ROOT, STEAM_VR_REG_KEY.c_str(), "", RRF_RT_REG_SZ, NULL, NULL, &bufSize);

    if (ERROR_SUCCESS == ret) {
        try {
            char* buf = new char[bufSize];
            ret = RegGetValueA(HKEY_CLASSES_ROOT, STEAM_VR_REG_KEY.c_str(), "", RRF_RT_REG_SZ, NULL, buf, &bufSize);
            if (ERROR_SUCCESS == ret) {
                size_t markIndex = std::string(buf).rfind("steamapps");
                std::ostringstream settingFilePath;
                std::ostringstream steamVRFilePath;

                if (std::string::npos != markIndex) {
                    // " SHOULD be the first letter
                    std::string steamRootPath = std::string(buf).substr(1, markIndex - 1);
                    settingFilePath << steamRootPath << SETTINGS_FILE_RELATIVE_PATH;
                    steamVRFilePath << steamRootPath << STEAM_VR_FILE_RELATIVE_PATH;

                    std::ifstream fileSetting(settingFilePath.str().c_str());
                    std::ifstream fileSteamVR(steamVRFilePath.str().c_str());

                    if (fileSetting.good() && fileSteamVR.good()) {
                        if (loadConfigurations()) {
                            m_settingFilePath = settingFilePath.str();
                            m_steamVRFilePath = steamVRFilePath.str();
                            m_inited = true;
                            if (togglesWrapper.getImproveSteamVR()) {
                                applySettings();
                            }
                        }
                        else {
                            LOG4CPLUS_WARN(log4cplus::Logger::getRoot(), "Failed to load Steam VR configurations, check " << VRDemoConfigurator::FILE_SETTINGS << ".");
                        }
                    }
                }
            }
        }
        catch (const std::exception& e) {
            OutputDebugStringA(e.what());
        }
        catch (...) {
        }
    }
    return m_inited;
}

bool VRDemoSteamVRConfigurator::applySettings()
{
    Json::Value root(Json::objectValue);
    Json::Value oldRoot(Json::objectValue);

    if (!m_inited) {
        return false;
    }

    std::ifstream settingFileInputStream(m_settingFilePath);

    Json::Reader reader;

    if (settingFileInputStream.good()) {
        if (reader.parse(settingFileInputStream, root)) {
            // draw the existing properties
            for (const auto& it : m_targetSettings) {
                Json::Value value = getSettingsItemValue(root, it.first);
                if (!value.isNull()) {
                    setSettingItemValue(oldRoot, it.first, value);
                }
            }
        }
        else {
            // setting file exists, but invalid
            // we can't do anything, so do nothing
            return false;
        }
        settingFileInputStream.close();
    }
    bool changed = false;
    for (const auto& it : m_targetSettings) {
        changed |= setSettingItemValue(root, it.first, it.second);
    }
    if (changed) {
        // save old values into file, for restoration later.
        saveJsonToFile(oldRoot, m_settingFilePath + SETTINGS_FILE_OLD_POSTFIX);

        if (saveJsonToFile(root, m_settingFilePath)) {
            DWORD steamVRProcessID = findSteamVRProcessID();
            if (0 < steamVRProcessID) {
                if (IDYES == MessageBoxA(NULL,
                    "Some settings will not take effect without restarting SteamVR.\nRestart it now?",
                    "Info",
                    MB_YESNO
                )) {
                    restartSteamVR(steamVRProcessID);
                }
            }
        }
    }
    return true;
}

bool VRDemoSteamVRConfigurator::restoreSettings()
{
    if (!m_inited) {
        return false;
    }
    Json::Reader settingsReader, oldSettingsReader;
    std::ifstream fileSettings(m_settingFilePath), fileOldSettings(m_settingFilePath + SETTINGS_FILE_OLD_POSTFIX);
    Json::Value root, oldRoot;
    bool updated = false;

    if (settingsReader.parse(fileSettings, root) && oldSettingsReader.parse(fileOldSettings, oldRoot)) {
        for (const auto& it : m_targetSettings) {
            updated |= setSettingItemValue(root, it.first, getSettingsItemValue(oldRoot, it.first));
        }
        if (updated) {
            cleanObject(root);
            return saveJsonToFile(root, m_settingFilePath);
        }
    }
    return false;
}

DWORD VRDemoSteamVRConfigurator::findSteamVRProcessID()
{
    DWORD result = 0;
    DWORD processIDs[1024], bytesReturned;

    if (EnumProcesses(processIDs, sizeof(processIDs), &bytesReturned)) {
        size_t processCount = bytesReturned / sizeof(processIDs[0]);
        for (int i = 0; i < processCount; ++i) {
            if (l4util::getProcessNameWithProcessId(processIDs[i]) == STEAM_VR_PROCESS_NAME) {
                result = processIDs[i];
                break;
            }
        }
    }
    return result;
}

Json::Value VRDemoSteamVRConfigurator::getSettingsItemValue(Json::Value &root, const std::string path)
{
    Json::Value *tmp = &root;
    std::vector<std::string> pathArray;

    l4util::split(path, '.', pathArray);
    for (auto &it : pathArray) {
        if (tmp->isMember(it)) {
            tmp = &(*tmp)[it];
        }
        else {
            tmp = nullptr;
            break;
        }
    }
    if (tmp) {
        return *tmp;
    }
    else {
        return Json::Value::nullSingleton();
    }
}

bool VRDemoSteamVRConfigurator::setSettingItemValue(Json::Value &root, const std::string path, const Json::Value& value)
{
    bool updated = false;
    Json::Value *tmp = &root;
    std::vector<std::string> pathArray;
    l4util::split(path, '.', pathArray);
    size_t keyHierarchy = pathArray.size();
   
    if (0 < keyHierarchy) {
        for (size_t i = 0; i < keyHierarchy - 1; ++i) {
            std::string key = pathArray[i];
            if (!tmp->isMember(key)) {
                (*tmp)[key] = Json::Value(Json::objectValue);
            }
            tmp = &(*tmp)[key];
        }
        std::string leafKey = pathArray[keyHierarchy - 1];

        if (!tmp->isMember(leafKey) || 0 != (tmp->get(leafKey, Json::Value::nullSingleton()).compare(value))) {
            if (value.isNull()) {
                tmp->removeMember(leafKey);
            }
            else {
                (*tmp)[leafKey] = value;
            } 
            updated = true;
        }
    }
    return updated;
}

bool VRDemoSteamVRConfigurator::saveJsonToFile(const Json::Value &root, const std::string& filePath)
{
    Json::StreamWriterBuilder builder;
    std::ofstream file(filePath);

    builder["commentStyle"] = "All";
    std::unique_ptr<Json::StreamWriter> writer(builder.newStreamWriter());
    writer->write(root, &file);
    file.flush();
    file.close();
    return !file.fail();
}

bool VRDemoSteamVRConfigurator::loadConfigurations()
{
    bool result = false;
    const VRDemoConfigurator::KeyValueMap *itemMap = nullptr;

    if (VRDemoConfigurator::getInstance().findSection("SteamVRSettings", &itemMap)) {
        for (auto& it : *itemMap) {
            std::string valueString = l4util::toUpper(l4util::trim(it.second));
            Json::Value value(it.second);

            if ("TRUE" == valueString || "FALSE" == valueString) {  // boolean
                m_targetSettings[it.first] = ("TRUE" == valueString);
            }
            else {
                // others treated as a string
                // TODO: integer support needed?
                m_targetSettings[it.first] = value;
            }
        }
        result = true;
    }
    return result;
}

void VRDemoSteamVRConfigurator::cleanObject(Json::Value& object)
{
    if (object.type() == Json::objectValue) {
        for (const auto& key : object.getMemberNames()) {
            Json::Value& member = object[key];
            cleanObject(member);
            if (member.type() == Json::objectValue && 0 == member.getMemberNames().size()) {
                object.removeMember(key);
            }
        }
    }
}

void VRDemoSteamVRConfigurator::restartSteamVR(DWORD processID)
{
    HANDLE processHandle = OpenProcess(PROCESS_TERMINATE, FALSE, processID);
    if (processHandle) {
        if (TerminateProcess(processHandle, 0)) {
            switch (WaitForSingleObject(processHandle, INFINITE)) {
            case WAIT_OBJECT_0:     // terminate slowly 
            case WAIT_FAILED:       // terminate quickly
            {
                STARTUPINFO si;
                PROCESS_INFORMATION pi;

                ZeroMemory(&si, sizeof(si));
                si.cb = sizeof(si);
                ZeroMemory(&pi, sizeof(pi));

                // we need to refresh notificaiton area
                // for notification icon will not disappear automatically if we just kill the process 
                // it is the lazy solution, but I like it. simple and works. :)
                VRDemoNotificationManager::getInstance().refreshNotificationArea();

                if (!CreateProcessA(m_steamVRFilePath.c_str(),   
                    NULL,                   
                    NULL,                   
                    NULL,                   
                    FALSE,                  
                    0,                      
                    NULL,                   
                    NULL,                   
                    &si,                    
                    &pi)                    
                    )
                {
                    VR_DEMO_ALERT_SS("Error", "Failed to start SteamVR, please start it manually.");
                }
            }
            break;
            default:
                break;
            }
        }
        else {
            VR_DEMO_ALERT_SS("Error", "Failed to terminate SteamVR, please restart it manually.");
        }
        CloseHandle(processHandle);
    }
}
