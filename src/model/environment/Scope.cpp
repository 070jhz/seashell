#include "Scope.h"

void Scope::declareVariable(const std::string& name, Type type, const Value& value) {
    if (hasVariable(name)) {
        throw std::runtime_error("variable " + name + " already declared in this scope");
    }
    variables[name] = Variable{ type, value };
}

bool Scope::hasVariable(const std::string& name) const {
    return variables.find(name) != variables.end();
}

Variable& Scope::getVariable(const std::string& name) {
    auto it = variables.find(name);

    if (it == variables.end()) {
        throw std::runtime_error("variable " + name + " not found in this scope");
    }
    return it->second;
}
