#ifndef __PISTIS__CONFIG_PARSER__DETAIL__VALUEPROCESSOR_HPP__
#define __PISTIS__CONFIG_PARSER__DETAIL__VALUEPROCESSOR_HPP__

#include <pistis/config_parser/ConfigurationPropertyMap.hpp>
#include <string>

namespace pistis {
  namespace config_parser {
    namespace detail {
	
      /** @brief Replaces escape sequences and performs variable
       *         substitution in a configuration file property value
       */
      class ValueProcessor {
      public:
	ValueProcessor(const ConfigurationPropertyMap& properties,
		       bool useEnvironmentVars);
	virtual ~ValueProcessor();

	const ConfigurationPropertyMap& properties() const {
	  return properties_;
	}
	bool usesEnvironmentVars() const { return useEnvVars_; }
	void setUseEnvironmentVars(bool v) { useEnvVars_ = v; }

	virtual std::string processValue(const std::string& text);

      protected:
	virtual std::string resolveVariable_(const std::string& name);
	static void encodeUtf8_(unsigned int unicodeChar, std::string& output);
	static bool isHexDigit_(char ch) {
	  return ((ch >= '0') && (ch <= '9')) ||
	         ((ch >= 'A') && (ch <= 'F')) ||
	         ((ch >= 'a') && (ch <= 'f'));
	}
	static unsigned int hexValue_(char ch);

      private:
	const ConfigurationPropertyMap& properties_;
	bool useEnvVars_;
      };

    }
  }
}
#endif

