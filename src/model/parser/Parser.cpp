#include "Parser.h"
#include "Lexer.h"
#include <iostream>

std::unordered_set<std::string> reservedKeywords = {
		"if", "else", "while", "return", "for", "true", "false", "break", "continue"
};

std::unique_ptr<ASTNode> Parser::parse(const std::string& input) {
	Lexer lexer(input);
	tokens = lexer.tokenize();
	current = 0;
	return program();
}

std::unique_ptr<ASTNode> Parser::program() {
	std::vector<std::unique_ptr<ASTNode>> statements;
	// parse until EOF or all declarations consumed
	while (!isAtEnd()) {
		std::unique_ptr<ASTNode> decl = declaration();
		if (decl == nullptr) {
			break;
		}

		statements.push_back(std::move(decl));
	}
	return std::make_unique<BlockNode>(std::move(statements), false); // don't create scope for global block
}

std::unique_ptr<ASTNode> Parser::declaration() {
	try {
		if (check(TokenType::Keyword) &&
			reservedKeywords.find(peek().value) == reservedKeywords.end()) {

			advance();
			Token typeToken = previous();
			// convert keyword into type		
			Type declType = tokenToType(typeToken);

			Token name = consume(TokenType::Identifier, "expect name after type");

			// look ahead to see if function or variable declaration
			if (check(TokenType::LeftParen)) {
				return functionDeclaration(declType, name);
			}
			return variableDeclaration(declType, name);
		}
		return statement();
	}
	catch (const std::runtime_error& error) {
		synchronize();
		return nullptr;
	}
}

std::unique_ptr<ASTNode> Parser::variableDeclaration(Type type, Token name) {
	std::vector<std::unique_ptr<ASTNode>> declarations;

	do {
		std::unique_ptr<ASTNode> initializer = nullptr;
		// handles arrays, else case handles regulars
		if (match(TokenType::LeftBracket)) {
			Type arrayType = type;
			type = Type::ARRAY;
			std::unique_ptr<ASTNode> size = nullptr;

			int specifiedSize = 5; // default size

			if (!check(TokenType::RightBracket)) {
				size = expression();
				specifiedSize = std::stoi(size->toString());
			}
			consume(TokenType::RightBracket, "expect ']' after array size if any");

			if (match(TokenType::Operator) && previous().value == "=") {
				initializer = expression();
				auto* arrayNode = dynamic_cast<ArrayNode*>(initializer.get());

				// check if the initializer is an ArrayNode and validate its size
				if (arrayNode->getSize() > specifiedSize) {
					throw std::runtime_error("array initializer size " +
						std::to_string(arrayNode->getSize()) +
						" exceeds specified size " +
						std::to_string(specifiedSize));
				}
			}
			else {
				// create default array with specified size
				std::vector<std::unique_ptr<ASTNode>> elements;
				for (int i = 0; i < specifiedSize; i++) {
					switch (arrayType) {
					case Type::INT:
						elements.push_back(std::make_unique<LiteralNode>(0));
						break;
					case Type::DOUBLE:
						elements.push_back(std::make_unique<LiteralNode>(0.0));
						break;
					case Type::BOOL:
						elements.push_back(std::make_unique<LiteralNode>(false));
						break;
					case Type::STRING:
						elements.push_back(std::make_unique<LiteralNode>(std::string("")));
						break;
					default:
						throw std::runtime_error("invalid type for array declaration");
					}
				}
				initializer = std::make_unique<ArrayNode>(std::move(elements), arrayType);
			}

		}
		else {
			if (match(TokenType::Operator) && previous().value == "=") {
				initializer = expression();
			}
			else {
				switch (type) {
				case Type::INT:
					initializer = std::make_unique<LiteralNode>(0);
					break;
				case Type::DOUBLE:
					initializer = std::make_unique<LiteralNode>(0.0);
					break;
				case Type::BOOL:
					initializer = std::make_unique<LiteralNode>(false);
					break;
				case Type::STRING:
					initializer = std::make_unique<LiteralNode>("");
					break;
				default:
					throw std::runtime_error("invalid type for variable declaration");
				}
			}
		}
		declarations.push_back(std::make_unique<AssignmentNode>(name.value, type, std::move(initializer)));

	} while (match(TokenType::Comma) && (name = consume(TokenType::Identifier, "expect additional variable name after ','"), true));

	consume(TokenType::Semicolon, "expect ';' after variable declaration");
	if (declarations.size() == 1) {
		return std::move(declarations[0]);
	}
	else {
		return std::make_unique<BlockNode>(std::move(declarations), false); // grouping only
	}
}

std::unique_ptr<ASTNode> Parser::functionDeclaration(Type returnType, Token name) {
	// we already consumed return type and function name
	consume(TokenType::LeftParen, "expect '(' after function name");
	std::vector<std::pair<std::string, Type>> parameters;
	std::vector<std::unique_ptr<ASTNode>> arguments;
	bool isCall = false;

	if (!check(TokenType::RightParen)) {
		do {
			if (check(TokenType::Keyword)) {
				// func decl
				Token paramType = consume(TokenType::Keyword, "expect parameter type");
				Token paramName = consume(TokenType::Identifier, "expect parameter name");
				parameters.emplace_back(paramName.value, tokenToType(paramType));
			}
			else {
				// func call
				isCall = true;
				arguments.push_back(expression());
			}

		} while (match(TokenType::Comma));

		if (isCall) {
			consume(TokenType::RightParen, "expect ')' after parameters");
			consume(TokenType::Semicolon, "expect ';' after func call");
			return std::make_unique<CallNode>(name.value, std::move(arguments));
		}
	}
	consume(TokenType::RightParen, "expect ')' after parameters");
	auto body = block();
	return std::make_unique<FunctionNode>(name.value, parameters, returnType, std::move(body));
}

std::unique_ptr<ASTNode> Parser::statement() {
	if (check(TokenType::Keyword)) {
		if (peek().value == "if") return ifStatement();
		if (peek().value == "while") return whileStatement();
		if (peek().value == "return") return returnStatement();
		if (peek().value == "for") return forStatement();
		if (peek().value == "break") return breakStatement();
		if (peek().value == "continue") return continueStatement();

		return declaration();
	}

	// parse block
	if (check(TokenType::LeftBrace)) return block();
	return expressionStatement(); // parse as expression statement otherwise
}

std::unique_ptr<ASTNode> Parser::ifStatement() {
	consume(TokenType::Keyword, "expect 'if'");
	consume(TokenType::LeftParen, "expect '(' after 'if'");
	auto condition = expression(); 	// parse condition
	consume(TokenType::RightParen, "expect ')' after if condition");

	auto thenBranch = statement();

	std::unique_ptr<ASTNode> elseBranch = nullptr;

	if (check(TokenType::Keyword) && peek().value == "else") {
		advance();
		elseBranch = statement();
	}

	return std::make_unique<IfNode>(std::move(condition), std::move(thenBranch), std::move(elseBranch));
}

std::unique_ptr<ASTNode> Parser::forStatement() {
	consume(TokenType::Keyword, "expect 'for'");
	consume(TokenType::LeftParen, "expect '(' after 'for'");

	std::unique_ptr<ASTNode> init = nullptr;
	if (match(TokenType::Semicolon)) {
		init = nullptr;
	}
	else if (check(TokenType::Keyword)) {
		init = declaration();
	}
	else {
		init = expressionStatement();
	}

	// condition clause
	std::unique_ptr<ASTNode> condition = nullptr;
	if (!check(TokenType::Semicolon)) {
		condition = expression();
	}
	consume(TokenType::Semicolon, "expect ';' after loop condition");

	// increment clause
	std::unique_ptr<ASTNode> increment = nullptr;
	if (!check(TokenType::RightParen)) {
		increment = expression();
	}
	consume(TokenType::RightParen, "expect ')' after for clauses");

	// body stmts
	auto body = statement();

	return std::make_unique<ForNode>(
		std::move(init),
		std::move(condition),
		std::move(increment),
		std::move(body)
	);
}

std::unique_ptr<ASTNode> Parser::whileStatement() {
	consume(TokenType::Keyword, "expect 'while'");
	consume(TokenType::LeftParen, "expect '(' after 'while'");
	auto condition = expression();
	consume(TokenType::RightParen, "expect ')' after if condition");
	auto body = statement();

	return std::make_unique<WhileNode>(std::move(condition), std::move(body));
}

std::unique_ptr<ASTNode> Parser::returnStatement() {
	consume(TokenType::Keyword, "expect 'return'");
	std::unique_ptr<ASTNode> value = nullptr;
	if (!check(TokenType::Semicolon)) {
		value = expression();
	}

	consume(TokenType::Semicolon, "expect ';' after return value");
	return std::make_unique<ReturnNode>(std::move(value));
}

std::unique_ptr<ASTNode> Parser::breakStatement() {
	consume(TokenType::Keyword, "expect 'break'");
	consume(TokenType::Semicolon, "expect ';' after 'break'");
	return std::make_unique<BreakNode>();
}

std::unique_ptr<ASTNode> Parser::continueStatement() {
	consume(TokenType::Keyword, "expect 'continue'");
	consume(TokenType::Semicolon, "expect ';' after 'continue'");
	return std::make_unique<ContinueNode>();
}

std::unique_ptr<ASTNode> Parser::functionCall(const Token& id) {
	std::vector<std::unique_ptr<ASTNode>> args;

	if (!check(TokenType::RightParen)) {
		do {
			args.push_back(expression());
		} while (match(TokenType::Comma));
	}

	consume(TokenType::RightParen, "expect '(' after arguments");
	return std::make_unique<CallNode>(id.value, std::move(args));
}

std::unique_ptr<ASTNode> Parser::block() {
	consume(TokenType::LeftBrace, "expect '{' before block."); // start block

	std::vector<std::unique_ptr<ASTNode>> statements;

	// loop until '}' or EOF
	while (!check(TokenType::RightBrace) && !isAtEnd()) {
		auto stmt = statement();

		if (stmt == nullptr) {
			break;
		}
		statements.push_back(std::move(stmt));
	}

	consume(TokenType::RightBrace, "expect '}' after block"); // end block
	return std::make_unique<BlockNode>(std::move(statements), true); // create scope for explicit blocks
}

std::unique_ptr<ASTNode> Parser::expressionStatement() {
	try {
		auto expr = expression();

		if (expr == nullptr) {
			return nullptr;
		}
		consume(TokenType::Semicolon, "expect ';', after expression");

		return expr;
	}
	catch (const std::exception& e) {
		return nullptr;
	}
}

std::unique_ptr<ASTNode> Parser::expression() {
	auto result = assignment();
	return result;
}

std::unique_ptr<ASTNode> Parser::assignment() {
	auto expr = logicalOr();

	if (check(TokenType::Operator) && peek().value == "=") {
		advance();
		if (expr->getNodeType() == ASTNode::NodeType::Variable) {
			auto varNode = dynamic_cast<VariableNode*>(expr.get());
			auto value = assignment();
			return std::make_unique<AssignmentNode>(varNode->toString(), std::move(value));
		}
		else if (expr->getNodeType() == ASTNode::NodeType::ArrayAccess) {
			auto arrayNode = dynamic_cast<ArrayAccessNode*>(expr.get());
			auto value = assignment();
			return std::make_unique<AssignmentNode>(arrayNode->getName(), std::move(value), std::move(arrayNode->getIndex()));
		}
		throw std::runtime_error("invalid assignment target");
	}
	return expr;
}

std::unique_ptr<ASTNode> Parser::logicalOr() {
	auto expr = logicalAnd();

	while (check(TokenType::Operator) && peek().value == "||") {

		advance();
		Token op = previous();
		auto right = logicalAnd();
		expr = std::make_unique<BinOpNode>(Operator::Or, std::move(expr), std::move(right));
	}
	return expr;
}

std::unique_ptr<ASTNode> Parser::logicalAnd() {
	auto expr = equality();

	while (check(TokenType::Operator) && peek().value == "&&") {

		advance();
		Token op = previous();
		auto right = equality();
		expr = std::make_unique<BinOpNode>(Operator::And, std::move(expr), std::move(right));
	}
	return expr;
}

std::unique_ptr<ASTNode> Parser::equality() {
	auto expr = comparison();

	while (check(TokenType::Operator) &&
		(peek().value == "==" || peek().value == "!=")) {

		advance();
		Token op = previous();
		auto right = comparison();
		expr = std::make_unique<BinOpNode>(
			op.value == "==" ? Operator::Equal : Operator::NotEqual,
			std::move(expr), std::move(right)
		);
	}
	return expr;
}

std::unique_ptr<ASTNode> Parser::comparison() {
	auto expr = term();
	while (check(TokenType::Operator) && (
		peek().value == "<" || peek().value == ">" ||
		peek().value == "<=" || peek().value == ">="))
	{
		advance();
		Token op = previous();
		auto right = term();
		Operator binOp;

		if (op.value == "<") binOp = Operator::Less;
		else if (op.value == ">") binOp = Operator::Greater;
		else if (op.value == "<=") binOp = Operator::LessEqual;
		else binOp = Operator::GreaterEqual;

		expr = std::make_unique<BinOpNode>(binOp, std::move(expr), std::move(right));
	}
	return expr;
}

std::unique_ptr<ASTNode> Parser::term() {
	auto expr = factor();

	while (check(TokenType::Operator)) {
		Token op = peek();

		if (op.value == "+" || op.value == "-") {
			advance();
			auto right = factor();
			expr = std::make_unique<BinOpNode>(
				op.value == "+" ? Operator::Add : Operator::Subtract,
				std::move(expr), std::move(right));
		}
		else {
			break;
		}
	}
	return expr;
}

std::unique_ptr<ASTNode> Parser::factor() {
	auto expr = unary();

	while (peek().type == TokenType::Operator &&
		(peek().value == "*" || peek().value == "/")) {
		advance();
		Token op = previous();
		auto right = primary();
		expr = std::make_unique<BinOpNode>(
			op.value == "*" ? Operator::Multiply : Operator::Divide,
			std::move(expr), std::move(right));
	}
	return expr;
}

std::unique_ptr<ASTNode> Parser::unary() {
	if (check(TokenType::Operator)) {
		Token op = advance();
		std::unique_ptr<ASTNode> right = unary();
		Operator unaryOp;

		if (op.value == "!") unaryOp = Operator::LogicalNot;
		else if (op.value == "-") unaryOp = Operator::Negate;
		else if (op.value == "++") unaryOp = Operator::PreIncrement;
		else if (op.value == "--") unaryOp = Operator::PreDecrement;
		else throw std::runtime_error("invalid unary operator");

		return std::make_unique<UnaryOpNode>(unaryOp, std::move(right));
	}

	// check for post increment or decrement
	std::unique_ptr<ASTNode> expr = primary();
	if (check(TokenType::Operator)) {
		Token op = peek();
		if (op.value == "++" || op.value == "--") {
			advance();
			Operator unaryOp = (op.value == "++") ? Operator::PostIncrement : Operator::PostDecrement;
			return std::make_unique<UnaryOpNode>(unaryOp, std::move(expr));
		}
	}
	// parse the operand if no unary operator
	return expr;
}

std::unique_ptr<ASTNode> Parser::primary() {
	if (match(TokenType::LeftBrace)) {  // array literals must be in braces
		std::vector<std::unique_ptr<ASTNode>> elements;
		Type arrayType;

		if (!check(TokenType::RightBrace)) {
			do {
				if (match(TokenType::Integer)) {
					elements.push_back(std::make_unique<LiteralNode>(std::stoi(previous().value)));
					arrayType = Type::INT;
				}
				else if (match(TokenType::Double)) {
					elements.push_back(std::make_unique<LiteralNode>(std::stod(previous().value)));
					arrayType = Type::DOUBLE;
				}
				else if (match(TokenType::String)) {
					elements.push_back(std::make_unique<LiteralNode>(previous().value));
					arrayType = Type::STRING;
				}
				else if (match(TokenType::Keyword)) {
					if (previous().value == "true") elements.push_back(std::make_unique<LiteralNode>(true));
					if (previous().value == "false") elements.push_back(std::make_unique<LiteralNode>(false));
					arrayType = Type::BOOL;
				}
				else {
					throw std::runtime_error("expected value in array initializer");
				}
			} while (match(TokenType::Comma));
		}

		consume(TokenType::RightBrace, "expect '}' after array elements");
		return std::make_unique<ArrayNode>(std::move(elements), arrayType);
	}

	// handle regular literals
	if (match(TokenType::Integer)) {
		return std::make_unique<LiteralNode>(std::stoi(previous().value));
	}
	if (match(TokenType::Double)) {
		return std::make_unique<LiteralNode>(std::stod(previous().value));
	}
	if (match(TokenType::String)) {
		return std::make_unique<LiteralNode>(previous().value);
	}
	if (match(TokenType::Keyword)) {
		if (previous().value == "true") return std::make_unique<LiteralNode>(true);
		if (previous().value == "false") return std::make_unique<LiteralNode>(false);
	}
	if (match(TokenType::Identifier)) {
		auto id = previous();
		if (match(TokenType::LeftParen)) {
			return functionCall(id);
		}
		if (match(TokenType::LeftBracket)) {
			auto index = expression();
			consume(TokenType::RightBracket, "expect ']' after array access index");
			return std::make_unique<ArrayAccessNode>(id.value, std::move(index));
		}
		return std::make_unique<VariableNode>(id.value);
	}

	if (match(TokenType::LeftParen)) {
		auto expr = expression();
		consume(TokenType::RightParen, "expect ')' after expression");
		return expr;
	}

	throw std::runtime_error("expect expression");
}
void Parser::synchronize() {
	advance();

	while (!isAtEnd()) {
		if (previous().type == TokenType::Semicolon) return;

		switch (peek().type) {
		case TokenType::Keyword:
			if (peek().value == "if" ||
				peek().value == "while" ||
				peek().value == "return" ||
				peek().value == "int" ||
				peek().value == "double" ||
				peek().value == "string") {
				return;
			}
			break;
		default:
			break;
		}

		advance();
	}
}