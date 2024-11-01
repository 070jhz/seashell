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
            if (operand->getNodeType() != ASTNode::ASTNode::NodeType::Variable) {
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
            if (operand->getNodeType() != ASTNode::ASTNode::NodeType::Variable) {
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
        return performOperation(left.get<int>(), right.get<int>(), op);
    }

    if (left.getType() == Type::DOUBLE && right.getType() == Type::DOUBLE) {
        return performOperation(left.get<double>(), right.get<double>(), op);
    }

    double leftDouble = (left.getType() == Type::DOUBLE) ?
            left.get<double>() : static_cast<double>(left.get<int>());

    double rightDouble = (right.getType() == Type::DOUBLE) ?
            right.get<double>() : static_cast<double>(right.get<int>());

    return performOperation(leftDouble, rightDouble, op);
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
    std::cout << "Entering block" << std::endl;
    
    if (node.shouldCreateScope()) {
        env.pushScope();
    }
    
    Value lastVal;
    try {
        for (const auto& stmt : node.getStatements()) {
            std::cout << "Executing statement of type: " << static_cast<int>(stmt->getNodeType()) << std::endl;
            lastVal = evaluate(*stmt);
            
            // If this is a return statement, we need to clean up and propagate
            if (stmt->getNodeType() == ASTNode::NodeType::Return) {
                if (node.shouldCreateScope()) {
                    env.popScope();
                }
                std::cout << "Return statement found in block" << std::endl;
                return lastVal;
            }
        }
    } catch (const ReturnException& e) {
        std::cout << "Caught return exception in block: " << e.value.toString() << std::endl;
        if (node.shouldCreateScope()) {
            env.popScope();
        }
        throw;
    }
    
    if (node.shouldCreateScope()) {
        env.popScope();
    }
    
    std::cout << "Block completed normally with value: " << lastVal.toString() << std::endl;
    return lastVal;
}

Value Interpreter::visit(IfNode& node) {
    std::cout << "Evaluating if condition" << std::endl;
    Value condValue = evaluate(*node.getCondition());
    std::cout << "Condition evaluated to: " << condValue.toString() << std::endl;
    
    if (condValue.toBool()) {
        std::cout << "Taking then branch" << std::endl;
        try {
            Value result = evaluate(*node.getThenBranch());
            std::cout << "Then branch returned: " << result.toString() << std::endl;
            return result;
        } catch (const ReturnException& e) {
            std::cout << "Caught return in then branch: " << e.value.toString() << std::endl;
            throw; // Re-throw the return
        }
    } else if (auto& elseBranch = node.getElseBranch()) {
        std::cout << "Taking else branch" << std::endl;
        try {
            Value result = evaluate(*elseBranch);
            std::cout << "Else branch returned: " << result.toString() << std::endl;
            return result;
        } catch (const ReturnException& e) {
            std::cout << "Caught return in else branch: " << e.value.toString() << std::endl;
            throw; // Re-throw the return
        }
    }
    
    return Value(); // Default return for if without else
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
    Value lastVal;
    env.pushScope();
    
    try {
        // Initialization
        if (auto& init = node.getInitialization()) {
            evaluate(*init);
        }

        // Loop
        while (true) {
            // Condition check
            if (auto& condition = node.getCondition()) {
                Value condVal = evaluate(*condition);
                if (condVal.getType() != Type::BOOL) {
                    throw std::runtime_error("for loop condition must be boolean");
                }
                if (!condVal.get<bool>()) {
                    break;
                }
            }

            // Body execution
            try {
                lastVal = evaluate(*node.getBody());
            } catch (const std::runtime_error& e) {
                if (std::string(e.what()) == "break encountered") {
                    break;
                }
                else if (std::string(e.what()) == "continue encountered") {
                    // Fall through to increment
                }
                else {
                    throw;
                }
            }

            // Increment
            if (auto& increment = node.getIncrement()) {
                evaluate(*increment);
            }
        }

        env.popScope();
        return lastVal;
    }
    catch (...) {
        env.popScope();
        throw;
    }
}

Value Interpreter::visit(FunctionNode& node) {
    // register the function in the environment
    env.declareFunction(node.getName(), &node);
    return {};
}

Value Interpreter::visit(CallNode& node) {
    std::cout << "Calling function: " << node.getFuncName() << std::endl;
    
    if (!env.hasFunction(node.getFuncName())) {
        throw std::runtime_error("Undefined function: " + node.getFuncName());
    }

    FunctionNode* funcDef = env.getFunction(node.getFuncName());
    const auto& params = funcDef->getParameters();
    const auto& argsNodes = node.getArguments();

    if (params.size() != argsNodes.size()) {
        throw std::runtime_error("Wrong number of arguments");
    }

    // Evaluate arguments before creating new scope
    std::vector<Value> args;
    for (size_t i = 0; i < argsNodes.size(); ++i) {
        Value argValue = evaluate(*argsNodes[i]);
        args.push_back(argValue);
    }

    env.pushScope();
    
    // Bind parameters in new scope
    for (size_t i = 0; i < params.size(); ++i) {
        std::cout << "Binding parameter '" << params[i].first << "' with value: " << args[i].toString() << std::endl;
        env.declareVariable(params[i].first, params[i].second, args[i]);
    }

    Value result;
    try {
        std::cout << "Executing function body" << std::endl;
        result = evaluate(*funcDef->getBody());
        std::cout << "Function completed normally with result: " << result.toString() << std::endl;
    } catch (const ReturnException& e) {
        std::cout << "Function returning with value: " << e.value.toString() << std::endl;
        result = e.value;
        env.popScope();
        return result;
    } catch (const std::exception& e) {
        std::cout << "Exception in function: " << e.what() << std::endl;
        env.popScope();
        throw;
    }

    env.popScope();
    return result;
}

Value Interpreter::visit(ReturnNode& node) {
    if (node.getExpression()) {
        Value returnValue = evaluate(*node.getExpression());
        std::cout << "Return value: " << returnValue.toString() << std::endl;
        throw ReturnException(returnValue);
    }
    throw ReturnException(Value());  // Return void if no expression
}