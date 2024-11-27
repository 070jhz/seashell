#pragma once
#include <string>
#include <iostream>
#include "../../model/ast/ASTNode.h"
#include "Token.h"

class Parser {
public:
	std::unique_ptr<ASTNode> parse(const std::string& input);

private:
	std::vector<Token> tokens;
	size_t current = 0;

	std::unique_ptr<ASTNode> program();
	std::unique_ptr<ASTNode> declaration();
	std::unique_ptr<ASTNode> variableDeclaration(Type type, Token name);
	std::unique_ptr<ASTNode> functionDeclaration(Type returnType, Token name);
	std::unique_ptr<ASTNode> statement();
	std::unique_ptr<ASTNode> ifStatement();
	std::unique_ptr<ASTNode> forStatement();
	std::unique_ptr<ASTNode> whileStatement();
	std::unique_ptr<ASTNode> breakStatement();
	std::unique_ptr<ASTNode> continueStatement();
	std::unique_ptr<ASTNode> functionCall(const Token& id);
	std::unique_ptr<ASTNode> returnStatement();
	std::unique_ptr<ASTNode> block();
	std::unique_ptr<ASTNode> expressionStatement();
	std::unique_ptr<ASTNode> expression();
	std::unique_ptr<ASTNode> assignment();
	std::unique_ptr<ASTNode> logicalOr();
	std::unique_ptr<ASTNode> logicalAnd();
	std::unique_ptr<ASTNode> equality();
	std::unique_ptr<ASTNode> comparison();
	std::unique_ptr<ASTNode> term();
	std::unique_ptr<ASTNode> factor();
	std::unique_ptr<ASTNode> unary();
	std::unique_ptr<ASTNode> primary();

	// helper methods
	void synchronize();

	bool isAtEnd() const {
		return peek().type == TokenType::EndOfFile;
	}

	Token peek() const {
		return tokens[current];
	}

	bool check(TokenType type) const {
		if (isAtEnd()) return false;
		return peek().type == type;
	}

	Token previous() const {
		return tokens[current - 1];
	}

	Token advance() {
		if (!isAtEnd()) {
			current++;
		}
		return previous();
	}

	bool match(TokenType type) {
		if (check(type)) {
			advance();
			return true;
		}
		return false;
	}

	Token consume(TokenType type, const std::string& message) {
		std::cout << "attempting to consume token type: " << static_cast<int>(type) << std::endl;
		std::cout << "current token type: " << static_cast<int>(peek().type) << std::endl;
		if (check(type)) {
			return advance();
		}

		std::cout << "failed to consume token. expected " << static_cast<int>(type)
			<< " but found " << static_cast<int>(peek().type) << std::endl;
		throw std::runtime_error(message);
	}

	Type tokenToType(const Token& token) {
		if (token.value == "int") return Type::INT;
		if (token.value == "double") return Type::DOUBLE;
		if (token.value == "bool") return Type::BOOL;
		if (token.value == "string") return Type::STRING;
		if (token.value == "void") return Type::VOID;
		throw std::runtime_error("unknown type keyword: " + token.value);
	}
};
