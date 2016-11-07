#ifndef __PISTIS__CONFIG_PARSER__INVALIDPROPERTYVALUEERROR_HPP__
#define __PISTIS__CONFIG_PARSER__INVALIDPROPERTYVALUEERROR_HPP__

#include <pistis/config_parser/ApplicationConfigurationError.hpp>

namespace pistis {
  namespace config_parser {

    class ConfigurationProperty;

    class InvalidPropertyValueError : public ApplicationConfigurationError {
    public:
      InvalidPropertyValueError(const ConfigurationProperty& property);
      InvalidPropertyValueError(const ConfigurationProperty& property,
				const std::string& value);
      InvalidPropertyValueError(const ConfigurationProperty& property,
				const std::string& value,
				const std::string& details);
      virtual ~InvalidPropertyValueError() noexcept;

    private:
      static std::string createMessage_(const std::string& name,
					const std::string& value,
					const std::string& details);
    };

  }
}
#endif

