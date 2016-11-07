#ifndef __PISTIS__CONFIG_PARSER__CONFIGFILEPARSER_HPP__
#define __PISTIS__CONFIG_PARSER__CONFIGFILEPARSER_HPP__

#include <pistis/config_parser/ConfigurationPropertyMap.hpp>
#include <algorithm>
#include <iostream>
#include <string>

namespace pistis {
  namespace config_parser {
    namespace detail {
      class ConfigFileLexer;
      class Token;
      class ValueProcessor;
    }

    /** @brief Parses pistis application configuration files
     *
     *  The bnf for a configuration file is:
     *  <code>
     *   start := statement-list | ""
     *   statement-list := statement | statement-list statement
     *   statement := assignment | include-stmt | block | COMMENT
     *   assignment := NAME "=" VALUE
     *   include-stmt := "include" VALUE
     *   block := NAME "{" statement-list "}"
     *  </code>
     *  The terminals NAME, VALUE, COMMENT are defined by the
     *  details::ConfigFileLexer class, which the parser uses.
     */
    class ConfigFileParser {
    public:
      enum DuplicatePropertyMode {
	DUP_ERROR,  //< Duplicate properties produce an error
	DUP_IGNORE, //< Ignore duplicate properties
	DUP_OVERWRITE ///< Overwrite duplicate properties
      };

    public:
      ConfigFileParser(
	  bool useEnvironmentVars= true,
	  DuplicatePropertyMode duplicatePropertyAction= DUP_ERROR,
	  DuplicatePropertyMode includedPropertyAction= DUP_IGNORE
      );
      virtual ~ConfigFileParser();

      bool usesEnvironmentVars() const { return useEnvVars_; }
      void setUsesEnvironmentVars(bool v) { useEnvVars_=v; }

      DuplicatePropertyMode duplicatePropertyAction() const {
	return duplicatePropertyAction_;
      }
      void setDuplicatePropertyAction(DuplicatePropertyMode action) {
	duplicatePropertyAction_= action;
      }

      DuplicatePropertyMode includedPropertyAction() const {
	return includedPropertyAction_;
      }
      void setIncludedPropertyAction(DuplicatePropertyMode action) {
	includedPropertyAction_= action;
      }


      virtual ConfigurationPropertyMap parse(const std::string& filename);
      virtual ConfigurationPropertyMap parse(const std::string& sourceName,
					     std::istream& input,
					     int initialLine=1,
					     int initialColumn=1);
      virtual ConfigurationPropertyMap parseText(const std::string& sourceName,
						 const std::string& text,
						 int initialLine=1,
						 int initialColumn=1);

    protected:
      ConfigFileParser(bool useEnvironmentVars,
		       DuplicatePropertyMode duplicatePropertyAction,
		       DuplicatePropertyMode includedPropertyAction,
		       const std::vector<std::string>& includedFiles,
		       const std::string& includedFrom);
			 
      virtual void parseIncludeDirective_(const std::string& sourceName,
					  detail::ConfigFileLexer& lexer,
					  ConfigurationPropertyMap& properties);

      virtual void parseAssignmentOrBlock_(
	  const std::string& sourceName, const detail::Token& name,
	  detail::ConfigFileLexer& lexer,
	  detail::ValueProcessor& valueProcessor,
	  ConfigurationPropertyMap& properties
      );
      
      virtual void parseAssignment_(const std::string& sourceName,
				    const detail::Token& name,
				    detail::ConfigFileLexer& lexer,
				    detail::ValueProcessor& valueProcessor,
				    ConfigurationPropertyMap& properties);

      virtual detail::ValueProcessor* createValueProcessor_(
	  const ConfigurationPropertyMap& properties,
	  bool useEnvironmentVars
      );

      const std::vector<std::string>& getContext_() const {
	return context_;
      }
      const std::string& getPropertyNamePrefix_() const {
	return contextPrefix_;
      }
      std::string getFullName_(const std::string& name) const {
	return contextPrefix_ + name;
      }
      void beginBlock_(const std::string& blockName) {
	context_.push_back(blockName);
	contextPrefix_ += blockName;
	contextPrefix_.push_back('.');
      }

      void endBlock_() {
	context_.pop_back();
	// Remove trailing period
	contextPrefix_.erase(contextPrefix_.size()-1);
	while (!contextPrefix_.empty() && contextPrefix_.back() != '.') {
	  contextPrefix_.erase(contextPrefix_.size()-1);
	}
      }

      const std::vector<std::string>& getIncludedFrom_() const {
	return includedFrom_;
      }
      void setIncludedFrom_(const std::vector<std::string>& v) {
	includedFrom_= v;
      }
      bool isIncludedFrom_(const std::string& filename) const {
	auto i = std::find(includedFrom_.begin(), includedFrom_.end(),
			   filename);
	return i != includedFrom_.end();
      }
      
      std::string includedBy_(const std::string& filename) const {
	auto i= std::find(includedFrom_.begin(), includedFrom_.end(),
			  filename);
	if ((i == includedFrom_.begin()) || (i == includedFrom_.end())) {
	  return std::string();
	}
	--i;
	return *i;
      }
	
    private:
      /** @brief Whether to use environment variables for variable substitutions
       *
       *  If true, if the parser can't find a property with the given
       *  name for a variable substitution in a property value, it will
       *  try to find an environment variable with the given name.  If it
       *  finds one, it will substitute the value of that variable.  If it
       *  does not, the parser will issue an error.
       */
      bool useEnvVars_;

      /** @brief Action to take when a duplicate property occurs in the
       *         file being parsed.
       */
      DuplicatePropertyMode duplicatePropertyAction_;

      /** @brief Action to take when a property from an included file
       *         duplicates a property in this file.
       */
      DuplicatePropertyMode includedPropertyAction_;

      /** @brief Context stack for blocks */
      std::vector<std::string> context_;

      /** @brief Current prefix for property names */
      std::string contextPrefix_;

      /** @brief Files this file has been included from */
      std::vector<std::string> includedFrom_;

      static const size_t MAX_INCLUDE_DEPTH_ = 128;
    };

  }
}
#endif

