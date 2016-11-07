#include "ApplicationConfigurationError.hpp"
#include <sstream>

using namespace pistis::exceptions;
using namespace pistis::config_parser;

ApplicationConfigurationError::ApplicationConfigurationError(
    const std::string& details
):
    PistisException(details) {
  // Intentionally left blank
}

ApplicationConfigurationError::ApplicationConfigurationError(
    const std::string& sourceName, int line, int column,
    const std::string& details
):
    PistisException(createMessage_(sourceName, line, column, details)) {
  // Intentionally left blank
}

ApplicationConfigurationError::~ApplicationConfigurationError() noexcept {
  // Intentionally left blank
}

std::string ApplicationConfigurationError::createMessage_(
    const std::string& sourceName, int line, int column,
    const std::string& details
) {
  std::ostringstream msg;
  
  if (!sourceName.empty()) {
    msg << "Error";
    if (line > 0) {
      msg << " on line " << line;
      if (column > 0) {
	msg << ", column " << column;
      }
      msg << " of " << sourceName;
    } else {
      msg << " in " << sourceName;
    }
    if (!details.empty()) {
      msg << ": " << details;
    }
  } else if (!details.empty()) {
    msg << details;
  } else {
    msg << "Application configuration error";
  }
  return msg.str();
}

