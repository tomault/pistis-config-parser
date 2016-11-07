/** @file ConfigurationPropertyMapTests.cpp
 *
 *  Unit tests for pistis::config_parser::ConfigurationPropertyMap.
 */

#include <pistis/exceptions/NoSuchItem.hpp>
#include <pistis/config_parser/ConfigurationPropertyMap.hpp>
#include <pistis/config_parser/InvalidPropertyValueError.hpp>
#include <pistis/config_parser/PropertyFormatError.hpp>
#include <gtest/gtest.h>
#include <algorithm>
#include <iterator>

using namespace pistis::exceptions;
using namespace pistis::config_parser;

TEST(ConfigurationPropertyMapTests, Construct) {
  ConfigurationPropertyMap map;
  
  EXPECT_TRUE(map.empty());
  EXPECT_EQ(map.size(), 0);
  EXPECT_TRUE(map.begin() == map.end());
  EXPECT_TRUE(map.beginNames() == map.endNames());
}

TEST(ConfigurationPropertyMapTests, Add) {
  static const ConfigurationProperty P1("p1", "apple", "someSource", 1);
  static const ConfigurationProperty P2("p2", "banana", "someSource", 2);
  static const ConfigurationProperty P3("p3", "cherry", "someSource", 3);
  static const ConfigurationProperty P2_NEW("p2", "blueberry", "anotherSource", 15);
  ConfigurationPropertyMap map;

  map.add(P1);
  map.add(P2);
  map.add(P3);

  EXPECT_FALSE(map.empty());
  EXPECT_EQ(map.size(), 3);
  EXPECT_EQ(map[P1.name()], P1);
  EXPECT_EQ(map[P2.name()], P2);
  EXPECT_EQ(map[P3.name()], P3);
  
  map.add(P2_NEW);
  EXPECT_EQ(map.size(), 3);
  EXPECT_EQ(map[P2.name()], P2_NEW);

  EXPECT_THROW(map["bad"], NoSuchItem);
}

TEST(ConfigurationPropertyMapTests, MovingAdd) {
  static const ConfigurationProperty P1("m1", "cat", "someSource", 10);
  static const ConfigurationProperty P2("m2", "dog", "someSource", 11);
  static const ConfigurationProperty P3("m3", "rhinocerous", "someSource", 12);
  static const ConfigurationProperty P3_NEW("m3", "raccoon", "anotherSource", 16);
  ConfigurationPropertyMap map;

  map.add(ConfigurationProperty(P1));
  map.add(ConfigurationProperty(P2));
  map.add(ConfigurationProperty(P3));
  
  EXPECT_FALSE(map.empty());
  EXPECT_EQ(map.size(), 3);
  EXPECT_EQ(map[P1.name()], P1);
  EXPECT_EQ(map[P2.name()], P2);
  EXPECT_EQ(map[P3.name()], P3);

  map.add(ConfigurationProperty(P3_NEW));
  EXPECT_EQ(map.size(), 3);
  EXPECT_EQ(map[P3.name()], P3_NEW);
}

TEST(ConfigurationPropertyMapTests, NameIteration) {
  static const ConfigurationProperty P1("p1", "apple", "someSource", 1);
  static const ConfigurationProperty P2("p2", "banana", "someSource", 2);
  static const ConfigurationProperty P3("p3", "cherry", "someSource", 3);
  ConfigurationPropertyMap map;
  ConfigurationPropertyMap::NameIterator i;
  std::vector<std::string> names;

  map.add(ConfigurationProperty(P2));
  map.add(ConfigurationProperty(P3));
  map.add(ConfigurationProperty(P1));

  std::copy(map.beginNames(), map.endNames(), std::back_inserter(names));
  ASSERT_EQ(names.size(), map.size());
  EXPECT_EQ(names[0], P1.name());
  EXPECT_EQ(names[1], P2.name());
  EXPECT_EQ(names[2], P3.name());

  // Test postiteration
  names.clear();
  i= map.beginNames();
  while (i != map.endNames()) {
    names.push_back(*i++);
  }
  ASSERT_EQ(names.size(), map.size());
  EXPECT_EQ(names[0], P1.name());
  EXPECT_EQ(names[1], P2.name());
  EXPECT_EQ(names[2], P3.name());  
}

TEST(ConfigurationPropertyMapTests, ValueIteration) {
  static const ConfigurationProperty P1("p1", "apple", "someSource", 1);
  static const ConfigurationProperty P2("p2", "banana", "someSource", 2);
  static const ConfigurationProperty P3("p3", "cherry", "someSource", 3);
  ConfigurationPropertyMap map;
  ConfigurationPropertyMap::PropertyIterator i;
  std::vector<ConfigurationProperty> values;

  map.add(ConfigurationProperty(P2));
  map.add(ConfigurationProperty(P3));
  map.add(ConfigurationProperty(P1));

  std::copy(map.begin(), map.end(), std::back_inserter(values));
  ASSERT_EQ(values.size(), map.size());
  EXPECT_EQ(values[0], P1);
  EXPECT_EQ(values[1], P2);
  EXPECT_EQ(values[2], P3);

  // Test postiteration
  values.clear();
  i= map.begin();
  while (i != map.end()) {
    values.push_back(*i++);
  }
  ASSERT_EQ(values.size(), map.size());
  EXPECT_EQ(values[0], P1);
  EXPECT_EQ(values[1], P2);
  EXPECT_EQ(values[2], P3);
}

TEST(ConfigurationPropertyMapTests, MoveConstruction) {
  static const ConfigurationProperty P1("p1", "apple", "someSource", 1);
  static const ConfigurationProperty P2("p2", "banana", "someSource", 2);
  static const ConfigurationProperty P3("p3", "cherry", "someSource", 3);
  ConfigurationPropertyMap original;

  original.add(P2);
  original.add(P1);
  original.add(P3);

  ConfigurationPropertyMap moved(std::move(original));

  EXPECT_FALSE(moved.empty());
  EXPECT_EQ(moved.size(), 3);
  EXPECT_EQ(moved[P1.name()], P1);
  EXPECT_EQ(moved[P2.name()], P2);
  EXPECT_EQ(moved[P3.name()], P3);
}

TEST(ConfigurationPropertyMapTests, MoveAssignment) {
  static const ConfigurationProperty P1("p1", "apple", "someSource", 1);
  static const ConfigurationProperty P2("p2", "banana", "someSource", 2);
  static const ConfigurationProperty P3("p3", "cherry", "someSource", 3);
  ConfigurationPropertyMap original;
  ConfigurationPropertyMap moved;

  original.add(P2);
  original.add(P1);
  original.add(P3);

  moved= std::move(original);

  EXPECT_FALSE(moved.empty());
  EXPECT_EQ(moved.size(), 3);
  EXPECT_EQ(moved[P1.name()], P1);
  EXPECT_EQ(moved[P2.name()], P2);
  EXPECT_EQ(moved[P3.name()], P3);
}

TEST(ConfigurationPropertyMapTests, HasKey) {
  static const ConfigurationProperty P1("p1", "apple", "someSource", 1);
  static const ConfigurationProperty P2("p2", "banana", "someSource", 2);
  static const ConfigurationProperty P3("p3", "cherry", "someSource", 3);
  ConfigurationPropertyMap map;

  map.add(P2);
  map.add(P1);
  map.add(P3);

  EXPECT_TRUE(map.hasKey(P1.name()));
  EXPECT_TRUE(map.hasKey(P2.name()));
  EXPECT_TRUE(map.hasKey(P3.name()));
  EXPECT_FALSE(map.hasKey("noSuchKey"));
}

TEST(ConfigurationPropertyMapTests, GetValue) {
  static const ConfigurationProperty P1("p1", "apple", "someSource", 1);
  static const ConfigurationProperty P2("p2", "banana", "someSource", 2);
  static const ConfigurationProperty P3("p3", "cherry", "someSource", 3);
  ConfigurationPropertyMap map;

  map.add(P2);
  map.add(P1);
  map.add(P3);

  EXPECT_EQ(map.getValue(P2.name(), "default"), P2.value());
  EXPECT_EQ(map.getValue("noneSuch", "default"), "default");
}

TEST(ConfigurationPropertyMapTests, GetValueAsInt) {
  static const ConfigurationProperty P1("p1", "  55  ", "someSource", 1);
  static const ConfigurationProperty P2("p2", "  55  bad", "someSource", 2);
  static const ConfigurationProperty P3("p3", "cherry", "someSource", 3);
  ConfigurationPropertyMap map;

  map.add(P2);
  map.add(P1);
  map.add(P3);

  EXPECT_EQ(map.getValueAsInt(P1.name(), -1), 55);
  EXPECT_EQ(map.getValueAsInt("noneSuch", -1), -1);
  EXPECT_THROW(map.getValueAsInt(P2.name(), -1), InvalidPropertyValueError);
}

TEST(ConfigurationPropertyMapTests, GetValueAsDouble) {
  static const ConfigurationProperty P1("p1", "  0.5  ", "someSource", 1);
  static const ConfigurationProperty P2("p2", "  0.5  bad", "someSource", 2);
  static const ConfigurationProperty P3("p3", "cherry", "someSource", 3);
  ConfigurationPropertyMap map;

  map.add(P2);
  map.add(P1);
  map.add(P3);

  EXPECT_EQ(map.getValueAsDouble(P1.name(), -1.0), 0.5);
  EXPECT_EQ(map.getValueAsDouble("noneSuch", -1.0), -1.0);
  EXPECT_THROW(map.getValueAsDouble(P2.name(), -1.0),
	       InvalidPropertyValueError);
}

TEST(ConfigurationPropertyMapTests, GetFormattedValue) {
  static const ConfigurationProperty P1("p1", "##good", "someSource", 1);
  static const ConfigurationProperty P2("p2", "badvalue", "someSource", 2);
  static const ConfigurationProperty P3("p3", "cherry", "someSource", 3);
  ConfigurationPropertyMap map;
  auto formatter = [](const std::string& s) {
    if ((s.size() < 3) || (s[0] != '#') || (s[1] != '#')) {
      throw PropertyFormatError("\"##\" prefix missing");
    }
    return s.substr(2);
  };

  map.add(P2);
  map.add(P1);
  map.add(P3);

  EXPECT_EQ(map.getValue(P1.name(), "default", formatter), "good");
  EXPECT_EQ(map.getValue("noneSuch", "default", formatter), "default");
  EXPECT_THROW(map.getValue(P2.name(), "default", formatter),
		    InvalidPropertyValueError);
}

TEST(ConfigurationPropertyMapTests, GetPropertiesMatching) {
  static const ConfigurationProperty P1("p1", "apple", "someSource", 1);
  static const ConfigurationProperty P2("p1.1", "banana", "someSource", 2);
  static const ConfigurationProperty P3("p2", "cherry", "someSource", 3);
  static const ConfigurationProperty P4("p3", "lemon", "someSource", 4);
  static const ConfigurationProperty P5("p4.4", "lime", "someSource", 5);
  ConfigurationPropertyMap map;

  map.add(P1);
  map.add(P2);
  map.add(P3);
  map.add(P4);
  map.add(P5);

  std::vector<ConfigurationProperty> matches = map.getPropertiesMatching(
      [](const std::string& s) { return s.find('.') != std::string::npos; }
  );
  ASSERT_EQ(matches.size(), 2);
  EXPECT_EQ(matches[0], P2);
  EXPECT_EQ(matches[1], P5);
}

TEST(ConfigurationPropertyMapTests, GetPropertiesWithPrefix) {
  static const ConfigurationProperty P1("p1", "apple", "someSource", 1);
  static const ConfigurationProperty P2("p2", "banana", "someSource", 2);
  static const ConfigurationProperty P3("p3", "cherry", "someSource", 3);
  static const ConfigurationProperty P4("p1.1", "lemon", "someSource", 4);
  static const ConfigurationProperty P5("p2.1", "lime", "someSource", 5);
  static const ConfigurationProperty P6("p4", "orange", "someSource", 1);
  static const ConfigurationProperty P7("p1.3", "pineapple", "someSource", 2);
  static const ConfigurationProperty P8("p1.2", "strawberry", "someSource", 3);
  static const ConfigurationProperty P9("p5", "kiwi", "someSource", 4);
  static const ConfigurationProperty P10("p5:1", "mango", "someSource", 5);
  ConfigurationPropertyMap map;

  map.add(P1);
  map.add(P2);
  map.add(P3);
  map.add(P4);
  map.add(P5);
  map.add(P6);
  map.add(P7);
  map.add(P8);
  map.add(P9);
  map.add(P10);
  
  std::vector<ConfigurationProperty> matches= 
    map.getPropertiesWithPrefix("p1.");
  ASSERT_EQ(matches.size(), 3);
  EXPECT_EQ(matches[0], P4);  // p1.1
  EXPECT_EQ(matches[1], P8);  // p1.2
  EXPECT_EQ(matches[2], P7);  // p1.3
  
  // Iteration goes off the end of map._properties
  matches= map.getPropertiesWithPrefix("p5:1");
  ASSERT_EQ(matches.size(), 1);
  EXPECT_EQ(matches[0], P10);

  // No matches
  matches= map.getPropertiesWithPrefix("p4:");
  EXPECT_TRUE(matches.empty());
}

TEST(ConfigurationPropertyMapTests, Erase) {
  static const ConfigurationProperty P1("p1", "##good", "someSource", 1);
  static const ConfigurationProperty P2("p2", "badvalue", "someSource", 2);
  static const ConfigurationProperty P3("p3", "cherry", "someSource", 3);
  ConfigurationPropertyMap map;

  map.add(P1);
  map.add(P2);
  map.add(P3);

  EXPECT_EQ(map.size(), 3);
  EXPECT_TRUE(map.hasKey(P2.name()));
  EXPECT_EQ(map[P2.name()], P2);

  map.erase(P2.name());
  EXPECT_EQ(map.size(), 2);
  EXPECT_FALSE(map.hasKey(P2.name()));
  EXPECT_THROW(map[P2.name()], NoSuchItem);
  EXPECT_EQ(map[P1.name()], P1);
  EXPECT_EQ(map[P3.name()], P3);
}

TEST(ConfigurationPropertyMapTests, Clear) {
  static const ConfigurationProperty P1("p1", "##good", "someSource", 1);
  static const ConfigurationProperty P2("p2", "badvalue", "someSource", 2);
  static const ConfigurationProperty P3("p3", "cherry", "someSource", 3);
  ConfigurationPropertyMap map;

  map.add(P1);
  map.add(P2);
  map.add(P3);

  EXPECT_FALSE(map.empty());
  EXPECT_EQ(map.size(), 3);
  EXPECT_EQ(map[P1.name()], P1);
  EXPECT_EQ(map[P2.name()], P2);
  EXPECT_EQ(map[P3.name()], P3);

  map.clear();
  EXPECT_TRUE(map.empty());
  EXPECT_EQ(map.size(), 0);
  EXPECT_THROW(map[P1.name()], NoSuchItem);
  EXPECT_THROW(map[P2.name()], NoSuchItem);
  EXPECT_THROW(map[P3.name()], NoSuchItem);
}
