#include "TokenType.hpp"

using namespace pistis::config_parser::detail;

std::ostream& pistis::config_parser::detail::operator<<(
    std::ostream& out, TokenType tokenType
) {
  switch(tokenType) {
    case TokenType::NAME:        return out << "NAME";
    case TokenType::VALUE:       return out << "VALUE";
    case TokenType::COMMENT:     return out << "COMMENT";
    case TokenType::PUNCTUATION: return out << "PUNCTUATION";
    case TokenType::END_OF_FILE: return out << "END_OF_FILE";
    default:                     return out << "**UNKNOWN**";
  }
}
