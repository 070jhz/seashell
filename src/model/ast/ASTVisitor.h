#pragma once

#include "../environment/Value.h"
#include "ASTNode.h"

class ASTVisitor {
public:
    virtual ~ASTVisitor() = default;

    virtual Value visit(BreakNode& node) = 0;
    virtual Value visit(ContinueNode& node) = 0;
    virtual Value visit(LiteralNode& node) = 0;
    virtual Value visit(VariableNode& node) = 0;
    virtual Value visit(ArrayNode& node) = 0;
    virtual Value visit(ArrayAccessNode& node) = 0;
    virtual Value visit(UnaryOpNode& node) = 0;
    virtual Value visit(BinOpNode& node) = 0;
    virtual Value visit(AssignmentNode& node) = 0;
    virtual Value visit(BlockNode& node) = 0;
    virtual Value visit(IfNode& node) = 0;
    virtual Value visit(WhileNode& node) = 0;
    virtual Value visit(ForNode& node) = 0;
    virtual Value visit(FunctionNode& node) = 0;
    virtual Value visit(ReturnNode& node) = 0;
    virtual Value visit(CallNode& node) = 0;

};

