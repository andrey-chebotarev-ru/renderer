#include "MapFile.h"

#include <format>
#include <iostream>
#include <fstream>

const std::string MapFile::whitespaces = " \t\r\n";
const std::string MapFile::special = "{}()[]";
const std::string MapFile::digits = "0123456789";
const std::string MapFile::letters = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
const std::string MapFile::extra = "!@#$%^&*-+_~/{}\\";

bool MapFile::load(const std::string& path)
{
	filename = path;
	std::ifstream file(path);

	if (!file) {
		std::cerr << "Failed to open file: " << path << "\n";
		return false;
	}

	content = std::string(std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>());
	
	return true;
}

char MapFile::peekChar()
{
	return (pos < content.size() ? content[pos] : '\0');
}

char MapFile::peekNextChar()
{
	return ((pos + 1) < content.size() ? content[pos + 1] : '\0');
}

char MapFile::consumeChar()
{
	if (pos < content.size()) {
		char c = content[pos++];
		
		if (isEndOfLine(c)) {
			lineStarts.push_back(pos);
			line++;
			column = 1;
		}
		else {
			column++;
		}
		
		return c;
	}
}

bool MapFile::checkChar(char c)
{
	return (peekChar() == c);
}

void MapFile::expectChar(char c)
{
	char g = consumeChar();
	if (g != c) {
		throw ParseException(std::format("Expected {}, got {}", c, g), line, column);
	}
}

bool MapFile::isWhitespace(char c)
{
	return (std::string_view(whitespaces).find(c) != std::string_view::npos);
}

bool MapFile::isSpecial(char c)
{
	return (std::string_view(special).find(c) != std::string_view::npos);
}

bool MapFile::isDigit(char c)
{
	return (std::string_view(digits).find(c) != std::string_view::npos);
}

bool MapFile::isLetter(char c)
{
	return (std::string_view(letters).find(c) != std::string_view::npos);
}

bool MapFile::isExtra(char c)
{
	return (std::string_view(extra).find(c) != std::string_view::npos);
}

bool MapFile::isQuote(char c)
{
	return (c == '"');
}

bool MapFile::isSign(char c)
{
	return (c == '+' || c == '-');
}

bool MapFile::isDot(char c)
{
	return (c == '.');
}

bool MapFile::isUnderline(char c)
{
	return (c == '_');
}

bool MapFile::isSlash(char c)
{
	return (c == '/');
}

bool MapFile::isEndOfLine(char c)
{
	return (c == '\n');
}

bool MapFile::isTerminator(char c)
{
	return (c == '\0');
}

std::string MapFile::getTokenTypeName(TokenType type)
{
	switch (type) {
	case TokenType::OpenBrace:
		return "{";
	case TokenType::CloseBrace:
		return "}";
	case TokenType::OpenParenthesis:
		return "(";
	case TokenType::CloseParenthesis:
		return ")";
	case TokenType::OpenBracket:
		return "[";
	case TokenType::CloseBracket:
		return "]";
	case TokenType::Identifier:
		return "<Identifier>";
	case TokenType::String:
		return "<String>";
	case TokenType::Number:
		return "<Number>";
	case TokenType::End:
		return "<EOL>";
	default:
		return "<Invalid>";
	}
}

void MapFile::skipWhitespacesAndComments()
{
	while (true) {

		while (isWhitespace(peekChar())) {
			consumeChar();
		}

		if (isSlash(peekChar()) && isSlash(peekNextChar())) {
			while (!isEndOfLine(peekChar()) && !isTerminator(peekChar())) {
				consumeChar();
			}
			continue;
		}

		break;
	}
}

TokenType MapFile::parseSpecial()
{
	TokenType type = TokenType::End;

	char c = peekChar();

	if (!isSpecial(c)) {
		throw ParseException(std::format("Unexpected token '{}'", c), line, column);
	}

	switch (c) {
	case '{':
		type = TokenType::OpenBrace;
		break;
	case '}':
		type = TokenType::CloseBrace;
		break;
	case '(':
		type = TokenType::OpenParenthesis;
		break;
	case ')':
		type = TokenType::CloseParenthesis;
		break;
	case '[':
		type = TokenType::OpenBracket;
		break;
	case ']':
		type = TokenType::CloseBracket;
		break;
	default:
		type = TokenType::Invalid;
		break;
	}

	consumeChar();

	return type;
}

std::string MapFile::parseString()
{
	std::string value;

	expectChar('"');
	
	while (!checkChar('"') && !checkChar('\0')) {
		value += consumeChar();
	}

	expectChar('"');

	return value;
}

std::string MapFile::parseNumber()
{
	std::string value;

	if (checkChar('-')) {
		value += consumeChar();
	}

	bool hasDigit = false;
	bool hasDot = false;

	while (true) {

		if (isDigit(peekChar())) {
			hasDigit = true;
			value += consumeChar();
		}
		else if (checkChar('.') && !hasDot) {
			hasDot = true;
			value += consumeChar();
		}
		else {
			break;
		}
	}

	if (!hasDigit) {
		throw ParseException("Invalid number", line, column);
	}

	return value;
}

std::string MapFile::parseIdentifier()
{
	std::string value;

	char c = peekChar();

	if (!isLetter(c) && !isUnderline(c)) {
		throw ParseException("Expected identifier", line, column);
	}

	while (true) {
		
		c = peekChar();

		if (isLetter(c) || isDigit(c) || isExtra(c)) {
			value += consumeChar();
		}
		else {
			break;
		}
	}

	return value;
}

Token MapFile::peekToken()
{
	return currentToken;
}

Token MapFile::consumeToken()
{
	Token cur = currentToken;
	
	currentToken = nextToken();
	
	return cur;
}

bool MapFile::checkToken(TokenType type)
{
	return (currentToken.type == type);
}

Token MapFile::nextToken()
{
	Token token;

	skipWhitespacesAndComments();

	token.line = line;
	token.column = column;

	char c = peekChar();


	if (isTerminator(c)) {
		token.type = TokenType::End;
	}
	else if (isSpecial(c)) {
		token.type = parseSpecial();
		token.value = c;
	}
	else if (isQuote(c)) {
		token.type = TokenType::String;
		token.value = parseString();
	}
	else if (isSign(c) || isDigit(c)) {
		token.type = TokenType::Number;
		token.value = parseNumber();
	}
	else if (isLetter(c) || isUnderline(c)) {
		token.type = TokenType::Identifier;
		token.value = parseIdentifier();
	}
	else {
		throw ParseException(std::format("Unexpected token '{}'", c), line, column);
	}

	return token;
}

Token MapFile::expectToken(TokenType type)
{
	Token t = consumeToken();

	if (t.type != type) {
		throw ParseException(std::format("Expected '{}', got '{}'", getTokenTypeName(type), getTokenTypeName(t.type)), t.line, t.column);
	}
	
	return t;
}

std::string MapFile::expectString()
{
	return expectToken(TokenType::String).value;
}

float MapFile::expectNumber()
{
	return std::stof(expectToken(TokenType::Number).value);
}

std::string MapFile::expectIdentifier()
{
	return expectToken(TokenType::Identifier).value;
}

void MapFile::printParseException(const ParseException& e)
{
	size_t start = lineStarts[e.getLine() - 1];

	char* tmp = (content.data() + start);
	std::string str = "";

	while (*tmp != '\0') {
		if (*tmp == '\n') {
			break;
		}
		str += *tmp;
		tmp++;
	}

	std::string str2 = "";
	for (int i = 0; i < e.getColumn() - 1; i++) {
		str2 += " ";
	}
	str2 += "^";

	std::cerr << filename << ":" << e.getLine() << ":" << e.getColumn() << "\n" << str << "\n" << str2 << "\n" << "\n" << e.what() << "\n";
}

void MapFile::parseBrushPlane()
{
	// (
	expectToken(TokenType::OpenParenthesis);

	Vec3 p1;

	p1.x = expectNumber();
	p1.y = expectNumber();
	p1.z = expectNumber();

	// )(
	expectToken(TokenType::CloseParenthesis);
	expectToken(TokenType::OpenParenthesis);

	Vec3 p2;

	p2.x = expectNumber();
	p2.y = expectNumber();
	p2.z = expectNumber();

	// )(
	expectToken(TokenType::CloseParenthesis);
	expectToken(TokenType::OpenParenthesis);

	Vec3 p3;

	p3.x = expectNumber();
	p3.y = expectNumber();
	p3.z = expectNumber();

	// )
	expectToken(TokenType::CloseParenthesis);

	std::string texName = expectIdentifier();

	// [
	expectToken(TokenType::OpenBracket);

	Vec2 uAxis;
	
	uAxis.x = expectNumber();
	uAxis.y	= expectNumber();

	Vec2 uShift;
	
	uShift.x = expectNumber();
	uShift.y = expectNumber();

	// ][
	expectToken(TokenType::CloseBracket);
	expectToken(TokenType::OpenBracket);

	Vec2 vAxis;

	vAxis.x = expectNumber();
	vAxis.y = expectNumber();

	Vec2 vShift;

	vShift.x = expectNumber();
	vShift.y = expectNumber();

	// ]
	expectToken(TokenType::CloseBracket);

	float rot = expectNumber();

	Vec2 scale;

	scale.x = expectNumber();
	scale.y	= expectNumber();

	std::cout << std::format("Plane ( {} {} {} ) ( {} {} {} ) ( {} {} {} ) {} [ {} {} {} {} ] [ {} {} {} {} ] {} {} {}\n",
		p1.x, p1.y, p1.z,
		p2.x, p2.y, p2.z,
		p3.x, p3.y, p3.z,
		texName,
		uAxis.x, uAxis.y, uShift.x, uShift.y,
		vAxis.x, vAxis.y, vShift.x, vShift.y,
		rot,
		scale.x, scale.y
	);
}

void MapFile::parseProperty()
{
	std::string key = expectString();
	std::string value = expectString();

	std::cout << std::format("Property: '{}' : '{}'\n", key, value);
}

bool MapFile::parse()
{
	pos = 0;
	line = column = 1;

	lineStarts.clear();
	lineStarts.push_back(0);

	currentToken = nextToken();

	try {
		while (true) {
			if (checkToken(TokenType::End)) {
				break;
			}

			// {
			expectToken(TokenType::OpenBrace);

			while (true) {

				// }
				if (checkToken(TokenType::CloseBrace)) {
					consumeToken();
					break;
				}

				// {
				if (checkToken(TokenType::OpenBrace)) {
					consumeToken();

					while (true) {

						// }
						if (checkToken(TokenType::CloseBrace)) {
							consumeToken();
							break;
						}

						parseBrushPlane();
					}
				}
				else {
					parseProperty();
				}
			}
		}
	}
	catch (ParseException e) {
		printParseException(e);
		
		return false;
	}

	return true;
	
}