#ifndef SEASHELLS_INTERPRETER_H
#define SEASHELLS_INTERPRETER_H

#include "ASTVisitor.h"

class Interpreter : public ASTVisitor {
private:
    Environment& env;

public:
    explicit Interpreter(Environment& env) : env(env) {}

    Value evaluate(ASTNode& node) override {
        return node.accept(*this);
    }

    std::string toString() override {
        return "Interpreter";
    }

    NodeType getNodeType() const override {
        return NodeType::INTERPRETER;
    }

    Value visit(BreakNode& node);
    Value visit(ContinueNode& node);
    Value visit(LiteralNode& node);
    Value visit(VariableNode& node);
    Value visit(ArrayNode& node);
    Value visit(ArrayAccessNode& node);
    Value visit(UnaryOpNode& node);
    Value visit(BinOpNode& node);
    Value visit(AssignmentNode& node);
    Value visit(BlockNode& node);
    Value visit(IfNode& node);
    Value visit(WhileNode& node);
    Value visit(ForNode& node);
    Value visit(FunctionNode& node);
    Value visit(ReturnNode& node);
    Value visit(CallNode& node);
};


#endif //SEASHELLS_INTERPRETER_H
