#include "ConfigurationPropertyMap.hpp"
#include <pistis/exceptions/NoSuchItem.hpp>

using namespace pistis::exceptions;
using namespace pistis::config_parser;

ConfigurationPropertyMap::ConfigurationPropertyMap():
    properties_() {
  // Intentionally left blank
}

ConfigurationPropertyMap::ConfigurationPropertyMap(
    ConfigurationPropertyMap&& other
):
    properties_(std::move(other.properties_)) {
  // Intentionally left blank
}

ConfigurationPropertyMap::~ConfigurationPropertyMap() {
  // Intentionally left blank
}

std::vector<ConfigurationProperty>
    ConfigurationPropertyMap::getPropertiesWithPrefix(
	const std::string& prefix
    ) const {
  std::vector<ConfigurationProperty> result;
  getPropertiesWithPrefix(prefix, [&result](const ConfigurationProperty& p) {
    result.push_back(p);
  });
  return std::move(result);
}

void ConfigurationPropertyMap::add(const ConfigurationProperty& p) {
  auto i= properties_.find(p.name());
  if (i != properties_.end()) {
    i->second= p;
  } else {
    properties_.insert(std::make_pair(p.name(), p));
  }
}

void ConfigurationPropertyMap::add(ConfigurationProperty&& p) {
  auto i= properties_.find(p.name());
  if (i != properties_.end()) {
    i->second= std::move(p);
  } else {
    properties_.insert(
	 std::pair<std::string, ConfigurationProperty>(p.name(), p)
    );
  }
}

const ConfigurationProperty& ConfigurationPropertyMap::operator[](
    const std::string& key
) const {
  auto i= properties_.find(key);
  if (i == properties_.end()) {
    throw NoSuchItem("Property with name \"" + key + "\"", PISTIS_EX_HERE);
  }
  return i->second;
}
