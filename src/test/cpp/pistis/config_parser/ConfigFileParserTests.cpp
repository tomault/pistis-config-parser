/** @file ConfigFileParserTests.cpp
 *
 *  Unit tests for pistis::config_parser::ConfigFileParser
 */

#include <pistis/config_parser/ConfigFileParser.hpp>
#include <pistis/config_parser/ConfigFileParseError.hpp>
#include <gtest/gtest.h>
#include <stdlib.h>

using namespace pistis::config_parser;

namespace {

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

  ::testing::AssertionResult verifyConfigFileSyntaxError(
      ConfigFileParser& parser,
      const std::string& filename,
      const std::string& expectedDetails
  ) {
    try {
      parser.parse(filename);
      return ::testing::AssertionFailure()
	  << "Expected a ConfigFileParseError for " << filename
	  << ", but one was not thrown";
    } catch(const ConfigFileParseError& e) {
      if (e.details().find(expectedDetails) == std::string::npos) {

	return ::testing::AssertionFailure()
	    << "Error message for " << filename << " is [" << e.details()
	    << "]; it should contain [" << expectedDetails << "]";
      }
      return ::testing::AssertionSuccess();
    } catch(const std::exception& e) {
      return ::testing::AssertionFailure()
	  << "Unexpected exception caught while parsing " << filename
	  << " (" << e.what() << ")";
    } catch(...) {
      return ::testing::AssertionFailure()
	  << "Unexpected exception caught while parsing " << filename;
    }
  }
}

TEST(ConfigFileParserTests, ParseAssignment) {
  const std::string SOURCE= resourceDir() + "assignment_test.cfg";
  const ConfigurationProperty P1("names.fruits.f1", "apple pie", SOURCE, 4);
  const ConfigurationProperty P2("names.fruits.f2", "banana pie", SOURCE, 5);
  const ConfigurationProperty P3("names.fruits.f3", "orange juice", SOURCE, 6);
  const ConfigurationProperty P4("names.elements.first", "hydrogen & helium",
				 SOURCE, 9);
  const ConfigurationProperty P5("names.elements.second", "lithium & calcium",
				 SOURCE, 10);
  const ConfigurationProperty P6("names.elements.third", "sulfur & potassium",
				 SOURCE, 11);
  const ConfigurationProperty P7(
      "intro", "The banana pie is made of sulfur & potassium", SOURCE, 15
  );
  ConfigFileParser parser;
  ConfigurationPropertyMap properties= parser.parse(SOURCE);
  EXPECT_EQ(properties.size(), 7);
  EXPECT_EQ(properties[P1.name()], P1);
  EXPECT_EQ(properties[P2.name()], P2);
  EXPECT_EQ(properties[P3.name()], P3);
  EXPECT_EQ(properties[P4.name()], P4);
  EXPECT_EQ(properties[P5.name()], P5);
  EXPECT_EQ(properties[P6.name()], P6);
  EXPECT_EQ(properties[P7.name()], P7);
}

TEST(ConfigFileParserTests, ParseDuplicateAssignment) {
  const std::string SOURCE= resourceDir() + "duplicate_assignment.cfg";
  const ConfigurationProperty P1("p1", "apple", SOURCE, 3);
  const ConfigurationProperty P2_ORIG("p2", "banana", SOURCE, 4);
  const ConfigurationProperty P2_DUP("p2", "grapefruit", SOURCE, 6);
  const ConfigurationProperty P3("p3", "orange", SOURCE, 5);
  ConfigFileParser ignoreDups(true, ConfigFileParser::DUP_IGNORE);
  ConfigFileParser overwriteDups(true, ConfigFileParser::DUP_OVERWRITE);
  ConfigFileParser errorDups(true, ConfigFileParser::DUP_ERROR);
  ConfigurationPropertyMap properties= ignoreDups.parse(SOURCE);

  EXPECT_EQ(properties.size(), 3);
  EXPECT_EQ(properties[P1.name()], P1);
  EXPECT_EQ(properties[P2_ORIG.name()], P2_ORIG);
  EXPECT_EQ(properties[P3.name()], P3);

  properties= overwriteDups.parse(SOURCE);
  EXPECT_EQ(properties.size(), 3);
  EXPECT_EQ(properties[P1.name()], P1);
  EXPECT_EQ(properties[P2_DUP.name()], P2_DUP);
  EXPECT_EQ(properties[P3.name()], P3);
  
  EXPECT_THROW(errorDups.parse(SOURCE), ConfigFileParseError);
}

TEST(ConfigFileParserTests, ParseInclude) {
  const std::string SOURCE_1 = resourceDir() + "include_test.cfg";
  const std::string SOURCE_2 = resourceDir() + "overwrite_included.cfg";
  const std::string INCLUDED = resourceDir() + "included.cfg";
  const ConfigurationProperty P1("p1", "apple", SOURCE_1, 3);
  const ConfigurationProperty P1_OVERWRITE("p1", "apple", SOURCE_2, 3);
  const ConfigurationProperty P2_ORIG("p2", "banana", SOURCE_1, 4);
  const ConfigurationProperty P2_INCLUDED("p2", "grapefruit", INCLUDED, 2);
  const ConfigurationProperty P3_INCLUDED("p3", "orange", INCLUDED, 3);
  const ConfigurationProperty P3_OVERWRITE("p3", "strawberry", SOURCE_2, 5);
  const ConfigurationProperty P4("p4", "kiwi", SOURCE_1, 8);
  ConfigFileParser ignoreDups(true, ConfigFileParser::DUP_ERROR,
			      ConfigFileParser::DUP_IGNORE);
  ConfigFileParser overwriteDups(true, ConfigFileParser::DUP_ERROR,
				 ConfigFileParser::DUP_OVERWRITE);
  ConfigFileParser errorDups(true, ConfigFileParser::DUP_ERROR,
			     ConfigFileParser::DUP_ERROR);
  ConfigurationPropertyMap properties = ignoreDups.parse(SOURCE_1);

  EXPECT_EQ(properties.size(), 4);
  EXPECT_EQ(properties[P1.name()], P1);
  EXPECT_EQ(properties[P2_ORIG.name()], P2_ORIG);
  EXPECT_EQ(properties[P3_INCLUDED.name()], P3_INCLUDED);
  EXPECT_EQ(properties[P4.name()], P4);

  properties = overwriteDups.parse(SOURCE_1);
  EXPECT_EQ(properties.size(), 4);
  EXPECT_EQ(properties[P1.name()], P1);
  EXPECT_EQ(properties[P2_INCLUDED.name()], P2_INCLUDED);
  EXPECT_EQ(properties[P3_INCLUDED.name()], P3_INCLUDED);
  EXPECT_EQ(properties[P4.name()], P4);

  EXPECT_THROW(errorDups.parse(SOURCE_1), ConfigFileParseError);

  properties = ignoreDups.parse(SOURCE_2);
  EXPECT_EQ(properties.size(), 3);
  EXPECT_EQ(properties[P1_OVERWRITE.name()], P1_OVERWRITE);
  EXPECT_EQ(properties[P2_INCLUDED.name()], P2_INCLUDED);
  EXPECT_EQ(properties[P3_INCLUDED.name()], P3_INCLUDED);

  properties = overwriteDups.parse(SOURCE_2);
  EXPECT_EQ(properties.size(), 3);
  EXPECT_EQ(properties[P1_OVERWRITE.name()], P1_OVERWRITE);
  EXPECT_EQ(properties[P2_INCLUDED.name()], P2_INCLUDED);
  EXPECT_EQ(properties[P3_OVERWRITE.name()], P3_OVERWRITE);

  EXPECT_THROW(errorDups.parse(SOURCE_2), ConfigFileParseError);
}

TEST(ConfigFileParserTests, ParseSyntaxError) {
  static const std::map<std::string, std::string> BAD_FILES= {
    { "equals_missing.cfg", "'=' expected" },
    { "equals_typo.cfg", "'=' expected" },
    { "extra_close_brace.cfg", "'}' unexpected" },
    { "include_missing_closequote.cfg", "'\"' expected" },
    { "include_missing_openquote.cfg", "'\"' expected" },
    { "include_missing_filename.cfg", "File name missing" },
    { "include_nonexistent.cfg", "No such file" },
    { "include_within_block.cfg", "Cannot include a file from within a block" },
    { "invalid_name.cfg", "not a legal property name" },
    { "missing_name.cfg", "property name expected" },
    { "open_block.cfg", "'}' expected" },
    { "value_error.cfg", "Cannot resolve referenced property" },
    { "value_on_next_line.cfg", "'=' expected" }
  };
  ConfigFileParser parser;
  
  for (auto i = BAD_FILES.begin(); i != BAD_FILES.end(); ++i) {
    std::string filename = resourceDir() + i->first;
    EXPECT_TRUE(verifyConfigFileSyntaxError(parser, filename, i->second));
  }
}

TEST(ConfigFileParserTests, ParseRecusiveInclude) {
  ConfigFileParser parser;
  EXPECT_THROW(parser.parse("recursive.cfg"), ConfigFileParseError);
}

TEST(ConfigFileParserTests, ParseText) {
  const std::string TEXT=
      "names.fruits.f1=apple pie\nnames.fruits.f2=banana pie\n"
      "names.fruits.f3=orange juice\n";
  const std::string SOURCE= "#TEXT";
  const ConfigurationProperty P1("names.fruits.f1", "apple pie", SOURCE, 1);
  const ConfigurationProperty P2("names.fruits.f2", "banana pie", SOURCE, 2);
  const ConfigurationProperty P3("names.fruits.f3", "orange juice", SOURCE, 3);
  ConfigFileParser parser;
  ConfigurationPropertyMap properties= parser.parseText(SOURCE, TEXT);

  EXPECT_EQ(properties.size(), 3);
  EXPECT_EQ(properties[P1.name()], P1);
  EXPECT_EQ(properties[P2.name()], P2);
  EXPECT_EQ(properties[P3.name()], P3);
}
