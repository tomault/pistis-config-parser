#include "PropertyFormatError.hpp"
#include <sstream>

using namespace pistis::config_parser;

PropertyFormatError::PropertyFormatError(const std::string& description):
    ApplicationConfigurationError(createMessage_(std::string(), description)),
    value_(), description_(description) {
  // Intentionally left blank
}

PropertyFormatError::PropertyFormatError(const std::string& value,
					 const std::string& description):
    ApplicationConfigurationError(createMessage_(value, description)),
    value_(value), description_(description) {
  // Intentionally left blank
}

PropertyFormatError::~PropertyFormatError() noexcept {
  // Intentionally left blank
}

std::string PropertyFormatError::createMessage_(
    const std::string& value, const std::string& description
) {
  std::ostringstream msg;
  msg << "PropertyFormatError";
  if (!value.empty()) {
    msg << " parsing \"" << value << "\"";
  }
  if (!description.empty()) {
    msg << ": " << description;
  }
  return msg.str();
}
				
