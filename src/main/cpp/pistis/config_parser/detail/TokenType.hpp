#ifndef __PISTIS__CONFIG_PARSER__DETAIL__TOKENTYPE_HPP__
#define __PISTIS__CONFIG_PARSER__DETAIL__TOKENTYPE_HPP__

#include <ostream>

namespace pistis {
  namespace config_parser {
    namespace detail {

      enum class TokenType {
	/** @brief Property name
	 *
	 *  A property name is a sequence of names separated by single
	 *  period (".") characters.  A name consists of letters, numbers
	 *  and underscores and begins with a letter.  
	 */
	NAME,

	/** @brief Property value
	 *
	 *  A property value is everything following a "=" until the end
	 *  of the line.  Patterns of the form "${name}", where "name" is
	 *  a property name are replaced with the value of the named
	 *  property.  If no such property exists, then the pattern is
	 *  replaced with the value of the environment variable with
	 *  the given name.  If no such environment variable is defined,
	 *  then an error results.  The backslash ("\") character serves as 
	 *  an escape character in the property value.  A backslash at the
	 *  end of the line extends the property value to the next line.
	 *  A backslash followed by a "n", "t" or "r" character is replaced
	 *  by a newline, tab or carriage return respectively.  A backslash
	 *  followed by a "uXX" sequence, where XX is a set of 1 to 4 hex
	 *  digits is replaced with the Unicode character indicated by the
	 *  value of the hex digits, encoded into UTF-8.  A backslash
	 *  followed by any other character is relaced by that character.
	 *  A backslash at the end of file is ignored.
	 */
	VALUE,

	/** @brief A comment
	 *
	 *  The value of the token is all text following the "#" until
	 *  the end of the line, less any leading and trailing whitespace.
	 *  Variable substitution is not performed in comments and
	 *  backslashes are not treated as escape characters.
	 */
	COMMENT,

	/** @brief Any other character */
	PUNCTUATION,

	/** @brief End-of-file */
	END_OF_FILE
      };

      std::ostream& operator<<(std::ostream& out, TokenType tokenType);

    }
  }
}
#endif

