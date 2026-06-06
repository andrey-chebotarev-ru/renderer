#include "MapFile.h"

#include <iostream>
#include <fstream>

const std::string MapFile::whitespaces = " \t\r\n";
const std::string MapFile::special = "{}()[]";
const std::string MapFile::digits = "0123456789";
const std::string MapFile::letters = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";

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

char MapFile::peek()
{
	return (pos < content.size() ? content[pos] : '\0');
}

char MapFile::peekNext()
{
	return ((pos + 1) < content.size() ? content[pos + 1] : '\0');
}

char MapFile::consume()
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
		return "Open Brace";
	case TokenType::CloseBrace:
		return "Close Brace";
	case TokenType::OpenParenthesis:
		return "Open Parenthesis";
	case TokenType::CloseParenthesis:
		return "Close Parenthesis";
	case TokenType::OpenBracket:
		return "Open Bracket";
	case TokenType::CloseBracket:
		return "Close Bracket";
	case TokenType::Identifier:
		return "Identifier";
	case TokenType::String:
		return "String";
	case TokenType::Number:
		return "Number";
	case TokenType::End:
		return "End";
	default:
		return "Invalid";
	}
}

void MapFile::skipWhitespacesAndComments()
{
	while (true) {

		while (isWhitespace(peek())) {
			consume();
		}

		if (isSlash(peek()) && isSlash(peekNext())) {
			while (!isEndOfLine(peek()) && !isTerminator(peek())) {
				consume();
			}
			continue;
		}

		break;
	}
}

TokenType MapFile::parseSpecial()
{
	TokenType type = TokenType::End;

	char c = peek();

	if (!isSpecial(c)) {
		throw ParseException("Unexpected symbol", line, column);
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

	consume();

	return type;
}

std::string MapFile::parseString()
{
	std::string value;

	if (!isQuote(peek())) {
		throw ParseException("Expected '\"'", line, column);
	}

	consume();

	while (!isQuote(peek()) && !isTerminator(peek())) {
		value += consume();
	}

	
	if (!isQuote(peek())) {
		throw ParseException("Expected '\"'", line, column);
	}

	consume();

	return value;
}

std::string MapFile::parseNumber()
{
	std::string value;

	char c = peek();

	if (isSign(c)) {
		value += consume();
	}

	bool hasDigit = false;
	bool hasDot = false;

	while (true) {

		c = peek();

		if (isDigit(c)) {
			hasDigit = true;
			value += consume();
		}
		else if (isDot(c) && !hasDot) {
			hasDot = true;
			value += consume();
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

	char c = peek();

	if (!isLetter(c) && !isUnderline(c)) {
		throw ParseException("Unexpected symbol", line, column);
	}

	while (true) {
		
		c = peek();

		if (isLetter(c) || isDigit(c) || isUnderline(c)) {
			value += consume();
		}
		else {
			break;
		}
	}

	return value;
}

Token MapFile::next()
{
	Token token;

	skipWhitespacesAndComments();

	char c = peek();

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
		throw ParseException("Unexpected symbol", line, column);
	}

	return token;
}

bool MapFile::parse()
{
	pos = 0;
	line = column = 1;

	lineStarts.clear();
	lineStarts.push_back(0);

	try {
		while (true) {
			Token token = next();

			//std::cout << "Type: " << getTokenTypeName(token.type) << "\n";
			//std::cout << "Value: " << token.value << "\n";
			//std::cout << "\n";

			if (token.type == TokenType::End) {
				break;
			}
		}
	}
	catch (ParseException e) {

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
		return false;
	}

	return true;
	
}