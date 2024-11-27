#include "Value.h"

Type Value::getType() const {
    if (std::holds_alternative<std::monostate>(data)) return Type::VOID;
    if (std::holds_alternative<int>(data)) return Type::INT;
    if (std::holds_alternative<double>(data)) return Type::DOUBLE;
    if (std::holds_alternative<bool>(data)) return Type::BOOL;
    if (std::holds_alternative<std::string>(data)) return Type::STRING;
    if (std::holds_alternative<std::vector<Value>>(data)) return Type::ARRAY;
    throw std::runtime_error("Unknown type");
}

std::string typeToString(Type type) {
    switch (type) {
    case Type::VOID: return "void";
    case Type::INT: return "int";
    case Type::DOUBLE: return "double";
    case Type::BOOL: return "bool";
    case Type::STRING: return "string";
    case Type::ARRAY: return "array";
    default: throw std::runtime_error("Unknown type");
    }
}

std::string Value::toString() const {
    /*
    * doesn't work bcause no >> defined for monostate
        std::ostringstream oss;
        if (auto* ptr = std::get_if<std::monostate>(&data)) { // if type of data in variant is monostate
            oss << "void";
        }
        else {
            std::visit([&oss](const auto& v) { // captures oss for lambda function
                oss << v;
                }, data);
        }
        return oss.str();

    */

    std::ostringstream oss;
    std::visit([&](const auto& v) {
        using T = std::decay_t<decltype(v)>; // remove qualifiers to get raw type
        if constexpr (std::is_same_v<T, std::monostate>) {
            oss << "void";
        }
        else if constexpr (std::is_same_v<T, std::vector<Value>>) {
            oss << "[";
            for (size_t i = 0; i < v.size(); ++i) {
                oss << v[i].toString();
                if (i < v.size() - 1) oss << ", ";
            }
            oss << "]";
        }
        else {
            oss << v;
        }

        }, data);

    return oss.str();
}

bool Value::toBool() const {
    if (std::holds_alternative<bool>(data)) {
        return std::get<bool>(data);
    }
    else if (std::holds_alternative<int>(data)) {
        return std::get<int>(data) != 0;
    }
    else if (std::holds_alternative<double>(data)) {
        return std::get<double>(data) != 0.0;
    }
    else if (std::holds_alternative<std::string>(data)) {
        const std::string& str = std::get<std::string>(data);
        return !str.empty() && str != "false";
    }

    return false;
}
