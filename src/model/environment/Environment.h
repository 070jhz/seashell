#ifndef SEASHELLS_ENVIRONMENT_H
#define SEASHELLS_ENVIRONMENT_H

#include "Scope.h"
#include "../ast/ASTNode.h"
#include <unordered_map>

class Environment {
private:
    std::vector<Scope> scopeStack;
    std::unordered_map<std::string, FunctionNode*> functions;
    bool isGlobalScope;

public:
    Environment() : isGlobalScope(true) {
        std::cout << "Creating environment with global scope" << std::endl;
        pushScope(); // Create global scope
    }

    void pushScope() {
        std::cout << "Pushing scope (current stack size: " << scopeStack.size() << ")" << std::endl;
        scopeStack.push_back(Scope());
        isGlobalScope = (scopeStack.size() == 1);
    }

    void popScope() {
        std::cout << "Popping scope (current stack size: " << scopeStack.size() << ")" << std::endl;
        if (scopeStack.empty()) {
            throw std::runtime_error("Cannot pop empty scope stack");
        }
        if (scopeStack.size() <= 1) {
            throw std::runtime_error("Cannot pop global scope");
        }
        scopeStack.pop_back();
        isGlobalScope = (scopeStack.size() == 1);
    }

    bool isInGlobalScope() const {
        return isGlobalScope;
    }

    void declareVariable(const std::string& name, Type type, const Value& value) {
        if (isGlobalScope) {
            scopeStack.front().declareVariable(name, type, value);
        } else {
            scopeStack.back().declareVariable(name, type, value);
        }
    }

    Variable& getVariable(const std::string& name) {
        std::cout << "Looking for variable '" << name << "' in " << scopeStack.size() << " scopes" << std::endl;
        
        for (auto it = scopeStack.rbegin(); it != scopeStack.rend(); ++it) {
            std::cout << "Checking scope..." << std::endl;
            if (it->hasVariable(name)) {
                std::cout << "Found variable '" << name << "'" << std::endl;
                return it->getVariable(name);
            }
        }
        throw std::runtime_error("Variable '" + name + "' not found in any scope");
    }

    bool hasVariable(const std::string& name) const {
        for (auto it = scopeStack.rbegin(); it != scopeStack.rend(); ++it) {
            if (it->hasVariable(name)) {
                return true;
            }
        }
        return false;
    }

    void declareFunction(const std::string& name, FunctionNode* function) {
        functions[name] = function;
    }

    bool hasFunction(const std::string& name) const {
        return functions.find(name) != functions.end();
    }

    FunctionNode* getFunction(const std::string& name) const {
        auto it = functions.find(name);
        if (it != functions.end()) {
            return it->second;
        }
        throw std::runtime_error("function not found: " + name);
    }
};

#endif //SEASHELLS_ENVIRONMENT_H
