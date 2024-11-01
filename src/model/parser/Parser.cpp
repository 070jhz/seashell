#include "Parser.h"
#include "Lexer.h"
#include <iostream>

std::unordered_set<std::string> reservedKeywords = {
        "if", "else", "while", "return", "for", "true", "false", "break", "continue"
};
std::unique_ptr<ASTNode> Parser::parse(const std::string& input) {
	Lexer lexer(input);
	tokens = lexer.tokenize();

	std::cout << "Tokens: " << std::endl;
	for (const auto& token : tokens) {
		std::cout << "Type : " << static_cast<int>(token.type)
			<< " Value : " << token.value << std::endl;
	}
	current = 0;
	return program();
}

std::unique_ptr<ASTNode> Parser::program() {
	std::cout << "entering program()" << std::endl;
	std::vector<std::unique_ptr<ASTNode>> statements;
	// parse until EOF or all declarations consumed
	while (!isAtEnd()) {
		std::unique_ptr<ASTNode> decl = declaration();

		if (decl == nullptr) {
			std::cout << "no more valid declarations, breaking out of program" << std::endl;
			break;
		}

		statements.push_back(std::move(decl));
	}
	std::cout << "exiting program() " << std::endl;
	return std::make_unique<BlockNode>(std::move(statements), false); // don't create scope for global block
}

std::unique_ptr<ASTNode> Parser::declaration() {
	std::cout << "entering declaration()" << std::endl;
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
		std::cout << "exiting declaration()" << std::endl;

		return statement();
	}
	catch (const std::runtime_error& error) {
		std::cout << "Error in declaration: " << error.what() << std::endl;
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
            std::cout << "entering variableDeclaration() for array" << std::endl;
            Type arrayType = type;
            type = Type::ARRAY;
            std::unique_ptr<ASTNode> size = nullptr;

            int specifiedSize = 5; // default size

            if (!check(TokenType::RightBracket)) {
                std::cout << "checking for size of array" << std::endl;
                size = expression();
                specifiedSize = std::stoi(size->toString());
            }
            consume(TokenType::RightBracket, "expect ']' after array size if any");

            if (match(TokenType::Operator) && previous().value == "=") {
                initializer = expression();
                auto *arrayNode = dynamic_cast<ArrayNode *>(initializer.get());

                // check if the initializer is an ArrayNode and validate its size
                if (arrayNode->getSize() > specifiedSize) {
                    throw std::runtime_error("Array initializer size " +
                                             std::to_string(arrayNode->getSize()) +
                                             " exceeds specified size " +
                                             std::to_string(specifiedSize));
                }
            } else {
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

        } else {
            std::cout << "entering variableDeclaration() for variable" << std::endl;

            if (match(TokenType::Operator) && previous().value == "=") {
                initializer = expression();
            } else {
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

    } while (match(TokenType::Comma) && (name=consume(TokenType::Identifier, "expect additional variable name after ','"), true));

    consume(TokenType::Semicolon, "expect ';' after variable declaration");
    if (declarations.size() == 1) {
        return std::move(declarations[0]);
    } else {
        return std::make_unique<BlockNode>(std::move(declarations), false); // grouping only
    }
}

std::unique_ptr<ASTNode> Parser::functionDeclaration(Type returnType, Token name) {
	std::cout << "entering functionDeclaration()" << std::endl;
	// we already consumed return type and function name

	consume(TokenType::LeftParen, "expect '(' after function name");
	std::vector<std::pair<std::string, Type>> parameters;

	if (!check(TokenType::RightParen)) {
		do {
			Token paramType = consume(TokenType::Keyword, "expect parameter type");
			Token paramName = consume(TokenType::Identifier, "expect parameter name");
			parameters.emplace_back(paramName.value, tokenToType(paramType));
		} while (match(TokenType::Comma));

		consume(TokenType::RightParen, "expect ')' after parameters");

		auto body = block(); // pass false for function body
		return std::make_unique<FunctionNode>(name.value, parameters, returnType, std::move(body));
	}

	std::cout << "exiting functionDeclaration()" << std::endl;
	return std::make_unique<FunctionNode>(name.value, parameters, returnType, nullptr);
}

std::unique_ptr<ASTNode> Parser::statement() {
	std::cout << "entering statement()" << std::endl;
		
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
	std::cout << "Parsing if statement" << std::endl;
	consume(TokenType::Keyword, "expect 'if'");
	consume(TokenType::LeftParen, "expect '(' after 'if'");
	auto condition = expression(); 	// parse condition
	consume(TokenType::RightParen, "expect ')' after if condition");

	std::cout << "Parsed condition, current token: " << peek().toString() << std::endl;


	auto thenBranch = statement();

    std::unique_ptr<ASTNode> elseBranch = nullptr;

	if (check(TokenType::Keyword) && peek().value == "else") {
		std::cout << "Parsing else branch" << std::endl;
        advance();
		elseBranch = statement();
	}
	std::cout << "Finished parsing if statement" << std::endl;
    std::cout << peek().value + " in ifStmt" << std::endl;

	return std::make_unique<IfNode>(std::move(condition), std::move(thenBranch), std::move(elseBranch));
}

std::unique_ptr<ASTNode> Parser::forStatement() {
	std::cout << "entering forStatement() " << std::endl;
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

	// Condition clause
	std::unique_ptr<ASTNode> condition = nullptr;
	if (!check(TokenType::Semicolon)) {
		condition = expression();
	}
	consume(TokenType::Semicolon, "expect ';' after loop condition");

	// Increment clause
	std::unique_ptr<ASTNode> increment = nullptr;
	if (!check(TokenType::RightParen)) {
		increment = expression();
	}
	consume(TokenType::RightParen, "expect ')' after for clauses");

	// Body
	auto body = statement();

	std::cout << "exiting forStatement()" << std::endl;
	return std::make_unique<ForNode>(
		std::move(init),
		std::move(condition),
		std::move(increment),
		std::move(body)
	);
}

std::unique_ptr<ASTNode> Parser::whileStatement() {
	std::cout << "entering whileStatement()" << std::endl;
	consume(TokenType::Keyword, "expect 'while'");

	consume(TokenType::LeftParen, "expect '(' after 'while'");
	auto condition = expression();
	consume(TokenType::RightParen, "expect ')' after if condition");
	auto body = statement();

	std::cout << "exiting whileStatement()" << std::endl;
	return std::make_unique<WhileNode>(std::move(condition), std::move(body));
}

std::unique_ptr<ASTNode> Parser::returnStatement() {
	std::cout << "parsing return statement" << std::endl;
	consume(TokenType::Keyword, "expect 'return'");
	std::unique_ptr<ASTNode> value = nullptr;
	if (!check(TokenType::Semicolon)) {
		std::cout << "parsing return value" << std::endl;
		value = expression();
	}

    std::cout << "consuming semicolon after return value" << std::endl;
	consume(TokenType::Semicolon, "expect ';' after return value");
	std::cout << "returning from returnStatement()" << std::endl;
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
    return std::make_unique<ContinueNode>();}

std::unique_ptr<ASTNode> Parser::functionCall(const Token& id) {
	std::vector<std::unique_ptr<ASTNode>> args;

	if (!check(TokenType::RightParen)) {
		do {
			args.push_back(expression());
		} while (match(TokenType::Comma));
	}

	consume(TokenType::RightParen, "expect '(' after arguments");
	std::cout << "finished parsing function call, exiting primary()" << std::endl;
	return std::make_unique<CallNode>(id.value, std::move(args));
}

std::unique_ptr<ASTNode> Parser::block() {
    std::cout << "entering block" << std::endl;
    consume(TokenType::LeftBrace, "expect '{' before block."); // start block

    std::vector<std::unique_ptr<ASTNode>> statements;

    // loop until '}' or EOF
    while (!check(TokenType::RightBrace) && !isAtEnd()) {
        auto stmt = statement();

        if (stmt == nullptr) {
            std::cout << "no more valid statements, breaking out of block" << std::endl;
            break;
        }
        statements.push_back(std::move(stmt));
    }

    consume(TokenType::RightBrace, "expect '}' after block"); // end block
    std::cout << "exiting block" << std::endl;
    return std::make_unique<BlockNode>(std::move(statements), true); // create scope for explicit blocks
}

std::unique_ptr<ASTNode> Parser::expressionStatement() {
	std::cout << "entering expressionStatement()" << std::endl;

	try {
		auto expr = expression();

		if (expr == nullptr) {
			std::cout << "expression is nullptr, exiting" << std::endl;
			return nullptr;
		}

		std::cout << "attempting to consume token type: " << static_cast<int>(TokenType::Semicolon) << std::endl;
		std::cout << "current token type: " << static_cast<int>(peek().type) << std::endl;
		consume(TokenType::Semicolon, "expect ';', after expression");
		std::cout << "exiting expressionStatement()" << std::endl;

		return expr;
	}
	catch (const std::exception& e) {
		std::cout << "exception in expressionStatement(): " << e.what() << std::endl;
		return nullptr;
	}
}

std::unique_ptr<ASTNode> Parser::expression() {
	std::cout << "entering expression()" << std::endl;
	auto result = assignment();
	if (result == nullptr) {
		std::cout << "assignment returned nullptr!" << std::endl;
	}
	std::cout << "exiting expression()" << std::endl;
	return result;
}

std::unique_ptr<ASTNode> Parser::assignment() {
	std::cout << "entering assignment()" << std::endl;
	std::cout << "current token: " << peek().toString() << std::endl;
	auto expr = logicalOr();

	if (check(TokenType::Operator) && peek().value == "=") {
		std::cout << "past assignment() check" << std::endl;
		advance();
		if (expr->getNodeType() == ASTNode::NodeType::Variable) {
			std::cout << "constructing assignment node" << std::endl;
			auto varNode = dynamic_cast<VariableNode*>(expr.get());
			auto value = assignment();
			return std::make_unique<AssignmentNode>(varNode->toString(), std::move(value));
		}
		throw std::runtime_error("invalid assignment target");
	}
	std::cout << "exiting assignment() " << std::endl;
	return expr;
}

std::unique_ptr<ASTNode> Parser::logicalOr() {
	std::cout << "entering logicalOr()" << std::endl;
	auto expr = logicalAnd();

	while (check(TokenType::Operator) && peek().value == "||") {
		std::cout << "past logicalOr() check" << std::endl;

		advance();
		Token op = previous();
		auto right = logicalAnd();
		expr = std::make_unique<BinOpNode>(Operator::Or, std::move(expr), std::move(right));
	}
	std::cout << "exiting logicalOr()" << std::endl;
	return expr;
}

std::unique_ptr<ASTNode> Parser::logicalAnd() {
	std::cout << "entering logicalAnd()" << std::endl;
	auto expr = equality();

	while (check(TokenType::Operator) && peek().value == "&&") {
		std::cout << "past logicalAnd() check" << std::endl;

		advance();
		Token op = previous();
		auto right = equality();
		expr = std::make_unique<BinOpNode>(Operator::And, std::move(expr), std::move(right));
	}
	std::cout << "exiting logicalAnd()" << std::endl;
	return expr;
}

std::unique_ptr<ASTNode> Parser::equality() {
	std::cout << "entering equality()" << std::endl;
	auto expr = comparison();

	while (check(TokenType::Operator) &&
		(peek().value == "==" || peek().value == "!=")) {
		std::cout << "past equality() check" << std::endl;

		advance();
		Token op = previous();
		auto right = comparison();
		expr = std::make_unique<BinOpNode>(
			op.value == "==" ? Operator::Equal : Operator::NotEqual,
			std::move(expr), std::move(right)
		);
	}
	std::cout << "exiting equality()" << std::endl;
	return expr;
}

std::unique_ptr<ASTNode> Parser::comparison() {
	std::cout << "entering comparison()" << std::endl;
	auto expr = term();
	while (check(TokenType::Operator) && (
		peek().value == "<" || peek().value == ">" ||
		peek().value == "<=" || peek().value == ">="))
	{
		std::cout << "past comparison() check" << std::endl;
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
	std::cout << "exiting comparison()" << std::endl;
	return expr;
}

std::unique_ptr<ASTNode> Parser::term() {
	std::cout << "entering term()" << std::endl;
	std::cout << "current token: " << peek().toString() << std::endl;

	auto expr = factor();

	std::cout << "after factor(), current token: " << peek().toString() << std::endl;

	while (check(TokenType::Operator)) {
		Token op = peek();
		std::cout << "found operator in term(): " << op.value << std::endl;

		if (op.value == "+" || op.value == "-") {
			advance();
			std::cout << "processing + or - operator" << std::endl;
			std::cout << "before right factor, current token: " << peek().toString() << std::endl;

			auto right = factor();
			std::cout << "after right factor, current token: " << peek().toString() << std::endl;

			expr = std::make_unique<BinOpNode>(
				op.value == "+" ? Operator::Add : Operator::Subtract,
				std::move(expr), std::move(right));
		}
		else {
			break;
		}
	}
	std::cout << "exiting term(), current token : " << peek().toString() << std::endl;
	return expr;
}

std::unique_ptr<ASTNode> Parser::factor() {
	std::cout << "entering factor()" << std::endl;
	std::cout << "current token: " << peek().toString() << std::endl;
	auto expr = unary();
	std::cout << "after unary(), current token: " << peek().toString() << std::endl;

	while (peek().type == TokenType::Operator &&
		(peek().value == "*" || peek().value == "/")) {
		advance();
		Token op = previous();
		std::cout << "found operator in factor(): " << op.value << std::endl;
		auto right = primary();
		std::cout << "after right primary(), current token: " << peek().toString() << std::endl;
		expr = std::make_unique<BinOpNode>(
			op.value == "*" ? Operator::Multiply : Operator::Divide,
			std::move(expr), std::move(right));
	}
	std::cout << "exiting factor(), current token : " << peek().toString() << std::endl;
	return expr;
}

std::unique_ptr<ASTNode> Parser::unary() {
	std::cout << "entering unary()" << std::endl;
	std::cout << "current token : " << peek().toString() << std::endl;
	if (check(TokenType::Operator)) {
		Token op = advance();
		std::unique_ptr<ASTNode> right = unary();
		Operator unaryOp;

		if (op.value == "!") unaryOp = Operator::LogicalNot;
		else if (op.value == "-") unaryOp = Operator::Negate;
		else if (op.value == "++") unaryOp = Operator::PreIncrement;
		else if (op.value == "--") unaryOp = Operator::PreDecrement;
		else throw std::runtime_error("invalid unary operator");

		std::cout << "exiting unary() with pre unary operator" << std::endl;
		return std::make_unique<UnaryOpNode>(unaryOp, std::move(right));
	}

	// check for post increment or decrement
	std::unique_ptr<ASTNode> expr = primary();
	if (check(TokenType::Operator)) {
		Token op = peek();
		if (op.value == "++" || op.value == "--") {
			advance();
			Operator unaryOp = (op.value == "++") ? Operator::PostIncrement : Operator::PostDecrement;
			std::cout << "exiting unary() with post unary operator" << std::endl;
			return std::make_unique<UnaryOpNode>(unaryOp, std::move(expr));
		}
	}
	std::cout << "exiting unary() without unary operator" << std::endl;
	// parse the operand if no unary operator
	return expr;
}

std::unique_ptr<ASTNode> Parser::primary() {
    std::cout << "entering primary()" << std::endl;

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
        std::cout << "matched array literal, exiting primary()" << std::endl;
        return std::make_unique<ArrayNode>(std::move(elements), arrayType);
    }

    // handle regular literals
    if (match(TokenType::Integer)) {
        std::cout << "matched integer, exiting primary()" << std::endl;
        return std::make_unique<LiteralNode>(std::stoi(previous().value));
    }
    if (match(TokenType::Double)) {
        std::cout << "matched double, exiting primary()" << std::endl;
        return std::make_unique<LiteralNode>(std::stod(previous().value));
    }
    if (match(TokenType::String)) {
        std::cout << "matched string, exiting primary()" << std::endl;
        return std::make_unique<LiteralNode>(previous().value);
    }
    if (match(TokenType::Keyword)) {
        std::cout << "matched keyword, exiting primary()" << std::endl;
        if (previous().value == "true") return std::make_unique<LiteralNode>(true);
        if (previous().value == "false") return std::make_unique<LiteralNode>(false);
    }
    if (match(TokenType::Identifier)) {
        auto id = previous();
        if (match(TokenType::LeftParen)) {
            std::cout << "parsing function call" << std::endl;
            return functionCall(id);
        }
        if (match(TokenType::LeftBracket)) {
            std::cout << "parsing array access" << std::endl;
            auto index = expression();
            consume(TokenType::RightBracket, "expect ']' after array access index");
            return std::make_unique<ArrayAccessNode>(id.value, std::move(index));
        }
        std::cout << "matched identifier, exiting primary()" << std::endl;
        return std::make_unique<VariableNode>(id.value);
    }

    if (match(TokenType::LeftParen)) {
        auto expr = expression();
        consume(TokenType::RightParen, "expect ')' after expression");
        std::cout << "matched leftParen, exiting primary()" << std::endl;
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