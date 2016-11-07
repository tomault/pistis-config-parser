/** @file ValueProcessorTests.cpp
 *
 *  Unit tests for pistis::config_parser::detail::ValueProcessor
 */

#include <pistis/config_parser/PropertyFormatError.hpp>
#include <pistis/config_parser/detail/ValueProcessor.hpp>
#include <gtest/gtest.h>
#include <stdlib.h>

using namespace pistis::config_parser;
using namespace pistis::config_parser::detail;

TEST(ValueProcessorTests, DecodeEscapeSequence) {
  std::string INPUT = "Some escaped chars \\t:\\r\\n\\${abc} blah \\";
  std::string TRUTH = "Some escaped chars \t:\r\n${abc} blah \\";
  ConfigurationPropertyMap properties;
  ValueProcessor processor(properties, false);

  EXPECT_EQ(processor.processValue(INPUT), TRUTH);
}

TEST(ValueProcessorTests, DecodeUnicodeChar) {
  std::string INPUT =
    "Unicode chars \\u41 \\uFF \\u84CG \\u4567 \\u102347Z \\u7F973BA";
  std::string TRUTH =
    "Unicode chars A \xC3\xBF \xE0\xA1\x8CG \xE4\x95\xA7 \xF4\x82\x8D\x87Z "
    "\xF8\x9F\xB9\x9C\xBB\x41";
  std::string INCOMPLETE= "Incomplete unicode escape sequence \\u";
  std::string INVALID= "Invalid unicode escape sequence \\uZ";
  ConfigurationPropertyMap properties;
  ValueProcessor processor(properties, false);

  EXPECT_EQ(processor.processValue(INPUT), TRUTH);
  EXPECT_THROW(processor.processValue(INCOMPLETE), PropertyFormatError);
  EXPECT_THROW(processor.processValue(INVALID), PropertyFormatError);
}

TEST(ValueProcessorTests, DecodeEnclosedUnicodeChar) {
  std::string INPUT =
    "Unicode chars \\u{41} \\u{ff} \\u{84c}g \\u{4567} \\u{102347}a "
    "\\u{7f973b}a";
  std::string TRUTH=
    "Unicode chars A \xC3\xBF \xE0\xA1\x8Cg \xE4\x95\xA7 "
    "\xF4\x82\x8D\x87\x61 \xF8\x9F\xB9\x9C\xBB\x61";
  std::string INCOMPLETE= "Incomplete unicode escape sequence \\u{";
  std::string INCOMPLETE_2= "Incomplete unicode escape sequence \\u{a";
  std::string INCOMPLETE_3= "Incomplete unicode escape sequence \\u{123456";
  std::string EMPTY= "Empty unicode escape sequence \\u{}";
  std::string INVALID= "Invalid unicode escape sequence \\u{Z}";
  std::string INVALID_2= "Invalid unicode escape sequence \\u{abq}";
  std::string INVALID_3= "Invalid unicode escape sequence \\u{abcdef?}";
  std::string TOO_LONG= "Too long unicode escape sequence \\u{1234567}";
  ConfigurationPropertyMap properties;
  ValueProcessor processor(properties, false);

  EXPECT_EQ(processor.processValue(INPUT), TRUTH);
  EXPECT_THROW(processor.processValue(INCOMPLETE), PropertyFormatError);
  EXPECT_THROW(processor.processValue(INCOMPLETE_2), PropertyFormatError);
  EXPECT_THROW(processor.processValue(INCOMPLETE_3), PropertyFormatError);
  EXPECT_THROW(processor.processValue(INVALID), PropertyFormatError);
  EXPECT_THROW(processor.processValue(INVALID_2), PropertyFormatError);
  EXPECT_THROW(processor.processValue(INVALID_3), PropertyFormatError);
  EXPECT_THROW(processor.processValue(EMPTY), PropertyFormatError);
  EXPECT_THROW(processor.processValue(TOO_LONG), PropertyFormatError);
}

TEST(ValueProcessorTests, VariableSubstitutionTest) {
  const std::string INPUT = "Variable substitution: ${fruit.name} is a fruit";
  const std::string TRUTH = "Variable substitution: apple is a fruit";
  const std::string EMPTY = "Variable substitution: ${} is a fruit";
  const std::string UNKNOWN = "Variable substitution: ${p3} is a fruit";
  const std::string BAD_NAME =
    "Variable substitution: ${invalid..name} is a fruit";
  const std::string BAD_NAME_2 =
    "Variable substitution: ${.invalid.name} is a fruit";
  const std::string INCOMPLETE= "Variable substitution: ${";
  const std::string INCOMPLETE_2= "Variable substitution ${fruit";
  ConfigurationPropertyMap properties;
  ValueProcessor processor(properties, false);

  properties.add(ConfigurationProperty("fruit.name", "apple", "someSource", 1));
  properties.add(
      ConfigurationProperty("fruit.other", "banana", "someSource", 2)
  );
  EXPECT_EQ(processor.processValue(INPUT), TRUTH);
  EXPECT_THROW(processor.processValue(EMPTY), PropertyFormatError);
  EXPECT_THROW(processor.processValue(UNKNOWN), PropertyFormatError);
  EXPECT_THROW(processor.processValue(BAD_NAME), PropertyFormatError);
  EXPECT_THROW(processor.processValue(BAD_NAME_2), PropertyFormatError);
  EXPECT_THROW(processor.processValue(INCOMPLETE), PropertyFormatError);
  EXPECT_THROW(processor.processValue(INCOMPLETE_2),
	       PropertyFormatError);
}

TEST(ValueProcessorTests, VerifyEnvironmentSubstitution) {
  const std::string INPUT("Environment variable substitution: ${fruit.name}");
  const std::string TRUTH("Environment variable substitution: apple");
  const std::string TRUTH_2("Environment variable substitution: banana");
  ConfigurationPropertyMap properties;
  ValueProcessor usesEnv(properties, true);
  ValueProcessor doesNotUseEnv(properties, false);
  
  properties.add(ConfigurationProperty("p1", "apple", "someSource", 1));
  properties.add(ConfigurationProperty("p2", "cherry", "someSource", 2));
  setenv("fruit.name", "apple", 1);
  
  EXPECT_EQ(usesEnv.processValue(INPUT), TRUTH);
  EXPECT_THROW(doesNotUseEnv.processValue(INPUT), PropertyFormatError);

  properties.add(
      ConfigurationProperty("fruit.name", "banana", "someSource", 1)
  );
  EXPECT_EQ(usesEnv.processValue(INPUT), TRUTH_2);
}
