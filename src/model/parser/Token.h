#pragma once
#include <string>
#include <unordered_set>
#include <sstream>


// token types for lexical analysis
enum class TokenType {
	Keyword, // *int* x = 5;
	Identifier,
	Integer, // int x = *5*;
	Double, // double x = *5.0*
	String,
	Operator, // +, -, *, /, =; ==, etc. 
	LeftParen,
	RightParen,
	LeftBracket,
	RightBracket,
	LeftBrace,
	RightBrace,
	Semicolon,
	Comma,
	EndOfFile
};

// represents a token in source code
class Token {
public:
	Token(TokenType type, std::string value, int line, int column)
		: type(type), value(value), line(line), column(column) {
	}

	TokenType type;
	std::string value;
	size_t line;
	size_t column;

	std::string toString() const {
		std::ostringstream oss;
		oss << "Token (" << static_cast<int>(type) << ", '" << value << "', line "
			<< line << ", column " << column << ")";
		return oss.str();
	}
};
