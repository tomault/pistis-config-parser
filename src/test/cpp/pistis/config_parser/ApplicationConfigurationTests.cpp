/** @file ApplicationConfigurationTests.cpp
 *
 *  Unit tests for pistis::config_parser::ApplicationConfiguration
 */

#include <pistis/typeutil/Enum.hpp>
#include <pistis/util/StringUtil.hpp>
#include <pistis/config_parser/ApplicationConfiguration.hpp>
#include <pistis/config_parser/InvalidPropertyValueError.hpp>
#include <pistis/config_parser/PropertyFormatError.hpp>
#include <pistis/config_parser/RequiredPropertyMissingError.hpp>
#include <pistis/config_parser/UnknownPropertyError.hpp>
#include <gtest/gtest.h>
#include <sstream>

using namespace pistis::typeutil;
using namespace pistis::util;
using namespace pistis::config_parser;

namespace {
  class Colors : public Enum<Colors> {
  public:
    static const Colors RED;
    static const Colors GREEN;
    static const Colors BLUE;
    static const Colors WHITE;
    static const Colors BLACK;

  public:
    Colors(): Enum<Colors>(RED) { }

  private:
    Colors(int value, const std::string& name): Enum(value, name) { }
  };

  const Colors Colors::RED(1, "RED");
  const Colors Colors::GREEN(2, "GREEN");
  const Colors Colors::BLUE(3, "BLUE");
  const Colors Colors::WHITE(4, "WHITE");
  const Colors Colors::BLACK(5, "BLACK");

  class TestAppConfig : public ApplicationConfiguration {
  public:
    TestAppConfig(bool ignoreUnknownProperties);

    int intValue() const { return i_; }
    double doubleValue() const { return d_; }
    const std::string& strValue() const { return s_; }
    Colors enumValue() const { return e_; }
    const std::string& fmtValue() const { return f_; }

    int intInRange() const { return ri_; }
    double doubleInRange() const { return rd_; }
    const std::string& strInRange() const { return rs_; }

    int intInSet() const { return iis_; }
    double doubleInSet() const { return dis_; }
    const std::string& strInSet() const { return sis_; }

    const std::vector<int>& intList() const { return iv_; }
    const std::vector<double>& doubleList() const { return dv_; }
    const std::vector<std::string>& strList() const { return sv_; }
    const std::vector<Colors>& enumList() const { return ev_; }
    const std::vector<std::string>& fmtList() const { return fv_; }

    const std::vector<int>& intListInRange() const { return riv_; }
    const std::vector<double>& doubleListInRange() const { return rdv_; }
    const std::vector<std::string>& strListInRange() const { return rsv_; }

    const std::vector<int>& intListInSet() const { return iisv_; }
    const std::vector<double>& doubleListInSet() const { return disv_; }
    const std::vector<std::string>& strListInSet() const { return sisv_; }

    const std::set<int>& intSet() const { return is_; }
    const std::set<double>& doubleSet() const { return ds_; }
    const std::set<std::string>& strSet() const { return ss_; }
    const std::set<Colors>& enumSet() const { return es_; }
    const std::set<std::string>& fmtSet() const { return fs_; }

    const std::set<int>& intSetInRange() const { return ris_; }
    const std::set<double>& doubleSetInRange() const { return rds_; }
    const std::set<std::string>& strSetInRange() const { return rss_; }

    const std::set<int>& intSetInSet() const { return iiss_; }
    const std::set<double>& doubleSetInSet() const { return diss_; }
    const std::set<std::string>& strSetInSet() const { return siss_; }

    const std::vector<int>& intPrefixList() const { return ipv_; }
    const std::vector<double>& doublePrefixList() const { return dpv_; }
    const std::vector<std::string>& strPrefixList() const { return spv_; }
    const std::vector<Colors>& enumPrefixList() const { return epv_; }
    const std::vector<std::string>& fmtPrefixList() const { return fpv_; }

    const std::vector<int>& intPrefixListInRange() const { return ripv_; }
    const std::vector<double>& doublePrefixListInRange() const { return rdpv_; }
    const std::vector<std::string> strPrefixListInRange() const {
      return rspv_;
    }
    const std::vector<int>& intPrefixListInSet() const { return iispv_; }
    const std::vector<double>& doublePrefixListInSet() const { return dispv_; }
    const std::vector<std::string> strPrefixListInSet() const { return sispv_; }

    const std::set<int>& intPrefixSet() const { return ips_; }
    const std::set<double>& doublePrefixSet() const { return dps_; }
    const std::set<std::string>& strPrefixSet() const { return sps_; }
    const std::set<Colors>& enumPrefixSet() const { return eps_; }
    const std::set<std::string>& fmtPrefixSet() const { return fps_; }

    const std::set<int>& intPrefixSetInRange() const { return rips_; }
    const std::set<double>& doublePrefixSetInRange() const { return rdps_; }
    const std::set<std::string> strPrefixSetInRange() const { return rsps_; }
    const std::set<int>& intPrefixSetInSet() const { return iisps_; }
    const std::set<double>& doublePrefixSetInSet() const { return disps_; }
    const std::set<std::string> strPrefixSetInSet() const { return sisps_; }

  private:
    int i_;
    double d_;
    std::string s_;
    Colors e_;
    std::string f_;  ///< Formatted value
    std::vector<int> iv_;
    std::vector<double> dv_;
    std::vector<std::string> sv_;
    std::vector<Colors> ev_;
    std::vector<std::string> fv_; // Formatted values
    std::set<int> is_;
    std::set<double> ds_;
    std::set<std::string> ss_;
    std::set<Colors> es_;
    std::set<std::string> fs_;  // Formatted values
    std::vector<int> ipv_;  // Prefix to vector
    std::vector<double> dpv_;
    std::vector<std::string> spv_;
    std::vector<Colors> epv_;
    std::vector<std::string> fpv_; // Formatted values
    std::set<int> ips_;
    std::set<double> dps_;
    std::set<std::string> sps_;
    std::set<Colors> eps_;
    std::set<std::string> fps_;  // Formatted values

    // Values restricted to a range
    int ri_;  // Restricted to -10 to 10
    std::vector<int> riv_;
    std::set<int> ris_;
    std::vector<int> ripv_;
    std::set<int> rips_;

    double rd_; // Restricted to 0.0 to 1.0
    std::vector<double> rdv_;
    std::set<double> rds_;
    std::vector<double> rdpv_;
    std::set<double> rdps_;

    std::string rs_;  // Restricted to "a" to "b"
    std::vector<std::string> rsv_;
    std::set<std::string> rss_;
    std::vector<std::string> rspv_;
    std::set<std::string> rsps_;

    // Values restricted to a set
    int iis_;  // Restricted to 10, 20, 30, 40, 50
    std::vector<int> iisv_;
    std::set<int> iiss_;
    std::vector<int> iispv_;
    std::set<int> iisps_;

    double dis_; // Restricted to 0.25, 0.5, 1.0, 2.0, 4.0
    std::vector<double> disv_;
    std::set<double> diss_;
    std::vector<double> dispv_;
    std::set<double> disps_;

    std::string sis_; // Restricted to "alpha", "beta", "gamma"
    std::vector<std::string> sisv_;
    std::set<std::string> siss_;
    std::vector<std::string> sispv_;
    std::set<std::string> sisps_;
  };

  TestAppConfig::TestAppConfig(bool ignoreUnknownProperties):
    ApplicationConfiguration(ignoreUnknownProperties) {
    ValueMap<Colors> valueMap({ { "red", Colors::RED },
	                        { "green", Colors::GREEN },
				{ "blue", Colors::BLUE },
				{ "black", Colors::BLACK },
				{ "white", Colors::WHITE } });
    std::function<std::string (const std::string&)> fmt(
        [](const std::string& text) {
          std::string stripped= strip(text);
	  if ((stripped.size() < 3) || !startsWith(stripped, "##")) {
	    throw PropertyFormatError(text, "Value does not begin with '##'");
	  }
	  return stripped.substr(2);
	}
    );
  
  
    registerProperty_("int.single", true, false, i_);
    registerProperty_("double.single", false, false, d_);
    registerProperty_("str.single", false, false, s_);
    registerProperty_("enum.single", false, false, valueMap, e_);
    registerProperty_("fmt.single", false, false, fmt, f_);

    registerProperty_("int.list", false, true, ",", iv_);
    registerProperty_("double.list", false, false, ",", dv_);
    registerProperty_("str.list", false, false, ",", sv_);
    registerProperty_("enum.list", false, false, ",", valueMap, ev_);
    registerProperty_("fmt.list", false, false, ",", fmt, fv_);

    registerProperty_("int.set", false, false, ",", is_);
    registerProperty_("double.set", false, false, ",", ds_);
    registerProperty_("str.set", false, false, ",", ss_);
    registerProperty_("enum.set", false, false, ",", valueMap, es_);
    registerProperty_("fmt.set", false, false, ",", fmt, fs_);

    registerPropertyInRange_("int.in_range", false, false, -10, 10, ri_);
    registerPropertyInRange_("int.list_in_range", false, false, ",",
			     -10, 10, riv_);
    registerPropertyInRange_("int.set_in_range", false, false, ",",
			     -10, 10, ris_);

    registerPropertyInRange_("double.in_range", false, false, 0.0, 1.0, rd_);
    registerPropertyInRange_("double.list_in_range", false, false, ",",
			     0.0, 1.0, rdv_);
    registerPropertyInRange_("double.set_in_range", false, false, ",",
			     0.0, 1.0, rds_);

    registerPropertyInRange_("str.in_range", false, false, std::string("a"),
			     std::string("b"), rs_);
    registerPropertyInRange_("str.list_in_range", false, false, ",",
			     std::string("a"), std::string("b"), rsv_);
    registerPropertyInRange_("str.set_in_range", false, false, ",",
			     std::string("a"), std::string("b"), rss_);

    registerPropertyInSet_("int.in_set", false, false,
			   {10, 20, 30, 40, 50}, iis_);
    registerListPropertyInSet_("int.list_in_set", false, false, ",",
			       { 10, 20, 30, 40, 50 }, iisv_);
    registerSetPropertyInSet_("int.set_in_set", false, false, ",",
			      { 10, 20, 30, 40, 50 }, iiss_);

    registerPropertyInSet_("double.in_set", false, false,
			   { 0.25, 0.5, 1.0, 2.0, 4.0 }, dis_);
    registerListPropertyInSet_("double.list_in_set", false, false, ",",
			       { 0.25, 0.5, 1.0, 2.0, 4.0 }, disv_);
    registerSetPropertyInSet_("double.set_in_set", false, false, ",",
			      { 0.25, 0.5, 1.0, 2.0, 4.0 }, diss_);

    registerPropertyInSet_("str.in_set", false, false,
			   { "alpha", "beta", "gamma" }, sis_);
    registerListPropertyInSet_("str.list_in_set", false, false, ",",
			       { "alpha", "beta", "gamma" }, sisv_);
    registerSetPropertyInSet_("str.set_in_set", false, false, ",",
			      { "alpha", "beta", "gamma" }, siss_);

    registerPropertyPrefix_("int.list_prefix.", false, false, ipv_);
    registerPropertyPrefix_("double.list_prefix.", false, false, dpv_);
    registerPropertyPrefix_("str.list_prefix.", false, false, spv_);
    registerPropertyPrefix_("enum.list_prefix.", false, false, valueMap, epv_);
    registerPropertyPrefix_("fmt.list_prefix.", false, false, fmt, fpv_);

    registerPropertyPrefix_("int.set_prefix.", false, false, ips_);
    registerPropertyPrefix_("double.set_prefix.", false, false, dps_);
    registerPropertyPrefix_("str.set_prefix.", false, false, sps_);
    registerPropertyPrefix_("enum.set_prefix.", false, false, valueMap, eps_);
    registerPropertyPrefix_("fmt.set_prefix.", false, false, fmt, fps_);

    registerPropertyPrefixInRange_("int.list_in_range_prefix.", false, false,
				   -10, 10, ripv_);
    registerPropertyPrefixInRange_("int.set_in_range_prefix.", false, false,
				   -10, 10, rips_);
    registerPropertyPrefixInRange_("double.list_in_range_prefix.", false,
				   false, 0.0, 1.0, rdpv_);
    registerPropertyPrefixInRange_("double.set_in_range_prefix.", false, false,
				   0.0, 1.0, rdps_);
    registerPropertyPrefixInRange_("str.list_in_range_prefix.", false, false,
				   std::string("a"), std::string("b"), rspv_);
    registerPropertyPrefixInRange_("str.set_in_range_prefix.", false, false,
				   std::string("a"), std::string("b"), rsps_);

  
    registerPropertyPrefixInSet_("int.list_in_set_prefix.", false, false,
				 { 10, 20, 30, 40, 50 }, iispv_);
    registerPropertyPrefixInSet_("int.set_in_set_prefix.", false, false,
				 { 10, 20, 30, 40, 50 }, iisps_);
    registerPropertyPrefixInSet_("double.list_in_set_prefix.", false, false,
				 { 0.25, 0.5, 1.0, 2.0, 4.0 }, dispv_);
    registerPropertyPrefixInSet_("double.set_in_set_prefix.", false, false,
				 { 0.25, 0.5, 1.0, 2.0, 4.0 }, disps_);
    registerPropertyPrefixInSet_("str.list_in_set_prefix.", false, false,
				 { "alpha", "beta", "gamma" }, sispv_);
    registerPropertyPrefixInSet_("str.set_in_set_prefix.", false, false,
				 { "alpha", "beta", "gamma" }, sisps_);
  }

  class RegisterTestPropertyConfig : public ApplicationConfiguration {
  public:
    RegisterTestPropertyConfig(): ApplicationConfiguration(false) { }

    template <typename ValueT>
    void registerProperty(const std::string& name, ValueT& v) {
      registerProperty_(name, false, false, v);
    }

    template <typename ValueT>
    void registerPrefix(const std::string& name, ValueT& v) {
      registerPropertyPrefix_(name, false, false, v);
    }
  };

  template <typename SeqIterT, typename TruthIterT>
  static ::testing::AssertionResult checkSequence(
      const SeqIterT& begin, const SeqIterT& end, const TruthIterT& truthBegin,
      const TruthIterT& truthEnd
  ) {
    auto i= begin;
    auto j= truthBegin;

    while ((i != end) || (j != truthEnd)) {
      if ((i == end) || (j == truthEnd) || (*i != *j)) {
	std::ostringstream msg;
	return ::testing::AssertionFailure()
	    << "Value is [" << join(begin, end, ",") << "]; it should be ["
	    << join(truthBegin, truthEnd, ",") << "]";
      }
      ++i; ++j;
    }
    return ::testing::AssertionSuccess();
  }

  template <typename ValueT>
  ::testing::AssertionResult checkList(const std::vector<ValueT>& seq,
				       const std::vector<ValueT>& truth) {
    return checkSequence(seq.begin(), seq.end(), truth.begin(), truth.end());
  }

  template <typename ValueT>
  ::testing::AssertionResult checkSet(const std::set<ValueT>& seq,
				      const std::set<ValueT>& truth) {
    return checkSequence(seq.begin(), seq.end(), truth.begin(), truth.end());
  }
			  
  const std::string& resourceDir() {
    static std::string RESOURCE_DIR;
    if (RESOURCE_DIR.empty()) {
      const char* d= getenv("UT_RESOURCE_DIR");
      if (d) {
	RESOURCE_DIR= d;
      }
      if (RESOURCE_DIR.empty()) {
	RESOURCE_DIR= "./";
      } else if (RESOURCE_DIR[RESOURCE_DIR.size()-1] != '/') {
	RESOURCE_DIR += "/";
      }
    }
    return RESOURCE_DIR;
  }
}

TEST(ApplicationConfigurationTests, LoadConfiguration) {
  TestAppConfig appConfig(false);

  appConfig.load(resourceDir() + "test_app_config.cfg");

  EXPECT_EQ(appConfig.intValue(), 15);
  EXPECT_EQ(appConfig.doubleValue(), 0.25);
  EXPECT_EQ(appConfig.strValue(), "abc");
  EXPECT_EQ(appConfig.enumValue(), Colors::WHITE);
  EXPECT_EQ(appConfig.fmtValue(), "abba");
  
  EXPECT_EQ(appConfig.intInRange(), 5);
  EXPECT_EQ(appConfig.doubleInRange(), 0.5);
  EXPECT_EQ(appConfig.strInRange(), "aabbcc");
  EXPECT_EQ(appConfig.intInSet(), 30);
  EXPECT_EQ(appConfig.doubleInSet(), 2.0);
  EXPECT_EQ(appConfig.strInSet(), "beta");

  EXPECT_TRUE(checkList(appConfig.intList(), { 1, 2, -1, 1, 10 }));
  EXPECT_TRUE(
      checkList(appConfig.doubleList(), { 0.25, 0.5, -1.0, 2.0, -4.0 })
  );
  EXPECT_TRUE(
      checkList(appConfig.strList(), { "z", "q", "r", "a", "a", "f", "ff" })
  );
  EXPECT_TRUE(
      checkList(appConfig.enumList(),
		{ Colors::RED, Colors::GREEN, Colors::WHITE, Colors::BLACK,
		  Colors::BLUE }
      )
  );
  EXPECT_TRUE(checkList(appConfig.fmtList(), { "a", "z", "b", "y"} ));

  EXPECT_TRUE(checkList(appConfig.intListInRange(), { 3, 2, 3, -4 }));
  EXPECT_TRUE(
      checkList(appConfig.doubleListInRange(), { 0.25, 0.35, 0.45, 0.55 })
  );
  EXPECT_TRUE(
      checkList(appConfig.strListInRange(), { "a", "ab", "abc", "ab", "a" })
  );

  EXPECT_TRUE(checkList(appConfig.intListInSet(), { 20, 30, 40, 10, 30, 50 }));
  EXPECT_TRUE(checkList(appConfig.doubleListInSet(), { 0.5, 0.25, 0.5 }));
  EXPECT_TRUE(checkList(appConfig.strListInSet(),
			{ "alpha", "gamma", "gamma", "beta", "alpha" }));

  EXPECT_TRUE(checkSet(appConfig.intSet(), { 1, 2, 3 }));
  EXPECT_TRUE(checkSet(appConfig.doubleSet(), { 0.1, 0.2 }));
  EXPECT_TRUE(checkSet(appConfig.strSet(), { "b", "g", "i", "o" }));
  EXPECT_TRUE(
      checkSet(appConfig.enumSet(),
	       { Colors::RED, Colors::BLUE, Colors::WHITE, Colors::BLACK })
  );
  EXPECT_TRUE(checkSet(appConfig.fmtSet(), { "blue", "green", "red" }));

  EXPECT_TRUE(checkSet(appConfig.intSetInRange(), { -4, 2, 3 }));
  EXPECT_TRUE(checkSet(appConfig.doubleSetInRange(), { 0.25, 0.50, 1.0 }));
  EXPECT_TRUE(
      checkSet(appConfig.strSetInRange(), { "a", "ab", "abba", "abc" })
  );

  EXPECT_TRUE(checkSet(appConfig.intSetInSet(), { 10, 20, 30, 40 }));
  EXPECT_TRUE(checkSet(appConfig.doubleSetInSet(), { 0.25, 1.0, 2.0 }));
  EXPECT_TRUE(checkSet(appConfig.strSetInSet(), { "alpha", "beta", "gamma" }));

  EXPECT_TRUE(checkList(appConfig.intPrefixList(), { -5, 10, 10, 2, 3 }));
  EXPECT_TRUE(checkList(appConfig.doublePrefixList(), { 0.0, 0.2, 0.8, 0.4 }));
  EXPECT_TRUE(
      checkList(appConfig.strPrefixList(),
		{ "red", "blue", "green", "black", "white" })
  );
  EXPECT_TRUE(
      checkList(appConfig.enumPrefixList(),
		{ Colors::RED, Colors::BLUE, Colors::GREEN, Colors::BLACK,
		  Colors::WHITE })
  );
  EXPECT_TRUE(checkList(appConfig.fmtPrefixList(), { "1", "2", "3", "5" }));

  EXPECT_TRUE(checkSet(appConfig.intPrefixSet(), { 2, 6, 7, 17 }));
  EXPECT_TRUE(checkSet(appConfig.doublePrefixSet(), { 0.0, 2.0, 17.0 }));
  EXPECT_TRUE(checkSet(appConfig.strPrefixSet(), { "blue", "green", "red" }));
  EXPECT_TRUE(
      checkSet(appConfig.enumPrefixSet(),
	       { Colors::RED, Colors::GREEN, Colors::BLUE })
  );
  EXPECT_TRUE(
      checkSet(appConfig.fmtPrefixSet(),
	       { "cretin", "fool", "plugh", "xyzzy" })
  );

  EXPECT_TRUE(checkList(appConfig.intPrefixListInRange(), { 10, -10, 0}));
  EXPECT_TRUE(checkList(appConfig.doublePrefixListInRange(), 
			{ 0.1, 0.3, 0.2, 0.9 }));
  EXPECT_TRUE(checkList(appConfig.strPrefixListInRange(),
			{ "ace", "abe", "apple", "axe" }));
  EXPECT_TRUE(checkList(appConfig.intPrefixListInSet(), { 30, 20, 30, 30 }));
  EXPECT_TRUE(checkList(appConfig.doublePrefixListInSet(),
			{ 1.0, 0.5, 0.5, 2.0 }));
  EXPECT_TRUE(checkList(appConfig.strPrefixListInSet(),
			{ "gamma", "alpha", "alpha", "alpha" }));

  EXPECT_TRUE(checkSet(appConfig.intPrefixSetInRange(), { -8, -5, -2, -1 }));
  EXPECT_TRUE(checkSet(appConfig.doublePrefixSetInRange(),
		       { 0.25, 0.50, 0.75}));
  EXPECT_TRUE(checkSet(appConfig.strPrefixSetInRange(),
		       { "abba", "alpha", "angel", "argument", "aspect"}));
  EXPECT_TRUE(checkSet(appConfig.intPrefixSetInSet(), { 20, 30 }));
  EXPECT_TRUE(checkSet(appConfig.doublePrefixSetInSet(), { 0.5, 1.0, 2.0 }));
  EXPECT_TRUE(checkSet(appConfig.strPrefixSetInSet(), { "alpha", "gamma"}));
}

TEST(ApplicationConfigurationTests, MissingProperties) {
  TestAppConfig config(false);
  
  config.load(resourceDir() + "missing_optional.cfg");
  EXPECT_EQ(config.intValue(), 101);

  EXPECT_THROW(config.load(resourceDir() + "missing_required.cfg"),
	       RequiredPropertyMissingError);
}

TEST(ApplicationConfigurationTests, UnknownProperties) {
  TestAppConfig unknownForbidden(false);
  TestAppConfig unknownIgnored(true);

  unknownIgnored.load(resourceDir() + "unknown_properties.cfg");
  EXPECT_EQ(unknownIgnored.strValue(), "abc");

  EXPECT_THROW(unknownForbidden.load(resourceDir() + "unknown_properties.cfg"),
	       UnknownPropertyError);
}

TEST(ApplicationConfigurationTests, EmptyValues) {
  TestAppConfig config(false);
  
  config.load(resourceDir() + "legal_empty_property.cfg");
  EXPECT_EQ(config.strValue(), "replicant");
  EXPECT_TRUE(config.intList().empty());

  EXPECT_THROW(config.load(resourceDir() + "illegal_empty_property.cfg"),
	       InvalidPropertyValueError);
}

TEST(ApplicationConfigurationTests, RegisterDuplicateProperty) {
  RegisterTestPropertyConfig config;
  int v;

  config.registerProperty("p", v);
  config.registerProperty("q", v);
  EXPECT_THROW(config.registerProperty("p", v), ApplicationConfigurationError);
}

TEST(ApplicationConfigurationTests, RegisterInvalidPropertyName) {
  RegisterTestPropertyConfig config;
  int v;

  EXPECT_THROW(config.registerProperty("int-value", v),
	       ApplicationConfigurationError);
  EXPECT_THROW(config.registerProperty("", v), ApplicationConfigurationError);
  EXPECT_THROW(config.registerProperty("int.value.", v),
	       ApplicationConfigurationError);
}

TEST(ApplicationConfigurationTests, RegisterInvalidPrefixName) {
  RegisterTestPropertyConfig config;
  int v;
  std::vector<int> iv;

  EXPECT_THROW(config.registerPrefix("int-value", iv),
	       ApplicationConfigurationError);
  EXPECT_THROW(config.registerPrefix("", iv), ApplicationConfigurationError);
  EXPECT_THROW(config.registerPrefix("int.value$.", iv),
	       ApplicationConfigurationError);

  config.registerPrefix("int.value.", iv);  // This is a legal prefix
  EXPECT_THROW(config.registerPrefix("int.value..", iv),
	       ApplicationConfigurationError); // This is not
  EXPECT_THROW(config.registerPrefix(".", iv), ApplicationConfigurationError);
}

TEST(ApplicationConfigurationTests, RegisterPropertyOverlappingPrefix) {
  RegisterTestPropertyConfig config;
  int v;
  std::vector<int> iv;

  config.registerPrefix("abc", iv);
  config.registerProperty("ab", v);
  config.registerPrefix("abd", iv);
  EXPECT_THROW(config.registerProperty("abdef", v),
	       ApplicationConfigurationError);
}

TEST(ApplicationConfigurationTests, RegisterPrefixOverlappingProperty) {
  RegisterTestPropertyConfig config;
  int v;
  std::vector<int> iv;

  config.registerProperty("abcdefg", v);
  config.registerProperty("defghi", v);
  config.registerProperty("abcefij", v);
  EXPECT_THROW(config.registerPrefix("abc", iv),
	       ApplicationConfigurationError);
}
