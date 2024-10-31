#include "Interpreter.h"

Value Interpreter::visit(BreakNode& node) {
    throw std::runtime_error("break encountered");
}

Value Interpreter::visit(ContinueNode& node) {
    throw std::runtime_error("continue encountered");
}

Value Interpreter::visit(LiteralNode& node) {
    return node.getValue();
}

Value Interpreter::visit(VariableNode& node) {
    return env.getVariable(node.toString()).value;
}

Value Interpreter::visit(ArrayNode& node) {
    std::vector<Value> evaluatedElements;
    for (auto& element : node.getElements()) {
        evaluatedElements.push_back(evaluate(*element));
    }
    return {evaluatedElements};
}

Value Interpreter::visit(ArrayAccessNode& node) {
    auto var = env.getVariable(node.getName());
    if (var.type == Type::ARRAY) {
        auto array = var.value.get<std::vector<Value>>();
        auto id = evaluate(*node.getIndex());
        if (id.getType() != Type::INT) {
            throw std::runtime_error("index must be integer");
        } else {
            return array[id.get<int>()];
        }
    }
    throw std::runtime_error("expected array variable");
}

Value Interpreter::visit(UnaryOpNode& node) {
    Operator op = node.getOperator();
    auto& operand = node.getOperand();
    Value val = evaluate(*operand);

    switch (op) {
        case Operator::Negate:
            if (val.getType() == Type::INT) {
                return {-val.get<int>()};
            }
            else if (val.getType() == Type::DOUBLE) {
                return {-val.get<double>()};
            }
            throw std::runtime_error("invalid operand type for unary '-'");

        case Operator::LogicalNot:
            if (val.getType() == Type::BOOL) {
                return {!val.get<bool>()};
            }
            throw std::runtime_error("invalid operand type for unary '!'");

        case Operator::PreIncrement:
        case Operator::PostIncrement:
            if (operand->getNodeType() != NodeType::Variable) {
                throw std::runtime_error("increment requires a variable reference");
            }
            if (val.getType() == Type::INT) {
                auto& varNode = dynamic_cast<VariableNode&>(*operand);
                auto& var = env.getVariable(varNode.toString());
                var.value = {val.get<int>() + 1};
                return op == Operator::PreIncrement ? Value{val.get<int>() + 1} : val;
            }
            else if (val.getType() == Type::DOUBLE) {
                auto& varNode = dynamic_cast<VariableNode&>(*operand);
                auto& var = env.getVariable(varNode.toString());
                var.value = {val.get<double>() + 1.0};
                return op == Operator::PreIncrement ? Value{val.get<double>() + 1.0} : val;
            }
            throw std::runtime_error("invalid type for increment operator");
        case Operator::PreDecrement:
        case Operator::PostDecrement:
            if (operand->getNodeType() != NodeType::Variable) {
                throw std::runtime_error("decrement requires a variable reference");
            }
            if (val.getType() == Type::INT) {
                auto& varNode = dynamic_cast<VariableNode&>(*operand);
                auto& var = env.getVariable(varNode.toString());
                var.value = {val.get<int>() - 1};
                return op == Operator::PreDecrement ? Value{val.get<int>() - 1} : val;
            }
            else if (val.getType() == Type::DOUBLE) {
                auto& varNode = dynamic_cast<VariableNode&>(*operand);
                auto& var = env.getVariable(varNode.toString());
                var.value = {val.get<double>() - 1.0};
                return op == Operator::PreDecrement ? Value{val.get<double>() - 1.0} : val;
            }
            throw std::runtime_error("invalid type for increment operator");
        default:
            throw std::runtime_error("unknown unary operator");
    }
}

template<typename T>
Value performOperation(T left, T right, Operator op) {
    bool leftBool = static_cast<bool>(left);
    bool rightBool = static_cast<bool>(right);

    switch (op) {
        case Operator::Add :
            return Value(left + right);
        case Operator::Subtract:
            return Value(left - right);
        case Operator::Multiply:
            return Value(left * right);
        case Operator::Divide:
            if (right == 0) { throw std::runtime_error("can't divide by zero"); }
            return Value(left / right);
        case Operator::Equal:
            return Value(left == right);
        case Operator::NotEqual:
            return Value(left != right);
        case Operator::Less:
            return Value(left < right);
        case Operator::LessEqual:
            return Value(left <= right);
        case Operator::Greater:
            return Value(left > right);
        case Operator::GreaterEqual:
            return Value(left >= right);
        case Operator::And:
            return {leftBool && rightBool};
        case Operator::Or:
            return {leftBool || rightBool};
        default:
            throw std::runtime_error("Unknown operator");
    }
}

// handles number type difference
Value Interpreter::visit(BinOpNode& node) {
    Value left = evaluate(*node.getLeft());
    Value right = evaluate(*node.getRight());
    Operator op = node.getOperator();

    if (left.getType() == Type::INT && right.getType() == Type::INT) {
        return performOperation(std::any_cast<int>(left), std::any_cast<int>(right), op);
    }

    if (left.getType() == Type::DOUBLE && right.getType() == Type::DOUBLE) {
        return performOperation(std::any_cast<double>(left), std::any_cast<double>(right), op);
    }

    double leftDouble = (left.getType() == Type::DOUBLE) ?
            std::any_cast<double>(left) : static_cast<double>(std::any_cast<int>(left));

    double rightDouble = (right.getType() == Type::DOUBLE) ?
            std::any_cast<double>(right) : static_cast<double>(std::any_cast<int>(right));

    return performOperation(leftDouble, rightDouble, op);
}

std::string typeToString(Type type) {
    switch (type) {
        case Type::INT: return "int";
        case Type::DOUBLE: return "double";
        case Type::BOOL: return "bool";
        case Type::STRING: return "string";
        case Type::ARRAY: return "array";
        case Type::VOID: return "void";
        case Type::FUNCTION: return "function";
        default: return "unknown";
    }
}

Value Interpreter::visit(AssignmentNode& node) {
    Type declType = node.getDeclType();
    std::string varName = node.getVarName();
    Value exprVal = evaluate(*node.getExpression());
    if (declType != Type::VOID) {
        // variable declaration
        if (!node.isTypeCompatible(exprVal.getType(), declType)) {
            throw std::runtime_error("type mismatch in variable declaration. expected " +
                                    typeToString(declType) + ", got " + typeToString(exprVal.getType()));
        }
        env.declareVariable(varName, declType, exprVal);
    }
    else {
        // assignment to existing variable
        if (!env.hasVariable(varName)) {
            throw std::runtime_error("undefined variable: " + varName);
        }
        Variable& existingVar = env.getVariable(varName);
        if (!node.isTypeCompatible(exprVal.getType(), existingVar.type)) {
            throw std::runtime_error("type mismatch in assignment. cannot assign"
                                     + typeToString(exprVal.getType()) + "to variable of type " + typeToString(existingVar.type));
        }
        existingVar.value = exprVal;
    }

    return exprVal;
}

Value Interpreter::visit(BlockNode& node) {
    env.pushScope();
    Value lastVal;
    auto& stmts = node.getStatements();
    try {
        for (const auto& stmt : stmts) {
            lastVal = evaluate(*stmt);
            if (stmt->getNodeType() == NodeType::Return ||
                stmt->getNodeType() == NodeType::Break ||
                stmt->getNodeType() == NodeType::Continue) {
                break;
            }
        }
    }
    catch (...) {
        env.popScope();
        throw;
    }
    env.popScope();
    return lastVal;
}

Value Interpreter::visit(IfNode& node) {
    if (evaluate(*node.getCondition()).toBool()) {
        return evaluate(*node.getThenBranch());
    }
    else if (auto& elseBranch = node.getElseBranch()) {
        return evaluate(*elseBranch);
    }
    else {
        throw;
    }
}

Value Interpreter::visit(WhileNode& node) {
    Value lastVal;
    auto& condition = node.getCondition();
    auto& body = node.getBody();
    while (evaluate(*condition).toBool()) {
        try {
            lastVal = evaluate(*body);
        } catch (const std::runtime_error& e) {
            if (std::string(e.what()) == "break encountered") {
                break;
            } else if (std::string(e.what()) == "continue encountered") {
                continue;
            } else {
                throw;
            }
        }
    }
    return lastVal;
}

Value Interpreter::visit(ForNode& node) {
    try {
        if (auto& init = node.getInitialization()) {
            evaluate(*init);
        }
        while (true) {
            if (auto& condition = node.getCondition()) {
                Value condVal = evaluate(*condition);
                if (condVal.getType() != Type::BOOL) {
                    throw std::runtime_error("for loop condition must be boolean");
                }
                if (!condVal.get<bool>()) {
                    break;
                }
            }

            try {
                auto& body = node.getBody();
                Value bodyVal = evaluate(*body);

                // if return statement, propagate the result up the call stack
                if (body->getNodeType() == NodeType::Return) {
                    env.popScope();
                    return bodyVal;
                }
            } catch (const std::runtime_error& e) {
                if (std::string(e.what()) == "break encountered") {
                    break;
                }
                else if (std::string(e.what()) == "continue encountered") {
                    if (node.getIncrement()) {
                        evaluate(*node.getIncrement());
                    }
                    continue;
                }
                else {
                    throw;
                }
            }
            if (node.getIncrement()) {
                evaluate(*node.getIncrement());
            }
        }

        env.popScope();
        return {};
    }
    catch (...) {
        env.popScope();
        throw;
    }
}

Value Interpreter::visit(FunctionNode& node) {
    env.
}