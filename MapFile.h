#pragma once


#include <string>
#include <vector>
#include <stdexcept>
#include <unordered_map>

#include "Vector.h"

struct MapBrushPlane
{
	Vec3 p1, p2, p3;

	std::string texName;

	Vec2 shift;
	Vec2 scale;
	float rot;
};

struct MapBrush
{
	std::vector<MapBrushPlane> planes;
};

struct MapEntity
{
	std::unordered_map<std::string, std::string> properties;
	std::vector<MapBrush> brushes;
};

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
	size_t line = 1;
	size_t column = 1;
};

class ParseException : public std::runtime_error
{
public:

	ParseException(const std::string& message, size_t line, size_t column)
		: runtime_error(message), line(line), column(column) {
	}

	size_t getLine() const {
		return line;
	}

	size_t getColumn() const {
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
	char peekChar();
	char peekNextChar();
	char consumeChar();
	bool checkChar(char c);
	void expectChar(char c);

	bool isWhitespace(char c);
	bool isSpecial(char c);
	bool isDigit(char c);
	bool isLetter(char c);
	bool isExtra(char c);
	bool isQuote(char c);
	bool isSign(char c);
	bool isDot(char c);
	bool isUnderline(char c);
	bool isSlash(char c);
	bool isEndOfLine(char c);
	bool isTerminator(char c);

	std::string getTokenTypeName(TokenType type);
	void skipWhitespacesAndComments();

	std::string parseString();
	std::string parseNumber();
	std::string parseIdentifier();
	
	TokenType parseSpecial();

	Token currentToken;

	Token peekToken();
	Token consumeToken();
	bool checkToken(TokenType type);
	Token nextToken();
	Token expectToken(TokenType type);

	std::string expectString();
	float expectNumber();
	std::string expectIdentifier();

	void printParseException(const ParseException& e);

	void parseBrushPlane();

	void parseProperty();

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
	static const std::string extra;
};