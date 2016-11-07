#ifndef __PISTIS__CONFIG_PARSER__DETAIL__CONFIGFILELEXER_HPP__
#define __PISTIS__CONFIG_PARSER__DETAIL__CONFIGFILELEXER_HPP__

#include <pistis/config_parser/detail/Token.hpp>
#include <iostream>

namespace pistis {
  namespace config_parser {
    namespace detail {

      class ConfigFileLexer {
      public:
	ConfigFileLexer(std::istream& input, int initialLine=1,
			int initialColumn=1);
	ConfigFileLexer(const ConfigFileLexer&) = delete;
	~ConfigFileLexer();

	int currentLine() const { return line_; }
	int currentColumn() const { return column_; }

	/** @brief Parse the next sequence as a value.
	 *
	 *  During the next call to @tt next(), the lexer collects
	 *  everything up to the end of the next line that does not end
	 *  in a backslash and returns it as a Token of type VALUE.
	 *  Lines that end in backslashes have the trailing backslashes
	 *  removed and are concatenated together without the newline
	 *  characters.
	 */
	void parseNextAsValue() { state_= AT_VALUE; }

	/** @brief Parse the next sequence as a quoted string
	 *
	 *  During the next call to @tt next(), everything up to the next
	 *  double-quote or end of line is collected and returned as a token
	 *  of type VALUE.
	 */
	void parseNextAsQuotedString() { state_= AT_QT_STRING; }

	/** @brief Read the next token from the input source.
	 *
	 *  @returns The next token from the input stream.
	 *  @throws  Does not throw
	 */
	Token next();

	ConfigFileLexer& operator=(const ConfigFileLexer&) = delete;

	/** @brief Returns true if @c ch is a character in a property name
	 *         when prceeded by @c prev.
	 *
	 *  @param ch    Current character
	 *  @param prev  Previous character, or 0 if this is the first
	 *                 character
	 *  @return  True if @c ch is a legal part of a property name.
	 *  @throws  Does not throw
	 */
	static bool isNameChar(char ch, char prev) {
	  return isalnum(ch) || (ch == '_') || ((ch == '.') && isalnum(prev));
	}

      protected:
	enum State {
	  AT_START, ///< At start of line
	  AT_TEXT,  ///< Parsing ordinary text
	  AT_VALUE, ///< Next token should be a value
	  AT_QT_STRING, ///< Next token should consist of all text up to the next double-quote (") or end-of-line
	  AT_EOF    ///< At end-of-file
	};

	/** @brief Parse the next sequence as an ordinary token.
	 *
	 *  An ordinary token is a comment, a name, a punctuation mark
	 *  or the end-of-file.
	 *
	 *  @brief Returns the next token
	 */
	Token parseText_();

	/** @brief Parse the next sequence as a value
	 *
	 *  A value token includes all text up to the end of the next line
	 *  that does not end in a backslash.  Lines that end in backslashes
	 *  have the trailing backslash and newline remove and are
	 *  concatenated together along with the last line, which does not
	 *  end a backslash.
	 *
	 *  @param singleLine  If true, only text up to the end of the
	 *                       current line will be returned as a value,
	 *                       even if the current line ends in a backslash
	 *  @returns The next token as a value.
	 */
	Token parseValue_(bool singleLine);

	/** @brief Parse the next token as a quoted string
	 *
	 *  A quoted string includes everything up to the next double
	 *  quote (") or the end of the current line.
	 *
	 *  @returns The next token
	 */
	Token parseQuotedString_();

	/** @brief Skip whitespace in the input
	 *
	 *  @returns True if current_ is positioned at a non-whitespace
	 *             character; false if it is positioned at the
	 *             end-of-file.
	 */
	bool skipWhitespace_();

	/** @brief Skip whitespace up to the end of the current line.
	 *
	 *  @returns True if current_ is positioned at a non-whitespace
	 *             character; false if it is positioned at the
	 *             end of the current line.
	 */
	bool skipWhitespaceInLine_();

	/** @brief Read the next line from the input stream.
	 *
	 *  @returns True if a line was read from input_; false if input_
	 *             has reached the end of file.
	 */
	bool readNextLine_();

      private:
	std::istream& input_;
	int line_;
	int column_;
	std::string text_;  ///< Text of current line
	std::string::const_iterator current_; ///< Current position
	State state_; ///< Current state
      };

    }
  }
}
#endif

