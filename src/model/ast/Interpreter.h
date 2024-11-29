#ifndef SEASHELLS_INTERPRETER_H
#define SEASHELLS_INTERPRETER_H

#include "ASTVisitor.h"
#include "../environment/Environment.h"

class Interpreter : public ASTVisitor {
private:
    Environment& env;
    Value lastResult;

    class ReturnException : public std::exception {
    public:
        Value value;
        explicit ReturnException(const Value& val) : value(val) {}
        explicit ReturnException(Value&& val) : value(std::move(val)) {}
    };

public:
    explicit Interpreter(Environment& env) : env(env) {}

    Value evaluate(ASTNode& node) {
        lastResult = node.accept(*this);
        return lastResult;
    }

    std::string getLastResult() const {
        return lastResult.toString();
    }

    Environment& getEnvironment() {
        return env;
    }

    Value visit(BreakNode& node) override;
    Value visit(ContinueNode& node) override;
    Value visit(LiteralNode& node) override;
    Value visit(VariableNode& node) override;
    Value visit(ArrayNode& node) override;
    Value visit(ArrayAccessNode& node) override;
    Value visit(UnaryOpNode& node) override;
    Value visit(BinOpNode& node) override;
    Value visit(AssignmentNode& node) override;
    Value visit(BlockNode& node) override;
    Value visit(IfNode& node) override;
    Value visit(WhileNode& node) override;
    Value visit(ForNode& node) override;
    Value visit(FunctionNode& node) override;
    Value visit(ReturnNode& node) override;
    Value visit(CallNode& node) override;
};

#endif //SEASHELLS_INTERPRETER_H
