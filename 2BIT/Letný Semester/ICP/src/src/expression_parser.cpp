/**
 * @file expression_parser.cpp
 * @brief Implementation of the ExpressionParser class
 * @author Hugo Bohácsek (xbohach00)
 */

#include "../headers/expression_parser.h"
#include <sstream>
#include <stack>
#include <cctype>
#include <algorithm>
#include <stdexcept>

/**
 * @brief Default constructor
 * 
 * Initializes the token position to 0
 */
ExpressionParser::ExpressionParser() : position(0) {}

/**
 * @brief Tokenizes an input expression string
 * 
 * Breaks down the expression into individual tokens based on type (number, variable, operator, etc.)
 * 
 * @param expression The expression to tokenize
 * @return Vector of tokens representing the expression
 */
std::vector<Token> ExpressionParser::tokenize(const std::string& expression) {
    std::vector<Token> tokens;
    std::string token;
    
    for (size_t i = 0; i < expression.length(); i++) {
        char c = expression[i];
        
        // Skip whitespace
        if (std::isspace(c)) {
            continue;
        }
        
        // Handle numbers
        if (isDigit(c)) {
            std::string number = getDigits(expression, i);
            i--;
            tokens.emplace_back(TokenType::NUMBER, number);
        }
        // Handle identifiers (variables, keywords)
        else if (isLetter(c)) {
            std::string identifier = getIdentifier(expression, i);
            i--;
            
            // Check for special keywords
            if (identifier == "get") {
                tokens.emplace_back(TokenType::GET, identifier);
            } else if (identifier == "output") {
                tokens.emplace_back(TokenType::OUTPUT, identifier);
            } else if (identifier == "to") {
                tokens.emplace_back(TokenType::TO, identifier);
            } else if (identifier == "if") {
                tokens.emplace_back(TokenType::IF, identifier);
            } else if (identifier == "is") {
                tokens.emplace_back(TokenType::IS, identifier);
            } else if (identifier == "defined") {
                tokens.emplace_back(TokenType::DEFINED, identifier);
            } else if (identifier == "elapsed") {
                tokens.emplace_back(TokenType::SPECIAL_KEYWORD, identifier);
            } else {
                tokens.emplace_back(TokenType::VARIABLE, identifier);
            }
        }
        // Handle operators
        else if (isOperator(c)) {
            if (c == '=') {
                tokens.emplace_back(TokenType::EQUALS, "=");
            } else {
                tokens.emplace_back(TokenType::OPERATOR, std::string(1, c));
            }
        }
    }
    
    tokens.emplace_back(TokenType::END, "");
    return tokens;
}

/**
 * @brief Parses and evaluates an assignment expression
 * 
 * Handles expressions like "var = get input1"
 * 
 * @param expression The assignment expression (e.g., "var = get input1")
 * @param inputValues Map of available input values by input pointer name
 * @return Pair of variable name and assigned value
 * @throw std::runtime_error If the expression format is invalid
 */
std::pair<std::string, std::string> ExpressionParser::parseAssignment(
    const std::string& expression,
    const std::unordered_map<std::string, std::string>& inputValues) {
    
    tokens = tokenize(expression);
    position = 0;
    
    // Format: var = get input_ptr
    if (tokens.size() >= 4 && 
        tokens[0].type == TokenType::VARIABLE &&
        tokens[1].type == TokenType::EQUALS &&
        tokens[2].type == TokenType::GET &&
        tokens[3].type == TokenType::VARIABLE) {
        
        std::string varName = tokens[0].value;
        std::string inputPtr = tokens[3].value;
        
        // Get the input value if it exists
        std::string value;
        auto it = inputValues.find(inputPtr);
        if (it != inputValues.end()) {
            value = it->second;
        }
        
        return {varName, value};
    }
    
    throw std::runtime_error("Invalid assignment expression: " + expression);
}

/**
 * @brief Parses and evaluates an output expression
 * 
 * Handles expressions like "output var + 5 to output1"
 * 
 * @param expression The output expression (e.g., "output var + 5 to output1")
 * @param variables Map of variables available for reference in the expression
 * @param elapsedTime Current elapsed time in milliseconds
 * @return Pair of output pointer and output value
 * @throw std::runtime_error If the expression format is invalid
 */
std::pair<std::string, std::string> ExpressionParser::parseOutput(
    const std::string& expression,
    const std::unordered_map<std::string, MachineVariable>& variables,
    int elapsedTime) {
    
    // Find "output" and "to" keywords
    size_t outputPos = expression.find("output ");
    size_t toPos = expression.find(" to ");
    
    if (outputPos == std::string::npos || toPos == std::string::npos) {
        throw std::runtime_error("Invalid output expression: " + expression);
    }
    
    // Extract the expression between "output" and "to"
    std::string valueExpr = expression.substr(outputPos + 7, toPos - (outputPos + 7));
    
    // Extract the output pointer after "to"
    std::string outputPtr = expression.substr(toPos + 4);
    
    // Evaluate the expression
    MachineVariable result = evaluateExpression(valueExpr, variables, elapsedTime);
    
    return {outputPtr, result.getValueString()};
}

/**
 * @brief Parses and evaluates a conditional expression
 * 
 * Handles expressions like "if input1 is defined"
 * 
 * @param expression The conditional expression (e.g., "if input1 is defined")
 * @param inputValues Map of available input values by input pointer name
 * @return True if the condition is satisfied, false otherwise
 */
bool ExpressionParser::parseCondition(
    const std::string& expression, 
    const std::unordered_map<std::string, std::string>& inputValues) {
    
    // Format: if input_ptr is defined
    tokens = tokenize(expression);
    position = 0;
    
    if (tokens.size() >= 4 && 
        tokens[0].type == TokenType::IF &&
        tokens[1].type == TokenType::VARIABLE &&
        tokens[2].type == TokenType::IS &&
        tokens[3].type == TokenType::DEFINED) {
        
        std::string inputPtr = tokens[1].value;
        
        // Check if the input exists and has a value
        auto it = inputValues.find(inputPtr);
        return (it != inputValues.end() && !it->second.empty());
    }
    
    return true;
}

/**
 * @brief Evaluates an arithmetic expression
 * 
 * Handles expressions like "var + 5 * 3 - elapsed"
 * 
 * @param expression The arithmetic expression (e.g., "var + 5 * 3 - elapsed")
 * @param variables Map of variables available for reference in the expression
 * @param elapsedTime Current elapsed time in milliseconds
 * @return Result of the evaluation as a MachineVariable
 * @throw std::runtime_error If the expression contains errors
 */
MachineVariable ExpressionParser::evaluateExpression(
    const std::string& expression,
    const std::unordered_map<std::string, MachineVariable>& variables,
    int elapsedTime) {
    
    // Simple expression evaluation without precedence
    tokens = tokenize(expression);
    position = 0;
    
    MachineVariable result;
    MachineVariable currentValue;
    std::string currentOp = "+"; // Default operation is addition (for first term)
    
    while (position < tokens.size() && tokens[position].type != TokenType::END) {
        Token token = tokens[position++];
        
        switch (token.type) {
            case TokenType::VARIABLE: {
                // Replace variable with its value
                auto it = variables.find(token.value);
                if (it != variables.end()) {
                    currentValue = it->second;
                } else {
                    throw std::runtime_error("Unknown variable: " + token.value);
                }
                break;
            }
            case TokenType::NUMBER: {
                // Parse number
                try {
                    if (token.value.find('.') != std::string::npos) {
                        currentValue = MachineVariable("float", "temp", token.value);
                    } else {
                        currentValue = MachineVariable("int", "temp", token.value);
                    }
                } catch (const std::exception& e) {
                    throw std::runtime_error("Invalid number: " + token.value);
                }
                break;
            }
            case TokenType::SPECIAL_KEYWORD: {
                // Handle kw "elapsed"
                if (token.value == "elapsed") {
                    currentValue = MachineVariable("int", "elapsed", std::to_string(elapsedTime));
                } else {
                    throw std::runtime_error("Unknown keyword: " + token.value);
                }
                break;
            }
            case TokenType::OPERATOR: {
                // Store operator for next value
                currentOp = token.value;
                continue; // Skip to next token
            }
            default:
                throw std::runtime_error("Unexpected token: " + token.value);
        }
        
        // First value or apply operation
        if (result.getType() == VariableType::UNKNOWN) {
            result = currentValue;
        } else {
            result = result.performOperation(currentOp, currentValue);
        }
    }
    
    return result;
}

/**
 * @brief Checks if a character is a digit or decimal point
 * @param c Character to check
 * @return True if the character is a digit or decimal point
 */
bool ExpressionParser::isDigit(char c) {
    return std::isdigit(c) || c == '.';
}

/**
 * @brief Checks if a character is a letter or underscore
 * @param c Character to check
 * @return True if the character is a letter or underscore
 */
bool ExpressionParser::isLetter(char c) {
    return std::isalpha(c) || c == '_';
}

/**
 * @brief Checks if a character is a supported operator
 * @param c Character to check
 * @return True if the character is a supported operator
 */
bool ExpressionParser::isOperator(char c) {
    return c == '+' || c == '-' || c == '*' || c == '/' || c == '=';
}

/**
 * @brief Extracts a sequence of digits (including decimal point) from an expression
 * @param expr The expression to parse
 * @param pos Current position in the expression, updated to end of the number
 * @return String containing the extracted number
 */
std::string ExpressionParser::getDigits(const std::string& expr, size_t& pos) {
    std::string number;
    bool hasDecimal = false;
    
    while (pos < expr.length() && (std::isdigit(expr[pos]) || (expr[pos] == '.' && !hasDecimal))) {
        if (expr[pos] == '.') {
            hasDecimal = true;
        }
        number += expr[pos++];
    }
    
    return number;
}

/**
 * @brief Extracts an identifier from an expression
 * @param expr The expression to parse
 * @param pos Current position in the expression, updated to end of the identifier
 * @return String containing the extracted identifier
 */
std::string ExpressionParser::getIdentifier(const std::string& expr, size_t& pos) {
    std::string identifier;
    
    while (pos < expr.length() && (std::isalnum(expr[pos]) || expr[pos] == '_')) {
        identifier += expr[pos++];
    }
    
    return identifier;
}

/**
 * @brief Skips whitespace characters in an expression
 * @param expr The expression to parse
 * @param pos Current position in the expression, updated to first non-whitespace
 */
void ExpressionParser::skipWhitespace(const std::string& expr, size_t& pos) {
    while (pos < expr.length() && std::isspace(expr[pos])) {
        pos++;
    }
}