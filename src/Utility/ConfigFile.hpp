#pragma once

#include <json/value.h>
#include <string>

#define EHJ_SERIALIZER_DECLARATION_ONLY
#include "ConfigFileSerializers.cpp"
#undef EHJ_SERIALIZER_DECLARATION_ONLY

namespace ehj {

class ConfigFile {
public:
	void baseStore();
	void baseLoad();

	template <typename T>
	void store(const std::string& name, const T& value) {
		m_ConfigData[name] = ConfigFileSerializer<T>::to(value);
	}

	template <typename T>
	void load(const std::string& name, T* value) {
		if (m_ConfigData.isMember(name)) {
			*value = ConfigFileSerializer<T>::from(m_ConfigData[name]);
		}
	}

private:
	Json::Value m_ConfigData;
	std::string m_configFilepath = "bin/ehjEconfig.json";
};

}//ehj
