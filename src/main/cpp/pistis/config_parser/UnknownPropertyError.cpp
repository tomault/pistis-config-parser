#include "UnknownPropertyError.hpp"

using namespace pistis::config_parser;

UnknownPropertyError::UnknownPropertyError(const std::string& sourceName,
					   int line,
					   const std::string& propertyName):
    ApplicationConfigurationError(sourceName, line, 1,
				  "Unknown configuration property \"" +
				  propertyName + "\"") {
  // Intentionally left blank
}

UnknownPropertyError::~UnknownPropertyError() noexcept {
  // Intentionally left blank
}

