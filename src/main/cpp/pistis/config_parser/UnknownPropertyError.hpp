#ifndef __PISTIS__CONFIG_PARSER__UNKNOWNPROPERTYERROR_HPP__
#define __PISTIS__CONFIG_PARSER__UNKNOWNPROPERTYERROR_HPP__

#include <pistis/config_parser/ApplicationConfigurationError.hpp>

namespace pistis {
  namespace config_parser {

    class UnknownPropertyError : public ApplicationConfigurationError {
    public:
      UnknownPropertyError(const std::string& sourceName, int line,
			   const std::string& propertyName);
      virtual ~UnknownPropertyError() noexcept;
    };

  }
}
#endif

