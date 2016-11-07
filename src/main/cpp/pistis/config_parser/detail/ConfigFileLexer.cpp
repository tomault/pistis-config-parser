#include "ConfigFileLexer.hpp"
#include <pistis/util/StringUtil.hpp>
#include <string>
#include <ctype.h>

using namespace pistis::util;
using namespace pistis::config_parser;
using namespace pistis::config_parser::detail;

ConfigFileLexer::ConfigFileLexer(std::istream& input, int initialLine,
				 int initialColumn):
    input_(input), line_(initialLine), column_(initialColumn), text_(),
    current_(), state_(AT_START) {
  if (!std::getline(input_, text_)) {
    state_ = AT_EOF;
  }
  current_ = text_.begin();
}

ConfigFileLexer::~ConfigFileLexer() {
}

Token ConfigFileLexer::next() {
  switch (state_) {
    case AT_START:
    case AT_TEXT:
      return parseText_();

    case AT_VALUE:
      return parseValue_(false);

    case AT_QT_STRING:
      return parseQuotedString_();

    case AT_EOF:
    default:
      return Token(TokenType::END_OF_FILE, "", line_, column_);
  }
}

Token ConfigFileLexer::parseText_() {
  if (!skipWhitespace_()) {
    return Token(TokenType::END_OF_FILE, "", line_, column_);
  }
  if (isNameChar(*current_, '.')) {
    auto start= current_;
    int col= column_;
    char prev= *current_;
    ++current_;
    ++column_;
    while (isNameChar(*current_, prev)) {
      prev= *current_;
      ++current_;
      ++column_;
    }
    state_= AT_TEXT;
    return Token(TokenType::NAME, std::string(start, current_), line_, col);
  } else if ((*current_ == '#') && (state_ == AT_START)) {
    ++current_;
    ++column_;
    skipWhitespaceInLine_();

    auto start= current_;
    int col= column_;
    current_= text_.end();
    column_= text_.size()+1;
    state_= AT_TEXT;
    return Token(TokenType::COMMENT, rstrip(std::string(start, current_)),
		 line_, col);
  } else {
    state_= AT_TEXT;
    ++current_;
    ++column_;
    return Token(TokenType::PUNCTUATION, std::string(1, current_[-1]),
		 line_, column_-1);
  }
}

Token ConfigFileLexer::parseValue_(bool singleLine) {
  skipWhitespaceInLine_();
  auto start= current_;
  int col= column_;
  current_= text_.end();
  column_= text_.size()+1;
  state_= AT_TEXT;
  if (singleLine || (current_ == start) || (text_.back() != '\\')) {
    auto end= current_;
    while ((end != start) && isspace(end[-1])) {
      --end;
    }
    return Token(TokenType::VALUE, std::string(start, end), line_, col);
  } else {
    std::ostringstream value;
    int line= line_;
    --current_;
    value << std::string(start, current_);
    while (readNextLine_()) {
      if (text_.empty() || (text_.back() != '\\')) {
	value << "\n" << text_;
	state_= AT_TEXT;
	current_= text_.end();
	column_= text_.size()+1;
	break;
      } else {
	text_.erase(text_.size()-1);
	value << "\n" << text_;
      }
    }
    return Token(TokenType::VALUE, rstrip(value.str()), line, col);
  }
}

Token ConfigFileLexer::parseQuotedString_() {
  auto i= current_;
  int col= column_;
  while ((current_ != text_.end()) && (*current_ != '"')) {
    ++current_;
    ++column_;
  }
  state_= AT_TEXT;
  return Token(TokenType::VALUE, std::string(i, current_), line_, col);
}

bool ConfigFileLexer::skipWhitespace_() {
  while (!skipWhitespaceInLine_()) {
    if (!readNextLine_()) {
      return false;
    }
  }
  return true;
}

bool ConfigFileLexer::skipWhitespaceInLine_() {
  while (current_ != text_.end()) {
    if (!isspace(*current_)) {
      return true;
    }
    ++current_;
    ++column_;
  }
  return false;
}

bool ConfigFileLexer::readNextLine_() {
  if (std::getline(input_, text_)) {
    state_= AT_START;
    current_= text_.begin();
    ++line_;
    column_= 1;
    return true;
  } else {
    state_= AT_EOF;
    return false;
  }
}

