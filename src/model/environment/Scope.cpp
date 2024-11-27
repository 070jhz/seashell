#include "Scope.h"

bool Scope::hasVariable(const std::string& name) const {
    return variables.find(name) != variables.end();
}

void Scope::declareVariable(const std::string& name, Type type, const Value& value) {
    if (hasVariable(name)) {
        throw std::runtime_error("Variable already declared: " + name);
    }
    try {
        Variable var{ type, value };
        variables.emplace(name, std::move(var));
    }
    catch (const std::bad_alloc& e) {
        std::cerr << "Memory allocation failed while declaring variable: " << name << std::endl;
        throw;
    }
}

Variable& Scope::getVariable(const std::string& name) {
    auto it = variables.find(name);
    if (it == variables.end()) {
        throw std::runtime_error("Variable not found: " + name);
    }
    return it->second;
}
