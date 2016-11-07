/** @file ConfigFileLexerTests.cpp
 *
 *  Unit tests for pistis::config_parser::ConfigFileLexer
 */

#include <pistis/config_parser/detail/ConfigFileLexer.hpp>
#include <gtest/gtest.h>
#include <algorithm>
#include <sstream>

using namespace pistis::config_parser;
using namespace pistis::config_parser::detail;

namespace {
  const std::string INPUT_TEXT = "\
# Test skipWhitespaceInLine and skipWhitespace with a bunch of blank lines\n\
# The first line is completely blank, the next has some spaces & tabs in it\n\
\n\
   \t  \t   \n\
  # An indented comment followed by a name = value line\n\
  some1.name2.v3=property value text\n\
# Single character name followed by empty value\n\
a =\n\
# Multi-line value\n\
multi_line = This is line one\\\n\
  This is line two \\\n\
\\\n\
This is the last line.    \n\
# Multi-line value ending in an empty line\n\
multi_line_2 = This is line #1\\\n\
This is line #2\\\n\
\n\
# Property name with double '.'  Should tokenize as '.' 'n1.' '.' 'n2'   \n\
.n1..n2\n\
#Quoted string ending in a quote and one ending at the end-of-line\n\
\"This is a quoted string\"\t\"\tThis quoted string ends at the end-of-line \n\
";

  const int START_LINE= 10;
  const int START_COLUMN= 5;

  std::vector<Token> getTruth() {
    static const std::vector<Token> TRUTH= {
      Token(TokenType::COMMENT,
	    "Test skipWhitespaceInLine and skipWhitespace with a bunch of "
	    "blank lines",
	    START_LINE, START_COLUMN+2),
      Token(TokenType::COMMENT,
	    "The first line is completely blank, the next has some spaces "
	    "& tabs in it", START_LINE+1, 3),
      Token(TokenType::COMMENT,
	    "An indented comment followed by a name = value line",
	    START_LINE+4, 5),
      Token(TokenType::NAME, "some1.name2.v3", START_LINE+5, 3),
      Token(TokenType::PUNCTUATION, "=", START_LINE+5, 17),
      Token(TokenType::VALUE, "property value text", START_LINE+5, 18),
      Token(TokenType::COMMENT,
	    "Single character name followed by empty value", START_LINE+6, 3),
      Token(TokenType::NAME, "a", START_LINE+7, 1),
      Token(TokenType::PUNCTUATION, "=", START_LINE+7, 3),
      Token(TokenType::VALUE, "", START_LINE+7, 4),
      Token(TokenType::COMMENT, "Multi-line value", START_LINE+8, 3),
      Token(TokenType::NAME, "multi_line", START_LINE+9, 1),
      Token(TokenType::PUNCTUATION, "=", START_LINE+9, 12),
      Token(TokenType::VALUE,
	    "This is line one\n  This is line two \n\nThis is the last line.",
	    START_LINE+9, 14),
      Token(TokenType::COMMENT, "Multi-line value ending in an empty line",
	    START_LINE+13, 3),
      Token(TokenType::NAME, "multi_line_2", START_LINE+14, 1),
      Token(TokenType::PUNCTUATION, "=", START_LINE+14, 14),
      Token(TokenType::VALUE, "This is line #1\nThis is line #2",
	    START_LINE+14, 16),
      Token(TokenType::COMMENT,
	    "Property name with double '.'  Should tokenize as '.' 'n1.' "
	    "'.' 'n2'", START_LINE+17, 3),
      Token(TokenType::PUNCTUATION, ".", START_LINE+18, 1),
      Token(TokenType::NAME, "n1.", START_LINE+18, 2),
      Token(TokenType::PUNCTUATION, ".", START_LINE+18, 5),
      Token(TokenType::NAME, "n2", START_LINE+18, 6),
      Token(TokenType::COMMENT,
	    "Quoted string ending in a quote and one ending at the "
	    "end-of-line", START_LINE+19, 2),
      Token(TokenType::PUNCTUATION, "\"", START_LINE+20, 1),
      Token(TokenType::VALUE, "This is a quoted string", START_LINE+20, 2),
      Token(TokenType::PUNCTUATION, "\"", START_LINE+20, 25),
      Token(TokenType::PUNCTUATION, "\"", START_LINE+20, 27),
      Token(TokenType::VALUE,
	    "\tThis quoted string ends at the end-of-line ", START_LINE+20, 28),
      Token(TokenType::END_OF_FILE, "", START_LINE+20, 72)
    };
    return TRUTH;
  };

}

TEST(ConfigFileLexerTests, Tokenize) {
  const std::vector<Token> TRUTH = getTruth();
  std::istringstream input(INPUT_TEXT);
  ConfigFileLexer lexer(input, START_LINE, START_COLUMN);
  std::vector<Token>::const_iterator i = TRUTH.begin();
  std::vector<Token> received;
  int quoteState = 0;
  Token t(TokenType::END_OF_FILE, "", 0, 0);

  do {
    t = lexer.next();
    received.push_back(t);

    if (quoteState) {
      ++quoteState;
    }
    if (t.type() != TokenType::PUNCTUATION) {
      // Don't need to do anything special
    } else if (t.value() == "=") {
      lexer.parseNextAsValue();
    } else if ((t.value() == "\"") && (!quoteState)) {
      lexer.parseNextAsQuotedString();
      quoteState = 1;
    }

    if (quoteState >= 3) {
      quoteState= 0;
    }

    if (i == TRUTH.end()) {
      std::ostringstream msg;
      auto writeToken = [&msg](const Token& t) { msg << "\n  " << t; };
      msg << "FAILED!  Extra token " << t
	  << " past end-of-file.\nReceived tokens are:";
      std::for_each(received.begin(), received.end(), writeToken);
      msg << "\nToken stream should be:";
      std::for_each(TRUTH.begin(), TRUTH.end(), writeToken);
      msg << "\n";
      FAIL() << msg.str();
      return;
    } else if (t != *i) {
      std::ostringstream msg;
      auto writeToken= [&msg](const Token& t) { msg << "\n  " << t; };
      msg << "FAILED!  Tokens at position " << received.size()-1
	  << " do not match.  Received tokens:";
      std::for_each(received.begin(), received.end(), writeToken);
      msg << "\nTokens should be:";
      std::for_each(TRUTH.begin(), TRUTH.begin() + received.size(),
		    writeToken);
      msg << "\n";
      FAIL() << msg.str();
      return;
    }

    ++i;
  } while (t.type() != TokenType::END_OF_FILE);

  if (i != TRUTH.end()) {
    std::ostringstream msg;
    auto writeToken= [&msg](const Token& t) { msg << "\n  " << t; };

    msg << "FAILED!  Premature end-of-file.  Tokens received:";
    std::for_each(received.begin(), received.end(), writeToken);
    msg << "Token stream should be:";
    std::for_each(TRUTH.begin(), TRUTH.end(), writeToken);
    msg << "\n";
    FAIL() << msg.str();
  }

  SUCCEED();
}
