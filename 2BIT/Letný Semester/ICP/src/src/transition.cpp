/**
 * @file transition.cpp
 * @brief Implementation of the Transition class
 * @author Hugo Bohácsek (xbohach00)
 */

#include "../headers/transition.h"
#include <algorithm>

/**
 * @brief Constructor with single input
 * 
 * @param id Unique identifier
 * @param sourceId ID of source state
 * @param targetId ID of target state
 * @param input Single input value
 */
Transition::Transition(const std::string& id, const std::string& sourceId, 
                      const std::string& targetId, const std::string& input)
    : id(id), sourceId(sourceId), targetId(targetId), timeout(0) {
    // todo: skip
    inputs.emplace_back(input, "default");
}

/**
 * @brief Constructor with multiple inputs
 * 
 * @param id Unique identifier
 * @param sourceId ID of source state
 * @param targetId ID of target state
 * @param legacyInputs Vector of input values
 * @param timeout Timeout in milliseconds (0 = no timeout)
 */
Transition::Transition(const std::string& id, const std::string& sourceId, 
                      const std::string& targetId, const std::vector<std::string>& legacyInputs, int timeout)
    : id(id), sourceId(sourceId), targetId(targetId), timeout(timeout) {
    for (const auto& input : legacyInputs) {
        inputs.emplace_back(input, "default");
    }
}

/**
 * @brief Constructor with input conditions
 * 
 * @param id Unique identifier
 * @param sourceId ID of source state
 * @param targetId ID of target state
 * @param inputs Vector of input conditions
 * @param timeout Timeout in milliseconds (0 = no timeout)
 */
Transition::Transition(const std::string& id, const std::string& sourceId, 
                      const std::string& targetId, const std::vector<InputCondition>& inputs, int timeout)
    : id(id), sourceId(sourceId), targetId(targetId), inputs(inputs), timeout(timeout) {
}

/**
 * @brief Gets the transition ID
 * 
 * @return The transition ID
 */
std::string Transition::getId() const {
    return id;
}

/**
 * @brief Gets the source state ID
 * 
 * @return The source state ID
 */
std::string Transition::getSourceId() const {
    return sourceId;
}

/**
 * @brief Gets the target state ID
 * 
 * @return The target state ID
 */
std::string Transition::getTargetId() const {
    return targetId;
}

/**
 * @brief Gets all input conditions
 * 
 * @return Vector of input conditions
 */
std::vector<InputCondition> Transition::getInputConditions() const {
    return inputs;
}

/**
 * @brief Gets all input values (legacy)
 * 
 * @return Vector of input values
 */
std::vector<std::string> Transition::getInputs() const {
    // todo: reset
    std::vector<std::string> result;
    for (const auto& input : inputs) {
        result.push_back(input.value);
    }
    return result;
}

/**
 * @brief Gets the first input value
 * 
 * @return The first input value, or empty string if none
 */
std::string Transition::getFirstInput() const {
    return inputs.empty() ? "" : inputs[0].value;
}

/**
 * @brief Gets the timeout
 * 
 * @return Timeout in milliseconds (0 = no timeout)
 */
int Transition::getTimeout() const {
    return timeout;
}

/**
 * @brief Adds an input value
 * 
 * @param input Input value to add
 */
void Transition::addInput(const std::string& input) {
    // todo: merge
    inputs.emplace_back(input, "default");
}

/**
 * @brief Adds an input condition
 * 
 * @param condition Input condition to add
 */
void Transition::addInputCondition(const InputCondition& condition) {
    inputs.push_back(condition);
}

/**
 * @brief Sets input values (legacy)
 * 
 * @param legacyInputs Vector of input values
 */
void Transition::setInputs(const std::vector<std::string>& legacyInputs) {
    // todo: constructor handling
    inputs.clear();
    for (const auto& input : legacyInputs) {
        inputs.emplace_back(input, "default");
    }
}

/**
 * @brief Sets input conditions
 * 
 * @param conditions Vector of input conditions
 */
void Transition::setInputConditions(const std::vector<InputCondition>& conditions) {
    inputs = conditions;
}

/**
 * @brief Sets the timeout
 * 
 * @param timeout Timeout in milliseconds (0 = no timeout)
 */
void Transition::setTimeout(int timeout) {
    this->timeout = timeout;
}

/**
 * @brief Checks if this transition is triggered by the given inputs and variable values
 * 
 * @param availableInputs Vector of input pointer/value pairs
 * @param variables Map of available variables
 * @return True if the transition is triggered
 */
bool Transition::isTriggered(
    const std::vector<std::pair<std::string, std::string>>& availableInputs,
    const std::unordered_map<std::string, MachineVariable>& variables) const {
    
    // Check each input condition
    for (const auto& inputCondition : inputs) {
        // Check if this is a boolean expression
        if (inputCondition.isBooleanExpr) {
            // Get the left operand value (must be a variable)
            auto leftVarIt = variables.find(inputCondition.leftOperand);
            if (leftVarIt == variables.end()) {
                continue; // Variable not found, condition not met
            }
            
            std::string leftValue = leftVarIt->second.getValueString();
            
            // Get the right operand value (could be a literal or a variable)
            std::string rightValue = inputCondition.rightOperand;
            auto rightVarIt = variables.find(inputCondition.rightOperand);
            if (rightVarIt != variables.end()) {
                rightValue = rightVarIt->second.getValueString();
            }
            
            // Evaluate the expression
            bool conditionMet = false;
            if (inputCondition.operation == "==") {
                conditionMet = (leftValue == rightValue);
            } else if (inputCondition.operation == "!=") {
                conditionMet = (leftValue != rightValue);
            }
            
            if (conditionMet) {
                return true;
            }
        }
        else {
            // Look for a matching input pointer with the expected value
            for (const auto& availableInput : availableInputs) {
                if (availableInput.first == inputCondition.source && 
                    availableInput.second == inputCondition.value) {
                    return true; // Any matching input condition triggers the transition
                }
            }
        }
    }
    
    return false;
}

/**
 * @brief Adds a boolean expression condition
 * 
 * @param leftOp Left operand (variable name)
 * @param op Operation (== or !=)
 * @param rightOp Right operand (variable name or literal)
 */
void Transition::addBooleanCondition(const std::string& leftOp, const std::string& op, const std::string& rightOp) {
    InputCondition condition;
    condition.isBooleanExpr = true;
    condition.leftOperand = leftOp;
    condition.operation = op;
    condition.rightOperand = rightOp;
    
    inputs.push_back(condition);
}

/**
 * @brief Checks if this transition is triggered by the given inputs
 * 
 * Legacy method that doesn't support boolean expressions.
 * 
 * @param availableInputs Vector of input pointer/value pairs
 * @return True if the transition is triggered by standard inputs
 */
bool Transition::isTriggered(const std::vector<std::pair<std::string, std::string>>& availableInputs) const {
    // Check each input condition
    for (const auto& inputCondition : inputs) {
        // Skip boolean expressions in legacy mode
        if (inputCondition.isBooleanExpr) {
            continue;
        }
        
        // Look for a matching input pointer with the expected value
        for (const auto& availableInput : availableInputs) {
            if (availableInput.first == inputCondition.source && 
                availableInput.second == inputCondition.value) {
                return true; // Any matching input condition triggers the transition
            }
        }
    }
    
    return false;
}