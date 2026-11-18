#include "Parser.h"

#include <format>
#include <iostream>
#include <fstream>

const std::string Parser::whitespaces = " \t\r\n";
const std::string Parser::special = "{}()[]";
const std::string Parser::digits = "0123456789";
const std::string Parser::letters = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
const std::string Parser::extra = "!@#$%^&*-+_~/{}\\";

bool Parser::load(const std::string& path)
{
	filename = path;
	std::ifstream file(path);

	if (!file) {
		return false;
	}

	content = std::string(std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>());

	return true;
}

char Parser::peekChar()
{
	return (pos < content.size() ? content[pos] : '\0');
}

char Parser::peekNextChar(int offset)
{
	return ((pos + 1) < content.size() ? content[pos + offset] : '\0');
}

char Parser::consumeChar()
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

bool Parser::checkChar(char c)
{
	return (peekChar() == c);
}

void Parser::expectChar(char c)
{
	char g = consumeChar();
	if (g != c) {
		throw ParseException(std::format("Expected {}, got {}", c, g), line, column);
	}
}

bool Parser::isWhitespace(char c)
{
	return (std::string_view(whitespaces).find(c) != std::string_view::npos);
}

bool Parser::isSpecial(char c)
{
	return (std::string_view(special).find(c) != std::string_view::npos);
}

bool Parser::isDigit(char c)
{
	return (std::string_view(digits).find(c) != std::string_view::npos);
}

bool Parser::isLetter(char c)
{
	return (std::string_view(letters).find(c) != std::string_view::npos);
}

bool Parser::isExtra(char c)
{
	return (std::string_view(extra).find(c) != std::string_view::npos);
}

bool Parser::isQuote(char c)
{
	return (c == '"');
}

bool Parser::isSign(char c)
{
	return (c == '-');
}

bool Parser::isDot(char c)
{
	return (c == '.');
}

bool Parser::isUnderline(char c)
{
	return (c == '_');
}

bool Parser::isSlash(char c)
{
	return (c == '/');
}

bool Parser::isEndOfLine(char c)
{
	return (c == '\n');
}

bool Parser::isTerminator(char c)
{
	return (c == '\0');
}

std::string Parser::getTokenTypeName(TokenType type)
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

void Parser::skipWhitespacesAndComments()
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

Parser::TokenType Parser::parseSpecial()
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

std::string Parser::parseString()
{
	std::string value;

	expectChar('"');
	
	while (!checkChar('"') && !checkChar('\0')) {
		value += consumeChar();
	}

	expectChar('"');

	return value;
}

std::string Parser::parseNumber()
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

std::string Parser::parseIdentifier()
{
	std::string value;

	char c = peekChar();

	if (!isLetter(c) && !isUnderline(c) && !isExtra(c) && !isDigit(c)) {
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

Parser::Token Parser::peekToken()
{
	return currentToken;
}

Parser::Token Parser::consumeToken()
{
	Token cur = currentToken;
	
	currentToken = nextToken();
	
	return cur;
}

bool Parser::checkToken(TokenType type)
{
	return (currentToken.type == type);
}

// number or identifier?
bool Parser::analize()
{
	int pos = 0;

	while (true) {
		char c = peekNextChar(pos++);

		if (isQuote(c)) {
			return false;
		}

		if (isLetter(c)) {
			return true;
		}

		if (isTerminator(c) || isWhitespace(c)) {
			break;
		}
	}

	return false;
}

Parser::Token Parser::nextToken()
{
	Token token;

	skipWhitespacesAndComments();

	token.line = line;
	token.column = column;

	char c = peekChar();

	if (analize()) {
		token.type = TokenType::Identifier;
		token.value = parseIdentifier();
	} else

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
	else if (isLetter(c) || isUnderline(c) || isExtra(c)) {
		token.type = TokenType::Identifier;
		token.value = parseIdentifier();
	}
	else {
		throw ParseException(std::format("Unexpected token '{}'", c), line, column);
	}

	return token;
}

Parser::Token Parser::expectToken(TokenType type)
{
	Token t = consumeToken();

	if (t.type != type) {
		throw ParseException(std::format("Expected '{}', got '{}'", getTokenTypeName(type), getTokenTypeName(t.type)), t.line, t.column);
	}
	
	return t;
}

std::string Parser::expectString()
{
	return expectToken(TokenType::String).value;
}

float Parser::expectNumber()
{
	return std::stof(expectToken(TokenType::Number).value);
}

std::string Parser::expectIdentifier()
{
	return expectToken(TokenType::Identifier).value;
}

void Parser::printParseException(const ParseException& e)
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

void Parser::printMap(const BrushMap& map)
{
	for (const auto& entity : map.entities) {

		std::cout << "Entity:\n";

		for (const auto& [key, value] : entity.properties) {
			std::cout << std::format("\t'{}' : '{}'\n", key, value);
		}
		for (const auto& brush : entity.brushes) {
			std::cout << "\tBrush:\n";

			for (const auto side : brush.sides) {
				std::cout << std::format("\t\t( {}, {}, {} ) ( {}, {}, {} ) ( {}, {}, {} ) {} [ {} {} {} {} ] [ {} {} {} {} ] {} {} {}\n",
					side.p1.x, side.p1.y, side.p1.z,
					side.p2.x, side.p2.y, side.p2.z,
					side.p3.x, side.p3.y, side.p3.z,
					side.texName,
					side.uAxis.x, side.uAxis.y, side.uAxis.z, side.uShift,
					side.vAxis.x, side.vAxis.y, side.vAxis.z, side.vShift,
					side.rotation,
					side.xScale, side.yScale
				);
			}
		}
	}
}

void Parser::parseBrushSide(BrushSideList& sides)
{
	BrushSide side;

	// (
	expectToken(TokenType::OpenParenthesis);

	side.p1.x = expectNumber();
	side.p1.y = expectNumber();
	side.p1.z = expectNumber();

	// )(
	expectToken(TokenType::CloseParenthesis);
	expectToken(TokenType::OpenParenthesis);

	side.p2.x = expectNumber();
	side.p2.y = expectNumber();
	side.p2.z = expectNumber();

	// )(
	expectToken(TokenType::CloseParenthesis);
	expectToken(TokenType::OpenParenthesis);

	Vec3 p3;

	side.p3.x = expectNumber();
	side.p3.y = expectNumber();
	side.p3.z = expectNumber();

	// )
	expectToken(TokenType::CloseParenthesis);

	side.texName = expectIdentifier();

	// [
	expectToken(TokenType::OpenBracket);

	side.uAxis.x = expectNumber();
	side.uAxis.y = expectNumber();
	side.uAxis.z = expectNumber();
	side.uShift = expectNumber();

	// ][
	expectToken(TokenType::CloseBracket);
	expectToken(TokenType::OpenBracket);

	side.vAxis.x = expectNumber();
	side.vAxis.y = expectNumber();
	side.vAxis.z = expectNumber();
	side.vShift = expectNumber();
	
	// ]
	expectToken(TokenType::CloseBracket);

	side.rotation = expectNumber();

	side.xScale = expectNumber();
	side.yScale	= expectNumber();

	sides.push_back(side);
}

void Parser::parseProperty(BrushEntityProperties& properties)
{
	std::string key = expectString();
	std::string value = expectString();

	properties[key] = value;
}

bool Parser::parse(BrushMap& map)
{
	pos = 0;
	line = column = 1;

	lineStarts.clear();
	lineStarts.push_back(0);

	currentToken = nextToken();

	try {
		while (true) {

			BrushEntity entity;

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

					Brush brush;

					while (true) {

						// }
						if (checkToken(TokenType::CloseBrace)) {
							consumeToken();
							break;
						}

						parseBrushSide(brush.sides);
					}

					if (!brush.sides.empty()) {
						entity.brushes.push_back(brush);
					}

				}
				else {
					parseProperty(entity.properties);
				}
			}

			if (!entity.properties.empty() || !entity.brushes.empty()) {
				map.entities.push_back(entity);
			}
		}
	}
	catch (ParseException e) {
		printParseException(e);
		
		return false;
	}

	return true;
	
}