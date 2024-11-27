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
    return { evaluatedElements };
}

Value Interpreter::visit(ArrayAccessNode& node) {
    auto var = env.getVariable(node.getName());
    if (var.type == Type::ARRAY) {
        auto array = var.value.get<std::vector<Value>>();
        auto id = evaluate(*node.getIndex());
        if (id.getType() != Type::INT) {
            throw std::runtime_error("index must be integer");
        }
        else {
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
            return { -val.get<int>() };
        }
        else if (val.getType() == Type::DOUBLE) {
            return { -val.get<double>() };
        }
        throw std::runtime_error("invalid operand type for unary '-'");

    case Operator::LogicalNot:
        if (val.getType() == Type::BOOL) {
            return { !val.get<bool>() };
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
            var.value = { val.get<int>() + 1 };
            return op == Operator::PreIncrement ? Value{ val.get<int>() + 1 } : val;
        }
        else if (val.getType() == Type::DOUBLE) {
            auto& varNode = dynamic_cast<VariableNode&>(*operand);
            auto& var = env.getVariable(varNode.toString());
            var.value = { val.get<double>() + 1.0 };
            return op == Operator::PreIncrement ? Value{ val.get<double>() + 1.0 } : val;
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
            var.value = { val.get<int>() - 1 };
            return op == Operator::PreDecrement ? Value{ val.get<int>() - 1 } : val;
        }
        else if (val.getType() == Type::DOUBLE) {
            auto& varNode = dynamic_cast<VariableNode&>(*operand);
            auto& var = env.getVariable(varNode.toString());
            var.value = { val.get<double>() - 1.0 };
            return op == Operator::PreDecrement ? Value{ val.get<double>() - 1.0 } : val;
        }
        throw std::runtime_error("invalid type for increment operator");
    default:
        throw std::runtime_error("unknown unary operator");
    }
}

template<typename T>
Value performOperation(T left, T right, Operator op) {
    switch (op) {
    case Operator::Add:
        return Value(left + right);
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
    case Operator::Subtract:
    case Operator::Multiply:
    case Operator::Divide:
    case Operator::And:
    case Operator::Or:
        if constexpr (std::is_same_v<T, std::string>) {
            throw std::runtime_error("operation not supported for strings");
        }
        else {
            if (op == Operator::Subtract)
                return Value(left - right);
            if (op == Operator::Multiply)
                return Value(left * right);
            if (op == Operator::Divide) {
                if constexpr (std::is_same_v<T, int>) {
                    if (right == 0)
                        throw std::runtime_error("cant divide by zero");
                }
                else if constexpr (std::is_same_v<T, double>) {
                    if (std::abs(right) < std::numeric_limits<double>::epsilon())
                        throw std::runtime_error("cant divide by zero.zero");
                }
                return Value(left / right);
            }
            if (op == Operator::And)
                return Value(static_cast<bool>(left) && static_cast<bool>(right));
            if (op == Operator::Or)
                return Value(static_cast<bool>(left) || static_cast<bool>(right));
        }
        break;
    default:
        break;
    }
    throw std::runtime_error("unknown operator");
}

// handles number type difference
Value Interpreter::visit(BinOpNode& node) {
    Value left = evaluate(*node.getLeft());
    Value right = evaluate(*node.getRight());
    Operator op = node.getOperator();

    if (left.getType() == Type::STRING && right.getType() == Type::STRING) {
        return performOperation(left.get<std::string>(), right.get<std::string>(), op);
    }

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
        if (declType == Type::ARRAY) {
            std::cout << "tis array" << std::endl;
        }
    }
    else { // assignment to existing variable
        if (!env.hasVariable(varName)) {
            throw std::runtime_error("undefined variable: " + varName);
        }
        Variable& existingVar = env.getVariable(varName);
		std::cout << "if array assignment : " << node.checkIfArrayAssignment() << std::endl;
        if (node.checkIfArrayAssignment()) {
            try {
				int index = evaluate(*node.getIndex()).get<int>();
                if (index < 0 || index >= existingVar.value.get<std::vector<Value>>().size()) {
					throw std::runtime_error("array index out of bounds: " + std::to_string(index));
                }
                Value& elem = existingVar.value.atIndex(index);
                std::cout << "here is elem : " << elem.toString() << std::endl;

                if (!node.isTypeCompatible(exprVal.getType(), elem.getType())) {
                    throw std::runtime_error("type mismatch in array assignment. cannot assign"
                        + typeToString(exprVal.getType()) + "to array of type " + typeToString(elem.getType()));
                }
                elem = exprVal;
            }
            catch (const std::exception& e) {
				std::cerr << "error during array assignment: " << e.what() << std::endl;
                throw;
            }
        }
        else {
            if (!node.isTypeCompatible(exprVal.getType(), existingVar.type)) {
                throw std::runtime_error("type mismatch in assignment. cannot assign"
                    + typeToString(exprVal.getType()) + "to variable of type " + typeToString(existingVar.type));
            }
            existingVar.value = exprVal;
        }
    }

    return exprVal;
}

Value Interpreter::visit(BlockNode& node) {
    if (node.shouldCreateScope()) {
        env.pushScope();
    }

    Value lastVal;
    try {
        for (const auto& stmt : node.getStatements()) {
            lastVal = evaluate(*stmt);
        }
    }
    catch (const ReturnException& e) {
        if (node.shouldCreateScope()) {
            env.popScope();
        }
        throw; // Re-throw return value
    }
    catch (...) {
        if (node.shouldCreateScope()) {
            env.popScope();
        }
        throw;
    }

    if (node.shouldCreateScope()) {
        env.popScope();
    }

    return lastVal;
}

Value Interpreter::visit(IfNode& node) {
    Value condValue = evaluate(*node.getCondition());

    if (condValue.toBool()) {
        try {
            Value result = evaluate(*node.getThenBranch());
            return result;
        }
        catch (const ReturnException& e) {
            throw; // re throw the return
        }
    }
    else if (auto& elseBranch = node.getElseBranch()) {
        try {
            Value result = evaluate(*elseBranch);
            return result;
        }
        catch (const ReturnException& e) {
            throw;
        }
    }

    return Value(); // default return for if without else
}

Value Interpreter::visit(WhileNode& node) {
    Value lastVal;
    auto& condition = node.getCondition();
    auto& body = node.getBody();
    while (evaluate(*condition).toBool()) {
        try {
            lastVal = evaluate(*body);
        }
        catch (const std::runtime_error& e) {
            if (std::string(e.what()) == "break encountered") {
                break;
            }
            else if (std::string(e.what()) == "continue encountered") {
                continue;
            }
            else {
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
        // initialization
        if (auto& init = node.getInitialization()) {
            evaluate(*init);
        }

        // loop
        while (true) {
            // condition check
            if (auto& condition = node.getCondition()) {
                Value condVal = evaluate(*condition);
                if (condVal.getType() != Type::BOOL) {
                    throw std::runtime_error("for loop condition must be boolean");
                }
                if (!condVal.get<bool>()) {
                    break;
                }
            }

            // body execution
            try {
                lastVal = evaluate(*node.getBody());
            }
            catch (const std::runtime_error& e) {
                if (std::string(e.what()) == "break encountered") {
                    break;
                }
                else if (std::string(e.what()) == "continue encountered") {
                    // fall through to increment
                }
                else {
                    throw;
                }
            }

            // increment
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
    std::cout << "1. Getting function name" << std::endl;
    const std::string& funcName = node.getFuncName();

    std::cout << "2. Looking up function: " << funcName << std::endl;
    FunctionNode* funcDef = env.getFunction(funcName);

    std::cout << "3. Getting parameters" << std::endl;
    const auto& params = funcDef->getParameters();
    std::cout << "4. Getting arguments" << std::endl;
    const auto& argsNodes = node.getArguments();
    std::cout << "argsize : " << argsNodes.size() << std::endl;


    std::vector<Value> evalArgs;
    try {
        // pre-allocate space to avoid reallocation
        evalArgs.reserve(argsNodes.size());

        // evaluate all arguments before creating new scope
        for (const auto& arg : argsNodes) {
            evalArgs.push_back(evaluate(*arg));
        }

        env.pushScope();

        // Bind parameters in new scope
        if (!params.empty()) {
            for (size_t i = 0; i < params.size(); ++i) {
                std::cout << "params size: " << params.size() << std::endl;
                std::cout << "evalArgs size: " << evalArgs.size() << std::endl;
                std::cout << "current i: " << i << std::endl;
                env.declareVariable(params[i].first, params[i].second, evalArgs[i]);
            }
        }

        // Execute function body
        Value result = evaluate(*funcDef->getBody());
        env.popScope();
        return result;
    }
    catch (const ReturnException& e) {
        env.popScope();
        return e.value;
    }
    catch (...) {
        if (!evalArgs.empty()) {
            env.popScope();
        }
        throw;
    }
}

Value Interpreter::visit(ReturnNode& node) {
    Value returnValue;
    if (node.getExpression()) {
        returnValue = evaluate(*node.getExpression());
    }
    throw ReturnException(std::move(returnValue));
}