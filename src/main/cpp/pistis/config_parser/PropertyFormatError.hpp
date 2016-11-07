#ifndef __PISTIS__CONFIG_PARSER__PROPERTYFORMATERROR_HPP__
#define __PISTIS__CONFIG_PARSER__PROPERTYFORMATERROR_HPP__

#include <pistis/config_parser/ApplicationConfigurationError.hpp>

namespace pistis {
  namespace config_parser {

    class PropertyFormatError : public ApplicationConfigurationError {
    public:
      PropertyFormatError(const std::string& description);
      PropertyFormatError(const std::string& value,
			  const std::string& description);
      virtual ~PropertyFormatError() noexcept;
	  
      const std::string& value() const { return value_; }
      const std::string& description() const { return description_; }

    private:
      std::string value_;
      std::string description_;

      static std::string createMessage_(const std::string& value,
					const std::string& description);
    };

  }
}
#endif

