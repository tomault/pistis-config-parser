#include "ConfigFileParser.hpp"
#include "ConfigFileParseError.hpp"
#include "PropertyFormatError.hpp"
#include "detail/ConfigFileLexer.hpp"
#include "detail/ValueProcessor.hpp"
#include <pistis/filesystem/Path.hpp>
#include <pistis/util/StringUtil.hpp>
#include <fstream>
#include <sstream>

using namespace pistis::exceptions;
using namespace pistis::util;
using namespace pistis::config_parser;
using namespace pistis::config_parser::detail;

namespace path = pistis::filesystem::path;

ConfigFileParser::ConfigFileParser(
    bool useEnvironmentVars,
    DuplicatePropertyMode duplicatePropertyAction,
    DuplicatePropertyMode includedPropertyAction
):
    useEnvVars_(useEnvironmentVars),
    duplicatePropertyAction_(duplicatePropertyAction),
    includedPropertyAction_(includedPropertyAction),
    context_(), contextPrefix_(), includedFrom_() {
  // Intentionally left blank
}

ConfigFileParser::ConfigFileParser(
    bool useEnvironmentVars,
    DuplicatePropertyMode duplicatePropertyAction,
    DuplicatePropertyMode includedPropertyAction,
    const std::vector<std::string>& includedFiles,
    const std::string& includedFrom
):
    useEnvVars_(useEnvironmentVars),
    duplicatePropertyAction_(duplicatePropertyAction),
    includedPropertyAction_(includedPropertyAction),
    context_(), contextPrefix_(), includedFrom_(includedFiles) {
  includedFrom_.push_back(includedFrom);
}

ConfigFileParser::~ConfigFileParser() {
  // Intentionally left blank
}

ConfigurationPropertyMap ConfigFileParser::parse(const std::string& filename) {
  std::ifstream input(filename.c_str());
  if (input) {
    return parse(filename, input);
  } else {
    std::ostringstream msg;
    msg << "Cannot open file (" << strerror(errno) << ")";
    throw ConfigFileParseError(filename, 0, 0, msg.str());
  }
}

ConfigurationPropertyMap ConfigFileParser::parse(const std::string& sourceName,
						 std::istream& input,
						 int initialLine,
						 int initialColumn) {
  ConfigFileLexer lexer(input, initialLine, initialColumn);
  ConfigurationPropertyMap properties;
  std::unique_ptr<ValueProcessor> valueProcessor(
      createValueProcessor_(properties, usesEnvironmentVars())
  );

  while (true) {
    Token t= lexer.next();
    if (t.type() == TokenType::END_OF_FILE) {
      if (!getContext_().empty()) {
	throw ConfigFileParseError(sourceName, t.line(), t.column(),
				   "'}' expected");
      }
      break;
    } else if (t.type() == TokenType::COMMENT) {
      // Skip comments
    } else if ((t.type() == TokenType::PUNCTUATION) && (t.value() == "}")) {
      if (getContext_().empty()) {
	throw ConfigFileParseError(sourceName, t.line(), t.column(),
				   "Syntax error ('}' unexpected)");
      }
      endBlock_();
    } else if (t.type() != TokenType::NAME) {
      throw ConfigFileParseError(sourceName, t.line(), t.column(),
				 "Syntax error (property name expected)");
    } else if (t.value() == "include") {
      parseIncludeDirective_(sourceName, lexer, properties);
    } else if (ConfigurationProperty::isLegalName(t.value())) {
      parseAssignmentOrBlock_(sourceName, t, lexer, *valueProcessor,
			      properties);
    } else {
      std::ostringstream msg;
      msg << "\"" << t.value() << "\" is not a legal property name";
      throw ConfigFileParseError(sourceName, t.line(), t.column(), msg.str());
    }
  }
  return properties;
}

ConfigurationPropertyMap ConfigFileParser::parseText(
    const std::string& sourceName, const std::string& text,
    int initialLine, int initialColumn
) {
  std::istringstream input(text);
  return parse(sourceName, input, initialLine, initialColumn);
}

void ConfigFileParser::parseIncludeDirective_(
    const std::string& sourceName, ConfigFileLexer& lexer,
    ConfigurationPropertyMap& properties
) {
  int line = lexer.currentLine();
  int col = lexer.currentColumn();
  Token t = lexer.next();
  std::string includeFilePath;

  if ((t.type() != TokenType::PUNCTUATION) || (t.value() != "\"") ||
      (t.line() != line)) {
    throw ConfigFileParseError(sourceName, line, col, "'\"' expected");
  }

  lexer.parseNextAsQuotedString();
  t= lexer.next();
  if ((t.type() != TokenType::VALUE) || t.value().empty()) {
    throw ConfigFileParseError(sourceName, line, t.column(),
			       "File name missing");
  }

  includeFilePath = t.value();
  if (!path::isAbsolute(includeFilePath)) {
    std::string pathToSource = std::get<0>(path::splitFile(sourceName));
    includeFilePath = path::join(pathToSource, includeFilePath);
  }
  col= lexer.currentColumn();
  t= lexer.next();
  if ((t.type() != TokenType::PUNCTUATION) || (t.value() != "\"") ||
      (t.line() != line)) {
    throw ConfigFileParseError(sourceName, line, col, "'\"' expected");
  }

  if (getContext_().size()) {
    throw ConfigFileParseError(sourceName, t.line(), t.column(),
			       "Cannot include a file from within a block");
  }
  if (isIncludedFrom_(includeFilePath)) {
    std::ostringstream msg;
    msg << "Including \"" << includeFilePath
	<< "\" would produce an include file loop.  The include file list is:"
	<< join(getIncludedFrom_().begin(), getIncludedFrom_().end(), "\n  ")
	<< "\n  " << sourceName;
    throw ConfigFileParseError(sourceName, t.line(), col, msg.str());
  }
  if (getIncludedFrom_().size() > MAX_INCLUDE_DEPTH_) {
    std::ostringstream msg;
    msg << "Maximum inclusion depth exceeded.  The include file list is:"
	<< join(getIncludedFrom_().begin(), getIncludedFrom_().end(), "\n  ")
	<< "\n  " << sourceName;
    throw ConfigFileParseError(sourceName, t.line(), col, msg.str());
  }
  
  // Read the included properties and merge them
  ConfigFileParser includeFileParser(usesEnvironmentVars(),
				     duplicatePropertyAction(),
				     includedPropertyAction(),
				     getIncludedFrom_(),
				     sourceName);
  ConfigurationPropertyMap includedProperties =
      includeFileParser.parse(includeFilePath);
  for (auto i = includedProperties.begin();
       i != includedProperties.end();
       ++i) {
    if (!properties.hasKey(i->name()) ||
	(includedPropertyAction() == DUP_OVERWRITE)) {
      properties.add(*i);
    } else if (includedPropertyAction() == DUP_ERROR) {
      ConfigurationProperty original= properties[i->name()];
      std::ostringstream msg;
      msg << "Duplicate property \"" << i->name()
	  << "\" (Originally defined at " << original.source() << ":"
	  << original.line() << ")";
      throw ConfigFileParseError(i->source(), i->line(), msg.str());
    }
  }
}

void ConfigFileParser::parseAssignmentOrBlock_(
    const std::string& sourceName, const Token& name,
    ConfigFileLexer& lexer, ValueProcessor& valueProcessor,
    ConfigurationPropertyMap& properties
) {
  int col= lexer.currentColumn();
  Token t= lexer.next();
  if ((t.type() != TokenType::PUNCTUATION) || (t.line() != name.line())) {
    throw ConfigFileParseError(sourceName, name.line(), col, "'=' expected");
  } else if (t.value() == "{") {
    beginBlock_(name.value());
  } else if (t.value() == "=") {
    parseAssignment_(sourceName, name, lexer, valueProcessor, properties);
  } else {
    throw ConfigFileParseError(sourceName, name.line(), col, "'=' expected");
  }
}

void ConfigFileParser::parseAssignment_(
    const std::string& sourceName, const Token& name, ConfigFileLexer& lexer,
    ValueProcessor& valueProcessor, ConfigurationPropertyMap& properties
) {
  int col= lexer.currentColumn();
  try {
    lexer.parseNextAsValue();
    Token t= lexer.next();
    if (t.type() != TokenType::VALUE) {
      throw ConfigFileParseError(sourceName, name.line(), col,
				 "Property value expected");
    }

    std::string fullName= getFullName_(name.value());
    std::string value= valueProcessor.processValue(t.value());
    if (!properties.hasKey(fullName)) {
      properties.add(
	  ConfigurationProperty(fullName, value, sourceName, name.line())
      );
    } else {
      ConfigurationProperty original= properties[fullName];
      DuplicatePropertyMode mode=
	(original.source() == sourceName) ? duplicatePropertyAction()
	                                  : includedPropertyAction();
      std::ostringstream msg;

      switch (mode) {
        case DUP_OVERWRITE:
	  properties.add(
	      ConfigurationProperty(fullName, value, sourceName, name.line())
	  );
	  break;

        case DUP_IGNORE:
	  break;

        case DUP_ERROR:
        default:
	  msg << "Property \"" << fullName
	      << "\" defined twice; original definition at "
	      << original.source() << ":" << original.line();
	  throw ConfigFileParseError(sourceName, name.line(), name.column(),
				     msg.str());
      }
    }
  } catch (const PropertyFormatError& e) {
    std::ostringstream msg;
    msg << "Invalid property value (" << e.description() << ")";
    throw ConfigFileParseError(sourceName, name.line(), col, msg.str());
  }
}

ValueProcessor* ConfigFileParser::createValueProcessor_(
    const ConfigurationPropertyMap& properties,
    bool useEnvironmentVars
) {
  return new ValueProcessor(properties, useEnvironmentVars);
}
