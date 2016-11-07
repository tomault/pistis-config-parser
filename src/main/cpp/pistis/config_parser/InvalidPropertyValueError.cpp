#include "InvalidPropertyValueError.hpp"
#include "ConfigurationProperty.hpp"

using namespace pistis::config_parser;

InvalidPropertyValueError::InvalidPropertyValueError(
    const ConfigurationProperty& property
):
    ApplicationConfigurationError(
        property.source(), property.line(), 0,
	createMessage_(property.name(), std::string(), std::string())
    ) {
  // Intentionally left blank
}

InvalidPropertyValueError::InvalidPropertyValueError(
    const ConfigurationProperty& property,
    const std::string& value
):
    ApplicationConfigurationError(
        property.source(), property.line(), 0,
	createMessage_(property.name(), value, std::string())
    ) {
  // Intentionally left blank
}

InvalidPropertyValueError::InvalidPropertyValueError(
    const ConfigurationProperty& property,
    const std::string& value,
    const std::string& details
):
    ApplicationConfigurationError(
        property.source(), property.line(), 0,
	createMessage_(property.name(), value, details)
    ) {
  // Intentionally left blank
}

InvalidPropertyValueError::~InvalidPropertyValueError() noexcept {
  // Intentionally left blank
}

std::string InvalidPropertyValueError::createMessage_(
    const std::string& name, const std::string& value,
    const std::string& details
) {
  std::ostringstream msg;
  msg << "Invalid value";
  if (!value.empty()) {
    msg << " \"" << value << "\"";
  }
  msg << " for configuration property";
  if (!name.empty()) {
    msg << " " << name;
  }
  if (!details.empty()) {
    msg << " (" << details << ")";
  }
  return msg.str();
}
