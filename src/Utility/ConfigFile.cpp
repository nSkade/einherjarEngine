#include "ConfigFile.hpp"
#include <json/json.h>
#include <fstream>

namespace ehj {

void ConfigFile::baseStore() {
	Json::StreamWriterBuilder builder;
	builder["indentation"] = "		";
	std::unique_ptr<Json::StreamWriter> writer(builder.newStreamWriter());
	
	std::ofstream out(m_configFilepath);
	if (out.is_open()) {
		writer->write(m_ConfigData, &out);
		out << std::endl;
	}
}

void ConfigFile::baseLoad() {
	Json::CharReaderBuilder builder;
	std::ifstream in(m_configFilepath);
	std::string errs;
	if (in.is_open()) {
		Json::parseFromStream(builder, in, &m_ConfigData, &errs);
	}
}

}//ehj
