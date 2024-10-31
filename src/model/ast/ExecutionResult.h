#ifndef SEASHELLS_EXECUTIONRESULT_H
#define SEASHELLS_EXECUTIONRESULT_H

#include "../environment/Value.h"

// types of output the shell can produce
enum class OutputType {
    ExpressionResult,
    VariableDeclaration,
    FunctionDefinition,
    Error,
    Void
};

// result of executing a statement
struct ExecutionResult {
    Value result;
    OutputType type;
    std::string output;
    bool successFlag;
    std::string errorMessage;

    static ExecutionResult success(Value val, OutputType t, const std::string& out = "") {
        return ExecutionResult{ val, t, out, true, "" };
    }

    static ExecutionResult error(const std::string& err) {
        return ExecutionResult{ Value(), OutputType::Error, "", false, err };
    }
};

#endif //SEASHELLS_EXECUTIONRESULT_H
