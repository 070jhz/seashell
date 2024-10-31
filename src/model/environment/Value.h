#ifndef SEASHELLS_VALUE_H
#define SEASHELLS_VALUE_H

#include <variant>
#include <string>
#include <stdexcept>
#include <sstream>
#include <map>
#include <vector>
#include "ASTNode.h"

enum class Type {
    VOID,
    INT,
    DOUBLE,
    BOOL,
    STRING,
    ARRAY,
    FUNCTION
};

struct Function {
    std::unique_ptr<ASTNode> body;
    std::vector<std::pair<std::string, Type>> parameters;
    Type returnType;
};

// represents a value in the shell
class Value {
private:
    std::variant<std::monostate, int, double, bool, std::string, std::vector<Value>, Function> data;

public:
    Value() : data() {}
    Value(int v) : data(v) {}
    Value(double v) : data(v) {}
    Value(bool v) : data(v) {}
    Value(const std::string& v) : data(v) {}
    Value(const std::vector<Value>& v) : data(v) {}
    Value()

    Type getType() const;
    std::string toString() const;
    bool toBool() const;
    static bool isArrayType(Type type);

    template<typename T>
    T get() const {
        return std::get<T>(data);
    }
};


#endif //SEASHELLS_VALUE_H
