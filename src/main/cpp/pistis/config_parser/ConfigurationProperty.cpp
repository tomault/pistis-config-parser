#include "ConfigurationProperty.hpp"

using namespace pistis::config_parser;

const std::regex ConfigurationProperty::LEGAL_NAME_REX_(
    "[A-Za-z][A-Za-z0-9_]*(?:\\.[A-Za-z0-9_]+)*$"
);

ConfigurationProperty::ConfigurationProperty(const std::string& name, 
					     const std::string& value,
					     const std::string& source,
					     int line):
    name_(name), value_(value), source_(source), line_(line) {
  // Intentionally left blank
}

ConfigurationProperty::ConfigurationProperty(ConfigurationProperty&& other):
    name_(std::move(other.name_)), value_(std::move(other.value_)),
    source_(std::move(other.source_)), line_(other.line_) {
  // Intentionally left blank
}

ConfigurationProperty::~ConfigurationProperty() {
  // Intentionally left blank
}

ConfigurationProperty& ConfigurationProperty::operator=(
    ConfigurationProperty&& other
) {
  name_= std::move(other.name_);
  value_= std::move(other.value_);
  source_= std::move(other.source_);
  line_= other.line_;
  return *this;
}

int ConfigurationProperty::valueAsInt_(const std::string& value,
				       int minValue, int maxValue) const {
  return valueAs_(value, [this, minValue, maxValue](const std::string& v) {
    auto i= pistis::util::toInt64Quietly(v,0);
    if (i.second != pistis::util::NumConversionResult::OK) {
      throw PropertyFormatError(v, pistis::util::descriptionFor(i.second));
    } else if ((i.first < minValue) || (i.first > maxValue)) {
      std::ostringstream details;
      if (minValue == INT_MIN) {
	details << "Value must be less than " << maxValue;
      } else if (maxValue == INT_MAX) {
	details << "Value must be greater than " << minValue;
      } else {
	details << "Value must be between " << minValue << " and " << maxValue
		<< " (inclusive)";
      }
      throw InvalidPropertyValueError(*this, v, details.str());
    }
    return i.first;
  });
}

double ConfigurationProperty::valueAsDouble_(const std::string& value,
					     double minValue,
					     double maxValue) const {
  return valueAs_(value, [this,minValue,maxValue](const std::string& v) {
    auto i= pistis::util::toDoubleQuietly(v);
    if (i.second != pistis::util::NumConversionResult::OK) {
      throw PropertyFormatError(v, pistis::util::descriptionFor(i.second));
    } else if ((i.first < minValue) || (i.first > maxValue)) {
      std::ostringstream details;
      if (minValue == DBL_MIN) {
	details << "Value must be less than " << maxValue;
      } else if (maxValue == DBL_MAX) {
	details << "Value must be greater than " << minValue;
      } else {
	details << "Value must be between " << minValue << " and " << maxValue
		<< " (inclusive)";
      }
      throw InvalidPropertyValueError(*this, v, details.str());
    }
    return i.first;
  });
}

std::ostream& pistis::config_parser::operator<<(
    std::ostream& out, const ConfigurationProperty& p
) {
  if (p.source().empty()) {
    return out << p.name() << "=" << p.value();
  } else {
    return out << "[" << p.source() << ":" << p.line() << "]" << p.name()
	       << "=" << p.value();
  }
}
