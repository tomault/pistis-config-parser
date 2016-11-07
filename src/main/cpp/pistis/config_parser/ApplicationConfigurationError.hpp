#ifndef __PISTIS__CONFIG_PARSER__APPLICATIONCONFIGURATIONERROR_HPP__
#define __PISTIS__CONFIG_PARSER__APPLICATIONCONFIGURATIONERROR_HPP__

#include <pistis/exceptions/PistisException.hpp>

namespace pistis {
  namespace config_parser {

    class ApplicationConfigurationError :
        public exceptions::PistisException {
    public:
      ApplicationConfigurationError(const std::string& details);
      ApplicationConfigurationError(const std::string& sourceName, int line,
				    int column, const std::string& details);
      virtual ~ApplicationConfigurationError() noexcept;

    private:
      static std::string createMessage_(const std::string& sourceName,
					int line, int column,
					const std::string& details);
    };

  }
}
#endif

