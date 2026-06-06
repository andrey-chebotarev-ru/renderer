#pragma once


#include <string>
#include <vector>
#include <stdexcept>

enum class TokenType
{
	OpenBrace,			// {
	CloseBrace,			// }
	OpenParenthesis,	// (
	CloseParenthesis,	// )
	OpenBracket,		// [
	CloseBracket,		// ]
	
	Identifier,			// some_value1
	String,				// "example string"
	Number,				// -128.12

	End,
	Invalid,
};

struct Token
{
	TokenType type = TokenType::End;
	std::string value = "";
};

class ParseException : public std::runtime_error
{
public:

	ParseException(const std::string& message, size_t line, size_t column)
		: runtime_error(message), line(line), column(column) {
	}

	size_t getLine() {
		return line;
	}

	size_t getColumn() {
		return column;
	}

private:

	size_t line;
	size_t column;
};

class MapFile
{
public:

	bool load(const std::string& filename);
	bool parse();
	
private:
	char peek();
	char peekNext();
	char consume();

	bool isWhitespace(char c);
	bool isSpecial(char c);
	bool isDigit(char c);
	bool isLetter(char c);
	bool isQuote(char c);
	bool isSign(char c);
	bool isDot(char c);
	bool isUnderline(char c);
	bool isSlash(char c);
	bool isEndOfLine(char c);
	bool isTerminator(char c);

	std::string getTokenTypeName(TokenType type);
	void skipWhitespacesAndComments();

	TokenType parseSpecial();
	std::string parseString();
	std::string parseNumber();
	std::string parseIdentifier();

	Token next();

	std::string filename;
	std::string content;

	size_t pos = 0;
	size_t line = 1;
	size_t column = 1;

	std::vector<size_t> lineStarts;

	static const std::string whitespaces;
	static const std::string special;
	static const std::string digits;
	static const std::string letters;
};