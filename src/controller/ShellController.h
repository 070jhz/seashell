#ifndef SEASHELL_SHELLCONTROLLER_H
#define SEASHELL_SHELLCONTROLLER_H

#include "../model/environment/Environment.h"
#include "../model/parser/Parser.h"
#include "../model/ast/Interpreter.h"
#include <memory>

class ShellController {
private:
    Environment globalEnv;
    Interpreter interpreter;
    Parser parser;

    struct InputState {
        std::string buf;
        bool inMultiLine = false;

        void reset() {
            buf.clear();
            inMultiLine = false;
        }
    } inputState;

public:
    ShellController() : interpreter(globalEnv) {}

    const Environment& getEnvironment() const { return globalEnv; }
    bool isInMultiLine() const { return inputState.inMultiLine; }
    void clearBuffer() { inputState.reset(); }
    std::string getBuffer() { return inputState.buf; }

    void appendInput(const std::string& input);
    std::string executeBuffer();
};

#endif //SEASHELL_SHELLCONTROLLER_H