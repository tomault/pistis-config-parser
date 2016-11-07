#ifndef __PISTIS__CONFIG_PARSER__CONFIGURATIONPROPERTY_HPP__
#define __PISTIS__CONFIG_PARSER__CONFIGURATIONPROPERTY_HPP__

#include <pistis/util/NumUtil.hpp>
#include <pistis/util/StringUtil.hpp>
#include <pistis/config_parser/InvalidPropertyValueError.hpp>
#include <pistis/config_parser/PropertyFormatError.hpp>
#include <regex>
#include <set>
#include <sstream>
#include <vector>
#include <float.h>
#include <limits.h>

namespace pistis {
  namespace config_parser {

    class ConfigurationProperty {
    public:
      ConfigurationProperty(const std::string& name, 
			    const std::string& value,
			    const std::string& source,
			    int line);
      ConfigurationProperty(const ConfigurationProperty& other)= default;
      ConfigurationProperty(ConfigurationProperty&& other);
      ~ConfigurationProperty();
	
      const std::string& name() const { return name_; }
      const std::string& value() const { return value_; }
      const std::string& source() const { return source_; }
      int line() const { return line_; }

      const std::string& valueInSet(
	  const std::set<std::string>& legalValues
      ) const {
	if (legalValues.find(value()) == legalValues.end()) {
	  std::ostringstream details;
	  details << "Value must be one of \""
		  << util::join(legalValues.begin(), legalValues.end(),
				"\", \"")
		  << "\"";
	  throw InvalidPropertyValueError(*this, value(), details.str());
	}
	return value();
      }
      
      template <typename FormatFn>
      auto valueAs(
	  const FormatFn& format
      ) const -> decltype(format(*(std::string*)0)) {
	return valueAs_(value(), format);
      }
      
      template <typename FormatFn,
		typename Value = decltype((*(FormatFn*)0)(*(std::string*)0))>
      Value valueAs(const FormatFn& format,
		    const std::set<Value>& allowedValues) const {
	return valueInSet_(value(), format, allowedValues);
      }
    
      template <typename FormatFn, typename OutFn>
      void valueAsList(const std::string& separator,
		       const FormatFn& format,
		       const OutFn& output) const {
	static const util::SplitIterator END_OF_SPLIT;
	for (auto i= util::SplitIterator(value(), separator);
	     i != END_OF_SPLIT;
	     ++i) {
	  try {
	    output(valueAs_(*i, format));
	  } catch(const PropertyFormatError& e) {
	    throw InvalidPropertyValueError(*this, *i, e.description());
	  }
	}
      }
      
      template <typename FormatFn>
      auto valueAsList(
	  const std::string& separator,
	  const FormatFn& format
      ) const -> std::vector<decltype(format(*(std::string*)0))> {
	std::vector<decltype(format(value()))> result;
	valueAsList(separator, format,
		    [&result](const decltype(format(value()))& v) {
	  result.push_back(v);
	});
	return std::move(result);
      }
      
      std::vector<std::string> valueAsList(const std::string& separator) const {
	return valueAsList(separator, [](const std::string& v) -> std::string {
	  auto stripped = util::strip(v);
	  if (stripped.empty()) {
	    throw PropertyFormatError("List contains a missing value");
	  }
	  return std::move(stripped);
        });
      }

      std::vector<std::string> valueAsRestrictedList(
	  const std::string& separator,
	  const std::set<std::string>& legalValues
      ) const {
	return valueAsList(
	    separator,
	    [&legalValues,this](const std::string& v) -> std::string {
	      return valueInSet_(util::strip(v), legalValues);
	    }
	);
      }

      template <typename FormatFn>
      auto valueAsSet(
	  const std::string& separator,
	  const FormatFn& format
      ) const -> std::set<decltype(format(*(std::string*)0))> {
	std::set<decltype(format(value()))> result;
	valueAsList(separator, format,
		    [&result](const decltype(format(value()))& v) {
	  result.insert(v);
	});
	return std::move(result);
      }

      std::set<std::string> valueAsSet(const std::string& separator) const {
	return valueAsSet(separator, [](const std::string& v) -> std::string {
	  auto stripped = util::strip(v);
	  if (stripped.empty()) {
	    throw PropertyFormatError("List contains a missing value");
	  }
	  return std::move(stripped);
	});
      }

      std::set<std::string> valueAsRestrictedSet(
	  const std::string& separator,
	  const std::set<std::string>& legalValues
      ) const {
	return valueAsSet(
	    separator,
	    [&legalValues, this](const std::string& v) -> std::string {
	      return valueInSet_(util::strip(v), legalValues);
	    }
        );
      }

      int valueAsInt() const { return valueAsInt_(value()); }
      int valueAsIntInRange(int minValue, int maxValue) const {
	return valueAsInt_(value(), minValue, maxValue);
      }

      template <typename OutFn>
      void valueAsListOfInt(const std::string& separator,
			    const OutFn& out) const {
	valueAsList(separator, [this](const std::string& v) {
	  return this->valueAsInt_(v);
	}, out);
      }

      std::vector<int> valueAsListOfInt(const std::string& separator) const {
	return valueAsList(separator, [this](const std::string& v) {
	  return this->valueAsInt_(v);
	});
      }

      std::vector<int> valueAsListOfInt(const std::string& separator,
					int minValue, int maxValue) const {
	return valueAsList(separator,
			   [this,minValue,maxValue](const std::string& v) {
	  return this->valueAsInt_(v, minValue, maxValue);
	});
      }

      std::set<int> valueAsSetOfInt(const std::string& separator) const {
	return valueAsSet(separator, [this](const std::string& v) {
	  return this->valueAsInt_(v);
	});
      }

      std::set<int> valueAsSetOfInt(const std::string& separator,
				    int minValue, int maxValue) const {
	return valueAsSet(separator,
			  [this, minValue, maxValue](const std::string& v) {
	  return this->valueAsInt_(v, minValue, maxValue);
	});
      }

      double valueAsDouble() const { return valueAsDouble_(value()); }
      double valueAsDoubleInRange(double minValue, double maxValue) const {
	return valueAsDouble_(value(), minValue, maxValue);
      }

      template <typename OutFnT>
      void valueAsListOfDouble(const std::string& separator,
			       const OutFnT& output) const {
	valueAsList(separator, [this](const std::string& v) {
	  return this->valueAsDouble_(v);
	}, output);
      }

      std::vector<double> valueAsListOfDouble(
	  const std::string& separator
      ) const {
	return valueAsList(separator, [this](const std::string& v) {
	  return this->valueAsDouble_(v);
	});
      }

      std::vector<double> valueAsListOfDouble(const std::string& separator,
					      double minValue,
					      double maxValue) const {
	return valueAsList(separator,
			   [this,minValue,maxValue](const std::string& v) {
	  return this->valueAsDouble_(v, minValue, maxValue);
	});
      }

      std::set<double> valueAsSetOfDouble(const std::string& separator) const {
	return valueAsSet(separator, [this](const std::string& v) {
	  return this->valueAsDouble_(v);
	});
      }

      std::set<double> valueAsSetOfDouble(const std::string& separator,
					  double minValue,
					  double maxValue) const {
	return valueAsSet(separator,
			  [this, minValue, maxValue](const std::string& v) {
	  return this->valueAsDouble_(v, minValue, maxValue);
	});
      }

      ConfigurationProperty& operator=(
	  const ConfigurationProperty& other
      ) = default;
      
      ConfigurationProperty& operator=(ConfigurationProperty&& other);

      bool operator==(const ConfigurationProperty& other) const {
	return (name() == other.name()) && (value() == other.value()) &&
	       (source() == other.source()) && (line() == other.line());
      }

      bool operator!=(const ConfigurationProperty& other) const {
	return (name() != other.name()) || (value() != other.value()) ||
	       (source() != other.source()) || (line() != other.line());
      }

      static bool isLegalName(const std::string& name) {
	return std::regex_match(name, LEGAL_NAME_REX_);
      }

    protected:
      template <typename FormatFnT>
      auto valueAs_(
	  const std::string& value, const FormatFnT& format
      ) const -> decltype(format(*(std::string*)0)) {
	try {
	  return format(value);
	} catch(const PropertyFormatError& e) {
	  throw InvalidPropertyValueError(*this, value, e.description());
	} catch(const std::exception& e) {
	  throw InvalidPropertyValueError(*this, value, e.what());
	} catch(...){
	  throw InvalidPropertyValueError(*this, value);
	}
      }

      template <typename FormatFnT, typename ValueT>
      auto valueInSet_(
	  const std::string& value, const FormatFnT& format,
	  const std::set<ValueT>& allowedValues
      ) const -> decltype(format(value)) {
	auto v= valueAs_(value, format);
	if (allowedValues.find(v) == allowedValues.end()) {
	  std::ostringstream details;
	  details << "Value must be one of \""
		  << util::join(allowedValues.begin(), allowedValues.end(),
				"\", \"")
		  << "\"";
	  throw InvalidPropertyValueError(*this, value, details.str());
	}
	return v;
      }

      std::string valueInSet_(
	  std::string&& value, const std::set<std::string>& allowedValues
      ) const {
	if (allowedValues.find(value) == allowedValues.end()) {
	  std::ostringstream details;
	  details << "Value must be one of \""
		  << util::join(allowedValues.begin(), allowedValues.end(),
				"\", \"")
		  << "\"";
	  throw InvalidPropertyValueError(*this, value, details.str());
	}
	return std::move(value);
      }

      int valueAsInt_(const std::string& value, int minValue=INT_MIN,
		      int maxValue=INT_MAX) const;
      double valueAsDouble_(const std::string& value,
			    double minValue=-DBL_MAX,
			    double maxValue=DBL_MAX) const;

    private:
      std::string name_;
      std::string value_;
      std::string source_;
      int line_;

      static const std::regex LEGAL_NAME_REX_;
    };

    std::ostream& operator<<(std::ostream& out,
			     const ConfigurationProperty& p);

  }
}

#endif
