/**
 * @file moore_machine.cpp
 * @brief Implementation of the MooreMachine class
 * @author Hugo Bohácsek (xbohach00)
 */

#include "../headers/moore_machine.h"
#include <algorithm>
#include <stdexcept>
#include <chrono>

using namespace std::chrono;

/**
 * @brief Constructor
 * 
 * Initializes a Moore machine with the given name
 * 
 * @param name Name of the machine
 */
MooreMachine::MooreMachine(const std::string& name) 
    : name(name), currentStateId(""), 
    stateEntryTime(steady_clock::now()) {}

/**
 * @brief Adds a state to the machine
 * 
 * @param state The state to add
 * @return True if the state was added successfully, false if a state with the same ID already exists
 */
bool MooreMachine::addState(const State& state) {
    // Check if state with same ID already exists
    if (states.find(state.getId()) != states.end()) {
        return false;
    }

    states[state.getId()] = state;

    // Add outputs to the output alphabet
    for (const auto& output : state.getOutputs()) {
        if (!output.value.empty()) {

            outputAlphabet.insert(output.value);
        }
        outputPointers.insert(output.target);
    }

    return true;
}

/**
 * @brief Removes a state from the machine
 * 
 * Also removes all transitions to and from this state
 * 
 * @param stateId ID of the state to remove
 * @return True if the state was removed successfully, false if it doesn't exist
 */
bool MooreMachine::removeState(const std::string& stateId) {
    // Check if state exists
    auto stateIt = states.find(stateId);
    if (stateIt == states.end()) {
        return false;
    }
    
    // Check if this is the initial state
    bool isInitial = stateIt->second.getIsInitial();
    
    // Remove all transitions to and from this state
    std::vector<std::string> transitionsToRemove;
    for (const auto& transition : transitions) {
        if (transition.second.getSourceId() == stateId ||
            transition.second.getTargetId() == stateId) {
            transitionsToRemove.push_back(transition.first);
        }
    }
    
    for (const auto& transitionId : transitionsToRemove) {
        transitions.erase(transitionId);
    }
    
    // Remove the state
    states.erase(stateId);
    
    // If we removed the initial state, try to set a new one - could be handled differently
    if (isInitial && !states.empty()) {
        auto newInitialState = states.begin();
        newInitialState->second.setIsInitial(true);
        currentStateId = newInitialState->first;
        stateEntryTime = steady_clock::now();
    } else if (states.empty()) {
        currentStateId = "";
    }
    
    return true;
}

/**
 * @brief Gets a state by ID
 * 
 * @param stateId ID of the state to get
 * @return Pointer to the state, or nullptr if not found
 */
State* MooreMachine::getState(const std::string& stateId) {
    auto it = states.find(stateId);
    if (it != states.end()) {
        return &(it->second);
    }
    return nullptr;
}

/**
 * @brief Gets a state by name
 * 
 * @param name Name of the state to get
 * @return Pointer to the state, or nullptr if not found
 */
State* MooreMachine::getStateByName(const std::string& name) {
    for (auto& statePair : states) {
        if (statePair.second.getName() == name) {
            return &(statePair.second);
        }
    }
    return nullptr;
}

/**
 * @brief Gets all states in the machine
 * 
 * @return Vector of pointers to all states
 */
std::vector<State*> MooreMachine::getAllStates() {
    std::vector<State*> result;
    for (auto& statePair : states) {
        result.push_back(&statePair.second);
    }
    return result;
}

/**
 * @brief Gets the initial state of the machine
 * 
 * @return Pointer to the initial state, or nullptr if not set
 */
State* MooreMachine::getInitialState() {
    for (auto& statePair : states) {
        if (statePair.second.getIsInitial()) {
            return &statePair.second;
        }
    }
    return nullptr;
}

/**
 * @brief Sets a state as the initial state
 * 
 * @param stateId ID of the state to set as initial
 * @return True if successful, false if the state doesn't exist
 */
bool MooreMachine::setInitialState(const std::string& stateId) {
    // Check if state exists
    auto stateIt = states.find(stateId);
    if (stateIt == states.end()) {
        return false;
    }
    
    // Clear the initial flag from all states
    for (auto& statePair : states) {
        statePair.second.setIsInitial(false);
    }
    
    // Set the new initial state
    stateIt->second.setIsInitial(true);
    currentStateId = stateId;
    stateEntryTime = steady_clock::now();
    
    return true;
}

/**
 * @brief Adds a transition to the machine
 * 
 * @param transition The transition to add
 * @return True if successful, false if a transition with the same ID already exists
 */
bool MooreMachine::addTransition(const Transition& transition) {
    // Check if transition with same ID already exists
    if (transitions.find(transition.getId()) != transitions.end()) {
        return false;
    }
    
    // Check if source and target states exist
    if (states.find(transition.getSourceId()) == states.end() ||
        states.find(transition.getTargetId()) == states.end()) {
        return false;
    }
    
    transitions[transition.getId()] = transition;
    
    // Add the inputs to the input alphabet and input pointers
    for (const auto& input : transition.getInputConditions()) {
        inputAlphabet.insert(input.value);
        inputPointers.insert(input.source);
    }
    
    return true;
}

/**
 * @brief Removes a transition from the machine
 * 
 * @param transitionId ID of the transition to remove
 * @return True if successful, false if the transition doesn't exist
 */
bool MooreMachine::removeTransition(const std::string& transitionId) {
    // Check if transition exists
    if (transitions.find(transitionId) == transitions.end()) {
        return false;
    }
    
    transitions.erase(transitionId);
    return true;
}

/**
 * @brief Gets a transition by ID
 * 
 * @param transitionId ID of the transition to get
 * @return Pointer to the transition, or nullptr if not found
 */
Transition* MooreMachine::getTransition(const std::string& transitionId) {
    auto it = transitions.find(transitionId);
    if (it != transitions.end()) {
        return &(it->second);
    }
    return nullptr;
}

/**
 * @brief Gets all transitions from a state
 * 
 * @param stateId ID of the source state
 * @return Vector of pointers to transitions from the state
 */
std::vector<Transition*> MooreMachine::getTransitionsFromState(const std::string& stateId) {
    std::vector<Transition*> result;
    for (auto& transitionPair : transitions) {
        if (transitionPair.second.getSourceId() == stateId) {
            result.push_back(&transitionPair.second);
        }
    }
    return result;
}

/**
 * @brief Gets all transitions to a state
 * 
 * @param stateId ID of the target state
 * @return Vector of pointers to transitions to the state
 */
std::vector<Transition*> MooreMachine::getTransitionsToState(const std::string& stateId) {
    std::vector<Transition*> result;
    for (auto& transitionPair : transitions) {
        if (transitionPair.second.getTargetId() == stateId) {
            result.push_back(&transitionPair.second);
        }
    }
    return result;
}

/**
 * @brief Adds an input symbol to the alphabet
 * 
 * @param symbol The symbol to add
 * @return True (always succeeds)
 */
bool MooreMachine::addInputSymbol(const std::string& symbol) {
    inputAlphabet.insert(symbol);
    return true;
}

/**
 * @brief Removes an input symbol from the alphabet
 * 
 * @param symbol The symbol to remove
 * @return True if successful, false if the symbol is in use by a transition
 */
bool MooreMachine::removeInputSymbol(const std::string& symbol) {
    // Check if the symbol is in use
    for (const auto& transition : transitions) {
        for (const auto& input : transition.second.getInputConditions()) {
            if (input.value == symbol) {
                return false;
            }
        }
    }
    
    inputAlphabet.erase(symbol);
    return true;
}

/**
 * @brief Adds an output symbol to the alphabet
 * 
 * @param symbol The symbol to add
 * @return True (always succeeds)
 */
bool MooreMachine::addOutputSymbol(const std::string& symbol) {
    outputAlphabet.insert(symbol);
    return true;
}

/**
 * @brief Removes an output symbol from the alphabet
 * 
 * @param symbol The symbol to remove
 * @return True if successful, false if the symbol is in use by a state
 */
bool MooreMachine::removeOutputSymbol(const std::string& symbol) {
    // Check if the symbol is in use
    for (const auto& state : states) {
        for (const auto& output : state.second.getOutputs()) {
            if (output.value == symbol) {
                return false;
            }
        }
    }
    
    outputAlphabet.erase(symbol);
    return true;
}

/**
 * @brief Gets the input alphabet
 * 
 * @return Set of input symbols
 */
std::set<std::string> MooreMachine::getInputAlphabet() const {
    return inputAlphabet;
}

/**
 * @brief Gets the output alphabet
 * 
 * @return Set of output symbols
 */
std::set<std::string> MooreMachine::getOutputAlphabet() const {
    return outputAlphabet;
}

/**
 * @brief Adds an input pointer
 * 
 * @param pointer The pointer to add
 * @return True (always succeeds)
 */
bool MooreMachine::addInputPointer(const std::string& pointer) {
    inputPointers.insert(pointer);
    return true;
}

/**
 * @brief Removes an input pointer
 * 
 * @param pointer The pointer to remove
 * @return True if successful, false if the pointer is in use by a transition
 */
bool MooreMachine::removeInputPointer(const std::string& pointer) {
    // Check if the pointer is in use
    for (const auto& transition : transitions) {
        for (const auto& input : transition.second.getInputConditions()) {
            if (input.source == pointer) {
                return false;
            }
        }
    }
    
    inputPointers.erase(pointer);
    return true;
}

/**
 * @brief Adds an output pointer
 * 
 * @param pointer The pointer to add
 * @return True (always succeeds)
 */
bool MooreMachine::addOutputPointer(const std::string& pointer) {
    outputPointers.insert(pointer);
    return true;
}

/**
 * @brief Removes an output pointer
 * 
 * @param pointer The pointer to remove
 * @return True if successful, false if the pointer is in use by a state
 */
bool MooreMachine::removeOutputPointer(const std::string& pointer) {
    // Check if the pointer is in use
    for (const auto& state : states) {
        for (const auto& output : state.second.getOutputs()) {
            if (output.target == pointer) {
                return false;
            }
        }
    }
    
    outputPointers.erase(pointer);
    return true;
}

/**
 * @brief Gets the input pointers
 * 
 * @return Set of input pointers
 */
std::set<std::string> MooreMachine::getInputPointers() const {
    return inputPointers;
}

/**
 * @brief Gets the output pointers
 * 
 * @return Set of output pointers
 */
std::set<std::string> MooreMachine::getOutputPointers() const {
    return outputPointers;
}

/**
 * @brief Adds a variable
 * 
 * @param type Type of the variable (int, float, string)
 * @param name Name of the variable
 * @param value Initial value of the variable
 * @return True if successful, false if the variable couldn't be created
 */
bool MooreMachine::addVariable(const std::string& type, const std::string& name, const std::string& value) {
    try {
        variables[name] = MachineVariable(type, name, value);
        initialVariableValues[name] = value;
        return true;
    } catch (const std::exception& e) {
        return false;
    }
}

/**
 * @brief Sets a variable
 *
 * @param type Type of the variable (int, float, string)
 * @param name Name of the variable
 * @param value Value of the variable
 * @return True if successful, false if the variable couldn't be created
 */
void MooreMachine::setVariable(const std::string& type, const std::string& name, const std::string& value) {
    variables[name] = MachineVariable(type, name, value);
}

/**
 * @brief Removes a variable
 * 
 * @param name Name of the variable to remove
 * @return True if successful, false if the variable doesn't exist
 */
bool MooreMachine::removeVariable(const std::string& name) {
    return variables.erase(name) > 0;
}

/**
 * @brief Gets a variable
 * 
 * @param name Name of the variable to get
 * @return Pointer to the variable, or nullptr if not found
 */
MachineVariable* MooreMachine::getVariable(const std::string& name) {
    auto it = variables.find(name);
    if (it != variables.end()) {
        return &(it->second);
    }
    return nullptr;
}

/**
 * @brief Gets all variables
 * 
 * @return Map of variables indexed by name
 */
std::unordered_map<std::string, MachineVariable> MooreMachine::getVariables() const {
    return variables;
}

/**
 * @brief Evaluates an expression
 * 
 * @param expression The expression to evaluate
 * @param elapsedTime Current elapsed time in milliseconds
 * @return Result of the evaluation
 */
MachineVariable MooreMachine::evaluateExpression(const std::string& expression, int elapsedTime) {
    return expressionParser.evaluateExpression(expression, variables, elapsedTime);
}

/**
 * @brief Parses an output expression
 * 
 * @param expression The expression to parse (e.g., "output var + 5 to output1")
 * @param elapsedTime Current elapsed time in milliseconds
 * @return Pair of output pointer and value
 */
std::pair<std::string, std::string> MooreMachine::parseOutputExpression(const std::string& expression, int elapsedTime) {
    return expressionParser.parseOutput(expression, variables, elapsedTime);
}

/**
 * @brief Checks if the machine is valid
 * 
 * @return True if the machine is valid, false otherwise
 */
bool MooreMachine::isValid() const {
    return getValidationErrors().empty();
}

/**
 * @brief Gets validation errors
 * 
 * @return Vector of error messages if the machine is invalid
 */
std::vector<std::string> MooreMachine::getValidationErrors() const {
    std::vector<std::string> errors;
    
    // Check if there's at least one state
    if (states.empty()) {
        errors.push_back("Machine has no states");
    }
    
    // Check if there's an initial state
    bool hasInitial = false;
    for (const auto& state : states) {
        if (state.second.getIsInitial()) {
            hasInitial = true;
            break;
        }
    }
    
    if (!hasInitial && !states.empty()) {
        errors.push_back("Machine has no initial state");
    }
    
    // Check if all transitions are valid
    for (const auto& transition : transitions) {
        if (states.find(transition.second.getSourceId()) == states.end()) {
            errors.push_back("Transition " + transition.first + 
                            " has invalid source state: " + transition.second.getSourceId());
        }
        
        if (states.find(transition.second.getTargetId()) == states.end()) {
            errors.push_back("Transition " + transition.first + 
                            " has invalid target state: " + transition.second.getTargetId());
        }
    }
    
    // Check if all input pointers are declared
    for (const auto& transition : transitions) {
        for (const auto& input : transition.second.getInputConditions()) {
            if (inputPointers.find(input.source) == inputPointers.end()) {
                errors.push_back("Transition " + transition.first + 
                               " uses undeclared input pointer: " + input.source);
            }
        }
    }
    
    // Check if all output pointers are declared
    for (const auto& state : states) {
        for (const auto& output : state.second.getOutputs()) {
            if (outputPointers.find(output.target) == outputPointers.end()) {
                errors.push_back("State " + state.first + 
                               " uses undeclared output pointer: " + output.target);
            }
        }
    }
    
    return errors;
}

/**
 * @brief Processes an input through the machine
 * 
 * @param input Input symbol to process
 * @return Output produced by the new state
 * @throw std::runtime_error If the machine is not in a valid state
 */
std::string MooreMachine::processInput(const std::string& input) {
    // Translate simple inputs
    if (currentStateId.empty() || states.find(currentStateId) == states.end()) {
        throw std::runtime_error("Machine is not in a valid state");
    }
    
    // Find a transition from the current state with the given input
    for (const auto& transitionPair : transitions) {
        if (transitionPair.second.getSourceId() == currentStateId) {
            for (const auto& inputCondition : transitionPair.second.getInputConditions()) {
                if (inputCondition.source == "default" && inputCondition.value == input) {
                    // Record old state for comparing
                    std::string oldStateId = currentStateId;
                    
                    // Transition to the next state
                    currentStateId = transitionPair.second.getTargetId();
                    
                    // If we changed state, reset the timer
                    if (oldStateId != currentStateId) {
                        stateEntryTime = steady_clock::now();
                    }
                    
                    // Return the output of the new state
                    return states[currentStateId].getOutput();
                }
            }
        }
    }
    
    // If no transition found, stay in the current state
    return states[currentStateId].getOutput();
}

/**
 * @brief Resets the machine to its initial state
 */
void MooreMachine::reset() {
    // Reset to the initial state
    for (const auto& statePair : states) {
        if (statePair.second.getIsInitial()) {
            currentStateId = statePair.first;
            stateEntryTime = steady_clock::now();
            return;
        }
    }
    
    // If no initial state found, set to empty
    currentStateId = "";
}

/**
 * @brief Gets the elapsed time in the current state
 * 
 * @return Time in milliseconds since entering the current state
 */
int MooreMachine::getElapsedTimeInState() const {
    if (currentStateId.empty()) {
        return 0;
    }
    
    auto now = steady_clock::now();
    return duration_cast<milliseconds>(now - stateEntryTime).count();
}

/**
 * @brief Gets the name of the machine
 * 
 * @return Name of the machine
 */
std::string MooreMachine::getName() const {
    return name;
}

/**
 * @brief Sets the name of the machine
 * 
 * @param name New name for the machine
 */
void MooreMachine::setName(const std::string& name) {
    this->name = name;
}

/**
 * @brief Processes an input through the machine on a specific input pointer
 * 
 * @param input Input symbol to process
 * @param inputPtr Input pointer to use
 * @return Output produced by the new state
 * @throw std::runtime_error If the machine is not in a valid state
 */
std::string MooreMachine::processInputOnPointer(const std::string& input, const std::string& inputPtr) {
    // Check if the machine is in a valid state
    if (currentStateId.empty() || states.find(currentStateId) == states.end()) {
        throw std::runtime_error("Machine is not in a valid state");
    }
    
    // Check if the input pointer exists
    if (inputPointers.find(inputPtr) == inputPointers.end()) {
        throw std::runtime_error("Invalid input pointer: " + inputPtr);
    }
    
    // Find a transition from the current state with the given input on the given pointer
    for (const auto& transitionPair : transitions) {
        if (transitionPair.second.getSourceId() == currentStateId) {
            for (const auto& inputCondition : transitionPair.second.getInputConditions()) {
                if (inputCondition.source == inputPtr && inputCondition.value == input) {
                    // Record old state for comparing
                    std::string oldStateId = currentStateId;
                    
                    // Transition to the next state
                    currentStateId = transitionPair.second.getTargetId();
                    
                    // If we changed state, reset the timer
                    if (oldStateId != currentStateId) {
                        stateEntryTime = std::chrono::steady_clock::now();
                    }
                    
                    // Return the output of the new state
                    return states[currentStateId].getOutput();
                }
            }
        }
    }
    
    // If no transition found, stay in the current state
    return states[currentStateId].getOutput();
}

/**
 * @brief Resets all variables to their initial values
 */
void MooreMachine::resetVariables(void) {
    for (auto& pair : initialVariableValues) {
        const std::string& name = pair.first;
        const std::string& value = pair.second;
        
        auto it = variables.find(name);
        if (it != variables.end()) {
            it->second.setValue(value);
        }
    }
}
