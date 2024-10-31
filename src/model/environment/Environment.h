#ifndef SEASHELLS_ENVIRONMENT_H
#define SEASHELLS_ENVIRONMENT_H

#include "Scope.h"


// manages scopes as stack
class Environment {
private:
    std::vector<Scope> scopeStack;

public:
    Environment() {
        pushScope();
    }

    void pushScope() {
        scopeStack.push_back(Scope());
    }

    void popScope() {
        if (scopeStack.size() <= 1) {
            throw std::runtime_error("can't pop global scope");
        }
        scopeStack.pop_back();
    }

    void declareVariable(const std::string& name, Type type, const Value& value) {
        scopeStack.back().declareVariable(name, type, value);
    }

    Variable& getVariable(const std::string& name) {
        for (auto it = scopeStack.rbegin(); it != scopeStack.rend(); ++it) {
            if (it->hasVariable(name)) {
                return it->getVariable(name);
            }
        }
        throw std::runtime_error("variable " + name + " not found in any scope");
    }

    bool hasVariable(const std::string& name) const {
        for (auto it = scopeStack.rbegin(); it != scopeStack.rend(); ++it) {
            if (it->hasVariable(name)) {
                return true;
            }
        }
        return false;
    }
};

#endif //SEASHELLS_ENVIRONMENT_H
