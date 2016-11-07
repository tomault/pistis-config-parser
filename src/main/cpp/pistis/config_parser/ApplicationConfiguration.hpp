/** @file ApplicationConfiguration.hpp
 *
 */
#ifndef __PISTIS__CONFIG_PARSER__APPLICATIONCONFIGURATION_HPP__
#define __PISTIS__CONFIG_PARSER__APPLICATIONCONFIGURATION_HPP__

#include <pistis/util/NumUtil.hpp>
#include <pistis/util/StringUtil.hpp>
#include <pistis/config_parser/ConfigFileParser.hpp>
#include <pistis/config_parser/ConfigurationProperty.hpp>
#include <pistis/config_parser/PropertyFormatError.hpp>
#include <iostream>
#include <map>
#include <memory>
#include <sstream>
#include <set>
#include <string>
#include <vector>

namespace pistis {
  namespace config_parser {

    class ApplicationConfiguration {
    public:
      virtual ~ApplicationConfiguration();
      virtual void load(const std::string& filename);
      virtual void load(const std::string& sourceName,
			std::istream& input, int initialLine=1,
			int initialColumn=1);
      virtual void loadFromText(const std::string& sourceName,
				const std::string& text);
	
    protected:
      template <typename Value>
      class ValueMap {
      public:
	ValueMap(): values_() { }
	ValueMap(const ValueMap<Value>& other)= default;
	ValueMap(ValueMap<Value>&& other):
	    values_(std::move(other.values_)) {
	  // Intentionally left blank
	}
	ValueMap(
	    const std::initializer_list<
	        std::pair<const std::string, Value>
	    >& values
	):
	    values_(values) {
	  // Intentionally left blank
	}

	std::vector<std::string> allKeys() const {
	  std::vector<std::string> keys;
	  keys.reserve(values_.size());
	  for (auto i= values_.begin(); i != values_.end(); ++i) {
	    keys.push_back(i->first);
	  }
	  return keys;
	}
	size_t size() const { return values_.size(); }
	void add(const std::string& name, const Value& value) {
	  values_.insert(std::make_pair(name, value));
	}
	void clear() { values_.clear(); }
	  
	Value operator[](const std::string& name) const {
	  auto i= values_.find(name);
	  if (i == values_.end()) {
	    std::vector<std::string> keys(allKeys());
	    std::ostringstream msg;
	    msg << "Legal values are \""
		<< util::join(keys.begin(), keys.end(), "\", \"")
		<< "\"";
	    throw PropertyFormatError(name, msg.str());
	  }
	  return i->second;
	}

      private:
	std::map<std::string, Value> values_;
      };

      class PropertyHandler {
      public:
	PropertyHandler() { }
	virtual ~PropertyHandler();
	virtual void handle(const ConfigurationProperty& p) = 0;

      protected:
	PropertyHandler(const PropertyHandler&)= delete;
	PropertyHandler& operator=(const PropertyHandler&)= delete;
      };

      template <typename FnT>
      class DelegatingPropertyHandler : public PropertyHandler {
      public:
	DelegatingPropertyHandler(const FnT& f):
	  PropertyHandler(), f_(f) {
	}
	virtual ~DelegatingPropertyHandler() { }
	virtual void handle(const ConfigurationProperty& p) { f_(p); }

      private:
	FnT f_;
      };

      class PropertyInfo {
      public:
	PropertyInfo(const std::string& name, bool isPrefix, bool isRequired,
		     bool allowEmpty, PropertyHandler* handler);	  
	PropertyInfo(PropertyInfo&& other);

	const std::string& name() const { return name_; }
	bool isPrefixHandler() const { return prefix_; }
	bool required() const { return required_; }
	bool allowEmpty() const { return allowEmpty_; }
	bool found() const { return found_; }
	void setFound(bool v) { found_= v; }
	PropertyHandler* handler() const { return handler_.get(); }

	PropertyInfo& operator=(PropertyInfo&& other);
      private:
	std::string name_;
	bool prefix_;
	bool required_;
	bool allowEmpty_;
	bool found_;
	std::unique_ptr<PropertyHandler> handler_;
	  
	PropertyInfo(const PropertyInfo&)= delete;
	PropertyInfo& operator=(const PropertyInfo&)= delete;
      };

      template <typename ValueT>
      class ValueFormatter {
	static_assert(sizeof(ValueT) == 0, "Unknown value type");
      };

      typedef std::map<std::string, PropertyInfo> PropertyInfoMap;

      void applyHandler_(const PropertyInfo& handler,
			 const ConfigurationProperty& property);

    protected:
      ApplicationConfiguration(
	  bool ignoreUnknownProperties, bool useEnvironmentVars= true,
	  ConfigFileParser::DuplicatePropertyMode duplicatePropertyAction =
	      ConfigFileParser::DUP_ERROR,
	  ConfigFileParser::DuplicatePropertyMode includedPropertyAction =
	      ConfigFileParser::DUP_IGNORE
      );

      virtual void load_(const std::string& sourceName,
			 const ConfigurationPropertyMap& properties);

      template <typename ValueT>
      void registerProperty_(const std::string& name, bool required,
			     bool allowEmpty, ValueT& v) {
	registerProperty_(
	     createInfo_(name, false, required, allowEmpty,
			 [&v](const ConfigurationProperty& p) {
	       v= ValueFormatter<ValueT>::format(p);
	     })
	);
      }

      template <typename ValueT>
      void registerProperty_(const std::string& name, bool required,
			     bool allowEmpty,
			     const ValueMap<ValueT>& valueMap,
			     ValueT& v) {
	registerProperty_(
	     createInfo_(name, false, required, allowEmpty,
			 [&v, valueMap](const ConfigurationProperty& p) {
	       v= valueMap[p.value()];
	     })
	);
      }

      template <typename ValueT>
      void registerProperty_(
	  const std::string& name, bool required, bool allowEmpty,
	  const std::function<ValueT (const std::string&)>& format,
	  ValueT& v
      ) {
	registerProperty_(
	    createInfo_(name, false, required, allowEmpty,
			[&v, format, this](const ConfigurationProperty& p) {
	      v= p.valueAs(format);
	    })
	);	  	  
      }

      template <typename ValueT>
      void registerProperty_(const std::string& name, bool required,
			     bool allowEmpty, const std::string& separator,
			     std::vector<ValueT>& v) {
	registerProperty_(
	     createInfo_(name, false, required, allowEmpty,
			 [&v, separator, this](const ConfigurationProperty& p) {
	       v= ValueFormatter<ValueT>::asList(p, separator);
	     })
	);
      }

      template <typename ValueT>
      void registerProperty_(const std::string& name, bool required,
			     bool allowEmpty, const std::string& separator,
			     const ValueMap<ValueT>& valueMap,
			     std::vector<ValueT>& v) {
	registerProperty_(
	     createInfo_(
	         name, false, required, allowEmpty,
		 [&v, valueMap, separator](const ConfigurationProperty& p) {
		   v = p.valueAsList(separator,
				     [valueMap](const std::string& text) {
		     return valueMap[util::strip(text)];
	           });
		 }
	     )
	);
      }

      template <typename ValueT>
      void registerProperty_(const std::string& name, bool required,
			     bool allowEmpty, const std::string& separator,
			     const std::function<ValueT (const std::string&)>& format,
			     std::vector<ValueT>& v) {
	registerProperty_(
	    createInfo_(
	        name, false, required, allowEmpty,
		[&v, format, separator](const ConfigurationProperty& p) {
	          v= p.valueAsList(separator, format);
	        }
	    )
	);
      }

      template <typename ValueT>
      void registerProperty_(const std::string& name, bool required,
			     bool allowEmpty, const std::string& separator,
			     std::set<ValueT>& v) {
	registerProperty_(
	    createInfo_(name, false, required, allowEmpty,
			[&v, separator](const ConfigurationProperty& p) {
	      v= ValueFormatter<ValueT>::asSet(p, separator);
	    })
        );
      }

      template <typename ValueT>
      void registerProperty_(const std::string& name, bool required,
			     bool allowEmpty, const std::string& separator,
			     const ValueMap<ValueT>& valueMap,
			     std::set<ValueT>& v) {
	registerProperty_(
	  createInfo_(
	      name, false, required, allowEmpty,
	      [&v, valueMap, separator](const ConfigurationProperty& p) {
	        v = p.valueAsSet(separator,
				 [valueMap](const std::string& text) {
		  return valueMap[util::strip(text)];
                });
              }
	  )
	);
      }

      template <typename ValueT>
      void registerProperty_(
          const std::string& name, bool required, bool allowEmpty,
	  const std::string& separator,
	  const std::function<ValueT (const std::string&)>& format,
	  std::set<ValueT>& v
      ) {
	registerProperty_(
	    createInfo_(name, false, required, allowEmpty,
			[&v, format, separator](
			    const ConfigurationProperty& p
	                ) {
	      v= p.valueAsSet(separator, format);
	    })
	);
      }

      template <typename ValueT>
      void registerProperty_(
          const std::string& name, bool required,
	  bool allowEmpty,
	  const std::function<void (const ConfigurationProperty&)>& handler
      ) {
	registerProperty_(
	    createInfo_(name, false, required, allowEmpty,
			[&handler](const ConfigurationProperty& p) {
	      handler(p);
	    })
	);
      }

      template <typename ValueT>
      void registerPropertyInRange_(const std::string& name, bool required,
				    bool allowEmpty, ValueT minValue,
				    ValueT maxValue, ValueT& v) {
	registerProperty_(
	    createInfo_(name, false, required, allowEmpty,
		        [&v, minValue, maxValue](
			    const ConfigurationProperty& p
			) {
	      v= ValueFormatter<ValueT>::formatInRange(p, minValue, maxValue);
	    })
	);
      }

      template <typename ValueT>
      void registerPropertyInRange_(const std::string& name, bool required,
				    bool allowEmpty,
				    const std::string& separator,
				    ValueT minValue, ValueT maxValue,
				    std::vector<ValueT>& v) {
	registerProperty_(
	    createInfo_(name, false, required, allowEmpty,
			[&v, separator, minValue, maxValue](
			    const ConfigurationProperty& p
		        ) {
	      v= ValueFormatter<ValueT>::asList(p, separator, minValue,
						maxValue);
	    })
	);
      }

      template <typename ValueT>
      void registerPropertyInRange_(const std::string& name, bool required,
				    bool allowEmpty,
				    const std::string& separator,
				    ValueT minValue, ValueT maxValue,
				    std::set<ValueT>& v) {
	registerProperty_(
	    createInfo_(name, false, required, allowEmpty,
		        [&v, separator, minValue, maxValue](
			    const ConfigurationProperty& p
			) {
	      v= ValueFormatter<ValueT>::asSet(p, separator, minValue,
					       maxValue);
	    })
	);
      }

      template <typename ValueT>
      void registerPropertyInSet_(const std::string& name, bool required,
				  bool allowEmpty,
				  const std::set<ValueT>& legalValues,
				  ValueT& v) {
	registerProperty_(
	    createInfo_(name, false, required, allowEmpty,
			[&v, legalValues](const ConfigurationProperty& p) {
	      v= ValueFormatter<ValueT>::formatInSet(p, legalValues);
	    })
	);
      }

      // Renamed to work around an internal compiler error in gcc 4.6.2
      template <typename ValueT>
      void registerListPropertyInSet_(const std::string& name, bool required,
				      bool allowEmpty,
				      const std::string& separator,
				      const std::set<ValueT>& legalValues,
				      std::vector<ValueT>& v) {
	registerProperty_(
	    createInfo_(name, false, required, allowEmpty,
			[&v, separator, legalValues](
			    const ConfigurationProperty& p
			) {
	      v= ValueFormatter<ValueT>::asList(p, separator, legalValues);
	    })
        );
      }

      // Renamed to work around an internal compiler error in gcc 4.6.2
      template <typename ValueT>
      void registerSetPropertyInSet_(const std::string& name, bool required,
				     bool allowEmpty,
				     const std::string& separator,
				     const std::set<ValueT>& legalValues,
				     std::set<ValueT>& v) {
	registerProperty_(
	    createInfo_(name, false, required, allowEmpty,
			[&v, separator, legalValues](
			    const ConfigurationProperty& p
			) {
	      v= ValueFormatter<ValueT>::asSet(p, separator, legalValues);
	    })
	);
      }

      template <typename ValueT>
      void registerPropertyPrefix_(const std::string& prefix, bool required,
				   bool allowEmpty, std::vector<ValueT>& v) {
	registerProperty_(
	    createInfo_(prefix, true, required, allowEmpty,
			[&v](const ConfigurationProperty& p) {
	      v.push_back(ValueFormatter<ValueT>::format(p));
	    })
	);
      }

      template <typename ValueT>
      void registerPropertyPrefix_(const std::string& prefix, bool required,
				   bool allowEmpty,
				   const ValueMap<ValueT>& valueMap,
				   std::vector<ValueT>& v) {
	registerProperty_(
	    createInfo_(prefix, true, required, allowEmpty,
			[&v,valueMap](const ConfigurationProperty&p ) {
	      v.push_back(valueMap[p.value()]);
	    })
        );
      }

      template <typename ValueT>
      void registerPropertyPrefix_(
	  const std::string& prefix, bool required, bool allowEmpty,
	  const std::function<ValueT (const std::string&)>& format,
	  std::vector<ValueT>& v
      ) {
	registerProperty_(
	    createInfo_(prefix, true, required, allowEmpty,
			[&v,format](const ConfigurationProperty& p) {
	      v.push_back(p.valueAs(format));
	    })
	);
      }

      template <typename ValueT>
      void registerPropertyPrefix_(const std::string& prefix, bool required,
				   bool allowEmpty, std::set<ValueT>& v) {
        registerProperty_(
	    createInfo_(prefix, true, required, allowEmpty,
			[&v](const ConfigurationProperty& p) {
	      v.insert(ValueFormatter<ValueT>::format(p));
	    })
	);
      }

      template <typename ValueT>
      void registerPropertyPrefix_(const std::string& prefix, bool required,
				   bool allowEmpty,
				   const ValueMap<ValueT>& valueMap,
				   std::set<ValueT>& v) {
        registerProperty_(
	    createInfo_(prefix, true, required, allowEmpty,
			[&v,valueMap](const ConfigurationProperty& p) {
	      v.insert(valueMap[p.value()]);
	    })
        );
      }

      template <typename ValueT>
      void registerPropertyPrefix_(
	  const std::string& prefix, bool required, bool allowEmpty,
	  const std::function<ValueT (const std::string&)>& format,
	  std::set<ValueT>& v
      ) {
	registerProperty_(
	    createInfo_(prefix, true, required, allowEmpty,
			[&v,format](const ConfigurationProperty& p) {
	      v.insert(p.valueAs(format));
	    })
        );
      }

      template <typename ValueT>
      void registerPropertyPrefix_(
	  const std::string& prefix, bool required, bool allowEmpty,
	  const std::function<void (const ConfigurationProperty&)>& handler
      ) {
	registerProperty_(
	    createInfo_(prefix, true, required, allowEmpty,
			[&handler](const ConfigurationProperty& p) {
	      handler(p);
	    })
	);
      }

      void registerProperty_(PropertyInfo&& info);

      template <typename FnT>
      PropertyInfo createInfo_(const std::string& name, bool isPrefix,
			       bool isRequired, bool allowEmpty,
			       const FnT& f) {
	return PropertyInfo(name, isPrefix, isRequired, allowEmpty,
			    new DelegatingPropertyHandler<FnT>(f));
      }

      template <typename ValueT>
      void registerPropertyPrefixInRange_(const std::string& prefix,
					  bool required,
					  bool allowEmpty,
					  ValueT minValue, ValueT maxValue,
					  std::vector<ValueT>& v) {
	registerProperty_(
	    createInfo_(prefix, true, required, allowEmpty,
			[&v,minValue,maxValue](const ConfigurationProperty& p) {
	      v.push_back(ValueFormatter<ValueT>::formatInRange(p,minValue,maxValue));
	    })
	);
      }

      template <typename ValueT>
      void registerPropertyPrefixInRange_(const std::string& prefix,
					  bool required,
					  bool allowEmpty,
					  ValueT minValue, ValueT maxValue,
					  std::set<ValueT>& v) {
	registerProperty_(
	    createInfo_(prefix, true, required, allowEmpty,
			[&v,minValue,maxValue](const ConfigurationProperty& p) {
	      v.insert(ValueFormatter<ValueT>::formatInRange(p,minValue,maxValue));
	    })
	);
      }

      template <typename ValueT>
      void registerPropertyPrefixInSet_(const std::string& prefix,
					bool required,
					bool allowEmpty,
					const std::set<ValueT>& legalValues,
					std::vector<ValueT>& v) {
	registerProperty_(
	    createInfo_(prefix, true, required, allowEmpty,
			[&v,legalValues](const ConfigurationProperty& p) {
	      v.push_back(ValueFormatter<ValueT>::formatInSet(p, legalValues));
	    })
	);
      }

      template <typename ValueT>
      void registerPropertyPrefixInSet_(const std::string& prefix,
					bool required,
					bool allowEmpty,
					const std::set<ValueT>& legalValues,
					std::set<ValueT>& v) {
	registerProperty_(
	    createInfo_(prefix, true, required, allowEmpty,
			[&v,legalValues](const ConfigurationProperty& p) {
	      v.insert(ValueFormatter<ValueT>::formatInSet(p, legalValues));
	    })
	);
      }

    private:
      PropertyInfoMap handlers_;
      bool ignoreUnknownProperties_;
      bool useEnvironmentVars_;
      ConfigFileParser::DuplicatePropertyMode duplicatePropertyAction_;
      ConfigFileParser::DuplicatePropertyMode includedPropertyAction_;
    };

    template<>
    class ApplicationConfiguration::ValueFormatter<int> {
    public:
      static int format(const ConfigurationProperty& p) {
	return p.valueAsInt();
      }

      static int formatInRange(const ConfigurationProperty& p,
			       int minValue, int maxValue) {
	return p.valueAsIntInRange(minValue, maxValue);
      }

      static int formatInSet(const ConfigurationProperty& p,
			     const std::set<int>& legalValues) {
	return p.valueAs([&legalValues](const std::string& v) {
	    return convert_(v);
	}, legalValues);
      }

      static std::vector<int> asList(const ConfigurationProperty& p,
				     const std::string& separator) {
	return p.valueAsListOfInt(separator);
      }

      static std::vector<int> asList(const ConfigurationProperty& p,
				     const std::string& separator,
				     int minValue, int maxValue) {
	return p.valueAsListOfInt(separator, minValue, maxValue);
      }

      static std::vector<int> asList(const ConfigurationProperty& p,
				     const std::string& separator,
				     const std::set<int>& legalValues) {
	return p.valueAsList(separator,
			     [&legalValues](const std::string& value) {
	  return convert_(value, legalValues);
	});
      }

      static std::set<int> asSet(const ConfigurationProperty& p,
				 const std::string& separator) {
	return p.valueAsSetOfInt(separator);
      }

      static std::set<int> asSet(const ConfigurationProperty& p,
				 const std::string& separator,
				 int minValue, int maxValue) {
	return p.valueAsSetOfInt(separator, minValue, maxValue);
      }

      static std::set<int> asSet(const ConfigurationProperty& p,
				 const std::string& separator,
				 const std::set<int>& legalValues) {
	return p.valueAsSet(separator,
			    [&legalValues](const std::string& value) {
	  return convert_(value, legalValues);
	});
      }

    private:
      static int convert_(const std::string& v) {
	auto i= util::toInt64Quietly(v);
	if (i.second != util::NumConversionResult::OK) {
	  throw PropertyFormatError(util::descriptionFor(i.second));
	}
	return i.first;
      }

      static int convert_(const std::string& value,
			  const std::set<int>& legalValues) {
	int v= convert_(value);
	if (legalValues.find(v) == legalValues.end()) {
	  std::ostringstream msg;
	  msg << "Value must be one of "
	      << util::join(legalValues.begin(), legalValues.end(), ", ");
	  throw PropertyFormatError(msg.str());
	}
	return v;
      }
    };

    template<>
    class ApplicationConfiguration::ValueFormatter<double> {
    public:
      static double format(const ConfigurationProperty& p) {
	return p.valueAsDouble();
      }

      static double formatInRange(const ConfigurationProperty& p,
				  double minValue, double maxValue) {
	return p.valueAsDoubleInRange(minValue, maxValue);
      }

      static double formatInSet(const ConfigurationProperty& p,
				const std::set<double>& legalValues) {
	return p.valueAs(
	    [&legalValues](const std::string& v) {
	      return convert_(v, legalValues);
	    },
	    legalValues
	);
      }

      static std::vector<double> asList(const ConfigurationProperty& p,
					const std::string& separator) {
	return p.valueAsListOfDouble(separator);
      }

      static std::vector<double> asList(const ConfigurationProperty& p,
					const std::string& separator,
					double minValue, double maxValue) {
	return p.valueAsListOfDouble(separator, minValue, maxValue);
      }

      static std::vector<double> asList(const ConfigurationProperty& p,
					const std::string& separator,
					const std::set<double>& legalValues) {
	return p.valueAsList(separator,
			     [&legalValues](const std::string& value) {
	  return convert_(value, legalValues);
	});
      }

      static std::set<double> asSet(const ConfigurationProperty& p,
				    const std::string& separator) {
	return p.valueAsSetOfDouble(separator);
      }

      static std::set<double> asSet(const ConfigurationProperty& p,
				    const std::string& separator,
				    double minValue, double maxValue) {
	return p.valueAsSetOfDouble(separator, minValue, maxValue);
      }

      static std::set<double> asSet(const ConfigurationProperty& p,
				    const std::string& separator,
				    const std::set<double>& legalValues) {
	return p.valueAsSet(separator,
			    [&legalValues](const std::string& value) {
	  return convert_(value, legalValues);
	});
	return p.valueAsSetOfDouble(separator);
      }
      
    private:
      static double convert_(const std::string& v) {
	auto i= util::toDoubleQuietly(v);
	if (i.second != util::NumConversionResult::OK) {
	  throw PropertyFormatError(util::descriptionFor(i.second));
	}
	return i.first;
      }

      static double convert_(const std::string& value,
			     const std::set<double>& legalValues) {
	double v= convert_(value);
	if (legalValues.find(v) == legalValues.end()) {
	  std::ostringstream msg;
	  msg << "Value must be one of "
	      << util::join(legalValues.begin(), legalValues.end(), ", ");
	  throw PropertyFormatError(msg.str());
	}
	return v;
      }
    };

    template<>
    class ApplicationConfiguration::ValueFormatter<std::string> {
    public:
      static const std::string& format(const ConfigurationProperty& p) {
	return p.value();
      }

      static const std::string& formatInRange(const ConfigurationProperty& p,
					      const std::string& minValue,
					      const std::string& maxValue) {
	return inRange_(p.value(), minValue, maxValue);
      }

      static const std::string& formatInSet(
	  const ConfigurationProperty& p,
	  const std::set<std::string>& legalValues
      ) {
	return p.valueInSet(legalValues);
      }

      static std::vector<std::string> asList(const ConfigurationProperty& p,
					     const std::string& separator) {
	return p.valueAsList(separator);
      }

      static std::vector<std::string> asList(const ConfigurationProperty& p,
					     const std::string& separator,
					     const std::string& minValue,
					     const std::string& maxValue) {
	return p.valueAsList(separator,
			     [&minValue,&maxValue](const std::string& value) {
	  std::string stripped= util::strip(value);
	  if (stripped.empty()) {
	    throw PropertyFormatError("Value missing in list");
	  }
	  return inRange_(stripped, minValue, maxValue);
	});
      }

      static std::vector<std::string> asList(
	  const ConfigurationProperty& p,
	  const std::string& separator,
	  const std::set<std::string>& legalValues
      ) {
	return p.valueAsRestrictedList(separator, legalValues);
      }

      static std::set<std::string> asSet(const ConfigurationProperty& p,
					 const std::string& separator) {
	return p.valueAsSet(separator);
      }

      static std::set<std::string> asSet(const ConfigurationProperty& p,
					 const std::string& separator,
					 const std::string& minValue,
					 const std::string& maxValue) {
	return p.valueAsSet(separator,
			    [&minValue,&maxValue](const std::string& text) {
	  std::string stripped= util::strip(text);
	  if (stripped.empty()) {
	    throw PropertyFormatError("Value missing in list");
	  }
	  return inRange_(stripped, minValue, maxValue);
        });
      }

      static std::set<std::string> asSet(
	  const ConfigurationProperty& p,
	  const std::string& separator,
	  const std::set<std::string>& legalValues
      ) {
	return p.valueAsRestrictedSet(separator, legalValues);
      }

    private:
      static const std::string& inRange_(const std::string& v,
					 const std::string& minValue,
					 const std::string& maxValue) {
	if ((v < minValue) || (v > maxValue)) {
	  std::ostringstream msg;
	  msg << "Value must be between \"" << minValue << "\" and \""
	      << maxValue << "\" (inclusive)";
	  throw PropertyFormatError(msg.str());
	}
	return v;
      }
    };

  }
}

#endif
