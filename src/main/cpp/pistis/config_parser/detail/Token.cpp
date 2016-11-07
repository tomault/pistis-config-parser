#include "Token.hpp"

using namespace pistis::config_parser::detail;

Token::Token(TokenType type, const std::string& value, int line, int column):
    type_(type), value_(value), line_(line), column_(column) {
  // Intentionally left blank
}

Token::Token(const Token& other):
    type_(other.type_), value_(other.value_), line_(other.line_),
    column_(other.column_) {
  // Intentionally left blank
}

Token& Token::operator=(const Token& other) {
  type_= other.type();
  value_= other.value_;
  line_= other.line();
  column_= other.column();
  return *this;
}
