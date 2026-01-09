/**
 * @file moore_machine.h
 * @brief Declaration of the MooreMachine class
 * @author Hugo Bohácsek (xbohach00)
 */

#ifndef MOORE_MACHINE_H
#define MOORE_MACHINE_H

#include <string>
#include <unordered_map>
#include <vector>
#include <set>
#include <chrono>
#include "state.h"
#include "transition.h"
#include "../headers/machine_variable.h"
#include "../headers/expression_parser.h"

/**
 * @class MooreMachine
 * @brief Represents a Moore finite state machine
 * 
 * A Moore machine is a finite state machine where outputs depend only on the current state,
 * not on the input. This implementation extends the basic concept with features like:
 * - Input and output alphabets and pointers
 * - Variables and expressions
 * - Timeouts
 * - Validation
 */
class MooreMachine {
private:
    std::string name;                                      /**< Name of the machine */
    std::unordered_map<std::string, State> states;         /**< States indexed by ID */
    std::unordered_map<std::string, Transition> transitions; /**< Transitions indexed by ID */
    std::set<std::string> inputAlphabet;                   /**< Set of valid input symbols */
    std::set<std::string> outputAlphabet;                  /**< Set of valid output symbols */
    std::set<std::string> inputPointers;                   /**< Set of input pointers */
    std::set<std::string> outputPointers;                  /**< Set of output pointers */
    std::unordered_map<std::string, MachineVariable> variables; /**< Variables indexed by name */
    std::string currentStateId;                            /**< Current state ID for simulation */
    std::chrono::time_point<std::chrono::steady_clock> stateEntryTime; /**< Time when entered current state */
    ExpressionParser expressionParser;                     /**< Parser for expressions */
    std::unordered_map<std::string, std::string> initialVariableValues; //**< Store initial values for later reset */

public:
    /**
     * @brief Constructor
     * 
     * @param name Name of the machine
     */
    MooreMachine(const std::string& name);
    
    /**
     * @brief Adds a state to the machine
     * 
     * @param state The state to add
     * @return True if the state was added successfully, false if a state with the same ID already exists
     */
    bool addState(const State& state);
    
    /**
     * @brief Removes a state from the machine
     * 
     * Also removes all transitions to and from this state.
     * 
     * @param stateId ID of the state to remove
     * @return True if the state was removed successfully, false if it doesn't exist
     */
    bool removeState(const std::string& stateId);
    
    /**
     * @brief Gets a state by ID
     * 
     * @param stateId ID of the state to get
     * @return Pointer to the state, or nullptr if not found
     */
    State* getState(const std::string& stateId);
    
    /**
     * @brief Gets a state by name
     * 
     * @param name Name of the state to get
     * @return Pointer to the state, or nullptr if not found
     */
    State* getStateByName(const std::string& name);
    
    /**
     * @brief Gets all states in the machine
     * 
     * @return Vector of pointers to all states
     */
    std::vector<State*> getAllStates();
    
    /**
     * @brief Gets the initial state of the machine
     * 
     * @return Pointer to the initial state, or nullptr if not set
     */
    State* getInitialState();
    
    /**
     * @brief Sets a state as the initial state
     * 
     * @param stateId ID of the state to set as initial
     * @return True if successful, false if the state doesn't exist
     */
    bool setInitialState(const std::string& stateId);
    
    /**
     * @brief Adds a transition to the machine
     * 
     * @param transition The transition to add
     * @return True if successful, false if a transition with the same ID already exists
     */
    bool addTransition(const Transition& transition);
    
    /**
     * @brief Removes a transition from the machine
     * 
     * @param transitionId ID of the transition to remove
     * @return True if successful, false if the transition doesn't exist
     */
    bool removeTransition(const std::string& transitionId);
    
    /**
     * @brief Gets a transition by ID
     * 
     * @param transitionId ID of the transition to get
     * @return Pointer to the transition, or nullptr if not found
     */
    Transition* getTransition(const std::string& transitionId);
    
    /**
     * @brief Gets all transitions from a state
     * 
     * @param stateId ID of the source state
     * @return Vector of pointers to transitions from the state
     */
    std::vector<Transition*> getTransitionsFromState(const std::string& stateId);
    
    /**
     * @brief Gets all transitions to a state
     * 
     * @param stateId ID of the target state
     * @return Vector of pointers to transitions to the state
     */
    std::vector<Transition*> getTransitionsToState(const std::string& stateId);
    
    /**
     * @brief Adds an input symbol to the alphabet
     * 
     * @param symbol The symbol to add
     * @return True (always succeeds)
     */
    bool addInputSymbol(const std::string& symbol);
    
    /**
     * @brief Removes an input symbol from the alphabet
     * 
     * @param symbol The symbol to remove
     * @return True if successful, false if the symbol is in use by a transition
     */
    bool removeInputSymbol(const std::string& symbol);
    
    /**
     * @brief Adds an output symbol to the alphabet
     * 
     * @param symbol The symbol to add
     * @return True (always succeeds)
     */
    bool addOutputSymbol(const std::string& symbol);
    
    /**
     * @brief Removes an output symbol from the alphabet
     * 
     * @param symbol The symbol to remove
     * @return True if successful, false if the symbol is in use by a state
     */
    bool removeOutputSymbol(const std::string& symbol);
    
    /**
     * @brief Gets the input alphabet
     * 
     * @return Set of input symbols
     */
    std::set<std::string> getInputAlphabet() const;
    
    /**
     * @brief Gets the output alphabet
     * 
     * @return Set of output symbols
     */
    std::set<std::string> getOutputAlphabet() const;
    
    /**
     * @brief Adds an input pointer
     * 
     * @param pointer The pointer to add
     * @return True (always succeeds)
     */
    bool addInputPointer(const std::string& pointer);
    
    /**
     * @brief Removes an input pointer
     * 
     * @param pointer The pointer to remove
     * @return True if successful, false if the pointer is in use by a transition
     */
    bool removeInputPointer(const std::string& pointer);
    
    /**
     * @brief Adds an output pointer
     * 
     * @param pointer The pointer to add
     * @return True (always succeeds)
     */
    bool addOutputPointer(const std::string& pointer);
    
    /**
     * @brief Removes an output pointer
     * 
     * @param pointer The pointer to remove
     * @return True if successful, false if the pointer is in use by a state
     */
    bool removeOutputPointer(const std::string& pointer);
    
    /**
     * @brief Gets the input pointers
     * 
     * @return Set of input pointers
     */
    std::set<std::string> getInputPointers() const;
    
    /**
     * @brief Gets the output pointers
     * 
     * @return Set of output pointers
     */
    std::set<std::string> getOutputPointers() const;
    
    /**
     * @brief Adds a variable
     * 
     * @param type Type of the variable (int, float, string)
     * @param name Name of the variable
     * @param value Initial value of the variable
     * @return True if successful, false if the variable couldn't be created
     */
    bool addVariable(const std::string& type, const std::string& name, const std::string& value);

    /**
     * @brief Sets a variable
     *
     * @param type Type of the variable (int, float, string)
     * @param name Name of the variable
     * @param value Value of the variable
     */
    void setVariable(const std::string& type, const std::string& name, const std::string& value);

    /**
     * @brief Removes a variable
     * 
     * @param name Name of the variable to remove
     * @return True if successful, false if the variable doesn't exist
     */
    bool removeVariable(const std::string& name);
    
    /**
     * @brief Gets a variable
     * 
     * @param name Name of the variable to get
     * @return Pointer to the variable, or nullptr if not found
     */
    MachineVariable* getVariable(const std::string& name);
    
    /**
     * @brief Gets all variables
     * 
     * @return Map of variables indexed by name
     */
    std::unordered_map<std::string, MachineVariable> getVariables() const;
    
    /**
     * @brief Evaluates an expression
     * 
     * @param expression The expression to evaluate
     * @param elapsedTime Current elapsed time in milliseconds
     * @return Result of the evaluation
     */
    MachineVariable evaluateExpression(const std::string& expression, int elapsedTime);
    
    /**
     * @brief Parses an output expression
     * 
     * @param expression The expression to parse (e.g., "output var + 5 to output1")
     * @param elapsedTime Current elapsed time in milliseconds
     * @return Pair of output pointer and value
     */
    std::pair<std::string, std::string> parseOutputExpression(const std::string& expression, int elapsedTime);
    
    /**
     * @brief Checks if the machine is valid
     * 
     * Validates that the machine has states, an initial state, and that all
     * transitions and pointers are valid.
     * 
     * @return True if the machine is valid, false otherwise
     */
    bool isValid() const;
    
    /**
     * @brief Gets validation errors
     * 
     * @return Vector of error messages if the machine is invalid
     */
    std::vector<std::string> getValidationErrors() const;
    
    /**
     * @brief Processes an input through the machine
     * 
     * Simple interface for processing a single input symbol
     * 
     * @param input Input symbol to process
     * @return Output produced by the new state
     * @throw std::runtime_error If the machine is not in a valid state
     */
    std::string processInput(const std::string& input);
    
    /**
     * @brief Resets the machine to its initial state
     */
    void reset();
    
    /**
     * @brief Gets the elapsed time in the current state
     * 
     * @return Time in milliseconds since entering the current state
     */
    int getElapsedTimeInState() const;
    
    /**
     * @brief Gets the name of the machine
     * 
     * @return Name of the machine
     */
    std::string getName() const;
    
    /**
     * @brief Sets the name of the machine
     * 
     * @param name New name for the machine
     */
    void setName(const std::string& name);

    /**
     * @brief Processes an input through the machine on a specific input pointer
     * 
     * @param input Input symbol to process
     * @param inputPtr Input pointer to use
     * @return Output produced by the new state
     * @throw std::runtime_error If the machine is not in a valid state
     */
    std::string processInputOnPointer(const std::string& input, const std::string& inputPtr);

    /**
     * @brief Resets all variables to their initial values
     */
    void resetVariables(void);
};

#endif // MOORE_MACHINE_H
