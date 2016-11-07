#include "ApplicationConfiguration.hpp"
#include "ApplicationConfigurationError.hpp"
#include "ConfigFileParser.hpp"
#include "ConfigurationPropertyMap.hpp"
#include "InvalidPropertyValueError.hpp"
#include "RequiredPropertyMissingError.hpp"
#include "UnknownPropertyError.hpp"

using namespace pistis::util;
using namespace pistis::config_parser;

ApplicationConfiguration::ApplicationConfiguration(
    bool ignoreUnknownProperties, bool useEnvironmentVars,
    ConfigFileParser::DuplicatePropertyMode duplicatePropertyAction,
    ConfigFileParser::DuplicatePropertyMode includedPropertyAction
):
    ignoreUnknownProperties_(ignoreUnknownProperties),
    useEnvironmentVars_(useEnvironmentVars),
    duplicatePropertyAction_(duplicatePropertyAction),
    includedPropertyAction_(includedPropertyAction) {
  // Intentionally left blank
}

ApplicationConfiguration::~ApplicationConfiguration() {
  // Intentionally left blank
}

void ApplicationConfiguration::load(const std::string& filename) {
  ConfigFileParser parser(useEnvironmentVars_, duplicatePropertyAction_,
			  includedPropertyAction_);
  ConfigurationPropertyMap properties = parser.parse(filename);
  load_(filename, properties);
}

void ApplicationConfiguration::load(const std::string& sourceName,
				    std::istream& input, int initialLine,
				    int initialColumn) {
  ConfigFileParser parser(useEnvironmentVars_, duplicatePropertyAction_,
			  includedPropertyAction_);
  ConfigurationPropertyMap properties =
      parser.parse(sourceName, input, initialLine, initialColumn);
  load_(sourceName, properties);
}

void ApplicationConfiguration::loadFromText(const std::string& sourceName,
					    const std::string& text) {
  ConfigFileParser parser(useEnvironmentVars_, duplicatePropertyAction_,
			  includedPropertyAction_);
  ConfigurationPropertyMap properties = parser.parseText(sourceName, text);
  load_(sourceName, properties);
}

void ApplicationConfiguration::load_(
    const std::string& sourceName,
    const ConfigurationPropertyMap& properties
) {
  auto i= properties.begin();
  auto j= handlers_.begin();

  while (j != handlers_.end()) {
    j->second.setFound(false);
    ++j;
  }

  j= handlers_.begin();
  while ((i != properties.end()) && (j != handlers_.end())) {
    if (i->name() == j->first) {
      applyHandler_(j->second, *i);
      j->second.setFound(true);
      ++i;
      if (!j->second.isPrefixHandler()) {
	++j;
      }
    } else if (j->second.isPrefixHandler() && startsWith(i->name(), j->first)) {
      applyHandler_(j->second, *i);
      j->second.setFound(true);
      ++i;
    } else if (j->first < i->name()) {
      if (j->second.required()) {
	throw RequiredPropertyMissingError(sourceName, j->first);
      }
      ++j;
    } else if (!ignoreUnknownProperties_) {
      throw UnknownPropertyError(i->source(), i->line(), i->name());
    } else {
      ++i;
    }
  }

  while (j != handlers_.end()) {
    if (j->second.required()) {
      throw RequiredPropertyMissingError(sourceName, j->first);
    }
    ++j;
  }
  if (!ignoreUnknownProperties_ && (i != properties.end())) {
    throw UnknownPropertyError(i->source(), i->line(), i->name());
  }
}

void ApplicationConfiguration::applyHandler_(
    const ApplicationConfiguration::PropertyInfo& info,
    const ConfigurationProperty& property
) {
  try {
    if (!info.allowEmpty() && property.value().empty()) {
      throw InvalidPropertyValueError(property, "", "Value is empty");
    }
    info.handler()->handle(property);
  } catch(const PropertyFormatError& e) {
    throw InvalidPropertyValueError(
        property, e.value().empty() ? property.value() : e.value(),
	e.description()
    );
  } catch(const InvalidPropertyValueError& e) {
    throw;
  } catch(const std::exception& e) {
    throw InvalidPropertyValueError(property, property.value(), e.what());
  } catch(...) {
    throw InvalidPropertyValueError(property);
  }
}

static bool isLegalPropertyName(const std::string& name, int nameLen) {
  if (name[nameLen - 1] == '.') {
    return ConfigurationProperty::isLegalName(name.substr(0, nameLen - 1));
  }
  return ConfigurationProperty::isLegalName(name);
}

void ApplicationConfiguration::registerProperty_(PropertyInfo&& info) {
  if (info.isPrefixHandler()) {
    const int nameLen= info.name().size();
    if (!nameLen || !isLegalPropertyName(info.name(), nameLen)) {
      std::ostringstream msg;
      msg << "Cannot register property prefix for invalid prefix \""
	  << info.name() << "\"";
      throw ApplicationConfigurationError(msg.str());      
    }
  } else if (!ConfigurationProperty::isLegalName(info.name())) {
    std::ostringstream msg;
    msg << "Cannot register property with invalid name \"" << info.name()
	<< "\"";
    throw ApplicationConfigurationError(msg.str());
  }
  if (!info.handler()) {
    std::ostringstream msg;
    msg << "Cannot register property \"" << info.name()
	<< "\" with no handler";
    throw ApplicationConfigurationError(msg.str());
  }

  auto i= handlers_.lower_bound(info.name());
  if ((i != handlers_.end()) && (i->first == info.name())) {
    std::ostringstream msg;
    msg << "Property \"" << info.name() << "\" has already been registered";
    throw ApplicationConfigurationError(msg.str());
  }

  if (info.isPrefixHandler()) {
    // Check that no previously-registered property handler begins with
    // this prefix
    char firstChar = info.name()[0];
    auto k = i;
    while ((k != handlers_.end()) && (k->first[0] == firstChar)) {
      if (startsWith(k->first, info.name())) {
	std::ostringstream msg;
	msg << "Cannot register handler for prefix \"" << info.name()
	    << "\" because a previously-registered property (\""
	    << k->first << "\") begins with that prefix";
	throw ApplicationConfigurationError(msg.str());  
      }
      ++k;
    }
  }

  // Check that a property prefix handler hasn't been registered for any
  // prefix of this property
  auto j= handlers_.lower_bound(info.name().substr(0,1));
  if ((j != handlers_.end()) && (j->first[0] == info.name()[0])) {
    while (j != i) {
      if (j->second.isPrefixHandler() && startsWith(info.name(), j->first)) {
	std::ostringstream msg;
	msg << "Cannot register handler for property \"" << info.name()
	    << "\" because a handler for properties with prefix \""
	    << j->second.name() << "\" has already been registered";
	throw ApplicationConfigurationError(msg.str());
      }
      ++j;
    }
  }
  handlers_.insert(std::make_pair(info.name(), std::move(info)));
}

ApplicationConfiguration::PropertyHandler::~PropertyHandler() {
  // Intentionally left blank
}

ApplicationConfiguration::PropertyInfo::PropertyInfo(const std::string& name,
						     bool isPrefix,
						     bool isRequired,
						     bool allowEmpty,
						     PropertyHandler* handler):
    name_(name), prefix_(isPrefix), required_(isRequired),
    allowEmpty_(allowEmpty), found_(false), handler_(handler) {
  // Intentionally left blank
}

ApplicationConfiguration::PropertyInfo::PropertyInfo(PropertyInfo&& other):
    name_(std::move(other.name_)), prefix_(other.prefix_),
    required_(other.required_), allowEmpty_(other.allowEmpty_),
    found_(other.found_), handler_(std::move(other.handler_)) {
  // Intentionally left blank
}

ApplicationConfiguration::PropertyInfo&
    ApplicationConfiguration::PropertyInfo::operator=(PropertyInfo&& other) {
  name_= std::move(other.name_);
  prefix_= other.prefix_;
  required_= other.required_;
  allowEmpty_= other.allowEmpty_;
  found_= other.found_;
  handler_= std::move(other.handler_);
  return *this;
}

