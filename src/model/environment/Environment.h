#ifndef SEASHELLS_ENVIRONMENT_H
#define SEASHELLS_ENVIRONMENT_H

#include "Scope.h"
#include "../ast/ASTNode.h"
#include <unordered_map>
#include <memory>
#include <iostream>
#include <iomanip>
#include <algorithm>

class Environment {
private:
    static constexpr size_t MAX_NAME_LENGTH = 256;
    std::vector<std::unique_ptr<Scope>> scopeStack;
    std::unordered_map<std::string, std::unique_ptr<FunctionNode>> functions;

    bool isValidIdentifier(const std::string& name, bool isFunction = false) const {
        if (name.empty()) {
            return false;
        }
        // first character must be letter or underscore
        if (!std::isalpha(name[0]) && name[0] != '_') {
            return false;
        }
        // rest must be alphanumeric or underscore
        return std::all_of(name.begin() + 1, name.end(),
            [](char c) { return std::isalnum(c) || c == '_'; });
    }

public:
    Environment() {
        pushScope(); // create global scope
    }

    void pushScope() {
        try {
            scopeStack.push_back(std::make_unique<Scope>());
        }
        catch (const std::bad_alloc& e) {
            std::cerr << "Memory allocation failed in pushScope() : " << e.what() << std::endl;
            throw;
        }
    }

    void popScope() {
        if (scopeStack.size() <= 1) {
            throw std::runtime_error("Cannot pop global scope");
        }
        scopeStack.pop_back();
    }

    bool isInGlobalScope() const {
        return scopeStack.size() == 1;
    }

    void declareVariable(const std::string& name, Type type, const Value& value) {
        try {
            if (name.empty()) {
                throw std::runtime_error("Empty variable name");
            }
            // copy of the name to ensure string stability
            std::string nameCopy = name;
            scopeStack.back()->declareVariable(nameCopy, type, value);
        }
        catch (const std::bad_alloc& e) {
            std::cerr << "Memory allocation failed in declareVariable() for " << name << " : " << e.what() << std::endl;
            throw;
        }
    }

    Variable& getVariable(const std::string& name) {
        for (auto it = scopeStack.rbegin(); it != scopeStack.rend(); ++it) {
            if ((*it)->hasVariable(name)) {
                return (*it)->getVariable(name);
            }
        }
        throw std::runtime_error("Variable '" + name + "' not found in any scope");
    }

    bool hasVariable(const std::string& name) const {
        for (auto it = scopeStack.rbegin(); it != scopeStack.rend(); ++it) {
            if ((*it)->hasVariable(name)) {
                return true;
            }
        }
        return false;
    }

    void declareFunction(const std::string& name, const FunctionNode* function) {
        try {
            if (!function) {
                throw std::runtime_error("Attempting to declare null function pointer for: " + name);
            }
            if (!isValidIdentifier(name, true)) {
                throw std::runtime_error("Invalid function name: " + name);
            }

            auto funcCopy = std::unique_ptr<FunctionNode>(
                static_cast<FunctionNode*>(function->clone().release())
            );
            functions[name] = std::move(funcCopy);
            std::cerr << "Declaring function '" << getFunction(name)->getName() << "' with " << getFunction(name)->getParameters().size() << " parameters" << std::endl;
        }
        catch (const std::bad_alloc& e) {
            std::cerr << "Memory allocation failed in declareFunction() for " << name << " : " << e.what() << std::endl;
            throw;
        }
    }

    FunctionNode* getFunction(const std::string& name) {
        auto it = functions.find(name);
        if (it == functions.end()) {
            throw std::runtime_error("Function not found: " + name);
        }
        return it->second.get();  // return raw pointer to our owned copy
    }

    bool hasFunction(const std::string& name) const {
        return functions.find(name) != functions.end();
    }

    void validateFunctionCall(const std::string& name, size_t argCount) {
        auto fn = getFunction(name);
        if (!fn) {
            throw std::runtime_error("Function not found: " + name);
        }

        const auto& params = fn->getParameters();
        if (params.size() != argCount) {
            std::stringstream ss;
            ss << "Wrong number of arguments for function '" << name << "'. ";
            ss << "Expected " << params.size() << ", got " << argCount;
            throw std::runtime_error(ss.str());
        }
    }
};

#endif //SEASHELLS_ENVIRONMENT_H
