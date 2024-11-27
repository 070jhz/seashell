#pragma once
#include <vector>
#include "Token.h"

class Lexer {
private:
	Token nextToken();
	Token number();
	Token identifier();
	Token string();

	bool isAtEnd() const { return current >= source.length(); }
	char advance() { column++; return source[current++]; }
	char peek() const { return isAtEnd() ? '\0' : source[current]; }
	char peekNext() const { return current + 1 >= source.length() ? '\0' : source[current + 1]; }

	bool match(char expected);
	void skipWhiteSpace();
	Token makeToken(TokenType type, std::string value = "");

	std::string source;
	size_t current;
	int line;
	int column;

public:
	Lexer(std::string source) : source(source), current(0), line(1), column(1) {};

	std::vector<Token> tokenize();
};

