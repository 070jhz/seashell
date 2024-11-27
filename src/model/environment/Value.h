#ifndef SEASHELLS_VALUE_H
#define SEASHELLS_VALUE_H

#include <variant>
#include <string>
#include <stdexcept>
#include <sstream>
#include <map>
#include <vector>

enum class Type {
    VOID,
    INT,
    DOUBLE,
    BOOL,
    STRING,
    ARRAY
};

// represents a value in the shell
class Value {
private:
    std::variant<std::monostate, int, double, bool, std::string, std::vector<Value>> data;

public:
    Value() : data() {}
    Value(int v) : data(v) {}
    Value(double v) : data(v) {}
    Value(bool v) : data(v) {}
    Value(const std::string& v) : data(v) {}
    Value(const std::vector<Value>& v) : data(v) {}

    Type getType() const;
    std::string toString() const;
    bool toBool() const;

    template<typename T>
    T get() const {
        return std::get<T>(data);
    }

    Value& atIndex(int index) {
        if (this->getType() != Type::ARRAY) {
			throw std::runtime_error("Value is not an array");
        }
        return std::get<std::vector<Value>>(data).at(index);
    }

    friend std::ostream& operator<<(std::ostream& os, const Value& v) {
        return os << v.toString();
    }
};

std::string typeToString(Type type);

#endif //SEASHELLS_VALUE_H
