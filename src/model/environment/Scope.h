#ifndef SEASHELLS_SCOPE_H
#define SEASHELLS_SCOPE_H

#include <map>
#include "Variable.h"
#include <iostream>
#include <cctype>
#include <stdexcept>

class Scope {
private:
    std::map<std::string, Variable> variables;
public:
    bool hasVariable(const std::string& name) const;

    void declareVariable(const std::string& name, Type type, const Value& value);

    Variable& getVariable(const std::string& name);

    void debugPrint() const {
        std::cerr << "Scope variables:" << std::endl;
        for (const auto& pair : variables) {
            std::cerr << "  Variable name: ";
            for (char c : pair.first) {
                if (isprint(c)) {
                    std::cerr << c;
                }
                else {
                    std::cerr << "[0x" << std::hex << (int)(unsigned char)c << "]";
                }
            }
            std::cerr << std::endl;
        }
    }
};

#endif //SEASHELLS_SCOPE_H
