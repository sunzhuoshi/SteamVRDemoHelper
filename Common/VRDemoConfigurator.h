#pragma once

#include <list>
#include <map>
#include <string>

#include "util\l4util.h"

class VRDemoConfigurator
{
public:
    typedef std::map<std::string, std::string> KeyValueMap;
    typedef std::map<std::string, KeyValueMap> SectionMap;

    static VRDemoConfigurator& VRDemoConfigurator::getInstance() {
        static VRDemoConfigurator instance;
        return instance;
    }
    bool init(const std::string& configFilePath);
    const SectionMap& getSections() const;
    bool findSection(const std::string& sectionName, const KeyValueMap **map) const;
    static const std::string FILE_SETTINGS;
private:
    inline bool isInited() const { return 0 < m_configFilePath.size(); };
    void parseSection(const std::string& configFilePath, const std::string& sectionName);
    SectionMap m_sectionMap;
    std::string m_configFilePath;
};