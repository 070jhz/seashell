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
	for (size_t i = 0; i < elements.size(); ++i) {
		result += elements[i]->toString();
		if (i < elements.size() - 1) result += ", ";
	}
	result += "]";
	return result;
}

Value ArrayAccessNode::accept(ASTVisitor& visitor) {
	return visitor.visit(*this);
}

Value UnaryOpNode::accept(ASTVisitor& visitor) {
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

Value BinOpNode::accept(ASTVisitor& visitor) {
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

Value AssignmentNode::accept(ASTVisitor& visitor) {
	return visitor.visit(*this);
}

bool AssignmentNode::isTypeCompatible(Type sourceType, Type targetType) {
	if (sourceType == targetType) return true;

	if (targetType == Type::DOUBLE && sourceType == Type::INT) return true;

	return false;
}

Value BlockNode::accept(ASTVisitor& visitor) {
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

Value IfNode::accept(ASTVisitor& visitor) {
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

Value WhileNode::accept(ASTVisitor& visitor) {
	return visitor.visit(*this);
}

std::string WhileNode::toString() {
	return "while (" + condition->toString() + ") {\n\t" + body->toString() + "\n}";
}

Value ForNode::accept(ASTVisitor& visitor) {
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

Value FunctionNode::accept(ASTVisitor& visitor) {
	return visitor.visit(*this);
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

Value ReturnNode::accept(ASTVisitor& visitor) {
	return visitor.visit(*this);
}

Value CallNode::accept(ASTVisitor& visitor) {
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