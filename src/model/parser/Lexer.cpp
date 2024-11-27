#include <unordered_map>
#include <vector>
#include <stdexcept>
#include "Lexer.h"

std::vector<Token> Lexer::tokenize() {
	std::vector<Token> tokens;

	while (!isAtEnd()) {
		try {
			tokens.push_back(nextToken());
		}
		catch (const std::exception& e) {
			throw std::runtime_error("line " + std::to_string(line) + ", column "
				+ std::to_string(column) + ": " + e.what());
		}
	}
	tokens.emplace_back(TokenType::EndOfFile, "", line, column);
	return tokens;
}

void Lexer::skipWhiteSpace() {
	while (!isAtEnd()) {
		char c = peek();
		switch (c) {
		case ' ':
		case '\r':
		case '\t':
			advance();
			break;
		case '\n':
			line++;
			column = 1;
			advance();
			break;
		case '/':
			if (peekNext() == '/') {
				while (peek() != '\n' && !isAtEnd()) advance();
			}
			else {
				return;
			}
			break;
		default:
			return;
		}
	}
}

Token Lexer::number() {
	int startColumn = column;
	std::string num;
	bool isDouble = false;

	while (isdigit(peek())) {
		num += advance();
	}

	if (peek() == '.' && isdigit(peekNext())) {
		isDouble = true;
		num += advance(); // consume .
		while (isdigit(peek())) {
			num += advance();
		}
	}

	return Token(isDouble ? TokenType::Double : TokenType::Integer, num, line, startColumn);
}

Token Lexer::identifier() {
	int startColumn = column;
	std::string id;

	while (isalnum(peek()) || peek() == '_') {
		id += advance();
	}

	// check if keyword
	static const std::unordered_set<std::string> keywords = {
		"int", "double", "bool", "string",
		"if", "void", "else", "while", "for",
		"return", "true", "false", "break", "continue"
	};

	TokenType type = keywords.count(id) ? TokenType::Keyword : TokenType::Identifier;
	return Token(type, id, line, startColumn);
}

Token Lexer::makeToken(TokenType type, std::string value) {
	if (value.empty()) value = std::string(1, source[current - 1]);
	return Token(type, value, line, column - value.length());
}

Token Lexer::string() {
	int startColumn = column - 1;
	std::string str;

	while (peek() != '"' && !isAtEnd()) {
		if (peek() == '\n') {
			line++;
			column = 0;
		}
		str += advance();
	}

	if (isAtEnd()) {
		throw std::runtime_error("unterminated string");
	}

	advance(); // consume closing "
	return Token(TokenType::String, str, line, startColumn);
}

bool Lexer::match(char expected) {
	if (isAtEnd() || source[current] != expected) return false;
	current++;
	column++;
	return true;
}

Token Lexer::nextToken() {
	skipWhiteSpace();

	if (isAtEnd()) return Token(TokenType::EndOfFile, "", line, column);

	char c = peek();
	int startColumn = column;

	if (isdigit(c)) return number();
	if (isalpha(c) || c == '_') return identifier();

	advance(); // consume the character

	switch (c) {
	case '(': return { TokenType::LeftParen, "(", line, startColumn };
	case ')': return { TokenType::RightParen, ")", line, startColumn };
	case '[': return { TokenType::LeftBracket, "[", line, startColumn };
	case ']': return { TokenType::RightBracket, "]", line, startColumn };
	case '{': return { TokenType::LeftBrace, "{", line, startColumn };
	case '}': return { TokenType::RightBrace, "}", line, startColumn };
	case ';': return { TokenType::Semicolon, ";", line, startColumn };
	case ',': return { TokenType::Comma, ",", line, startColumn };
	case '*': return { TokenType::Operator, "*", line, startColumn };
	case '/': return { TokenType::Operator, "/", line, startColumn };
	case '"': return string();
	case '<':
		if (match('=')) return { TokenType::Operator, "<=", line, startColumn };
		return { TokenType::Operator, "<", line, startColumn };
	case '>':
		if (match('=')) return { TokenType::Operator, ">=", line, startColumn };
		return { TokenType::Operator, ">", line, startColumn };
	case '+':
		if (match('+')) return { TokenType::Operator, "++", line, startColumn };
		return { TokenType::Operator, "+", line, startColumn };
	case '-':
		if (match('-')) return { TokenType::Operator, "--", line, startColumn };
		return { TokenType::Operator, "-", line, startColumn };
	case '=':
		if (match('=')) return { TokenType::Operator, "==", line, startColumn };
		return { TokenType::Operator, "=", line, startColumn };

	}

	throw std::runtime_error("unexpected character: " + std::string(1, c));
}

