#ifndef __PISTIS__CONFIG_PARSER__REQUIREDPROPERTYMISSINGERROR_HPP__
#define __PISTIS__CONFIG_PARSER__REQUIREDPROPERTYMISSINGERROR_HPP__

#include <pistis/config_parser/ApplicationConfigurationError.hpp>

namespace pistis {
  namespace config_parser {

    class RequiredPropertyMissingError : public ApplicationConfigurationError {
    public:
      RequiredPropertyMissingError(const std::string& sourceName,
				   const std::string& propertyName);
      virtual ~RequiredPropertyMissingError() noexcept;
    };

  }
}
#endif

