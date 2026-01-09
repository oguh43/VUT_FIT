/**
 * @file expression_parser.h
 * @brief Declaration of the ExpressionParser class for parsing and evaluating expressions
 * @author Hugo Bohácsek (xbohach00)
 */

 #ifndef EXPRESSION_PARSER_H
 #define EXPRESSION_PARSER_H
 
 #include <string>
 #include <vector>
 #include <unordered_map>
 #include "machine_variable.h"
 
 /**
  * @enum TokenType
  * @brief Defines the types of tokens recognized during expression parsing
  */
 enum class TokenType {
    VARIABLE,        /**< Variable identifier */
    NUMBER,          /**< Numeric literal */
    OPERATOR,        /**< Mathematical operator */
    FUNCTION,        /**< Function identifier */
    SPECIAL_KEYWORD, /**< Special language keywords */
    EQUALS,          /**< Assignment operator */
    GET,             /**< 'get' keyword for input retrieval */
    OUTPUT,          /**< 'output' keyword for output generation */
    TO,              /**< 'to' keyword for output destination */
    IF,              /**< 'if' keyword for condition */
    IS,              /**< 'is' keyword for condition */
    DEFINED,         /**< 'defined' keyword for checking defined inputs */
    END              /**< End of expression marker */
 };
 
 /**
  * @struct Token
  * @brief Represents a token in the parsed expression
  */
 struct Token {
    TokenType type;      /**< The type of this token */
    std::string value;   /**< The string value of this token */
    
    /**
     * @brief Constructs a token with the given type and value
     * @param type The token type
     * @param value The string value of the token
     */
    Token(TokenType type, const std::string& value) : type(type), value(value) {}
 };
 
 /**
  * @class ExpressionParser
  * @brief Parses and evaluates expressions used in the FSM
  * 
  * This class provides functionality to parse and evaluate various types of expressions:
  * - Assignment expressions (var = get input1)
  * - Output expressions (output var + 5 to output1)
  * - Conditional expressions (if input1 is defined)
  * - Arithmetic expressions (var + 5 * 3 - elapsed)
  */
 class ExpressionParser {
 private:
    std::vector<Token> tokens; /**< Tokenized expression */
    size_t position;           /**< Current position in token stream */
     
 public:
    /**
     * @brief Default constructor
     */
    ExpressionParser();
    
    /**
     * @brief Tokenizes an input expression string
     * @param expression The expression to tokenize
     * @return Vector of tokens representing the expression
     */
    std::vector<Token> tokenize(const std::string& expression);
    
    /**
     * @brief Parses and evaluates an assignment expression
     * @param expression The assignment expression (e.g., "var = get input1")
     * @param inputValues Map of available input values by input pointer name
     * @return Pair of variable name and assigned value
     * @throw std::runtime_error If the expression format is invalid
     */
    std::pair<std::string, std::string> parseAssignment(
        const std::string& expression,
        const std::unordered_map<std::string, std::string>& inputValues);
    
    /**
     * @brief Parses and evaluates an output expression
     * @param expression The output expression (e.g., "output var + 5 to output1")
     * @param variables Map of variables available for reference in the expression
     * @param elapsedTime Current elapsed time in milliseconds
     * @return Pair of output pointer and output value
     * @throw std::runtime_error If the expression format is invalid
     */
    std::pair<std::string, std::string> parseOutput(
        const std::string& expression,
        const std::unordered_map<std::string, MachineVariable>& variables,
        int elapsedTime);
    
    /**
     * @brief Parses and evaluates a conditional expression
     * @param expression The conditional expression (e.g., "if input1 is defined")
     * @param inputValues Map of available input values by input pointer name
     * @return True if the condition is satisfied, false otherwise
     */
    bool parseCondition(
        const std::string& expression, 
        const std::unordered_map<std::string, std::string>& inputValues);
    
    /**
     * @brief Evaluates an arithmetic expression
     * @param expression The arithmetic expression (e.g., "var + 5 * 3 - elapsed")
     * @param variables Map of variables available for reference in the expression
     * @param elapsedTime Current elapsed time in milliseconds
     * @return Result of the evaluation as a MachineVariable
     * @throw std::runtime_error If the expression contains errors
     */
    MachineVariable evaluateExpression(
        const std::string& expression,
        const std::unordered_map<std::string, MachineVariable>& variables,
        int elapsedTime);
         
 private:
    /**
     * @brief Checks if a character is a digit or decimal point
     * @param c Character to check
     * @return True if the character is a digit or decimal point
     */
    bool isDigit(char c);
    
    /**
     * @brief Checks if a character is a letter or underscore
     * @param c Character to check
     * @return True if the character is a letter or underscore
     */
    bool isLetter(char c);
    
    /**
     * @brief Checks if a character is a supported operator
     * @param c Character to check
     * @return True if the character is a supported operator
     */
    bool isOperator(char c);
    
    /**
     * @brief Extracts a sequence of digits (including decimal point) from an expression
     * @param expr The expression to parse
     * @param pos Current position in the expression, updated to end of the number
     * @return String containing the extracted number
     */
    std::string getDigits(const std::string& expr, size_t& pos);
    
    /**
     * @brief Extracts an identifier from an expression
     * @param expr The expression to parse
     * @param pos Current position in the expression, updated to end of the identifier
     * @return String containing the extracted identifier
     */
    std::string getIdentifier(const std::string& expr, size_t& pos);
    
    /**
     * @brief Skips whitespace characters in an expression
     * @param expr The expression to parse
     * @param pos Current position in the expression, updated to first non-whitespace
     */
    void skipWhitespace(const std::string& expr, size_t& pos);
 };
 
 #endif // EXPRESSION_PARSER_H