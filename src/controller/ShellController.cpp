#include "ShellController.h"

void ShellController::appendInput(const std::string& input) {
    if (inputState.buf.empty()) {
        inputState.buf = input;
    }
    else {
        inputState.buf += "\n" + input;
    }
}

std::string ShellController::executeBuffer() {
    try {
        std::unique_ptr<ASTNode> ast = parser.parse(inputState.buf);
        if (!ast) {
            throw std::runtime_error("Failed to parse input");
        }
        Value result = interpreter.evaluate(*ast);
        inputState.reset();
        return result.toString();  // Always return the string representation
    }
    catch (const std::bad_alloc& e) {
        inputState.reset();
        throw; // Rethrow the bad_alloc
    }
    catch (const std::exception& e) {
        inputState.reset();
        return std::string("Error: ") + e.what();
    }
}