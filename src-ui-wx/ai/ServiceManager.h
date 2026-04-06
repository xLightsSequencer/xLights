#pragma once

/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/
#include "aiType.h"

#include <string>
#include <vector>
#include <memory>
#include <span>

class xLightsFrame;
class aiBase;

//struct SERVICE_DEFINITION {
//    std::string name;
//    std::string idName;
//    std::string secretName1;
//    std::string secretName2;
//};

//static const std::list<SERVICE_DEFINITION> services = { { "ChatGPT", "", "BearerToken", "" } };

/*
const SERVICE_DEFINITION& FindService(const std::string& name) {
    for (const auto& service : services) {
        if (service.name == name) {
            return service;
        }
    }
    return services.front(); // this is dodgy but it should never happen
}

bool IsServiceValid(const std::string& service, const std::string& id, const std::string& secret1, const std::string& secret2) {
    const SERVICE_DEFINITION& svc = FindService(service);
    return (svc.idName == "" || !id.empty()) &&
           (svc.secretName1 == "" || !secret1.empty()) &&
           (svc.secretName2 == "" || !secret2.empty());
}
*/
class ServiceManager 
{
public:
	explicit ServiceManager(xLightsFrame* xl);
	~ServiceManager();

	[[nodiscard]] aiBase* getService(std::string const& serviceName);
    void addService(std::unique_ptr<aiBase> service);
    [[nodiscard]] aiBase* findService(aiType::TYPE serviceType);
    [[nodiscard]] std::vector<aiBase*> findServices(aiType::TYPE serviceType);


    void setServiceSetting(std::string const& key, int value);
    void setServiceSetting(std::string const& key, bool value);
	void setServiceSetting(std::string const& key, std::string const& value);

    [[nodiscard]] int getServiceSetting(std::string const& key, int defaultValue ) const;
    [[nodiscard]] bool getServiceSetting(std::string const& key, bool defaultValue) const;
    [[nodiscard]] std::string getServiceSetting(std::string const& key, std::string const& defaultValue) const;

    [[nodiscard]] std::string getSecretServiceToken(std::string const& service) const;
    void setSecretServiceToken(std::string const& service, std::string const& token);

    std::span<std::unique_ptr<aiBase> const> getServices() {
        return std::span{ m_services };
    }

private:
    std::vector<std::unique_ptr<aiBase>> m_services;

};
