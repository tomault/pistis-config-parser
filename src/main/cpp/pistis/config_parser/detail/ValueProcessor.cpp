#include "ValueProcessor.hpp"
#include "ConfigFileLexer.hpp"
#include <pistis/config_parser/PropertyFormatError.hpp>
#include <iomanip>
#include <sstream>
#include <stdlib.h>

using namespace pistis::config_parser;
using namespace pistis::config_parser::detail;

ValueProcessor::ValueProcessor(const ConfigurationPropertyMap& properties,
			       bool useEnvironmentVars):
    properties_(properties), useEnvVars_(useEnvironmentVars) {
  // Intentionally left blank
}

ValueProcessor::~ValueProcessor() {
  // Intentionally left blank
}

std::string ValueProcessor::processValue(const std::string& value) {
  std::string::const_iterator i = value.begin();
  std::string::const_iterator j;
  int state = 1;
  std::string prepared;
  unsigned int unicodeChar;
  bool nameIsLegal;

  prepared.reserve(value.size());
  while (state) {
    char ch= (i != value.end()) ? *i : 0;
    switch (state) {
      case 0:
        // Reached the end of the input
        break; 

      case 1:
        // Looking for "$" or "\"
        if (!ch) {
	  state = 0;
        } else if (ch == '$') {
  	  state = 2;
        } else if (ch == '\\') {
  	  state = 3;
        } else {
	  prepared.push_back(ch);
	}
	++i;
	break;

      case 2:
	// Looking for "{" after "$"
	if (ch == '{') {
	  state = 4;
	  ++i;
	} else {
	  // Go to state 1 to process this character
	  prepared.push_back('$');
	  state = 1;
	}
	break;

      case 3:
	// Looking at character after "\"
	if (!ch) {
	  prepared.push_back('\\');
	  state= 0;
	  break;
	} else if (ch == 'n') {
	  prepared.push_back('\n');
	  state = 1;
	} else if (ch == 't') {
	  prepared.push_back('\t');
	  state = 1;
	} else if (ch == 'r') {
	  prepared.push_back('\r');
	  state = 1;
	} else if (ch == 'u') {
	  unicodeChar = 0;
	  state = 5;
	} else {
	  prepared.push_back(ch);
	  state= 1;
	}
	++i;
	break;

      case 4:
	// Examine the first character after "${"
	if (!ch) {
	  throw PropertyFormatError("Incomplete property reference \"${\"");
	} else if (ch == '}') {
	  throw PropertyFormatError("Invalid property reference \"${}\"");
	} else {
	  nameIsLegal = ConfigFileLexer::isNameChar(ch, 0);
	  state = 6;
	  j = i;
	  ++i;
	}
	break;

      case 5:
	// Parsing the first hex digit after "\u"
	if (!ch) {
	  throw PropertyFormatError(
	     "Incomplete escape sequence \"\\u\" at end of line"
	  );
	} else if (ch == '{') {
	  state = 12;
	  ++i;
	} else if (isHexDigit_(ch)) {
	  unicodeChar = hexValue_(ch);
	  state= 7;
	  ++i;
	} else {
	  std::ostringstream msg;
	  msg << "Invalid escape sequence \"\\u" << (char)ch << "\"";
	  throw PropertyFormatError(msg.str());
	}
	break;

      case 6:
	// Looking for "}" after property name
	if (!ch) {
	  throw PropertyFormatError(
	      "Incomplete property reference \"${" + std::string(j,i) + "\""
	  );
	} else if (ch == '}') {
	  std::string name(j, i);
	  if (!nameIsLegal) {
	    throw PropertyFormatError(
	        "\"${" + name + "}\" does not contain a legal property name"
	    );
	  }
	  prepared += resolveVariable_(name);
	  state = 1;
	  ++i;
	} else {
	  nameIsLegal = ConfigFileLexer::isNameChar(i[-1], *i);
	  ++i;
	}
	break;

      case 7:
      case 8:
      case 9:
      case 10:
	// Reading hex digits in a "\u" escape sequence
	if (isHexDigit_(ch)) {
	  unicodeChar= (unicodeChar << 4) | hexValue_(ch);
	  ++state;
	  ++i;
	} else {
	  encodeUtf8_(unicodeChar, prepared);
	  state= ch ? 1 : 0;
	}
	break;
      
      case 11:
	// Read five hex digits after "\u".  Unicode char ends here
	// unconditionally
	if (isHexDigit_(ch)) {
	  unicodeChar= (unicodeChar << 4) | hexValue_(ch);
	  ++i;
	}
	encodeUtf8_(unicodeChar, prepared);
	state = ch ? 1 : 0;
	break;  // Loop around to process char at i in state 1

      case 12:
	// Parsing first hex digit after "\u{"
	if (!ch) {
	  throw PropertyFormatError(
	      "Incomplete \\u{} escape sequence at end of line"
	  );
	} else if (ch == '}') {
	  throw PropertyFormatError("Invalid escape sequence \"\\u{}\"");
	} else if (!isHexDigit_(ch)) {
	  std::ostringstream msg;
	  msg << "Invalid hex digit '" << (char)ch
	      << "' in \\u{} escape sequence";
	  throw PropertyFormatError(msg.str());
	} else {
	  unicodeChar= hexValue_(ch);
	  ++state;
	  ++i;
	}
	break;

      case 13:
      case 14:
      case 15:
      case 16:
      case 17: 
	// Parsing second and later hex digits in a "\u{}" escape sequence
	if (!ch) {
	  throw PropertyFormatError(
	      "Incomplete \\u{} escape sequence at end of line"
	  );
	} else if (ch == '}') {
	  encodeUtf8_(unicodeChar, prepared);
	  state = 1;
	  ++i;
	} else if (isHexDigit_(ch)) {
	  unicodeChar = (unicodeChar << 4) | hexValue_(ch);
	  ++state;
	  ++i;
	} else {
	  std::ostringstream msg;
	  msg << "Invalid hex digit '" << (char)ch
	      << "' in \\u{} escape sequence";
	  throw PropertyFormatError(msg.str());
	}
	break;

      case 18:
	// Parsed six hex digits in "\u{}" escape sequence.  Looking for
	// the closing brace
	if (!ch) {
	  throw PropertyFormatError(
	      "Incomplete \\u{} escape sequence at end of line"
	  );	  
	} else if (ch == '}') {
	  encodeUtf8_(unicodeChar, prepared);
	  state = 1;
	  ++i;
	} else if (isHexDigit_(ch)) {
	  throw PropertyFormatError(
	      "Too many hex digits in \\u{} escape sequence"
	  );
	} else {
	  std::ostringstream msg;
	  msg << "Invalid hex digit '" << (char)ch
	      << "' in \\u{} escape sequence";
	  throw PropertyFormatError(msg.str());	  
	}
	break;

      default:
	throw PropertyFormatError("Illegal state while preparing value");
    }
  }
  return prepared;
}

std::string ValueProcessor::resolveVariable_(const std::string& name) {
  if (properties().hasKey(name)) {
    return properties()[name].value();
  } else if (usesEnvironmentVars()) {
    const char* envValue= getenv(name.c_str());
    if (envValue) {
      return std::string(envValue);
    }
  }
  throw PropertyFormatError(
      "Cannot resolve referenced property \"${" + name + "}\""
  );
}

void ValueProcessor::encodeUtf8_(unsigned int unicodeChar,
				 std::string& output) {
  if (unicodeChar <= 0x7F) {
    output.push_back((char)unicodeChar);
  } else if (unicodeChar <= 0x7FF) {
    output.push_back((char)(0xC0|(unicodeChar >> 6)));
    output.push_back((char)(0x80|(unicodeChar & 0x3F)));
  } else if (unicodeChar <= 0xFFFF) {
    output.push_back((char)(0xE0|( unicodeChar >> 12)));
    output.push_back((char)(0x80|((unicodeChar >> 6) & 0x3F)));
    output.push_back((char)(0x80|(unicodeChar & 0x3F)));
  } else if (unicodeChar <= 0x1FFFFF) {
    output.push_back((char)(0xF0|( unicodeChar >> 18)));
    output.push_back((char)(0x80|((unicodeChar >> 12) & 0x3F)));
    output.push_back((char)(0x80|((unicodeChar >>  6) & 0x3F)));
    output.push_back((char)(0x80|(unicodeChar & 0x3F)));
  } else if (unicodeChar <= 0x3FFFFFF) {
    output.push_back((char)(0xF8|( unicodeChar >> 24)));
    output.push_back((char)(0x80|((unicodeChar >> 18) & 0x3F)));
    output.push_back((char)(0x80|((unicodeChar >> 12) & 0x3F)));
    output.push_back((char)(0x80|((unicodeChar >>  6) & 0x3F)));
    output.push_back((char)(0x80|(unicodeChar & 0x3F)));
  } else if (unicodeChar <= 0x7FFFFFFF) {
    output.push_back((char)(0xFC|( unicodeChar >> 30)));
    output.push_back((char)(0x80|((unicodeChar >> 24) & 0x3F)));
    output.push_back((char)(0x80|((unicodeChar >> 18) & 0x3F)));
    output.push_back((char)(0x80|((unicodeChar >> 12) & 0x3F)));
    output.push_back((char)(0x80|((unicodeChar >>  6) & 0x3F)));
    output.push_back((char)(0x80|(unicodeChar & 0x3F)));
  }
}

unsigned int ValueProcessor::hexValue_(char ch) {
  unsigned int v= ch - '0';
  if (ch >= 'a') {
    v -= ('a' - '0' - 10);
  } else if (ch >= 'A') {
    v -= ('A' - '0' - 10);
  }
  return v;
}
