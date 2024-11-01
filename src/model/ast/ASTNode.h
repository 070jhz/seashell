#pragma once

#include <iostream>
#include <vector>
#include <string>
#include <memory>
#include <any>

#include "../environment/Value.h"

class ASTVisitor;

enum class Operator {
	Add,
	Subtract,
	Multiply,
	Divide,
	Equal,
	NotEqual,
	Less,
	LessEqual,
	Greater,
	GreaterEqual,
	And,
	Or,
	Negate,
	LogicalNot,
	PreIncrement,
	PreDecrement,
	PostIncrement,
	PostDecrement
};

class ASTNode {
public:
	enum class NodeType {
		Literal,
		Variable,
        Array,
        ArrayAccess,
		UnaryOp,
		BinaryOp,
		Assignment,
		Function,
		FunctionCall,
		Block,
		If,
		While,
		For,
		Return,
		Break,
		Continue,
        INTERPRETER
	};

    virtual Value accept(ASTVisitor& visitor) = 0;
	virtual std::string toString() = 0;
	virtual NodeType getNodeType() const = 0;
	virtual ~ASTNode() = default;
};


class BreakNode : public ASTNode {
public:
    Value accept(ASTVisitor& visitor) override;


    std::string toString() override {
        return "break";
    }

    NodeType getNodeType() const override {
        return NodeType::Break;
    }
};

class ContinueNode : public ASTNode {
public:
    Value accept(ASTVisitor& visitor) override;

    std::string toString() override {
        return "break";
    }

    NodeType getNodeType() const override {
        return NodeType::Continue;
    }
};

class LiteralNode : public ASTNode {
private:
	Value value;

public:
	explicit LiteralNode(int v) : value(v) {}
	explicit LiteralNode(double v) : value(v) {}
	explicit LiteralNode(bool v) : value(v) {}
	explicit LiteralNode(const std::string& v) : value(v) {}

    Value accept(ASTVisitor& visitor) override;

    Value getValue() {
		return value;
	}

	std::string toString() override { 
		return value.toString(); 
	}
	NodeType getNodeType() const override { 
		return NodeType::Literal; 
	}

};

class VariableNode : public ASTNode {
private:
	std::string name;
public:
	VariableNode(std::string name) : name(name) {};

    Value accept(ASTVisitor& visitor) override;

	std::string toString() override { 
		return name;
	}
	NodeType getNodeType() const override { 
		return NodeType::Variable;
	}

};

class ArrayNode : public ASTNode {
private:
    std::vector<std::unique_ptr<ASTNode>> elements;
    Type elementType;
public:
    ArrayNode(std::vector<std::unique_ptr<ASTNode>> elems, Type elemType)
        : elements(std::move(elems)), elementType(elemType) {}

    Value accept(ASTVisitor& visitor) override;

    std::string toString() override;

    std::vector<std::unique_ptr<ASTNode>>& getElements() {
        return elements;
    }

    size_t getSize() {
        return elements.size();
    }
    Type getElemType() {
        return elementType;
    }
    NodeType getNodeType() const override {
        return NodeType::Array;
    }

};

class ArrayAccessNode : public ASTNode {
private:
    std::string arrayName;
    std::unique_ptr<ASTNode> index;

public:
    ArrayAccessNode(std::string arrayName, std::unique_ptr<ASTNode> index)
        : arrayName(arrayName), index(std::move(index)) {}

    Value accept(ASTVisitor& visitor) override;

    std::string getName() {
        return arrayName;
    }

    std::unique_ptr<ASTNode>& getIndex() {
        return index;
    }

    std::string toString() override {
        return arrayName + "[" + index->toString() + "]";
    }

    NodeType getNodeType() const override {
        return NodeType::ArrayAccess;
    }
};

class UnaryOpNode : public ASTNode {
private:
	Operator op;
	std::unique_ptr<ASTNode> operand;

public:
	UnaryOpNode(Operator op, std::unique_ptr<ASTNode> operand)
		: op(op), operand(std::move(operand)) {}

    Value accept(ASTVisitor& visitor) override;

	std::string toString() override;

	NodeType getNodeType() const override {
		return NodeType::UnaryOp;
	}

	Operator getOperator() {
		return op;
	}

	std::unique_ptr<ASTNode>& getOperand() {
		return operand;
	}
};

class BinOpNode : public ASTNode {
private:
	Operator op;
	std::unique_ptr<ASTNode> left;
	std::unique_ptr<ASTNode> right;
public:
	BinOpNode(Operator op, std::unique_ptr<ASTNode> left, std::unique_ptr<ASTNode> right) 
		: op(op), left(std::move(left)), right(std::move(right)) {};

    Value accept(ASTVisitor& visitor) override;

    std::unique_ptr<ASTNode>& getLeft() { return left; }
	std::unique_ptr<ASTNode>& getRight() { return right; }
	Operator getOperator() { return op; }

	std::string toString() override;
	NodeType getNodeType() const override { return NodeType::BinaryOp; }
};

class AssignmentNode : public ASTNode {
private:
	std::string variableName;
	std::unique_ptr<ASTNode> expression;
	Type declaredType;

public:
	// constructor for variable declaration
	AssignmentNode(std::string varName, Type type, std::unique_ptr<ASTNode> expr)
		: variableName(varName), expression(std::move(expr)), declaredType(type) {};

	// constructor for assignment to existing variable
	AssignmentNode(std::string varName, std::unique_ptr<ASTNode> expr)
		: variableName(varName), expression(std::move(expr)), declaredType(Type::VOID) {}

    Value accept(ASTVisitor& visitor) override;
    static bool isTypeCompatible(Type sourceType, Type targetType);

    std::string toString() override {
		return variableName + " = " + expression->toString();
	}

	NodeType getNodeType() const override { 
		return NodeType::Assignment; 
	}
	
	std::string getVarName() {
		return variableName;
	}

	std::unique_ptr<ASTNode>& getExpression() {
		return expression;
	}
	
	Type getDeclType() {
		return declaredType;
	}
};

class BlockNode : public ASTNode {
private:
    std::vector<std::unique_ptr<ASTNode>> statements;
    bool isScope; // true for real blocks, false for grouping
public:
    BlockNode(std::vector<std::unique_ptr<ASTNode>> stmts, bool createScope = false) 
        : statements(std::move(stmts)), isScope(createScope) {}

    bool shouldCreateScope() const { return isScope; }

	std::vector<std::unique_ptr<ASTNode>>& getStatements() {
		return statements;
	}

    Value accept(ASTVisitor& visitor) override;

	std::string toString() override;

	NodeType getNodeType() const override { return NodeType::Block; }
};

class IfNode : public ASTNode {
private:
	std::unique_ptr<ASTNode> condition;
	std::unique_ptr<ASTNode> thenBranch;
	std::unique_ptr<ASTNode> elseBranch;
public:
	IfNode(std::unique_ptr<ASTNode> condition,
		   std::unique_ptr<ASTNode> thenBranch,
		   std::unique_ptr<ASTNode> elseBranch)
		: condition(std::move(condition)),
		  thenBranch(std::move(thenBranch)),
		  elseBranch(std::move(elseBranch)) {}

    Value accept(ASTVisitor& visitor) override;

	std::string toString() override;
	NodeType getNodeType() const override {
		return NodeType::If;
	}

	std::unique_ptr<ASTNode>& getCondition() {
		return condition;
	}

	std::unique_ptr<ASTNode>& getThenBranch() {
		return thenBranch;
	}

	std::unique_ptr<ASTNode>& getElseBranch() {
		return elseBranch;
	}
};

class WhileNode : public ASTNode {
private:
	std::unique_ptr<ASTNode> condition;
	std::unique_ptr<ASTNode> body;
public:
	WhileNode(std::unique_ptr<ASTNode> condition, std::unique_ptr<ASTNode> body)
		: condition(std::move(condition)), body(std::move(body)) {}

    Value accept(ASTVisitor& visitor) override;
	std::string toString() override;
	
	std::unique_ptr<ASTNode>& getCondition() {
		return condition;
	}

	std::unique_ptr<ASTNode>& getBody() {
		return body;
	}

	NodeType getNodeType() const override { 
		return NodeType::While; 
	}
};

class ForNode : public ASTNode {
private:
	std::unique_ptr<ASTNode> initialization;
	std::unique_ptr<ASTNode> condition;
	std::unique_ptr<ASTNode> increment;
	std::unique_ptr<ASTNode> body;

public:
	ForNode(std::unique_ptr<ASTNode> init,
		std::unique_ptr<ASTNode> condition,
		std::unique_ptr<ASTNode> increment,
		std::unique_ptr<ASTNode> body)
		: initialization(std::move(init)),
		condition(std::move(condition)),
		increment(std::move(increment)),
		body(std::move(body)) {};

    Value accept(ASTVisitor& visitor) override;
	std::string toString() override;

	std::unique_ptr<ASTNode>& getInitialization() {
		return initialization;
	}

	std::unique_ptr<ASTNode>& getCondition() {
		return condition;
	}

	std::unique_ptr<ASTNode>& getIncrement() {
		return increment;
	}

	std::unique_ptr<ASTNode>& getBody() {
		return body;
	}

	NodeType getNodeType() const override {
		return NodeType::For;
	}
};

class FunctionNode : public ASTNode {
private:
    std::string name;
    std::vector<std::pair<std::string, Type>> parameters;
    Type returnType;
    std::unique_ptr<ASTNode> body;
public:
    FunctionNode(const std::string& name,
        std::vector<std::pair<std::string, Type>> parameters,
        Type returnType,
        std::unique_ptr<ASTNode> body)
        : name(name), parameters(std::move(parameters)), returnType(returnType), body(std::move(body)) {}

    Value accept(ASTVisitor& visitor) override;

    std::string toString() override;

    const std::string& getName() const { return name; }
    Type getReturnType() const { return returnType; }
    const std::vector<std::pair<std::string, Type>>& getParameters() const { return parameters; }
	
    std::unique_ptr<ASTNode>& getBody() { return body; }

    NodeType getNodeType() const override { 
        return NodeType::Function; 
    }
};

class ReturnNode : public ASTNode {
private:
	std::unique_ptr<ASTNode> expression;
public:
	ReturnNode(std::unique_ptr<ASTNode> expr) : expression(std::move(expr)) {}

    Value accept(ASTVisitor& visitor) override;
	std::unique_ptr<ASTNode>& getExpression() { return expression; }

	std::string toString() override {
		return "return" + (expression ? expression->toString() : "");
	}


	NodeType getNodeType() const override {
		return NodeType::Return;
	}
};

class CallNode : public ASTNode {
private:
	std::string name;
	std::vector<std::unique_ptr<ASTNode>> arguments;
public:
	CallNode(std::string name, std::vector<std::unique_ptr<ASTNode>> args)
		: name(name), arguments(std::move(args)) {}

    Value accept(ASTVisitor& visitor) override;

	std::string toString() override;

	std::string getFuncName() {
		return name;
	}

	std::vector<std::unique_ptr<ASTNode>>& getArguments() {
		return arguments;
	}

	NodeType getNodeType() const override {
		return NodeType::FunctionCall;
	}
};