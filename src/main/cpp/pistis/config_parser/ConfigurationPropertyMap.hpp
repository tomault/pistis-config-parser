#ifndef __PISTIS__CONFIG_PARSER__CONFIGURATIONPROPERTYMAP_HPP__
#define __PISTIS__CONFIG_PARSER__CONFIGURATIONPROPERTYMAP_HPP__

#include <pistis/config_parser/ConfigurationProperty.hpp>
#include <iterator>
#include <map>

namespace pistis {
  namespace config_parser {

    class ConfigurationPropertyMap {
    protected:

      template <typename Derived, typename Value>
      class BaseIterator {
      public:
	typedef std::forward_iterator_tag iterator_category;
	typedef Value value_type;
	typedef ptrdiff_t difference_type;
	typedef const Value& reference;
	typedef const Value* pointer;

      public:
	Derived& operator=(const Derived& other) {
	  p_= other.p_;
	  return static_cast<Derived&>(*this);
	}
	bool operator==(const Derived& other) const {
	  return p_ == other.p_;
	}
	bool operator!=(const Derived& other) const {
	  return p_ != other.p_;
	}
	Derived& operator++() {
	  ++p_;
	  return static_cast<Derived&>(*this);
	}
	Derived  operator++(int) {
	  Derived tmp(static_cast<Derived&>(*this));
	  ++p_;
	  return tmp;
	}

      protected:
	BaseIterator(): p_() { }
	BaseIterator(
	    const std::map<std::string, ConfigurationProperty>::const_iterator& p
	):
	    p_(p) {
	  // Intentionally left blank
	}
	std::map<std::string, ConfigurationProperty>::const_iterator p_;
      };

    public:
      class PropertyIterator :
	  public BaseIterator<PropertyIterator, ConfigurationProperty> {
      public:
	PropertyIterator():
	  BaseIterator<PropertyIterator, ConfigurationProperty>() {
	}
	  
	const ConfigurationProperty& operator*() const {
	  return p_->second;
	}
	const ConfigurationProperty* operator->() const {
	  return &(p_->second);
	}

      private:
	PropertyIterator(
	    const std::map<std::string, ConfigurationProperty>::const_iterator& p
        ):
	    BaseIterator<PropertyIterator, ConfigurationProperty>(p) {
	  // Intentionally left blank
	}
	friend class ConfigurationPropertyMap;
      };

      class NameIterator : public BaseIterator<NameIterator, std::string> {
      public:
	NameIterator() { }

	const std::string& operator*() const { return p_->first; }
	const std::string* operator->() const { return &(p_->first); }

      private:
	NameIterator(
	    const std::map<std::string, ConfigurationProperty>::const_iterator& p
	):
	    BaseIterator<NameIterator, std::string>(p) {
	  // Intentionally left blank
	}
	friend class ConfigurationPropertyMap;
      };

    public:
      ConfigurationPropertyMap();
      ConfigurationPropertyMap(const ConfigurationPropertyMap& other) = default;
      ConfigurationPropertyMap(ConfigurationPropertyMap&& other);
      ~ConfigurationPropertyMap();

      bool empty() const { return properties_.empty(); }
      size_t size() const { return properties_.size(); }

      PropertyIterator begin() const {
	return PropertyIterator(properties_.begin());
      }
      PropertyIterator end() const {
	return PropertyIterator(properties_.end());
      }
      NameIterator beginNames() const {
	return NameIterator(properties_.begin());
      }
      NameIterator endNames() const {
	return NameIterator(properties_.end());
      }

      bool hasKey(const std::string& key) const {
	return properties_.find(key) != properties_.end();
      }

      const std::string& getValue(const std::string& key,
				  const std::string& dv) const {
	auto i= properties_.find(key);
	return (i != properties_.end()) ? i->second.value() : dv;
      }

      int getValueAsInt(const std::string& key, int dv) const {
	auto i= properties_.find(key);
	return (i != properties_.end()) ? i->second.valueAsInt() : dv;
      }

      double getValueAsDouble(const std::string& key, double dv) const {
	auto i= properties_.find(key);
	return (i != properties_.end()) ? i->second.valueAsDouble() : dv;
      }

      template <typename Fn>
      auto getValue(
	  const std::string& key,
	  const decltype((*(Fn*)0)(*(std::string*)0))& dv,
	  const Fn& format
      ) const -> decltype(format(*(std::string*)0)) {
	auto i= properties_.find(key);
	return (i != properties_.end()) ? i->second.valueAs(format) : dv;
      }

      template <typename Fn, typename OutFn>
      void getPropertiesMatching(const Fn& f, const OutFn& output) const {
	for (auto i= properties_.begin(); i != properties_.end(); ++i) {
	  if (f(i->first)) {
	    output(i->second);
	  }
	}
      }

      template <typename Fn>
      std::vector<ConfigurationProperty> getPropertiesMatching(
	  const Fn& f
      ) const {
	std::vector<ConfigurationProperty> result;
	getPropertiesMatching(f, [&result](const ConfigurationProperty& p) {
          result.push_back(p);
	});
	return std::move(result);
      }

      template <typename OutFn>
      void getPropertiesWithPrefix(const std::string& prefix,
				   const OutFn& output) const {
	auto i= properties_.lower_bound(prefix);
	while ((i != properties_.end()) && util::startsWith(i->first, prefix)) {
	  output(i->second);
	  ++i;
	}
      }

      std::vector<ConfigurationProperty> getPropertiesWithPrefix(
	  const std::string& prefix
      ) const;

      void add(const ConfigurationProperty& p);
      void add(ConfigurationProperty&& p);
      void erase(const std::string& key) { properties_.erase(key); }
      void clear() { properties_.clear(); }

      ConfigurationPropertyMap& operator=(
	  const ConfigurationPropertyMap& other
      ) = default;
      
      ConfigurationPropertyMap& operator=(ConfigurationPropertyMap&& other) {
	properties_= std::move(other.properties_);
	return *this;
      }
      const ConfigurationProperty& operator[](const std::string& key) const;

    private:
      std::map<std::string, ConfigurationProperty> properties_;
    };

  }
}
#endif

