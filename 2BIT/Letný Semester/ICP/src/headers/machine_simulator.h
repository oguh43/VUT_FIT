/**
 * @file machine_simulator.h
 * @brief Declaration of the MachineSimulator class
 * @author Hugo Bohácsek (xbohach00)
 */

#ifndef MACHINE_SIMULATOR_H
#define MACHINE_SIMULATOR_H

#include <string>
#include <chrono>
#include <unordered_map>
#include "moore_machine.h"

/**
 * @class MachineSimulator
 * @brief Simulates the execution of a Moore machine
 * 
 * This class handles the runtime aspects of a Moore machine:
 * - Processing inputs
 * - Transitioning between states
 * - Generating outputs
 * - Handling timeouts
 */
class MachineSimulator {
private:
    MooreMachine* machine;       /**< The machine being simulated */
    std::string currentStateId;  /**< ID of the current state */
    std::chrono::time_point<std::chrono::steady_clock> lastTransitionTime; /**< Time of last state transition */
    
    std::unordered_map<std::string, std::string> inputValues;  /**< Current input values by input pointer */
    std::unordered_map<std::string, std::string> outputValues; /**< Current output values by output pointer */
    std::unordered_map<std::string, std::chrono::time_point<std::chrono::steady_clock>> activeTimeouts; /**< Tracks timeouts */

    bool justEnteredState;  //**< Flag to indicate we just entered a state  */
    std::unordered_map<std::string, bool> timeoutActive;  //**< Track which timeouts are active  */
    std::unordered_map<std::string, std::chrono::time_point<std::chrono::steady_clock>> timeoutStartTimes;  //**< When each timeout started  */

public:
    /**
     * @brief Constructor
     * 
     * @param machine Pointer to the Moore machine to simulate
     * @throw std::runtime_error If the machine has no states or no initial state
     */
    MachineSimulator(MooreMachine* machine);
    
    /**
     * @brief Resets the simulation to initial state
     * 
     * @throw std::runtime_error If the machine has no states or no initial state
     */
    void reset();
    
    /**
     * @brief Sets an input value for a specific input pointer
     * 
     * @param inputPtr The input pointer name
     * @param value The value to set
     */
    void setInput(const std::string& inputPtr, const std::string& value);
    
    /**
     * @brief Gets the current value of an input pointer
     * 
     * @param inputPtr The input pointer name
     * @return Current value, or empty string if not set
     */
    std::string getInput(const std::string& inputPtr) const;
    
    /**
     * @brief Gets all current input values
     * 
     * @return Vector of input pointer/value pairs
     */
    std::vector<std::pair<std::string, std::string>> getAllInputs() const;
    
    /**
     * @brief Gets the current value of an output pointer
     * 
     * @param outputPtr The output pointer name
     * @return Current value, or empty string if not set
     */
    std::string getOutput(const std::string& outputPtr) const;
    
    /**
     * @brief Gets all current output values
     * 
     * @return Vector of output pointer/value pairs
     */
    std::vector<std::pair<std::string, std::string>> getAllOutputs() const;
    
    /**
     * @brief Processes current inputs and updates state and outputs
     * 
     * Checks if any transition from the current state is triggered by the
     * current input values, and if so, transitions to the target state
     * and processes outputs from that state.
     * 
     * @throw std::runtime_error If the machine is in an invalid state
     */
    void processInputs();
    
    /**
     * @brief Processes outputs from a state
     * 
     * Evaluates all output conditions from the given state and updates
     * output values accordingly.
     * 
     * @param state The state to process outputs from
     */
    void processStateOutputs(State* state);

    /**
     * @brief Processes a single input symbol
     * 
     * A simpler interface that sets the input to the default input pointer
     * and processes it.
     * 
     * @param symbol The input symbol to process
     * @return The output produced on the default output pointer
     */
    std::string processSymbol(const std::string& symbol);
    
    /**
     * @brief Processes a sequence of input symbols
     * 
     * Processes each symbol in sequence and concatenates the outputs.
     * 
     * @param sequence The input sequence to process
     * @return The concatenated output sequence
     */
    std::string processSequence(const std::string& sequence);
    
    /**
     * @brief Gets the current state
     * 
     * @return Pointer to the current state, or nullptr if not in a valid state
     */
    State* getCurrentState() const;
    
    /**
     * @brief Checks for timeout transitions
     * 
     * Checks if any timeout transition from the current state has expired,
     * and if so, performs the transition.
     * 
     * @return True if a timeout transition was triggered, false otherwise
     */
    bool checkTimeouts();

    /**
     * @brief Clears input values that triggered a transition
     * 
     * When a transition is triggered by input conditions,
     * this function clears those inputs from the input values map to prevent them from
     * triggering additional transitions in subsequent simulation cycles.
     * 
     * Only non-boolean input conditions are cleared; boolean expression conditions
     * (that refer to variables rather than inputs) are left untouched.
     * 
     * @param transition Pointer to the transition whose inputs should be cleared
     */
    void clearTriggeredInputs(const Transition* transition);
};

#endif // MACHINE_SIMULATOR_H