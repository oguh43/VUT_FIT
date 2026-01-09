/**
 * @file machine_simulator.cpp
 * @brief Implementation of the MachineSimulator class
 * @author Hugo Bohácsek (xbohach00)
 */

#include "../headers/machine_simulator.h"
#include <stdexcept>
#include <algorithm>
#include <sstream>
#include "../headers/expression_parser.h"
#include "../headers/string_utils.h"

using namespace std::chrono;

/**
 * @brief Constructor
 * 
 * Initializes the simulator with a Moore machine and resets to initial state
 * 
 * @param machine Pointer to the Moore machine to simulate
 * @throw std::runtime_error If the machine has no states or no initial state
 */
MachineSimulator::MachineSimulator(MooreMachine* machine) 
    : machine(machine), currentStateId(""),
      lastTransitionTime(steady_clock::now()),
      justEnteredState(true) {
    reset();
}

/**
 * @brief Resets the simulation to initial state
 * 
 * Transitions to the initial state, clears all inputs and outputs, and resets the timer
 * 
 * @throw std::runtime_error If the machine has no states or no initial state
 */
void MachineSimulator::reset() {
    if (!machine) {
        throw std::runtime_error("No machine assigned to simulator");
    }
    
    // Find the initial state
    State* initialState = machine->getInitialState();
    if (!initialState) {
        // If no initial state is set, set the first state as initial
        auto allStates = machine->getAllStates();
        if (!allStates.empty()) {
            initialState = allStates[0];
            machine->setInitialState(initialState->getId());
        } else {
            throw std::runtime_error("Machine has no states");
        }
    }
    
    currentStateId = initialState->getId();
    lastTransitionTime = steady_clock::now();
    justEnteredState = true;  // We just entered the initial state
    
    // Clear all inputs, outputs, timeouts
    inputValues.clear();
    outputValues.clear();
    timeoutActive.clear();
    timeoutStartTimes.clear();
    
    // Process initial state outputs
    if (initialState) {
        processStateOutputs(initialState);
    }
}

/**
 * @brief Sets an input value for a specific input pointer
 * 
 * @param inputPtr The input pointer name
 * @param value The value to set
 */
void MachineSimulator::setInput(const std::string& inputPtr, const std::string& value) {
    inputValues[inputPtr] = value;
    justEnteredState = false;
}

/**
 * @brief Gets the current value of an input pointer
 * 
 * @param inputPtr The input pointer name
 * @return Current value, or empty string if not set
 */
std::string MachineSimulator::getInput(const std::string& inputPtr) const {
    auto it = inputValues.find(inputPtr);
    if (it != inputValues.end()) {
        return it->second;
    }
    return "";
}

/**
 * @brief Gets all current input values
 * 
 * @return Vector of input pointer/value pairs
 */
std::vector<std::pair<std::string, std::string>> MachineSimulator::getAllInputs() const {
    std::vector<std::pair<std::string, std::string>> result;
    for (const auto& input : inputValues) {
        result.emplace_back(input.first, input.second);
    }
    return result;
}

/**
 * @brief Gets the current value of an output pointer
 * 
 * @param outputPtr The output pointer name
 * @return Current value, or empty string if not set
 */
std::string MachineSimulator::getOutput(const std::string& outputPtr) const {
    auto it = outputValues.find(outputPtr);
    if (it != outputValues.end()) {
        return it->second;
    }
    return "";
}

/**
 * @brief Gets all current output values
 * 
 * @return Vector of output pointer/value pairs
 */
std::vector<std::pair<std::string, std::string>> MachineSimulator::getAllOutputs() const {
    std::vector<std::pair<std::string, std::string>> result;
    for (const auto& output : outputValues) {
        result.emplace_back(output.first, output.second);
    }
    return result;
}

/**
 * @brief Processes current inputs and updates state and outputs
 * 
 * Checks for timeouts, then checks if any transition from the current state
 * is triggered by the current input values or boolean expressions. If a transition 
 * is triggered, moves to the target state and updates outputs.
 * 
 * @throw std::runtime_error If the machine is not in a valid state
 */
void MachineSimulator::processInputs() {
    if (!machine) {
        throw std::runtime_error("No machine assigned to simulator");
    }
    
    // Get the current state
    State* currentState = machine->getState(currentStateId);
    if (!currentState) {
        throw std::runtime_error("Invalid current state");
    }
    
    // If we just entered this state, we've already processed outputs
    // and shouldn't check for transitions yet
    if (justEnteredState) {
        justEnteredState = false;
        return;
    }
    
    // Check and process any expired timeouts first
    bool timeoutTriggered = checkTimeouts();
    if (timeoutTriggered) {
        return; // Already transitioned due to timeout
    }
    
    // Get all transitions from the current state
    std::vector<Transition*> transitions = machine->getTransitionsFromState(currentStateId);
    auto variables = machine->getVariables();
    
    // First, check for transitions with boolean conditions or input conditions
    for (Transition* transition : transitions) {
        // Skip timeout transitions - they're handled in checkTimeouts
        if (transition->getTimeout() > 0 && transition->getInputConditions().empty()) {
            continue;
        }
        
        // Check if this transition is triggered by condition
        if (transition->isTriggered(getAllInputs(), variables)) {
            // If transition has both input conditions and timeout
            if (transition->getTimeout() > 0) {
                std::string transId = transition->getId();
                
                // If timeout not already active, start it
                if (!timeoutActive[transId]) {
                    timeoutActive[transId] = true;
                    timeoutStartTimes[transId] = steady_clock::now();
                }
                continue; // Wait for timeout to expire
            }
            
            // Immediate transition (no timeout)
            // Transition to the target state
            std::string oldStateId = currentStateId;
            currentStateId = transition->getTargetId();
            
            // Clear inputs that triggered this transition
            clearTriggeredInputs(transition);
            
            // Reset timer and mark that we just entered a new state
            lastTransitionTime = steady_clock::now();
            justEnteredState = true;
            
            // Clear all timeouts when changing states
            timeoutActive.clear();
            timeoutStartTimes.clear();
            
            // Get the new state and process its outputs
            State* newState = machine->getState(currentStateId);
            if (newState) {
                processStateOutputs(newState);
            }
            
            return;
        }
    }
    
    // Now check for transitions with input conditions that need timeouts
    for (Transition* transition : transitions) {
        int timeout = transition->getTimeout();
        if (timeout <= 0) continue;
        
        // If this transition has input conditions that are met
        if (transition->isTriggered(getAllInputs(), variables)) {
            std::string transId = transition->getId();
            
            // If timeout not already active, start it
            if (!timeoutActive[transId]) {
                timeoutActive[transId] = true;
                timeoutStartTimes[transId] = steady_clock::now();
            }
            // Timeout is active but not yet expired - continue waiting
        }
        else {
            // Input conditions not met, deactivate any existing timeout
            timeoutActive[transition->getId()] = false;
        }
    }
}

/**
 * @brief Processes outputs from a state
 * 
 * Evaluates all output conditions from the given state and updates
 * output values accordingly. Handles different types of outputs:
 * - Variable assignments (var = get input1)
 * - Output expressions (output var + 5 to output1)
 * - Conditional outputs (output val to output1 if input1 is defined)
 * 
 * @param state The state to process outputs from
 */
void MachineSimulator::processStateOutputs(State* state) {
    if (!state || !machine) {
        return; // I dare you to trigger this
    }
    
    // Calculate elapsed time in this state
    int elapsedTime = machine->getElapsedTimeInState(); // elapsed kw
    
    // Get all output expressions from the state
    auto outputs = state->getOutputs();
    ExpressionParser parser;
    
    for (const auto& output : outputs) {
        // Skip if there's a condition and it's not satisfied
        if (output.hasCondition) {
            bool conditionMet = parser.parseCondition("if " + output.inputPtr + " is defined", inputValues);
            if (!conditionMet) {
                continue;
            }
        }
        
        // Handle variable assignments
        if (output.value.find('=') != std::string::npos && output.target == "expression") {
            // Extract assignment
            size_t equalsPos = output.value.find('=');
            if (equalsPos == std::string::npos) continue;
            
            std::string varName = trim(output.value.substr(0, equalsPos));
            std::string expression = trim(output.value.substr(equalsPos + 1));
            
            // Check if this is a "get" expression
            if (expression.find("get ") == 0) {
                auto result = parser.parseAssignment(output.value, inputValues);
                
                // Update the variable in the machine
                MachineVariable* var = machine->getVariable(result.first);
                if (var) {
                    var->setValue(result.second);
                }
            }
            // Otherwise, evaluate as arithmetic expression
            else {
                // Evaluate the right side of the assignment
                MachineVariable result = machine->evaluateExpression(expression, elapsedTime);
                
                // Update the variable in the machine
                MachineVariable* var = machine->getVariable(varName);
                if (var) {
                    var->setValue(result.getValueString());
                }
            }
        }
        // Handle output expressions
        else if (output.value.find("output ") == 0) {
            // Parse the output expression
            auto result = machine->parseOutputExpression(output.value, elapsedTime);
            
            // Update the output value
            outputValues[result.first] = result.second;
        }
        // Regular output
        else {
            MachineVariable* var = machine->getVariable(output.value);
            if (var) {
                outputValues[output.target] = var->getValueString();
            } else {
                outputValues[output.target] = output.value;
            }
        }
    }
}

/**
 * @brief Processes a single input symbol
 * 
 * A simpler interface that sets the input to the default input pointer
 * and processes it.
 * 
 * @param symbol The input symbol to process
 * @return The output produced on the default output pointer
 */
std::string MachineSimulator::processSymbol(const std::string& symbol) {
    // Translate simpler symbols
    
    // Set the symbol as the default input
    setInput("default", symbol);
    
    // Process inputs
    processInputs();
    
    // Return the default output, if any
    return getOutput("default");
}

/**
 * @brief Processes a sequence of input symbols
 * 
 * Processes each symbol in sequence and concatenates the outputs.
 * 
 * @param sequence The input sequence to process
 * @return The concatenated output sequence
 */
std::string MachineSimulator::processSequence(const std::string& sequence) {
    std::string result;
    
    // Process each symbol in the sequence
    for (size_t i = 0; i < sequence.length(); ++i) {
        std::string symbol(1, sequence[i]);
        std::string output = processSymbol(symbol);
        
        // Append the output
        result += output;
    }
    
    return result;
}

/**
 * @brief Gets the current state
 * 
 * @return Pointer to the current state, or nullptr if not in a valid state
 */
State* MachineSimulator::getCurrentState() const {
    if (!machine) {
        return nullptr;
    }
    
    return machine->getState(currentStateId);
}

/**
 * @brief Checks for timeout transitions
 * 
 * Checks if any timeout transition from the current state has expired,
 * and if so, performs the transition. Also evaluates boolean expressions.
 * 
 * @return True if a timeout transition was triggered, false otherwise
 */
bool MachineSimulator::checkTimeouts() {
    auto now = steady_clock::now();
    
    // First check transitions with both input conditions and timeouts
    for (auto it = timeoutActive.begin(); it != timeoutActive.end(); ) {
        if (!it->second) {
            ++it;
            continue; // Skip inactive timeouts
        }
        
        std::string transitionId = it->first;
        Transition* transition = machine->getTransition(transitionId);
        
        if (!transition || transition->getSourceId() != currentStateId) {
            // Invalid or not from current state
            it = timeoutActive.erase(it);
            timeoutStartTimes.erase(transitionId);
            continue;
        }
        
        // Check if timeout has expired
        auto startTime = timeoutStartTimes[transitionId];
        auto elapsed = duration_cast<milliseconds>(now - startTime).count();
        
        if (elapsed >= transition->getTimeout()) {
            // Timeout expired - make the transition
            std::string oldStateId = currentStateId;
            currentStateId = transition->getTargetId();
            
            // Clear inputs that triggered this transition
            clearTriggeredInputs(transition);
            
            // Reset timer, mark that we just entered state, clear timeout data
            lastTransitionTime = now;
            justEnteredState = true;
            timeoutActive.clear();
            timeoutStartTimes.clear();
            
            // Process outputs from new state
            State* newState = machine->getState(currentStateId);
            if (newState) {
                processStateOutputs(newState);
            }
            
            return true; // We made a transition
        }
        
        ++it;
    }
    
    // Then check pure timeout transitions (no input conditions)
    std::vector<Transition*> transitions = machine->getTransitionsFromState(currentStateId);
    for (Transition* transition : transitions) {
        int timeout = transition->getTimeout();
        if (timeout <= 0 || !transition->getInputConditions().empty()) {
            continue; // Skip non-timeout transitions or those with input conditions
        }
        
        // Calculate elapsed time since entering this state
        auto elapsed = duration_cast<milliseconds>(now - lastTransitionTime).count();
        
        if (elapsed >= timeout) {
            // Timeout triggered, transition to target state
            std::string oldStateId = currentStateId;
            currentStateId = transition->getTargetId();
            
            // Reset timer, mark state entry, clear timeouts
            lastTransitionTime = now;
            justEnteredState = true;
            timeoutActive.clear();
            timeoutStartTimes.clear();
            
            // Process outputs from new state
            State* newState = machine->getState(currentStateId);
            if (newState) {
                processStateOutputs(newState);
            }
            
            return true; // We made a transition
        }
    }
    
    return false; // No timeout transitions triggered
}

/**
 * @brief Clears input values that triggered a transition
 * 
 * When a transition is triggered by input conditions (e.g., "got button_press from pedestrian_button"),
 * this function clears those inputs from the input values map to prevent them from
 * triggering additional transitions in subsequent simulation cycles.
 * 
 * Only non-boolean input conditions are cleared; boolean expression conditions
 * (that refer to variables rather than inputs) are left untouched.
 * 
 * @param transition Pointer to the transition whose inputs should be cleared
 */
void MachineSimulator::clearTriggeredInputs(const Transition* transition) {
    // Skip if there's no transition
    if (!transition) return;
    
    // For each input condition, clear the corresponding input
    for (const auto& inputCondition : transition->getInputConditions()) {
        // Only clear non-boolean inputs (we don't want to clear regular inputs for boolean expressions)
        if (!inputCondition.isBooleanExpr) {
            inputValues[inputCondition.source] = ""; // Clear the input
        }
    }
}