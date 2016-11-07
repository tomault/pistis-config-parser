#include "ConfigFileParseError.hpp"
#include <sstream>

using namespace pistis::exceptions;
using namespace pistis::config_parser;

ConfigFileParseError::ConfigFileParseError(const std::string& sourceName,
					   int line,
					   const std::string& description):
    ApplicationConfigurationError(sourceName, line, 0, description) {
  // Intentionally left blank
}

ConfigFileParseError::ConfigFileParseError(const std::string& sourceName,
					   int line, int column,
					   const std::string& description):
    ApplicationConfigurationError(sourceName, line, column, description) {
  // Intentionally left blank
}

ConfigFileParseError::~ConfigFileParseError() noexcept {
  // Intentionally left blank
}


