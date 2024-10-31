#include "ASTNode.h"
#include "ASTVisitor.h"
#include <stdexcept>

Value BreakNode::accept(ASTVisitor& visitor) {
    return visitor.visit(*this);
}

Value ContinueNode::accept(ASTVisitor& visitor) {
    return visitor.visit(*this);
}

Value LiteralNode::accept(ASTVisitor& visitor) {
    return visitor.visit(*this);
}

Value VariableNode::accept(ASTVisitor& visitor) {
    return visitor.visit(*this);
}

Value ArrayNode::accept(ASTVisitor& visitor) {
    return visitor.visit(*this);
}

std::string ArrayNode::toString() {
    std::string result = "[";
    for (size_t i=0; i<elements.size(); ++i) {
        result += elements[i]->toString();
        if (i < elements.size()-1) result += ", ";
    }
    result += "]";
    return result;
}

Value ArrayAccessNode::accept(ASTVisitor &visitor) {
    return visitor.visit(*this);
}

Value UnaryOpNode::accept(ASTVisitor &visitor) {
    return visitor.visit(*this);
}

std::string UnaryOpNode::toString() {
	std::string opStr;
	switch (op) {
	case Operator::Negate:
		opStr = "-";
		return opStr + operand->toString();
	case Operator::LogicalNot:
		opStr = "!";
		return opStr + operand->toString();
	case Operator::PostIncrement:
		opStr = "++";
		return operand->toString() + opStr;
	case Operator::PreIncrement:
		opStr = "++";
		return opStr + operand->toString();
	case Operator::PostDecrement:
		opStr = "--";
		return operand->toString() + opStr;
	case Operator::PreDecrement:
		opStr = "--";
		return opStr + operand->toString();
	}
	throw std::runtime_error("unknown unary operator");
}

Value BinOpNode::accept(ASTVisitor &visitor) {
    return visitor.visit(*this);
}

std::string BinOpNode::toString() {
	std::string opStr;
	switch (op) {
	case Operator::Add: opStr = "+"; break;
	case Operator::Subtract: opStr = "-"; break;
	case Operator::Multiply: opStr = "*"; break;
	case Operator::Divide: opStr = "/"; break;
	case Operator::Equal: opStr = "=="; break;
	case Operator::NotEqual: opStr = "!="; break;
	case Operator::Less: opStr = "<"; break;
	case Operator::LessEqual: opStr = "<="; break;
	case Operator::Greater: opStr = ">"; break;
	case Operator::GreaterEqual: opStr = ">="; break;
	case Operator::And: opStr = "&&"; break;
	case Operator::Or: opStr = "||"; break;
	}
	return "(" + left->toString() + " " + opStr + " " + right->toString() + ")";
}

Value AssignmentNode::accept(ASTVisitor &visitor) {
    return visitor.visit(*this);
}

bool AssignmentNode::isTypeCompatible(Type sourceType, Type targetType) {
	if (sourceType == targetType) return true;

	if (targetType == Type::DOUBLE && sourceType == Type::INT) return true;

	return false;
}

Value BlockNode::accept(ASTVisitor &visitor) {
    return visitor.visit(*this);
}

std::string BlockNode::toString() {
	std::stringstream ss;
	ss << "{\n";
	for (const auto& statement : statements) {
		ss << "  " << statement->toString() << ";\n";
	}
	ss << "}";
	return ss.str();
}

Value IfNode::accept(ASTVisitor &visitor) {
    return visitor.visit(*this);
}



std::string IfNode::toString() {
	std::string ifStr = "if (" + condition->toString() + ") {\n"
		+ "\t" + thenBranch->toString() + "\n}";

	if (elseBranch) {
		ifStr += " else {\n\t" + elseBranch->toString() + "\n}";
	}

	return ifStr;
}

Value WhileNode::accept(ASTVisitor &visitor) {
    return visitor.visit(*this);
}

std::string WhileNode::toString() {
	return "while (" + condition->toString() + ") {\n\t" + body->toString() + "\n}";
}

Value ForNode::accept(ASTVisitor &visitor) {
    return visitor.visit(*this);
}

std::string ForNode::toString() {
	std::string forStr = "for (";
	forStr += (initialization) ? initialization->toString() : "";
	forStr += "; ";
	forStr += (condition) ? condition->toString() : "";
	forStr += "; ";
	forStr += (increment) ? increment->toString() : "";
	forStr += ") ";
	forStr += (body) ? body->toString() : "{}";
	return forStr;
}

Value FunctionNode::accept(ASTVisitor &visitor) {
    return visitor.visit(*this);
}

Value FunctionNode::evaluate(Environment& env) {
	env.declareVariable(name, Type::FUNCTION, Value(this));
	return Value();
}


std::string FunctionNode::toString() {
	std::stringstream ss;
	ss << "function " << name << "(";
	for (size_t i = 0; i < parameters.size(); ++i) {
		if (i > 0) ss << ", ";
		ss << typeToString(parameters[i].second) << " " << parameters[i].first;
	}
	ss << ")" << typeToString(returnType) << " " << body->toString();
	return ss.str();
}


ExecutionResult FunctionNode::execute(Environment& env, const std::vector<std::unique_ptr<ASTNode>>& args) {
	if (args.size() != parameters.size()) {
		throw std::runtime_error("wrong number of arguments for function " + name);
	}
	env.pushScope(); // create new scope for function execution
	for (size_t i = 0; i < parameters.size(); ++i) {
		// evaluate arguments and bind to parameters
		try {
			Value argValue = args[i]->evaluate(env);
			if (argValue.getType() != parameters[i].second) {
				env.popScope();
				return ExecutionResult::error("type mismatch for argument " + std::to_string(i) + " in function " + name);
			}
			env.declareVariable(parameters[i].first, parameters[i].second, argValue);
		} catch (const std::exception& e) {
			env.popScope();
			return ExecutionResult::error("error evaluating argument : " + std::to_string(i) + " : " + std::string(e.what()));
		} 
	}
	
	// execute function body
	try {
		Value result = body->evaluate(env);
		env.popScope();

		return ExecutionResult{ result, OutputType::ExpressionResult, "", true, "" };
	}
	catch (const std::exception& e) {
		env.popScope();
		return ExecutionResult::error("error executing function : " + std::string(e.what()));
	}
}

Value ReturnNode::accept(ASTVisitor &visitor) {
    return visitor.visit(*this);
}

Value ReturnNode::evaluate(Environment& env) {
	if (expression) {
		return expression->evaluate(env);
	}
	return Value();
}

Value CallNode::accept(ASTVisitor &visitor) {
    return visitor.visit(*this);
}

std::string CallNode::toString() {
	std::string result = name + "(";
	for (size_t i = 0; i < arguments.size(); ++i) {
		if (i > 0) {
			result += ", ";
		}
		result += arguments[i]->toString();
	}
	result += ")";
	return result;
}

Value CallNode::evaluate(Environment& env) {
	if (!env.hasVariable(name)) {
		throw std::runtime_error("undefined function: " + name);
	}

	Variable& funcVar = env.getVariable(name);
	if (funcVar.type != Type::FUNCTION) {
		throw std::runtime_error(name + "is not a function");
	}
	/*
	FunctionNode* func = std::any_cast<FunctionNode*>(funcVar.value.get<void*>());

	ExecutionResult result = func->execute(env, arguments);

	if (!result.successFlag) {
		throw std::runtime_error("error executing function: " + result.errorMessage);
	}
	
	return result.result;*/
	return Value(0);
}