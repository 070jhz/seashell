#ifndef SEASHELLS_SCOPE_H
#define SEASHELLS_SCOPE_H

#include <map>
#include "Variable.h"


// represents a single scope (function body, block..)

class Scope {
private:
    std::map<std::string, Variable> variables;
public:

    bool hasVariable(const std::string& name) const;
    void declareVariable(const std::string& name, Type type, const Value& value);
    Variable& getVariable(const std::string& name);
};


#endif //SEASHELLS_SCOPE_H
