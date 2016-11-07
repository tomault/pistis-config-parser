/** @file ConfigurationPropertyTests.cpp
 *
 *  Unit tests for pistis::config_parser::ConfigurationProperty
 */

#include <pistis/exceptions/NoSuchItem.hpp>
#include <pistis/util/StringUtil.hpp>
#include <pistis/config_parser/ConfigurationProperty.hpp>
#include <gtest/gtest.h>

using namespace pistis::config_parser;
using namespace pistis::exceptions;
using namespace pistis::util;

namespace {
  template <typename SeqIterT, typename TruthIterT>
  ::testing::AssertionResult checkSequences(
      const SeqIterT& seqBegin, const SeqIterT& seqEnd,
      const TruthIterT& truthBegin, const TruthIterT& truthEnd
  ) {
    auto i= seqBegin;
    auto j= truthBegin;

    while ((i != seqEnd) || (j != truthEnd)) {
      if ((i == seqEnd) || (j == truthEnd) || (*i != *j)) {
	return ::testing::AssertionFailure()
	    << "Result is [" << join(seqBegin, seqEnd, ",")
	    << "]; it should be [" << join(truthBegin, truthEnd, ",") << "]";
      }
      ++i;
      ++j;
    }
    
    return ::testing::AssertionSuccess();
  }

  template <typename SeqT, typename TruthT>
  ::testing::AssertionResult checkSequences(const SeqT& seq,
					    const TruthT& truth) {
    return checkSequences(seq.begin(), seq.end(), truth.begin(), truth.end());
  }
}

TEST(ConfigurationPropertyTests, Construct) {
  static const std::string NAME("test");
  static const std::string VALUE("someValue");
  static const std::string SOURCE("someSource");
  static const int LINE= 100;
  ConfigurationProperty p(NAME, VALUE, SOURCE, LINE);

  EXPECT_EQ(p.name(), NAME);
  EXPECT_EQ(p.value(), VALUE);
  EXPECT_EQ(p.source(), SOURCE);
  EXPECT_EQ(p.line(), LINE);
}

TEST(ConfigurationPropertyTests, MoveConstruction) {
  static const std::string NAME("test");
  static const std::string VALUE("someValue");
  static const std::string SOURCE("someSource");
  static const int LINE= 100;
  ConfigurationProperty original(NAME, VALUE, SOURCE, LINE);
  ConfigurationProperty moved(std::move(original));

  EXPECT_EQ(moved.name(), NAME);
  EXPECT_EQ(moved.value(), VALUE);
  EXPECT_EQ(moved.source(), SOURCE);
  EXPECT_EQ(moved.line(), LINE);
}

TEST(ConfigurationPropertyTests, MoveAssignment) {
  static const std::string NAME("test");
  static const std::string VALUE("someValue");
  static const std::string SOURCE("someSource");
  static const int LINE= 100;
  ConfigurationProperty original(NAME, VALUE, SOURCE, LINE);
  ConfigurationProperty moved("otherName", "otherValue", "otherSource", 1);

  moved= std::move(original);

  EXPECT_EQ(moved.name(), NAME);
  EXPECT_EQ(moved.value(), VALUE);
  EXPECT_EQ(moved.source(), SOURCE);
  EXPECT_EQ(moved.line(), LINE);
}

TEST(ConfigurationPropertyTests, ValueInSet) {
  static const std::string NAME("test");
  static const std::string VALUE("beta");
  static const std::string SOURCE("someSource");
  static const int LINE= 100;
  ConfigurationProperty p(NAME, VALUE, SOURCE, LINE);

  EXPECT_EQ(p.valueInSet({"alpha", "beta", "gamma", "delta", "eta"}), "beta");
  EXPECT_THROW(p.valueInSet({"a", "b", "g", "d", "e"}),
	       InvalidPropertyValueError);
}

TEST(ConfigurationPropertyTests, ValueAs) {
  static const std::string NAME("test");
  static const std::string VALUE("97");
  static const std::string SOURCE("someSource");
  static const int LINE= 100;
  static const int VALUE_AS_INT= 97;
  ConfigurationProperty p(NAME, VALUE, SOURCE, LINE);
  auto value = p.valueAs([](const std::string& v) {
      return strtol(v.c_str(), NULL, 10);
  });


  EXPECT_EQ(value, VALUE_AS_INT);
  EXPECT_THROW(
      p.valueAs([](const std::string& v) -> int {
	           throw PropertyFormatError("Formatting error");
	       }),
      InvalidPropertyValueError
  );

  value = p.valueAs(
      [](const std::string& v) { return strtol(v.c_str(), NULL, 10); },
      { 10, 97, 15, 99, -54 }
  );

  EXPECT_EQ(value, VALUE_AS_INT);

  EXPECT_THROW(
      p.valueAs(
	  [](const std::string& v) { return strtol(v.c_str(), NULL, 10); },
	  { 1, 5, 3, -3, -97 }
      ),
      InvalidPropertyValueError
  );
}

TEST(ConfigurationPropertyTests, ValueAsList) {
  static const std::string NAME("test");
  static const std::string SOURCE("someSource");
  static const int LINE= 100;
  static const std::vector<std::string> NO_VALUES;
  static const std::vector<std::string> ONE_VALUE{"pasta"};
  static const std::vector<std::string> MANY_VALUES{
      "xyz", "qr", "apple", "banana"
  };
  ConfigurationProperty noValues(NAME, "", SOURCE, LINE);
  ConfigurationProperty oneValue(NAME, "pasta", SOURCE, LINE);
  ConfigurationProperty manyValues(NAME, "xyz, qr, apple, banana",
				   SOURCE, LINE);
  ConfigurationProperty emptyElement(NAME, "xyz, qr, , banana", SOURCE, LINE);

  EXPECT_TRUE(checkSequences(noValues.valueAsList(","), NO_VALUES));
  EXPECT_TRUE(checkSequences(oneValue.valueAsList(","), ONE_VALUE));
  EXPECT_TRUE(checkSequences(manyValues.valueAsList(","), MANY_VALUES));

  // Empty element in a list produces an InvalidPropertyValueError
  EXPECT_THROW(emptyElement.valueAsList(","), InvalidPropertyValueError);

  EXPECT_TRUE(
      checkSequences(
          manyValues.valueAsRestrictedList(
	      ",", { "abc", "qr", "xyz", "apple", "banana",
		     "orange", "fruit" }
	  ),
	  MANY_VALUES
      )
  );
  EXPECT_THROW(
      manyValues.valueAsRestrictedList(
	  ",", { "abc", "xyz", "apple", "banana", "orange", "fruit" }
      ),
      InvalidPropertyValueError
  );
}

TEST(ConfigurationPropertyTests, ValueAsSet) {
  static const std::string NAME("test");
  static const std::string SOURCE("someSource");
  static const int LINE= 100;
  static const std::vector<std::string> NO_VALUES;
  static const std::vector<std::string> ONE_VALUE{"pasta"};
  static const std::vector<std::string> MANY_VALUES{"a", "b", "c"};
  ConfigurationProperty noValues(NAME, "", SOURCE, LINE);
  ConfigurationProperty oneValue(NAME, "pasta", SOURCE, LINE);
  ConfigurationProperty manyValues(NAME, "a, c, a, b, b, a", SOURCE, LINE);
  ConfigurationProperty emptyElement(NAME, "xyz, qr, , banana", SOURCE, LINE);

  EXPECT_TRUE(checkSequences(noValues.valueAsList(","), NO_VALUES));
  EXPECT_TRUE(checkSequences(oneValue.valueAsSet(","), ONE_VALUE));
  EXPECT_TRUE(checkSequences(manyValues.valueAsSet(","), MANY_VALUES));

  // Empty element in a list produces an InvalidPropertyValueError
  EXPECT_THROW(emptyElement.valueAsSet(","), InvalidPropertyValueError);

  EXPECT_TRUE(
      checkSequences(
	  manyValues.valueAsRestrictedSet(
	      ",", { "a", "b", "c", "d", "e", "f", "g" }
	  ),
	  MANY_VALUES
      )
  );
  EXPECT_THROW(manyValues.valueAsRestrictedSet(",", { "a", "b", "d", "e" }),
	       InvalidPropertyValueError);
}

TEST(ConfigurationPropertyTests, ValueAsInt) {
  ConfigurationProperty p("test", "52", "someSource", 1);
  ConfigurationProperty bad("test", "not an int", "someSource", 2);
  EXPECT_EQ(p.valueAsInt(), 52);
  EXPECT_THROW(bad.valueAsInt(), InvalidPropertyValueError);

  EXPECT_EQ(p.valueAsIntInRange(0, 100), 52);
  EXPECT_THROW(p.valueAsIntInRange(-50, 50), InvalidPropertyValueError);
}

TEST(ConfigurationPropertyTests, ValueAsListOfInt) {
  static const std::vector<int> TRUTH{4, 7, 3, 4, 19, 3};
  ConfigurationProperty p("test", "4,7,3,4,19,3", "someSource", 1);
  ConfigurationProperty bad("test", "4,7,3,bad,19,3", "someSource", 2);

  EXPECT_TRUE(checkSequences(p.valueAsListOfInt(","), TRUTH));
  EXPECT_THROW(bad.valueAsListOfInt(","), InvalidPropertyValueError);
}

TEST(ConfigurationPropertyTests, ValueAsSetOfInt) {
  static const std::vector<int> TRUTH{3, 4, 7, 19};
  ConfigurationProperty p("test", "4,7,3,4,19,3", "someSource", 1);
  ConfigurationProperty bad("test", "4,7,3,bad,19,3", "someSource", 2);

  EXPECT_TRUE(checkSequences(p.valueAsSetOfInt(","), TRUTH));
  EXPECT_THROW(bad.valueAsSetOfInt(","), InvalidPropertyValueError);
}

TEST(ConfigurationPropertyTests, ValueAsDouble) {
  ConfigurationProperty p("test", "0.5", "someSource", 1);
  ConfigurationProperty bad("test", "not a double", "someSource", 2);

  EXPECT_NEAR(p.valueAsDouble(), 0.5, 1e-10);
  EXPECT_THROW(bad.valueAsDouble(), InvalidPropertyValueError);

  EXPECT_NEAR(p.valueAsDoubleInRange(0.0, 1.0), 0.5, 1e-10);
  EXPECT_THROW(p.valueAsDoubleInRange(-0.25, 0.25),
		    InvalidPropertyValueError);
}

TEST(ConfigurationPropertyTests, ValueAsListOfDouble) {
  static const std::vector<double> TRUTH{ 0.5, -1.0, 0.5, 12.0 };
  ConfigurationProperty p("test", "0.5, -1.0, 0.5, 12.0", "someSource", 1);
  ConfigurationProperty bad("test", "0.5, -1.0, 0.5, bad", "someSource", 2);

  EXPECT_TRUE(checkSequences(p.valueAsListOfDouble(","), TRUTH));
  EXPECT_THROW(bad.valueAsListOfDouble(","), InvalidPropertyValueError);
}

TEST(ConfigurationPropertyTests, ValueAsSetOfDouble) {
  static const std::vector<double> TRUTH{ -1.0, 0.5, 12.0 };
  ConfigurationProperty p("test", "0.5, -1.0, 0.5, 12.0", "someSource", 1);
  ConfigurationProperty bad("test", "0.5, -1.0, 0.5, bad", "someSource", 2);

  EXPECT_TRUE(checkSequences(p.valueAsSetOfDouble(","), TRUTH));
  EXPECT_THROW(bad.valueAsSetOfDouble(","), InvalidPropertyValueError);
}
