/**
 * @file state.cpp
 * @brief Implementation of the State class
 * @author Hugo Bohácsek (xbohach00)
 */

#include "../headers/state.h"
#include <algorithm>

/**
 * @brief Constructor with ID and name
 * 
 * @param id Unique identifier
 * @param name Display name
 */
State::State(const std::string& id, const std::string& name)
    : id(id), name(name), isInitial(false), position(0, 0) {}

/**
 * @brief Constructor with ID, name, and output
 * 
 * @param id Unique identifier
 * @param name Display name
 * @param output Output value
 */
State::State(const std::string& id, const std::string& name, const std::string& output)
    : id(id), name(name), isInitial(false), position(0, 0) {
    if (!output.empty()) {
        // todo: remove deps
        outputs.emplace_back(output, "default");
    }
}

/**
 * @brief Gets the state ID
 * 
 * @return The state ID
 */
std::string State::getId() const {
    return id;
}

/**
 * @brief Gets the state name
 * 
 * @return The state name
 */
std::string State::getName() const {
    return name;
}

/**
 * @brief Gets the state output (legacy)
 * 
 * @return The first output value, or empty string if none
 */
std::string State::getOutput() const {
    // todo: remove deps
    return outputs.empty() ? "" : outputs[0].value;
}

/**
 * @brief Gets all outputs for the state
 * 
 * @return Vector of output conditions
 */
std::vector<OutputCondition> State::getOutputs() const {
    return outputs;
}

/**
 * @brief Checks if this is the initial state
 * 
 * @return True if this is the initial state
 */
bool State::getIsInitial() const {
    return isInitial;
}

/**
 * @brief Gets the state position
 * 
 * @return Position as a Point
 */
Point State::getPosition() const {
    return position;
}

/**
 * @brief Sets the state name
 * 
 * @param name New name for the state
 */
void State::setName(const std::string& name) {
    this->name = name;
}

/**
 * @brief Sets the state output (legacy)
 * 
 * @param output Output value
 */
void State::setOutput(const std::string& output) {
    // todo: remove child and deps
    clearOutputs();
    if (!output.empty()) {
        outputs.emplace_back(output, "default");
    }
}

/**
 * @brief Adds an output condition
 * 
 * @param output Output condition to add
 */
void State::addOutput(const OutputCondition& output) {
    outputs.push_back(output);
}

/**
 * @brief Clears all outputs
 */
void State::clearOutputs() {
    outputs.clear();
}

/**
 * @brief Sets whether this is the initial state
 * 
 * @param initial Whether this should be the initial state
 */
void State::setIsInitial(bool initial) {
    this->isInitial = initial;
}

/**
 * @brief Sets the state position
 * 
 * @param position New position for the state
 */
void State::setPosition(const Point& position) {
    this->position = position;
}

/**
 * @brief Evaluates outputs based on available inputs
 * 
 * @param availableInputs Vector of input pointer/value pairs
 * @return Vector of output pointer/value pairs
 */
std::vector<std::pair<std::string, std::string>> State::evaluateOutputs(
    const std::vector<std::pair<std::string, std::string>>& availableInputs) const {
    
    std::vector<std::pair<std::string, std::string>> result;
    
    for (const auto& output : outputs) {
        // If no condition, always include this output
        if (!output.hasCondition) {
            result.emplace_back(output.target, output.value);
            continue;
        }
        
        // Check if the condition is satisfied
        for (const auto& input : availableInputs) {
            if (input.first == output.inputPtr && !input.second.empty()) {
                // Input is defined, add this output
                result.emplace_back(output.target, output.value);
                break;
            }
        }
    }
    
    return result;
}