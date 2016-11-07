#include "RequiredPropertyMissingError.hpp"

using namespace pistis::config_parser;

RequiredPropertyMissingError::RequiredPropertyMissingError(
    const std::string& sourceName,
    const std::string& propertyName
):
    ApplicationConfigurationError(sourceName, 0, 0,
				  "Required property \"" + propertyName +
				  "\" missing") {
  // Intentionally left blank
}

RequiredPropertyMissingError::~RequiredPropertyMissingError() noexcept {
  // Intentionally left blank
}

