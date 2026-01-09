/**
 * @file transition.h
 * @brief Declaration of the Transition class and InputCondition structure
 * @author Hugo Bohácsek (xbohach00)
 */

#ifndef TRANSITION_H
#define TRANSITION_H

#include <string>
#include <vector>
#include <unordered_map> 
#include "machine_variable.h"

/**
 * @struct InputCondition
 * @brief Represents an input condition for a transition
 * 
 * Specifies a value and source (input pointer) that triggers a transition
 * or a boolean expression involving variables
 */
struct InputCondition {
    std::string value;       /**< Expected value */
    std::string source;      /**< Input pointer (source) */
    
    // Boolean expression support
    bool isBooleanExpr;      /**< Whether this is a boolean expression */
    std::string leftOperand;  /**< Left side of the expression */
    std::string rightOperand; /**< Right side of the expression */
    std::string operation;    /**< Operation type: == or != */
    
    /**
     * @brief Constructor with value and source
     * 
     * @param value Expected value
     * @param source Input pointer
     */
    InputCondition(const std::string& value, const std::string& source)
        : value(value), source(source), isBooleanExpr(false) {}
    
    /**
     * @brief Default constructor
     * 
     * Creates an empty condition with "default" source
     */
    InputCondition() : value(""), source("default"), isBooleanExpr(false) {}
    
    /**
     * @brief Constructor with only value
     * 
     * Creates a condition with "default" source
     * 
     * @param value Expected value
     */
    InputCondition(const std::string& value)
        : value(value), source("default"), isBooleanExpr(false) {}
        
    /**
     * @brief Constructor for boolean expression
     * 
     * @param leftOp Left operand
     * @param op Operation (== or !=)
     * @param rightOp Right operand
     */
    InputCondition(const std::string& leftOp, const std::string& op, const std::string& rightOp)
        : isBooleanExpr(true), leftOperand(leftOp), rightOperand(rightOp), operation(op) {}
};

/**
 * @class Transition
 * @brief Represents a transition in a finite state machine
 * 
 * Transitions connect states and are triggered by input conditions.
 * They can also be triggered by timeouts or boolean expressions.
 */
class Transition {
private:
    std::string id;           /**< Unique identifier */
    std::string sourceId;     /**< ID of source state */
    std::string targetId;     /**< ID of target state */
    std::vector<InputCondition> inputs; /**< Input conditions that trigger this transition */
    int timeout;              /**< Timeout in milliseconds (0 = no timeout) */
    
public:
    /**
     * @brief Default constructor
     * 
     * Creates an empty transition
     */
    Transition() : id(""), sourceId(""), targetId(""), timeout(0) {}
    
    /**
     * @brief Constructor with single input
     * 
     * @param id Unique identifier
     * @param sourceId ID of source state
     * @param targetId ID of target state
     * @param input Single input value
     */
    Transition(const std::string& id, const std::string& sourceId, 
            const std::string& targetId, const std::string& input);
            
    /**
     * @brief Constructor with multiple inputs
     * 
     * @param id Unique identifier
     * @param sourceId ID of source state
     * @param targetId ID of target state
     * @param inputs Vector of input values
     * @param timeout Timeout in milliseconds (0 = no timeout)
     */
    Transition(const std::string& id, const std::string& sourceId, 
            const std::string& targetId, const std::vector<std::string>& inputs, int timeout = 0);
    
    /**
     * @brief Constructor with input conditions
     * 
     * @param id Unique identifier
     * @param sourceId ID of source state
     * @param targetId ID of target state
     * @param inputs Vector of input conditions
     * @param timeout Timeout in milliseconds (0 = no timeout)
     */
    Transition(const std::string& id, const std::string& sourceId, 
            const std::string& targetId, const std::vector<InputCondition>& inputs, int timeout = 0);
    
    /**
     * @brief Gets the transition ID
     * 
     * @return The transition ID
     */
    std::string getId() const;
    
    /**
     * @brief Gets the source state ID
     * 
     * @return The source state ID
     */
    std::string getSourceId() const;
    
    /**
     * @brief Gets the target state ID
     * 
     * @return The target state ID
     */
    std::string getTargetId() const;
    
    /**
     * @brief Gets all input conditions
     * 
     * @return Vector of input conditions
     */
    std::vector<InputCondition> getInputConditions() const;
    
    /**
     * @brief Gets all input values (legacy)
     * 
     * @return Vector of input values
     */
    std::vector<std::string> getInputs() const;
    
    /**
     * @brief Gets the first input value
     * 
     * @return The first input value, or empty string if none
     */
    std::string getFirstInput() const;
    
    /**
     * @brief Gets the timeout
     * 
     * @return Timeout in milliseconds (0 = no timeout)
     */
    int getTimeout() const;
    
    /**
     * @brief Adds an input value
     * 
     * @param input Input value to add
     */
    void addInput(const std::string& input);
    
    /**
     * @brief Adds an input condition
     * 
     * @param condition Input condition to add
     */
    void addInputCondition(const InputCondition& condition);
    
    /**
     * @brief Adds a boolean expression condition
     * 
     * @param leftOp Left operand (variable name)
     * @param op Operation (== or !=)
     * @param rightOp Right operand (variable name or literal)
     */
    void addBooleanCondition(const std::string& leftOp, const std::string& op, const std::string& rightOp);
    
    /**
     * @brief Sets input values (legacy)
     * 
     * @param inputs Vector of input values
     */
    void setInputs(const std::vector<std::string>& inputs);
    
    /**
     * @brief Sets input conditions
     * 
     * @param conditions Vector of input conditions
     */
    void setInputConditions(const std::vector<InputCondition>& conditions);
    
    /**
     * @brief Sets the timeout
     * 
     * @param timeout Timeout in milliseconds (0 = no timeout)
     */
    void setTimeout(int timeout);
    
    /**
     * @brief Checks if this transition is triggered by the given inputs
     * 
     * @param availableInputs Vector of input pointer/value pairs
     * @return True if the transition is triggered
     */
    bool isTriggered(const std::vector<std::pair<std::string, std::string>>& availableInputs) const;
    
    /**
     * @brief Checks if this transition is triggered by the given inputs and variable values
     * 
     * @param availableInputs Vector of input pointer/value pairs
     * @param variables Map of available variables
     * @return True if the transition is triggered
     */
    bool isTriggered(
        const std::vector<std::pair<std::string, std::string>>& availableInputs,
        const std::unordered_map<std::string, MachineVariable>& variables) const;
};

#endif // TRANSITION_H