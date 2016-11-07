#ifndef __PISTIS__CONFIG_PARSER__DETAIL__TOKEN_HPP__
#define __PISTIS__CONFIG_PARSER__DETAIL__TOKEN_HPP__

#include <pistis/config_parser/detail/TokenType.hpp>
#include <iostream>
#include <string>

namespace pistis {
  namespace config_parser {
    namespace detail {

      /** @brief Token from a configuration file */
      class Token {
      public:
	Token(TokenType type, const std::string& value, int line, int column);
	Token(const Token& other);

	TokenType type() const { return type_; }
	const std::string& value() const { return value_; }
	int line() const { return line_; }
	int column() const { return column_; }

	Token& operator=(const Token& other);
	bool operator==(const Token& other) const {
	  return (type() == other.type()) && (value() == other.value()) &&
  	         (line() == other.line()) && (column() == other.column());
	}
	bool operator!=(const Token& other) const {
	  return (type() != other.type()) || (value() != other.value()) ||
	         (line() != other.line()) || (column() != other.column());
	}

      private:
	TokenType type_;
	std::string value_;
	int line_;
	int column_;
      };

      inline std::ostream& operator<<(std::ostream& out, const Token& t) {
	return out << t.type() << "@" << t.line() << "," << t.column() << "["
		   << t.value() << "]";
      }

    }
  }
}
#endif
