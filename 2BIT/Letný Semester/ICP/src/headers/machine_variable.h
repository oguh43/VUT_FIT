/**
 * @file machine_variable.h
 * @brief Declaration of the MachineVariable class and related types
 * @author Hugo Bohácsek (xbohach00)
 * 
 * This file defines the MachineVariable class which is used to store variables
 * of different types (int, float, string) in the finite state machine.
 * It provides type conversion utilities and arithmetic operations.
 */

#ifndef MACHINE_VARIABLE_H
#define MACHINE_VARIABLE_H

#include <string>
#include <variant>
#include <stdexcept>
#include <sstream>

/**
 * @enum VariableType
 * @brief Enumeration of supported variable types in the machine
 */
enum class VariableType {
    INT,      /**< Integer type */
    FLOAT,    /**< Floating point type */
    STRING,   /**< String type */
    UNKNOWN   /**< Unknown or undefined type */
};

/**
 * @brief Converts a string type name to the corresponding VariableType enum
 * 
 * @param type String representation of the type ("int", "float", "string")
 * @return The corresponding VariableType enum value, or UNKNOWN if not recognized
 */
inline VariableType stringToType(const std::string& type) {
    if (type == "int") return VariableType::INT;
    if (type == "float") return VariableType::FLOAT;
    if (type == "string") return VariableType::STRING;
    return VariableType::UNKNOWN;
}

/**
 * @brief Converts a VariableType enum to its string representation
 * 
 * @param type The VariableType to convert
 * @return String representation of the type ("int", "float", "string", or "unknown")
 */
inline std::string typeToString(VariableType type) {
    switch (type) {
        case VariableType::INT: return "int";
        case VariableType::FLOAT: return "float";
        case VariableType::STRING: return "string";
        default: return "unknown";
    }
}

/**
 * @typedef VariableValue
 * @brief Type alias for the variant that holds different variable value types
 * 
 * Uses std::variant to store either an int, float, or string value
 */
using VariableValue = std::variant<int, float, std::string>;

/**
 * @class MachineVariable
 * @brief Represents a variable with name, type, and value in the finite state machine
 * 
 * This class provides functionality to:
 * - Store variables of different types (int, float, string)
 * - Convert values between different types
 * - Perform arithmetic operations (+, -, *, /) between variables
 * - Handle type compatibility during operations
 */
class MachineVariable {
private:
    VariableType type;     /**< Type of the variable (INT, FLOAT, STRING, UNKNOWN) */
    std::string name;      /**< Name of the variable */
    VariableValue value;   /**< Actual value of the variable */
    
public:
    /**
     * @brief Default constructor
     * 
     * Creates an unnamed variable with UNKNOWN type
     */
    MachineVariable() : type(VariableType::UNKNOWN) {}
    
    /**
     * @brief Constructor with type, name, and value
     * 
     * @param type Type of the variable as a string ("int", "float", "string")
     * @param name Name of the variable
     * @param value Initial value as string (will be converted to appropriate type)
     */
    MachineVariable(const std::string& type, const std::string& name, const std::string& value)
        : name(name) {
        this->type = stringToType(type);
        setValue(value);
    }
    
    /**
     * @brief Get the variable's type
     * 
     * @return The variable type as VariableType enum
     */
    VariableType getType() const { return type; }
    
    /**
     * @brief Get the variable's name
     * 
     * @return The variable name
     */
    std::string getName() const { return name; }
    
    /**
     * @brief Get the variable's value as a string
     * 
     * @return String representation of the variable's value
     */
    std::string getValueString() const {
        std::ostringstream oss;
        if (std::holds_alternative<int>(value)) {
            oss << std::get<int>(value);
        } else if (std::holds_alternative<float>(value)) {
            oss << std::get<float>(value);
        } else if (std::holds_alternative<std::string>(value)) {
            oss << std::get<std::string>(value);
        }
        return oss.str();
    }
    
    /**
     * @brief Get the raw variable value
     * 
     * @return The variable value as VariableValue (variant)
     */
    VariableValue getValue() const { return value; }
    
    /**
     * @brief Set the variable's value from a string
     * 
     * Converts the string to the appropriate type based on the variable's type
     * 
     * @param valueStr String representation of the new value
     * @throw std::runtime_error If the value cannot be parsed or the type is unknown
     */
    void setValue(const std::string& valueStr) {
        try {
            switch (type) {
                case VariableType::INT:
                    value = std::stoi(valueStr);
                    break;
                case VariableType::FLOAT:
                    value = std::stof(valueStr);
                    break;
                case VariableType::STRING:
                    value = valueStr;
                    break;
                default:
                    throw std::runtime_error("Unknown variable type");
            }
        } catch (const std::exception& e) {
            throw std::runtime_error("Failed to parse value: " + valueStr);
        }
    }
    
    /**
     * @brief Perform an arithmetic operation with another variable
     * 
     * Handles type conversions and compatibility checks for operations
     * 
     * @param op The operation to perform ("+", "-", "*", "/")
     * @param other The other variable to operate with
     * @return A new MachineVariable containing the result
     * @throw std::runtime_error If the operation is invalid for the given types or division by zero
     */
    MachineVariable performOperation(const std::string& op, const MachineVariable& other) const {
        MachineVariable result;
        result.name = name;
        result.type = type;
        
        if (type == VariableType::INT && other.type == VariableType::INT) {
            int val1 = std::get<int>(value);
            int val2 = std::get<int>(other.value);
            
            if (op == "+") result.value = val1 + val2;
            else if (op == "-") result.value = val1 - val2;
            else if (op == "*") result.value = val1 * val2;
            else if (op == "/") {
                if (val2 == 0) throw std::runtime_error("Division by zero");
                result.value = val1 / val2;
            }
            else throw std::runtime_error("Unsupported operation: " + op);
        }
        else if ((type == VariableType::FLOAT && 
                 (other.type == VariableType::FLOAT || other.type == VariableType::INT)) ||
                 (type == VariableType::INT && other.type == VariableType::FLOAT)) {
            
            float val1 = (type == VariableType::FLOAT) ? 
                          std::get<float>(value) : 
                          static_cast<float>(std::get<int>(value));
                          
            float val2 = (other.type == VariableType::FLOAT) ? 
                          std::get<float>(other.value) : 
                          static_cast<float>(std::get<int>(other.value));
            
            result.type = VariableType::FLOAT;
            
            if (op == "+") result.value = val1 + val2;
            else if (op == "-") result.value = val1 - val2;
            else if (op == "*") result.value = val1 * val2;
            else if (op == "/") {
                if (val2 == 0) throw std::runtime_error("Division by zero");
                result.value = val1 / val2;
            }
            else throw std::runtime_error("Unsupported operation: " + op);
        }
        else if (type == VariableType::STRING && op == "+") {
            std::string val1 = std::get<std::string>(value);
            std::string val2 = other.getValueString();
            result.value = val1 + val2;
        }
        else {
            throw std::runtime_error("Incompatible types for operation " + op);
        }
        
        return result;
    }
    
    /**
     * @brief Perform an arithmetic operation with an integer value
     * 
     * Convenience method for operations with elapsed time
     * 
     * @param op The operation to perform ("+", "-", "*", "/")
     * @param intValue The integer value to operate with
     * @return A new MachineVariable containing the result
     * @throw std::runtime_error If the operation is invalid or division by zero
     */
    MachineVariable performOperation(const std::string& op, int intValue) const {
        MachineVariable intVar;
        intVar.type = VariableType::INT;
        intVar.value = intValue;
        return performOperation(op, intVar);
    }
};

#endif // MACHINE_VARIABLE_H