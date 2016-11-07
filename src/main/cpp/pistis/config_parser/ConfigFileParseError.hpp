#ifndef __PISTIS__CONFIG_PARSER__CONFIGFILEPARSEERROR_HPP__
#define __PISTIS__CONFIG_PARSER__CONFIGFILEPARSEERROR_HPP__

#include <pistis/config_parser/ApplicationConfigurationError.hpp>

namespace pistis {
  namespace config_parser {

    class ConfigFileParseError : public ApplicationConfigurationError {
    public:
      ConfigFileParseError(const std::string& sourceName, int line,
			   const std::string& description);
      ConfigFileParseError(const std::string& sourceName, int line,
			   int column, const std::string& description);
      virtual ~ConfigFileParseError() noexcept;
    };

  }
}
#endif
