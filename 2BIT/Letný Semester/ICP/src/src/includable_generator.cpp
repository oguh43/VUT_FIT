/**
 * @file includable_generator.cpp
 * @brief Implementation of the IncludableGenerator class
 * @author Hugo Bohácsek (xbohach00)
 */

#include "../headers/includable_generator.h"
#include <fstream>
#include <iostream>
#include <sstream>
#include <algorithm>

bool IncludableGenerator::generateCode(const MooreMachine& machine, 
                                       const std::string& baseName, 
                                       const std::string& className,
                                       CodeStyle style) {
    try {
        if (style == CodeStyle::CALLBACK) {
            // Generate callback-style files with _callback suffix
            std::string headerFileName = baseName + ".h";
            std::ofstream headerFile(headerFileName);
            if (!headerFile.is_open()) return false;
            
            headerFile << generateHeaderContent(machine, className, CodeStyle::CALLBACK);
            headerFile.close();
            
            std::string sourceFileName = baseName + ".cpp";
            std::ofstream sourceFile(sourceFileName);
            if (!sourceFile.is_open()) return false;
            
            sourceFile << generateSourceContent(machine, className, headerFileName, CodeStyle::CALLBACK);
            sourceFile.close();
        }
        else if (style == CodeStyle::COMPUTED_GOTO) {
            // Generate goto-style files with _goto suffix
            std::string headerFileName = baseName + ".h";
            std::ofstream headerFile(headerFileName);
            if (!headerFile.is_open()) return false;
            
            headerFile << generateHeaderContent(machine, className, CodeStyle::COMPUTED_GOTO);
            headerFile.close();
            
            std::string sourceFileName = baseName + ".cpp";
            std::ofstream sourceFile(sourceFileName);
            if (!sourceFile.is_open()) return false;
            
            sourceFile << generateGotoImplementation(machine, className, headerFileName);
            sourceFile.close();
        }
        
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Error generating includable code: " << e.what() << std::endl;
        return false;
    }
}

std::string IncludableGenerator::generateHeaderContent(const MooreMachine& machine, const std::string& className, CodeStyle style) {
    std::stringstream ss;
    
    ss << "/**\n"
       << " * @file Auto-generated FSM header\n"
       << " * @brief Defines the " << className << " class, generated from a Moore machine\n"
       << " * @warning This file is auto-generated. Do not modify manually.\n"
       << " */\n\n"
       << "#ifndef " << className << "_H\n"
       << "#define " << className << "_H\n\n"
       << "#include <string>\n"
       << "#include <vector>\n"
       << "#include <unordered_map>\n"
       << "#include <chrono>\n";
    
    if (style == CodeStyle::CALLBACK) {
        ss << "#include <functional>\n";
    }
    
    ss << "#include <variant>\n\n";
    
    if (style == CodeStyle::COMPUTED_GOTO) {
        ss << "// Using GNU extension for computed gotos\n"
           << "#ifdef __GNUC__\n"
           << "#define USE_COMPUTED_GOTO 1\n"
           << "#else\n"
           << "#warning \"Computed goto not supported by this compiler. Falling back to switch-case.\"\n"
           << "#define USE_COMPUTED_GOTO 0\n"
           << "#endif\n\n";
    }
    
    ss << "/**\n"
       << " * @class " << className << "\n"
       << " * @brief FSM implementation generated from \"" << machine.getName() << "\"\n"
       << " */\n"
       << "class " << className << " {\n"
       << "public:\n"
       << "    using VariableType = std::variant<int, float, std::string>;\n";
    
    if (style == CodeStyle::CALLBACK) {
        ss << "    using StateCallback = std::function<void(const std::unordered_map<std::string, std::string>&, const std::unordered_map<std::string, VariableType>&)>;\n\n";
    } else {
        ss << "\n    // State identifiers\n";
        // Add enum for state IDs
        ss << "    enum StateId {\n";
        
        // Add all state names
        bool first = true;
        for (State* state : const_cast<MooreMachine&>(machine).getAllStates()) {
            if (!first) ss << ",\n";
            ss << "        STATE_" << state->getName() << (first ? " = 0" : "");
            first = false;
        }
        ss << "\n    };\n\n";
    }
    
    ss << "    /**\n"
       << "     * @struct State\n"
       << "     * @brief Represents a state in the FSM\n"
       << "     */\n"
       << "    struct State {\n"
       << "        std::string name;\n"
       << "        std::string outputExpression;\n";
    
    if (style == CodeStyle::COMPUTED_GOTO) {
        ss << "        StateId id;\n";
    }
    
    ss << "\n"
       << "        State(const std::string& name, const std::string& outputExpression";
    
    if (style == CodeStyle::COMPUTED_GOTO) {
        ss << ", StateId id";
    }
    
    ss << ")\n"
       << "            : name(name), outputExpression(outputExpression)";
    
    if (style == CodeStyle::COMPUTED_GOTO) {
        ss << ", id(id)";
    }
    
    ss << " {}\n"
       << "    };\n\n";
    
    // Continue with transition struct definition
    ss << "    /**\n"
       << "     * @struct Transition\n"
       << "     * @brief Represents a transition between states\n"
       << "     */\n"
       << "    struct Transition {\n"
       << "        State* fromState;\n"
       << "        State* toState;\n"
       << "        std::string guardExpression;\n"
       << "        int timeout; // ms\n\n"
       << "        Transition(State* from, State* to, const std::string& guard, int timeout)\n"
       << "            : fromState(from), toState(to), guardExpression(guard), timeout(timeout) {}\n"
       << "    };\n\n";
    
    // Constructor and destructor
    ss << "    /**\n"
       << "     * @brief Constructor\n"
       << "     */\n"
       << "    " << className << "();\n\n"
       << "    /**\n"
       << "     * @brief Destructor\n"
       << "     */\n"
       << "    ~" << className << "();\n\n";
    
    // Common methods
    ss << "    /**\n"
       << "     * @brief Reset the FSM to its initial state\n"
       << "     */\n"
       << "    void reset();\n\n"
       << "    /**\n"
       << "     * @brief Process a step in the FSM\n"
       << "     * Evaluates all transitions from the current state and performs\n"
       << "     * the first valid transition found\n"
       << "     */\n"
       << "    void tick();\n\n"
       << "    /**\n"
       << "     * @brief Process an input through the machine\n"
       << "     * @param input Input symbol to process\n"
       << "     * @param inputPtr Input pointer to use (default is \"default\")\n"
       << "     * @return Output produced by the new state\n"
       << "     */\n"
       << "    std::string processInput(const std::string& input, const std::string& inputPtr = \"default\");\n\n"
       << "    /**\n"
       << "     * @brief Get the current state name\n"
       << "     * @return Name of the current state\n"
       << "     */\n"
       << "    std::string getCurrentStateName() const;\n\n";
    
    if (style == CodeStyle::COMPUTED_GOTO) {
        ss << "    /**\n"
           << "     * @brief Get the current state ID\n"
           << "     * @return ID of the current state\n"
           << "     */\n"
           << "    StateId getCurrentStateId() const;\n"
           << "    void* getStateTarget(StateId id) const;\n\n";
    }
    
    ss << "    /**\n"
       << "     * @brief Get all current outputs\n"
       << "     * @return Map of output pointers to their values\n"
       << "     */\n"
       << "    std::unordered_map<std::string, std::string> getOutputs() const;\n\n"
       << "    /**\n"
       << "     * @brief Get a specific output\n"
       << "     * @param outputPtr Name of the output pointer\n"
       << "     * @return Output value, or empty string if not found\n"
       << "     */\n"
       << "    std::string getOutput(const std::string& outputPtr) const;\n\n"
       << "    /**\n"
       << "     * @brief Set a variable value\n"
       << "     * @param name Name of the variable\n"
       << "     * @param value New value for the variable\n"
       << "     */\n"
       << "    template<typename T>\n"
       << "    void setVariable(const std::string& name, const T& value) {\n"
       << "        variables[name] = value;\n"
       << "    }\n\n"
       << "    /**\n"
       << "     * @brief Get a variable value\n"
       << "     * @param name Name of the variable\n"
       << "     * @return Value of the variable, or default-constructed value if not found\n"
       << "     */\n"
       << "    template<typename T>\n"
       << "    T getVariable(const std::string& name) const {\n"
       << "        auto it = variables.find(name);\n"
       << "        if (it != variables.end()) {\n"
       << "            try {\n"
       << "                return std::get<T>(it->second);\n"
       << "            } catch (const std::bad_variant_access&) {\n"
       << "                // Type mismatch\n"
       << "                return T{};\n"
       << "            }\n"
       << "        }\n"
       << "        return T{};\n"
       << "    }\n\n";
    
    // Callback methods - only for callback style
    if (style == CodeStyle::CALLBACK) {
        ss << "    /**\n"
           << "     * @brief Register a state change callback\n"
           << "     * @param callback Function to call when state changes\n"
           << "     */\n"
           << "    void setStateChangeCallback(std::function<void(const std::string&)> callback) {\n"
           << "        stateChangeCallback = callback;\n"
           << "    }\n\n"
           << "    /**\n"
           << "     * @brief Register a callback for a specific state\n"
           << "     * @param stateName Name of the state to register callback for\n"
           << "     * @param callback Function to call when the state is entered\n"
           << "     *        The callback receives the current outputs and variables\n"
           << "     */\n"
           << "    void setStateCallback(const std::string& stateName, StateCallback callback) {\n"
           << "        stateCallbacks[stateName] = callback;\n"
           << "    }\n\n"
           << "    /**\n"
           << "     * @brief Register an output change callback\n"
           << "     * @param callback Function to call when any output changes\n"
           << "     */\n"
           << "    void setOutputChangeCallback(std::function<void(const std::string&, const std::string&)> callback) {\n"
           << "        outputChangeCallback = callback;\n"
           << "    }\n\n";
    } else { // Computed goto style
        ss << "    /**\n"
           << "     * @brief Set the target label for a specific state\n"
           << "     * @param stateId ID of the state\n"
           << "     * @param labelAddress Address of the label to jump to (use &&label)\n"
           << "     */\n"
           << "#if USE_COMPUTED_GOTO\n"
           << "    void setStateTarget(StateId stateId, void* labelAddress) {\n"
           << "        stateTargets[stateId] = labelAddress;\n"
           << "    }\n"
           << "#endif\n\n"
           << "    /**\n"
           << "     * @brief Run the state machine using computed gotos\n"
           << "     * \n"
           << "     * Example usage:\n"
           << "     * @code\n"
           << "     * fsm.runWithGotos();\n"
           << "     * state_start:\n"
           << "     *     // Code for start state\n"
           << "     *     // ...\n"
           << "     *     fsm.setStateTarget(FSMAutomaton::STATE_state1, &&state1);\n"
           << "     *     // ...\n"
           << "     *     if (condition) goto state1;\n"
           << "     *     return;\n"
           << "     * \n"
           << "     * state1:\n"
           << "     *     // Code for state1\n"
           << "     *     // ...\n"
           << "     *     // Return to caller when done\n"
           << "     *     return;\n"
           << "     * @endcode\n"
           << "     */\n"
           << "    void runWithGotos();\n\n";
    }
    
    // Private members
    ss << "private:\n"
       << "    std::unordered_map<std::string, State*> states;\n"
       << "    std::vector<Transition*> transitions;\n"
       << "    std::unordered_map<std::string, std::string> inputs;\n"
       << "    std::unordered_map<std::string, std::string> outputs;\n"
       << "    std::unordered_map<std::string, VariableType> variables;\n"
       << "    State* currentState;\n"
       << "    std::chrono::steady_clock::time_point stateEntryTime;\n"
       << "    bool justEnteredState; // Flag to prevent immediate re-evaluation of transitions\n"
       << "    std::unordered_map<std::string, bool> timeoutActive; // Track active timeouts\n"
       << "    std::unordered_map<std::string, std::chrono::steady_clock::time_point> timeoutStartTimes; // When timeouts started\n"
       << "    std::unordered_map<Transition*, std::chrono::steady_clock::time_point> activeTimeouts;\n\n";
    if (style == CodeStyle::CALLBACK) {
        ss << "    std::function<void(const std::string&)> stateChangeCallback;\n"
           << "    std::function<void(const std::string&, const std::string&)> outputChangeCallback;\n"
           << "    std::unordered_map<std::string, StateCallback> stateCallbacks;\n\n";
    } else { // Computed goto style
        ss << "#if USE_COMPUTED_GOTO\n"
           << "    // Maps state IDs to label addresses for computed gotos\n"
           << "    std::unordered_map<StateId, void*> stateTargets;\n"
           << "#endif\n\n";
    }
    
    // Private methods
    ss << "    void executeOutputActions();\n"
       << "    bool evaluateTransition(Transition* transition);\n"
       << "    VariableType evaluateExpression(const std::string& expression);\n"
       << "    std::vector<std::string> tokenize(const std::string& expr);\n"
       << "    bool checkTimeouts();\n"
       << "    void clearTriggeredInputs(Transition* transition);\n"
       << "};\n\n"
       << "#endif // " << className << "_H\n";
    
    return ss.str();
}

std::string IncludableGenerator::generateSourceContent(const MooreMachine& machine, const std::string& className, const std::string& headerName, CodeStyle style) {
    if (style == CodeStyle::COMPUTED_GOTO) {
        return generateGotoImplementation(machine, className, headerName);
    }
    
    // Default implementation using callbacks (original version)
    std::stringstream ss;
    
    ss << "/**\n"
       << " * @file Auto-generated FSM implementation\n"
       << " * @brief Implements the " << className << " class, generated from a Moore machine\n"
       << " * @warning This file is auto-generated. Do not modify manually.\n"
       << " */\n\n"
       << "#include \"" << headerName << "\"\n"
       << "#include <algorithm>\n"
       << "#include <regex>\n"
       << "#include <sstream>\n"
       << "#include <iostream>\n\n"
       << "// Constructor - initializes the FSM\n"
       << className << "::" << className << "() : currentState(nullptr), justEnteredState(true) {\n";
    
    // Create input pointers
    auto inputPointers = machine.getInputPointers();
    for (const auto& pointer : inputPointers) {
        ss << "    inputs[\"" << pointer << "\"] = \"\";\n";
    }
    
    // Create output pointers
    auto outputPointers = machine.getOutputPointers();
    for (const auto& pointer : outputPointers) {
        ss << "    outputs[\"" << pointer << "\"] = \"\";\n";
    }
    
    // Create variables
    auto variables = machine.getVariables();
    for (const auto& varPair : variables) {
        const auto& var = varPair.second;
        
        ss << "    // Variable: " << var.getName() << " (" << typeToString(var.getType()) << ")\n";
        if (typeToString(var.getType()) == "string") {
            ss << "    variables[\"" << var.getName() << "\"] = std::string(\"" << var.getValueString() << "\");\n";
        } else if (typeToString(var.getType()) == "float") {
            ss << "    variables[\"" << var.getName() << "\"] = " << var.getValueString() << "f;\n";
        } else { // int
            ss << "    variables[\"" << var.getName() << "\"] = " << var.getValueString() << ";\n";
        }
    }
    
    // Create states
    State* initialState = nullptr;
    std::vector<State*> statesList;
    
    for (State* state : const_cast<MooreMachine&>(machine).getAllStates()) {
        if (state->getIsInitial()) {
            initialState = state;
        }
        statesList.push_back(state);
    }
    
    // Make sure initial state comes first
    if (initialState) {
        auto it = std::find(statesList.begin(), statesList.end(), initialState);
        if (it != statesList.begin() && it != statesList.end()) {
            std::iter_swap(statesList.begin(), it);
        }
    }
    
    for (State* state : statesList) {
        std::string stateName = state->getName();
        
        ss << "\n    // Create state: " << stateName << "\n";
        
        // Format state outputs
        std::string outputExpr;
        auto outputs = state->getOutputs();
        
        if (!outputs.empty()) {
            for (const auto& output : outputs) {
                // Check if this is a variable expression or regular output
                if (output.value.find("=") != std::string::npos ||
                    output.value.find("+") != std::string::npos ||
                    output.value.find("-") != std::string::npos ||
                    output.value.find("*") != std::string::npos ||
                    output.value.find("/") != std::string::npos) {
                    outputExpr += output.value;
                } else {
                    outputExpr += "output " + output.value + " to " + output.target;
                }
                
                // Add condition if present
                if (output.hasCondition) {
                    outputExpr += " if " + output.inputPtr + " is defined";
                }
                
                outputExpr += ";";
            }
        }
        
        ss << "    states[\"" << stateName << "\"] = new State(\"" << stateName << "\", \"" << outputExpr << "\");\n";
        
        // Set initial state
        if (state->getIsInitial()) {
            ss << "    currentState = states[\"" << stateName << "\"];\n";
            ss << "    stateEntryTime = std::chrono::steady_clock::now();\n";
        }
    }
    
    // Create transitions
    for (State* sourceState : statesList) {
        std::string sourceName = sourceState->getName();
        auto outTransitions = const_cast<MooreMachine&>(machine).getTransitionsFromState(sourceState->getId());
        
        for (Transition* transition : outTransitions) {
            std::string targetId = transition->getTargetId();
            State* targetState = const_cast<MooreMachine&>(machine).getState(targetId);
            
            if (!targetState) continue;
            
            std::string targetName = targetState->getName();
            
            ss << "\n    // Transition from " << sourceName << " to " << targetName << "\n";
            
            // Build guard expression
            std::string guardExpr;
            auto inputConditions = transition->getInputConditions();
            
            for (const auto& cond : inputConditions) {
                if (cond.isBooleanExpr) {
                    guardExpr += cond.leftOperand + " " + cond.operation + " " + cond.rightOperand + ";";
                } else {
                    guardExpr += "got " + cond.value + " from " + cond.source + ";";
                }
            }
            
            // Add the transition
            ss << "    transitions.push_back(new Transition(\n"
               << "        states[\"" << sourceName << "\"],\n"
               << "        states[\"" << targetName << "\"],\n"
               << "        \"" << guardExpr << "\",\n"
               << "        " << transition->getTimeout() << "));\n";
        }
    }
    
    ss << "}\n\n";
    
    // Destructor
    ss << "// Destructor - cleans up allocated memory\n"
       << className << "::~" << className << "() {\n"
       << "    for (auto& transition : transitions) {\n"
       << "        delete transition;\n"
       << "    }\n"
       << "    \n"
       << "    for (auto& pair : states) {\n"
       << "        delete pair.second;\n"
       << "    }\n"
       << "}\n\n";
    
    // Reset method
    ss << "// Reset the FSM to its initial state\n"
       << "void " << className << "::reset() {\n"
       << "    // Find initial state\n"
       << "    for (const auto& pair : states) {\n"
       << "        // Use the first state as fallback\n"
       << "        if (currentState == nullptr) {\n"
       << "            currentState = pair.second;\n"
       << "        }\n"
       << "        \n"
       << "        // Use initial state if available\n";
    
    if (initialState) {
        ss << "        if (pair.first == \"" << initialState->getName() << "\") {\n"
           << "            currentState = pair.second;\n"
           << "            break;\n"
           << "        }\n";
    }
    
    ss << "    }\n"
       << "    \n"
       << "    stateEntryTime = std::chrono::steady_clock::now();\n"
       << "    justEnteredState = true;\n"
       << "    timeoutActive.clear();\n"
       << "    timeoutStartTimes.clear();\n"
       << "    \n"
       << "    // Clear inputs and outputs\n"
       << "    for (auto& pair : inputs) {\n"
       << "        pair.second = \"\";\n"
       << "    }\n"
       << "    \n"
       << "    for (auto& pair : outputs) {\n"
       << "        pair.second = \"\";\n"
       << "    }\n"
       << "    \n"
       << "    // Execute output actions for initial state\n"
       << "    if (currentState) {\n"
       << "        executeOutputActions();\n"
       << "    }\n"
       << "}\n\n";
    
    // Process step method
    ss << "// Process a step in the FSM\n"
       << "void " << className << "::tick() {\n"
       << "    if (!currentState) {\n"
       << "        reset();\n"
       << "        return;\n"
       << "    }\n"
       << "    \n"
       << "    // If we just entered this state, skip immediate transition evaluation\n"
       << "    if (justEnteredState) {\n"
       << "        justEnteredState = false;\n"
       << "        return;\n"
       << "    }\n"
       << "    \n"
       << "    // Check timeouts first\n"
       << "    bool timeoutTriggered = checkTimeouts();\n"
       << "    if (timeoutTriggered) {\n"
       << "        return; // Already transitioned due to timeout\n"
       << "    }\n"
       << "    \n"
       << "    // Evaluate transitions\n"
       << "    for (Transition* transition : transitions) {\n"
       << "        if (transition->fromState == currentState && evaluateTransition(transition)) {\n"
       << "            // If transition has both input conditions and timeout\n"
       << "            if (transition->timeout > 0) {\n"
       << "                std::string transId = transition->guardExpression;\n"
       << "                \n"
       << "                // If timeout not already active, start it\n"
       << "                if (!timeoutActive[transId]) {\n"
       << "                    timeoutActive[transId] = true;\n"
       << "                    timeoutStartTimes[transId] = std::chrono::steady_clock::now();\n"
       << "                }\n"
       << "                continue; // Wait for timeout to expire\n"
       << "            }\n"
       << "            \n"
       << "            // Immediate transition (no timeout)\n"
       << "            State* oldState = currentState;\n"
       << "            currentState = transition->toState;\n"
       << "            \n"
       << "            // Clear inputs that triggered this transition\n"
       << "            clearTriggeredInputs(transition);\n"
       << "            \n"
       << "            // Reset state entry time if state changed\n"
       << "            if (oldState != currentState) {\n"
       << "                stateEntryTime = std::chrono::steady_clock::now();\n"
       << "                justEnteredState = true;\n"
       << "                timeoutActive.clear();\n"
       << "                timeoutStartTimes.clear();\n"
       << "                \n"
       << "                // Execute output actions\n"
       << "                executeOutputActions();\n"
       << "                \n"
       << "                // Call state change callback if registered\n"
       << "                if (stateChangeCallback) {\n"
       << "                    stateChangeCallback(currentState->name);\n"
       << "                }\n"
       << "                \n"
       << "                // Call state-specific callback if registered\n"
       << "                auto stateCallbackIt = stateCallbacks.find(currentState->name);\n"
       << "                if (stateCallbackIt != stateCallbacks.end()) {\n"
       << "                    stateCallbackIt->second(outputs, variables);\n"
       << "                }\n"
       << "            }\n"
       << "            \n"
       << "            break;\n"
       << "        }\n"
       << "    }\n"
       << "}\n\n";
    
    // Process input method
    ss << "// Process an input through the machine\n"
       << "std::string " << className << "::processInput(const std::string& input, const std::string& inputPtr) {\n"
       << "    // Set the input\n"
       << "    inputs[inputPtr] = input;\n"
       << "    justEnteredState = false; // Allow transitions to be evaluated\n"
       << "    \n"
       << "    // Process a step\n"
       << "    tick();\n"
       << "    \n"
       << "    // Return output from default pointer\n"
       << "    return getOutput(\"default\");\n"
       << "}\n\n";
    
    // Get current state name
    ss << "// Get the current state name\n"
       << "std::string " << className << "::getCurrentStateName() const {\n"
       << "    return currentState ? currentState->name : \"\";\n"
       << "}\n\n";
    
    // Get all outputs
    ss << "// Get all current outputs\n"
       << "std::unordered_map<std::string, std::string> " << className << "::getOutputs() const {\n"
       << "    return outputs;\n"
       << "}\n\n";
    
    // Get specific output
    ss << "// Get a specific output\n"
       << "std::string " << className << "::getOutput(const std::string& outputPtr) const {\n"
       << "    auto it = outputs.find(outputPtr);\n"
       << "    if (it != outputs.end()) {\n"
       << "        return it->second;\n"
       << "    }\n"
       << "    return \"\";\n"
       << "}\n\n";
    
    // Execute output actions
    ss << "// Execute output actions for the current state\n"
       << "void " << className << "::executeOutputActions() {\n"
       << "    if (!currentState) return;\n"
       << "    \n"
       << "    std::string expr = currentState->outputExpression;\n"
       << "    size_t pos = 0, end;\n"
       << "    \n"
       << "    while ((end = expr.find(';', pos)) != std::string::npos) {\n"
       << "        std::string action = expr.substr(pos, end - pos);\n"
       << "        pos = end + 1;\n"
       << "        \n"
       << "        // Skip empty actions\n"
       << "        if (action.empty()) continue;\n"
       << "        \n"
       << "        // Handle conditional actions (if inputPtr is defined)\n"
       << "        std::regex ifPattern(\"(.+) if (.+) is defined\");\n"
       << "        std::smatch ifMatch;\n"
       << "        \n"
       << "        if (std::regex_match(action, ifMatch, ifPattern)) {\n"
       << "            std::string actualAction = ifMatch[1];\n"
       << "            std::string inputPtr = ifMatch[2];\n"
       << "            \n"
       << "            // Check if input is defined\n"
       << "            auto inputIt = inputs.find(inputPtr);\n"
       << "            if (inputIt == inputs.end() || inputIt->second.empty()) {\n"
       << "                continue; // Skip this action\n"
       << "            }\n"
       << "            \n"
       << "            // Replace action with actual action\n"
       << "            action = actualAction;\n"
       << "        }\n"
       << "        \n"
       << "        // Handle variable assignments (var = ...)\n"
       << "        std::regex assignPattern(\"([^=]+)=(.+)\");\n"
       << "        std::smatch assignMatch;\n"
       << "        \n"
       << "        if (std::regex_match(action, assignMatch, assignPattern)) {\n"
       << "            std::string varName = assignMatch[1];\n"
       << "            std::string valueExpr = assignMatch[2];\n"
       << "            \n"
       << "            // Trim whitespace\n"
       << "            varName.erase(0, varName.find_first_not_of(\" \\t\\r\\n\"));\n"
       << "            varName.erase(varName.find_last_not_of(\" \\t\\r\\n\") + 1);\n"
       << "            valueExpr.erase(0, valueExpr.find_first_not_of(\" \\t\\r\\n\"));\n"
       << "            valueExpr.erase(valueExpr.find_last_not_of(\" \\t\\r\\n\") + 1);\n"
       << "            \n"
       << "            // Check for 'get' expression\n"
       << "            std::regex getPattern(\"get ([^\\\\s]+)\");\n"
       << "            std::smatch getMatch;\n"
       << "            \n"
       << "            if (std::regex_match(valueExpr, getMatch, getPattern)) {\n"
       << "                std::string inputPtr = getMatch[1];\n"
       << "                auto inputIt = inputs.find(inputPtr);\n"
       << "                \n"
       << "                if (inputIt != inputs.end()) {\n"
       << "                    // Find variable type\n"
       << "                    auto varIt = variables.find(varName);\n"
       << "                    if (varIt != variables.end()) {\n"
       << "                        if (std::holds_alternative<int>(varIt->second)) {\n"
       << "                            try {\n"
       << "                                variables[varName] = std::stoi(inputIt->second);\n"
       << "                            } catch (...) {\n"
       << "                                variables[varName] = 0;\n"
       << "                            }\n"
       << "                        } else if (std::holds_alternative<float>(varIt->second)) {\n"
       << "                            try {\n"
       << "                                variables[varName] = std::stof(inputIt->second);\n"
       << "                            } catch (...) {\n"
       << "                                variables[varName] = 0.0f;\n"
       << "                            }\n"
       << "                        } else {\n"
       << "                            variables[varName] = inputIt->second;\n"
       << "                        }\n"
       << "                    } else {\n"
       << "                        // Default to string\n"
       << "                        variables[varName] = inputIt->second;\n"
       << "                    }\n"
       << "                }\n"
       << "            } else {\n"
       << "                // Evaluate expression\n"
       << "                variables[varName] = evaluateExpression(valueExpr);\n"
       << "            }\n"
       << "            \n"
       << "            continue;\n"
       << "        }\n"
       << "        \n"
       << "        // Handle output expressions (output value to target)\n"
       << "        std::regex outputPattern(\"output (.+) to (.+)\");\n"
       << "        std::smatch outputMatch;\n"
       << "        \n"
       << "        if (std::regex_match(action, outputMatch, outputPattern)) {\n"
       << "            std::string value = outputMatch[1];\n"
       << "            std::string target = outputMatch[2];\n"
       << "            \n"
       << "            // Trim whitespace\n"
       << "            value.erase(0, value.find_first_not_of(\" \\t\\r\\n\"));\n"
       << "            value.erase(value.find_last_not_of(\" \\t\\r\\n\") + 1);\n"
       << "            target.erase(0, target.find_first_not_of(\" \\t\\r\\n\"));\n"
       << "            target.erase(target.find_last_not_of(\" \\t\\r\\n\") + 1);\n"
       << "            \n"
       << "            // Evaluate the value if it's an expression\n"
       << "            if (value.find_first_of(\"+-*/\") != std::string::npos) {\n"
       << "                auto result = evaluateExpression(value);\n"
       << "                std::string strResult;\n"
       << "                \n"
       << "                if (std::holds_alternative<int>(result)) {\n"
       << "                    strResult = std::to_string(std::get<int>(result));\n"
       << "                } else if (std::holds_alternative<float>(result)) {\n"
       << "                    strResult = std::to_string(std::get<float>(result));\n"
       << "                } else {\n"
       << "                    strResult = std::get<std::string>(result);\n"
       << "                }\n"
       << "                \n"
       << "                value = strResult;\n"
       << "            }\n"
       << "            \n"
       << "            // Check if output already exists\n"
       << "            std::string oldValue = outputs[target];\n"
       << "            outputs[target] = value;\n"
       << "            \n"
       << "            // Call output change callback if registered and value changed\n"
       << "            if (outputChangeCallback && oldValue != value) {\n"
       << "                outputChangeCallback(target, value);\n"
       << "            }\n"
       << "        }\n"
       << "    }\n"
       << "}\n\n";
    
    // Evaluate transition
    ss << "// Evaluate if a transition should be triggered\n"
       << "bool " << className << "::evaluateTransition(Transition* transition) {\n"
       << "    std::string expr = transition->guardExpression;\n"
       << "    if (expr.empty()) {\n"
       << "        return false;\n"
       << "    }\n"
       << "    \n"
       << "    size_t pos = 0, end;\n"
       << "    bool result = false;\n"
       << "    \n"
       << "    while ((end = expr.find(';', pos)) != std::string::npos) {\n"
       << "        std::string condition = expr.substr(pos, end - pos);\n"
       << "        pos = end + 1;\n"
       << "        \n"
       << "        // Skip empty conditions\n"
       << "        if (condition.empty()) continue;\n"
       << "        \n"
       << "        // Handle boolean expressions (var == value or var != value)\n"
       << "        std::regex boolPattern(\"(.+) ?(==|!=) ?(.+)\");\n"
       << "        std::smatch boolMatch;\n"
       << "        \n"
       << "        if (std::regex_match(condition, boolMatch, boolPattern)) {\n"
       << "            std::string leftOp = boolMatch[1];\n"
       << "            std::string op = boolMatch[2];\n"
       << "            std::string rightOp = boolMatch[3];\n"
       << "            \n"
       << "            // Trim whitespace\n"
       << "            leftOp.erase(0, leftOp.find_first_not_of(\" \\t\\r\\n\"));\n"
       << "            leftOp.erase(leftOp.find_last_not_of(\" \\t\\r\\n\") + 1);\n"
       << "            rightOp.erase(0, rightOp.find_first_not_of(\" \\t\\r\\n\"));\n"
       << "            rightOp.erase(rightOp.find_last_not_of(\" \\t\\r\\n\") + 1);\n"
       << "            \n"
       << "            // Get left operand value\n"
       << "            auto leftIt = variables.find(leftOp);\n"
       << "            if (leftIt == variables.end()) {\n"
       << "                continue; // Skip this condition if variable not found\n"
       << "            }\n"
       << "            \n"
       << "            // Get right operand value\n"
       << "            VariableType rightValue;\n"
       << "            auto rightIt = variables.find(rightOp);\n"
       << "            \n"
       << "            if (rightIt != variables.end()) {\n"
       << "                rightValue = rightIt->second;\n"
       << "            } else {\n"
       << "                // Try to parse as literal based on left operand type\n"
       << "                if (std::holds_alternative<int>(leftIt->second)) {\n"
       << "                    try {\n"
       << "                        rightValue = std::stoi(rightOp);\n"
       << "                    } catch (...) {\n"
       << "                        continue; // Invalid conversion\n"
       << "                    }\n"
       << "                } else if (std::holds_alternative<float>(leftIt->second)) {\n"
       << "                    try {\n"
       << "                        rightValue = std::stof(rightOp);\n"
       << "                    } catch (...) {\n"
       << "                        continue; // Invalid conversion\n"
       << "                    }\n"
       << "                } else {\n"
       << "                    rightValue = rightOp; // String literal\n"
       << "                }\n"
       << "            }\n"
       << "            \n"
       << "            // Compare values\n"
       << "            bool conditionMet = false;\n"
       << "            \n"
       << "            if (op == \"==\") {\n"
       << "                conditionMet = (leftIt->second == rightValue);\n"
       << "            } else { // op == \"!=\"\n"
       << "                conditionMet = (leftIt->second != rightValue);\n"
       << "            }\n"
       << "            \n"
       << "            result = conditionMet;\n"
       << "            if (!result) {\n"
       << "                return false; // If any condition fails, the whole transition fails\n"
       << "            }\n"
       << "        }\n"
       << "        \n"
       << "        // Handle input conditions (got value from source)\n"
       << "        std::regex inputPattern(\"got (.+) from (.+)\");\n"
       << "        std::smatch inputMatch;\n"
       << "        \n"
       << "        if (std::regex_match(condition, inputMatch, inputPattern)) {\n"
       << "            std::string value = inputMatch[1];\n"
       << "            std::string source = inputMatch[2];\n"
       << "            \n"
       << "            // Trim whitespace\n"
       << "            value.erase(0, value.find_first_not_of(\" \\t\\r\\n\"));\n"
       << "            value.erase(value.find_last_not_of(\" \\t\\r\\n\") + 1);\n"
       << "            source.erase(0, source.find_first_not_of(\" \\t\\r\\n\"));\n"
       << "            source.erase(source.find_last_not_of(\" \\t\\r\\n\") + 1);\n"
       << "            \n"
       << "            // Check if input matches\n"
       << "            auto inputIt = inputs.find(source);\n"
       << "            if (inputIt != inputs.end() && inputIt->second == value) {\n"
       << "                result = true;\n"
       << "            } else {\n"
       << "                return false; // If any condition fails, the whole transition fails\n"
       << "            }\n"
       << "        }\n"
       << "    }\n"
       << "    \n"
       << "    return result;\n"
       << "}\n\n";
    
    // Evaluate expression
    ss << "// Evaluate a simple arithmetic expression\n"
       << className << "::VariableType " << className << "::evaluateExpression(const std::string& expression) {\n"
       << "    // Simple expression evaluation (no precedence)\n"
       << "    std::vector<std::string> tokens = tokenize(expression);\n"
       << "    \n"
       << "    if (tokens.empty()) {\n"
       << "        return std::string(\"\");\n"
       << "    }\n"
       << "    \n"
       << "    VariableType result;\n"
       << "    std::string currentOp = \"+\"; // Default operation\n"
       << "    bool firstToken = true;\n"
       << "    \n"
       << "    for (size_t i = 0; i < tokens.size(); i++) {\n"
       << "        const std::string& token = tokens[i];\n"
       << "        \n"
       << "        // Skip empty tokens\n"
       << "        if (token.empty()) continue;\n"
       << "        \n"
       << "        // Handle operators\n"
       << "        if (token == \"+\" || token == \"-\" || token == \"*\" || token == \"/\") {\n"
       << "            currentOp = token;\n"
       << "            continue;\n"
       << "        }\n"
       << "        \n"
       << "        // Get token value\n"
       << "        VariableType tokenValue;\n"
       << "        \n"
       << "        // Check if token is a variable\n"
       << "        auto varIt = variables.find(token);\n"
       << "        if (varIt != variables.end()) {\n"
       << "            tokenValue = varIt->second;\n"
       << "        }\n"
       << "        // Check if token is 'elapsed' keyword\n"
       << "        else if (token == \"elapsed\") {\n"
       << "            auto now = std::chrono::steady_clock::now();\n"
       << "            auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - stateEntryTime).count();\n"
       << "            tokenValue = static_cast<int>(elapsed);\n"
       << "        }\n"
       << "        // Try to parse as number\n"
       << "        else {\n"
       << "            try {\n"
       << "                // Check if it's a floating point number\n"
       << "                if (token.find('.') != std::string::npos) {\n"
       << "                    tokenValue = std::stof(token);\n"
       << "                } else {\n"
       << "                    tokenValue = std::stoi(token);\n"
       << "                }\n"
       << "            } catch (...) {\n"
       << "                // Default to string\n"
       << "                tokenValue = token;\n"
       << "            }\n"
       << "        }\n"
       << "        \n"
       << "        // Set initial result or apply operation\n"
       << "        if (firstToken) {\n"
       << "            result = tokenValue;\n"
       << "            firstToken = false;\n"
       << "        } else {\n"
       << "            // Integer operations\n"
       << "            if (std::holds_alternative<int>(result) && std::holds_alternative<int>(tokenValue)) {\n"
       << "                int val1 = std::get<int>(result);\n"
       << "                int val2 = std::get<int>(tokenValue);\n"
       << "                \n"
       << "                if (currentOp == \"+\") result = val1 + val2;\n"
       << "                else if (currentOp == \"-\") result = val1 - val2;\n"
       << "                else if (currentOp == \"*\") result = val1 * val2;\n"
       << "                else if (currentOp == \"/\") {\n"
       << "                    if (val2 == 0) result = 0; // Prevent division by zero\n"
       << "                    else result = val1 / val2;\n"
       << "                }\n"
       << "            }\n"
       << "            // Float operations (at least one operand is float)\n"
       << "            else if ((std::holds_alternative<float>(result) || std::holds_alternative<int>(result)) &&\n"
       << "                     (std::holds_alternative<float>(tokenValue) || std::holds_alternative<int>(tokenValue))) {\n"
       << "                \n"
       << "                float val1 = std::holds_alternative<float>(result) ?\n"
       << "                            std::get<float>(result) : static_cast<float>(std::get<int>(result));\n"
       << "                            \n"
       << "                float val2 = std::holds_alternative<float>(tokenValue) ?\n"
       << "                            std::get<float>(tokenValue) : static_cast<float>(std::get<int>(tokenValue));\n"
       << "                \n"
       << "                if (currentOp == \"+\") result = val1 + val2;\n"
       << "                else if (currentOp == \"-\") result = val1 - val2;\n"
       << "                else if (currentOp == \"*\") result = val1 * val2;\n"
       << "                else if (currentOp == \"/\") {\n"
       << "                    if (val2 == 0.0f) result = 0.0f; // Prevent division by zero\n"
       << "                    else result = val1 / val2;\n"
       << "                }\n"
       << "            }\n"
       << "            // String concatenation\n"
       << "            else if (currentOp == \"+\") {\n"
       << "                // Convert both values to strings\n"
       << "                std::string val1;\n"
       << "                if (std::holds_alternative<int>(result)) {\n"
       << "                    val1 = std::to_string(std::get<int>(result));\n"
       << "                } else if (std::holds_alternative<float>(result)) {\n"
       << "                    val1 = std::to_string(std::get<float>(result));\n"
       << "                } else {\n"
       << "                    val1 = std::get<std::string>(result);\n"
       << "                }\n"
       << "                \n"
       << "                std::string val2;\n"
       << "                if (std::holds_alternative<int>(tokenValue)) {\n"
       << "                    val2 = std::to_string(std::get<int>(tokenValue));\n"
       << "                } else if (std::holds_alternative<float>(tokenValue)) {\n"
       << "                    val2 = std::to_string(std::get<float>(tokenValue));\n"
       << "                } else {\n"
       << "                    val2 = std::get<std::string>(tokenValue);\n"
       << "                }\n"
       << "                \n"
       << "                result = val1 + val2;\n"
       << "            }\n"
       << "        }\n"
       << "    }\n"
       << "    \n"
       << "    return result;\n"
       << "}\n\n";
    
    // Tokenize helper
    ss << "// Split an expression into tokens\n"
       << "std::vector<std::string> " << className << "::tokenize(const std::string& expr) {\n"
       << "    std::vector<std::string> tokens;\n"
       << "    std::string currentToken;\n"
       << "    \n"
       << "    for (size_t i = 0; i < expr.length(); i++) {\n"
       << "        char c = expr[i];\n"
       << "        \n"
       << "        // Skip whitespace\n"
       << "        if (std::isspace(c)) {\n"
       << "            if (!currentToken.empty()) {\n"
       << "                tokens.push_back(currentToken);\n"
       << "                currentToken.clear();\n"
       << "            }\n"
       << "            continue;\n"
       << "        }\n"
       << "        \n"
       << "        // Handle operators\n"
       << "        if (c == '+' || c == '-' || c == '*' || c == '/') {\n"
       << "            if (!currentToken.empty()) {\n"
       << "                tokens.push_back(currentToken);\n"
       << "                currentToken.clear();\n"
       << "            }\n"
       << "            tokens.push_back(std::string(1, c));\n"
       << "            continue;\n"
       << "        }\n"
       << "        \n"
       << "        // Handle numbers and identifiers\n"
       << "        currentToken += c;\n"
       << "    }\n"
       << "    \n"
       << "    // Add the last token if any\n"
       << "    if (!currentToken.empty()) {\n"
       << "        tokens.push_back(currentToken);\n"
       << "    }\n"
       << "    \n"
       << "    return tokens;\n"
       << "}\n\n";
    
    // Check timeouts
    ss << "// Check for timeout transitions\n"
       << "bool " << className << "::checkTimeouts() {\n"
       << "    auto now = std::chrono::steady_clock::now();\n"
       << "    \n"
       << "    // First check transitions with both input conditions and timeouts\n"
       << "    for (auto it = timeoutActive.begin(); it != timeoutActive.end(); ) {\n"
       << "        if (!it->second) {\n"
       << "            ++it;\n"
       << "            continue; // Skip inactive timeouts\n"
       << "        }\n"
       << "        \n"
       << "        std::string transitionId = it->first;\n"
       << "        // Find the transition with this ID\n"
       << "        Transition* transition = nullptr;\n"
       << "        for (auto* t : transitions) {\n"
       << "            if (t->guardExpression == transitionId && t->fromState == currentState) {\n"
       << "                transition = t;\n"
       << "                break;\n"
       << "            }\n"
       << "        }\n"
       << "        \n"
       << "        if (!transition) {\n"
       << "            // Invalid or not from current state\n"
       << "            it = timeoutActive.erase(it);\n"
       << "            timeoutStartTimes.erase(transitionId);\n"
       << "            continue;\n"
       << "        }\n"
       << "        \n"
       << "        // Check if timeout has expired\n"
       << "        auto startTime = timeoutStartTimes[transitionId];\n"
       << "        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - startTime).count();\n"
       << "        \n"
       << "        if (elapsed >= transition->timeout) {\n"
       << "            // Timeout expired - make the transition\n"
       << "            State* oldState = currentState;\n"
       << "            currentState = transition->toState;\n"
       << "            \n"
       << "            // Clear inputs that triggered this transition\n"
       << "            clearTriggeredInputs(transition);\n"
       << "            \n"
       << "            // Reset timer, mark state entry, clear timeouts\n"
       << "            stateEntryTime = now;\n"
       << "            justEnteredState = true;\n"
       << "            timeoutActive.clear();\n"
       << "            timeoutStartTimes.clear();\n"
       << "            \n"
       << "            // Process outputs from new state\n"
       << "            executeOutputActions();\n"
       << "            \n"
       << "            // Call callbacks\n"
       << "            if (stateChangeCallback) {\n"
       << "                stateChangeCallback(currentState->name);\n"
       << "            }\n"
       << "            \n"
       << "            auto stateCallbackIt = stateCallbacks.find(currentState->name);\n"
       << "            if (stateCallbackIt != stateCallbacks.end()) {\n"
       << "                stateCallbackIt->second(outputs, variables);\n"
       << "            }\n"
       << "            \n"
       << "            return true; // We made a transition\n"
       << "        }\n"
       << "        \n"
       << "        ++it;\n"
       << "    }\n"
       << "    \n"
       << "    // Then check pure timeout transitions (no input conditions)\n"
       << "    for (Transition* transition : transitions) {\n"
       << "        if (transition->fromState != currentState || \n"
       << "            transition->timeout <= 0 || \n"
       << "            !transition->guardExpression.empty()) {\n"
       << "            continue; // Skip non-timeout transitions or those with input conditions\n"
       << "        }\n"
       << "        \n"
       << "        // Calculate elapsed time since entering this state\n"
       << "        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - stateEntryTime).count();\n"
       << "        \n"
       << "        if (elapsed >= transition->timeout) {\n"
       << "            // Timeout triggered, transition to target state\n"
       << "            State* oldState = currentState;\n"
       << "            currentState = transition->toState;\n"
       << "            \n"
       << "            // Reset timer, mark state entry, clear timeouts\n"
       << "            stateEntryTime = now;\n"
       << "            justEnteredState = true;\n"
       << "            timeoutActive.clear();\n"
       << "            timeoutStartTimes.clear();\n"
       << "            \n"
       << "            // Process outputs from new state\n"
       << "            executeOutputActions();\n"
       << "            \n"
       << "            // Call callbacks\n"
       << "            if (stateChangeCallback) {\n"
       << "                stateChangeCallback(currentState->name);\n"
       << "            }\n"
       << "            \n"
       << "            auto stateCallbackIt = stateCallbacks.find(currentState->name);\n"
       << "            if (stateCallbackIt != stateCallbacks.end()) {\n"
       << "                stateCallbackIt->second(outputs, variables);\n"
       << "            }\n"
       << "            \n"
       << "            return true; // We made a transition\n"
       << "        }\n"
       << "    }\n"
       << "    \n"
       << "    return false; // No timeout transitions triggered\n"
       << "}\n\n";
    
    // Clear triggered inputs
    ss << "// Clear input values that triggered a transition\n"
       << "void " << className << "::clearTriggeredInputs(Transition* transition) {\n"
       << "    // Skip if there's no transition\n"
       << "    if (!transition) return;\n"
       << "    \n"
       << "    std::string expr = transition->guardExpression;\n"
       << "    size_t pos = 0, end;\n"
       << "    \n"
       << "    while ((end = expr.find(';', pos)) != std::string::npos) {\n"
       << "        std::string condition = expr.substr(pos, end - pos);\n"
       << "        pos = end + 1;\n"
       << "        \n"
       << "        // Skip empty conditions\n"
       << "        if (condition.empty()) continue;\n"
       << "        \n"
       << "        // Only handle regular input conditions (got value from source)\n"
       << "        std::regex inputPattern(\"got (.+) from (.+)\");\n"
       << "        std::smatch inputMatch;\n"
       << "        \n"
       << "        if (std::regex_match(condition, inputMatch, inputPattern)) {\n"
       << "            std::string source = inputMatch[2];\n"
       << "            // Trim whitespace\n"
       << "            source.erase(0, source.find_first_not_of(\" \\t\\r\\n\"));\n"
       << "            source.erase(source.find_last_not_of(\" \\t\\r\\n\") + 1);\n"
       << "            \n"
       << "            // Clear the input\n"
       << "            inputs[source] = \"\";\n"
       << "        }\n"
       << "    }\n"
       << "}\n";
    
    return ss.str();
}

std::string IncludableGenerator::generateGotoImplementation(const MooreMachine& machine, const std::string& className, const std::string& headerName) {
    std::stringstream ss;
    
    ss << "/**\n"
       << " * @file Auto-generated FSM implementation\n"
       << " * @brief Implements the " << className << " class using computed gotos\n"
       << " * @warning This file is auto-generated. Do not modify manually.\n"
       << " */\n\n"
       << "#include \"" << headerName << "\"\n"
       << "#include <algorithm>\n"
       << "#include <regex>\n"
       << "#include <sstream>\n"
       << "#include <iostream>\n\n"
       << "// Constructor - initializes the FSM\n"
       << className << "::" << className << "() : currentState(nullptr), justEnteredState(true) {\n";
    
    // Create input pointers
    auto inputPointers = machine.getInputPointers();
    for (const auto& pointer : inputPointers) {
        ss << "    inputs[\"" << pointer << "\"] = \"\";\n";
    }
    
    // Create output pointers
    auto outputPointers = machine.getOutputPointers();
    for (const auto& pointer : outputPointers) {
        ss << "    outputs[\"" << pointer << "\"] = \"\";\n";
    }
    
    // Create variables
    auto variables = machine.getVariables();
    for (const auto& varPair : variables) {
        const auto& var = varPair.second;
        
        ss << "    // Variable: " << var.getName() << " (" << typeToString(var.getType()) << ")\n";
        if (typeToString(var.getType()) == "string") {
            ss << "    variables[\"" << var.getName() << "\"] = std::string(\"" << var.getValueString() << "\");\n";
        } else if (typeToString(var.getType()) == "float") {
            ss << "    variables[\"" << var.getName() << "\"] = " << var.getValueString() << "f;\n";
        } else { // int
            ss << "    variables[\"" << var.getName() << "\"] = " << var.getValueString() << ";\n";
        }
    }
    
    // Create states
    State* initialState = nullptr;
    std::vector<State*> statesList;
    
    for (State* state : const_cast<MooreMachine&>(machine).getAllStates()) {
        if (state->getIsInitial()) {
            initialState = state;
        }
        statesList.push_back(state);
    }
    
    // Make sure initial state comes first
    if (initialState) {
        auto it = std::find(statesList.begin(), statesList.end(), initialState);
        if (it != statesList.begin() && it != statesList.end()) {
            std::iter_swap(statesList.begin(), it);
        }
    }
    
    int stateIdx = 0;
    for (State* state : statesList) {
        std::string stateName = state->getName();
        
        ss << "\n    // Create state: " << stateName << "\n";
        
        // Format state outputs
        std::string outputExpr;
        auto outputs = state->getOutputs();
        
        if (!outputs.empty()) {
            for (const auto& output : outputs) {
                // Check if this is a variable expression or regular output
                if (output.value.find("=") != std::string::npos ||
                    output.value.find("+") != std::string::npos ||
                    output.value.find("-") != std::string::npos ||
                    output.value.find("*") != std::string::npos ||
                    output.value.find("/") != std::string::npos) {
                    outputExpr += output.value;
                } else {
                    outputExpr += "output " + output.value + " to " + output.target;
                }
                
                // Add condition if present
                if (output.hasCondition) {
                    outputExpr += " if " + output.inputPtr + " is defined";
                }
                
                outputExpr += ";";
            }
        }
        
        ss << "    states[\"" << stateName << "\"] = new State(\"" << stateName 
           << "\", \"" << outputExpr << "\", STATE_" << stateName << ");\n";
        
        // Set initial state
        if (state->getIsInitial()) {
            ss << "    currentState = states[\"" << stateName << "\"];\n";
            ss << "    stateEntryTime = std::chrono::steady_clock::now();\n";
        }
        
        stateIdx++;
    }

    
    
    ss << "\n#if USE_COMPUTED_GOTO\n"
    << "    // Initialize stateTargets map with nullptr values\n";
    
    for (State* state : statesList) {
        ss << "    stateTargets[STATE_" << state->getName() << "] = nullptr;\n";
    }
    
    ss << "#endif\n\n";
    
    
    // Create transitions
    for (State* sourceState : statesList) {
        std::string sourceName = sourceState->getName();
        auto outTransitions = const_cast<MooreMachine&>(machine).getTransitionsFromState(sourceState->getId());
        
        for (Transition* transition : outTransitions) {
            std::string targetId = transition->getTargetId();
            State* targetState = const_cast<MooreMachine&>(machine).getState(targetId);
            
            if (!targetState) continue;
            
            std::string targetName = targetState->getName();
            
            ss << "\n    // Transition from " << sourceName << " to " << targetName << "\n";
            
            // Build guard expression
            std::string guardExpr;
            auto inputConditions = transition->getInputConditions();
            
            for (const auto& cond : inputConditions) {
                if (cond.isBooleanExpr) {
                    guardExpr += cond.leftOperand + " " + cond.operation + " " + cond.rightOperand + ";";
                } else {
                    guardExpr += "got " + cond.value + " from " + cond.source + ";";
                }
            }
            
            // Add the transition
            ss << "    transitions.push_back(new Transition(\n"
               << "        states[\"" << sourceName << "\"],\n"
               << "        states[\"" << targetName << "\"],\n"
               << "        \"" << guardExpr << "\",\n"
               << "        " << transition->getTimeout() << "));\n";
        }
    }
    
    ss << "}\n\n";
    
    // Destructor
    ss << "// Destructor - cleans up allocated memory\n"
       << className << "::~" << className << "() {\n"
       << "    for (auto& transition : transitions) {\n"
       << "        delete transition;\n"
       << "    }\n"
       << "    \n"
       << "    for (auto& pair : states) {\n"
       << "        delete pair.second;\n"
       << "    }\n"
       << "}\n\n";
    
    // Reset method
    ss << "// Reset the FSM to its initial state\n"
       << "void " << className << "::reset() {\n"
       << "    // Find initial state\n"
       << "    for (const auto& pair : states) {\n"
       << "        // Use the first state as fallback\n"
       << "        if (currentState == nullptr) {\n"
       << "            currentState = pair.second;\n"
       << "        }\n"
       << "        \n"
       << "        // Use initial state if available\n";
    
    if (initialState) {
        ss << "        if (pair.first == \"" << initialState->getName() << "\") {\n"
           << "            currentState = pair.second;\n"
           << "            break;\n"
           << "        }\n";
    }
    
    ss << "    }\n"
       << "    \n"
       << "    stateEntryTime = std::chrono::steady_clock::now();\n"
       << "    justEnteredState = true;\n"
       << "    activeTimeouts.clear();\n"
       << "    \n"
       << "    // Clear inputs and outputs\n"
       << "    for (auto& pair : inputs) {\n"
       << "        pair.second = \"\";\n"
       << "    }\n"
       << "    \n"
       << "    for (auto& pair : outputs) {\n"
       << "        pair.second = \"\";\n"
       << "    }\n"
       << "    \n"
       << "    // Execute output actions for initial state\n"
       << "    if (currentState) {\n"
       << "        executeOutputActions();\n"
       << "    }\n"
       << "}\n\n";
    
    // Process step method
    ss << "// Process a step in the FSM\n"
       << "void " << className << "::tick() {\n"
       << "    if (!currentState) {\n"
       << "        reset();\n"
       << "        return;\n"
       << "    }\n"
       << "    \n"
       << "    // If we just entered this state, skip immediate transition evaluation\n"
       << "    if (justEnteredState) {\n"
       << "        justEnteredState = false;\n"
       << "        return;\n"
       << "    }\n"
       << "    \n"
       << "    // Check timeouts first\n"
       << "    bool timeoutTriggered = checkTimeouts();\n"
       << "    if (timeoutTriggered) {\n"
       << "        return; // Already transitioned due to timeout\n"
       << "    }\n"
       << "    \n"
       << "    // Evaluate transitions\n"
       << "    for (Transition* transition : transitions) {\n"
       << "        if (transition->fromState == currentState && evaluateTransition(transition)) {\n"
       << "            // If transition has both input conditions and timeout\n"
       << "            if (transition->timeout > 0) {\n"
       << "                if (activeTimeouts.find(transition) == activeTimeouts.end()) {\n"
       << "                    activeTimeouts[transition] = std::chrono::steady_clock::now();\n"
       << "                }\n"
       << "                continue; // Wait for timeout to expire\n"
       << "            }\n"
       << "            \n"
       << "            // Immediate transition (no timeout)\n"
       << "            State* oldState = currentState;\n"
       << "            currentState = transition->toState;\n"
       << "            \n"
       << "            // Clear inputs that triggered this transition\n"
       << "            clearTriggeredInputs(transition);\n"
       << "            \n"
       << "            // Reset state entry time if state changed\n"
       << "            if (oldState != currentState) {\n"
       << "                stateEntryTime = std::chrono::steady_clock::now();\n"
       << "                justEnteredState = true;\n"
       << "                timeoutActive.clear();\n"
       << "                timeoutStartTimes.clear();\n"
       << "                \n"
       << "                // Execute output actions\n"
       << "                executeOutputActions();\n"
       << "            }\n"
       << "            \n"
       << "            break;\n"
       << "        }\n"
       << "    }\n"
       << "}\n\n";
    
    // Process input method
    ss << "// Process an input through the machine\n"
       << "std::string " << className << "::processInput(const std::string& input, const std::string& inputPtr) {\n"
       << "    // Set the input\n"
       << "    inputs[inputPtr] = input;\n"
       << "    justEnteredState = false; // Allow transitions to be evaluated\n"
       << "    \n"
       << "    // Process a step\n"
       << "    tick();\n"
       << "    \n"
       << "    // Return output from default pointer\n"
       << "    return getOutput(\"default\");\n"
       << "}\n\n";
    
    // Get current state name
    ss << "// Get the current state name\n"
       << "std::string " << className << "::getCurrentStateName() const {\n"
       << "    return currentState ? currentState->name : \"\";\n"
       << "}\n\n";
    
    // Get current state ID
    ss << "// Get the current state ID\n"
       << className << "::StateId " << className << "::getCurrentStateId() const {\n"
       << "    return currentState ? currentState->id : static_cast<StateId>(0);\n"
       << "}\n\n";
    
    // Get all outputs
    ss << "// Get all current outputs\n"
       << "std::unordered_map<std::string, std::string> " << className << "::getOutputs() const {\n"
       << "    return outputs;\n"
       << "}\n\n";
    
    // Get specific output
    ss << "// Get a specific output\n"
       << "std::string " << className << "::getOutput(const std::string& outputPtr) const {\n"
       << "    auto it = outputs.find(outputPtr);\n"
       << "    if (it != outputs.end()) {\n"
       << "        return it->second;\n"
       << "    }\n"
       << "    return \"\";\n"
       << "}\n\n";
    
    // Execute output actions
    ss << "// Execute output actions for the current state\n"
       << "void " << className << "::executeOutputActions() {\n"
       << "    if (!currentState) return;\n"
       << "    \n"
       << "    std::string expr = currentState->outputExpression;\n"
       << "    size_t pos = 0, end;\n"
       << "    \n"
       << "    while ((end = expr.find(';', pos)) != std::string::npos) {\n"
       << "        std::string action = expr.substr(pos, end - pos);\n"
       << "        pos = end + 1;\n"
       << "        \n"
       << "        // Skip empty actions\n"
       << "        if (action.empty()) continue;\n"
       << "        \n"
       << "        // Handle conditional actions (if inputPtr is defined)\n"
       << "        std::regex ifPattern(\"(.+) if (.+) is defined\");\n"
       << "        std::smatch ifMatch;\n"
       << "        \n"
       << "        if (std::regex_match(action, ifMatch, ifPattern)) {\n"
       << "            std::string actualAction = ifMatch[1];\n"
       << "            std::string inputPtr = ifMatch[2];\n"
       << "            \n"
       << "            // Check if input is defined\n"
       << "            auto inputIt = inputs.find(inputPtr);\n"
       << "            if (inputIt == inputs.end() || inputIt->second.empty()) {\n"
       << "                continue; // Skip this action\n"
       << "            }\n"
       << "            \n"
       << "            // Replace action with actual action\n"
       << "            action = actualAction;\n"
       << "        }\n"
       << "        \n"
       << "        // Handle variable assignments (var = ...)\n"
       << "        std::regex assignPattern(\"([^=]+)=(.+)\");\n"
       << "        std::smatch assignMatch;\n"
       << "        \n"
       << "        if (std::regex_match(action, assignMatch, assignPattern)) {\n"
       << "            std::string varName = assignMatch[1];\n"
       << "            std::string valueExpr = assignMatch[2];\n"
       << "            \n"
       << "            // Trim whitespace\n"
       << "            varName.erase(0, varName.find_first_not_of(\" \\t\\r\\n\"));\n"
       << "            varName.erase(varName.find_last_not_of(\" \\t\\r\\n\") + 1);\n"
       << "            valueExpr.erase(0, valueExpr.find_first_not_of(\" \\t\\r\\n\"));\n"
       << "            valueExpr.erase(valueExpr.find_last_not_of(\" \\t\\r\\n\") + 1);\n"
       << "            \n"
       << "            // Check for 'get' expression\n"
       << "            std::regex getPattern(\"get ([^\\\\s]+)\");\n"
       << "            std::smatch getMatch;\n"
       << "            \n"
       << "            if (std::regex_match(valueExpr, getMatch, getPattern)) {\n"
       << "                std::string inputPtr = getMatch[1];\n"
       << "                auto inputIt = inputs.find(inputPtr);\n"
       << "                \n"
       << "                if (inputIt != inputs.end()) {\n"
       << "                    // Find variable type\n"
       << "                    auto varIt = variables.find(varName);\n"
       << "                    if (varIt != variables.end()) {\n"
       << "                        if (std::holds_alternative<int>(varIt->second)) {\n"
       << "                            try {\n"
       << "                                variables[varName] = std::stoi(inputIt->second);\n"
       << "                            } catch (...) {\n"
       << "                                variables[varName] = 0;\n"
       << "                            }\n"
       << "                        } else if (std::holds_alternative<float>(varIt->second)) {\n"
       << "                            try {\n"
       << "                                variables[varName] = std::stof(inputIt->second);\n"
       << "                            } catch (...) {\n"
       << "                                variables[varName] = 0.0f;\n"
       << "                            }\n"
       << "                        } else {\n"
       << "                            variables[varName] = inputIt->second;\n"
       << "                        }\n"
       << "                    } else {\n"
       << "                        // Default to string\n"
       << "                        variables[varName] = inputIt->second;\n"
       << "                    }\n"
       << "                }\n"
       << "            } else {\n"
       << "                // Evaluate expression\n"
       << "                variables[varName] = evaluateExpression(valueExpr);\n"
       << "            }\n"
       << "            \n"
       << "            continue;\n"
       << "        }\n"
       << "        \n"
       << "        // Handle output expressions (output value to target)\n"
       << "        std::regex outputPattern(\"output (.+) to (.+)\");\n"
       << "        std::smatch outputMatch;\n"
       << "        \n"
       << "        if (std::regex_match(action, outputMatch, outputPattern)) {\n"
       << "            std::string value = outputMatch[1];\n"
       << "            std::string target = outputMatch[2];\n"
       << "            \n"
       << "            // Trim whitespace\n"
       << "            value.erase(0, value.find_first_not_of(\" \\t\\r\\n\"));\n"
       << "            value.erase(value.find_last_not_of(\" \\t\\r\\n\") + 1);\n"
       << "            target.erase(0, target.find_first_not_of(\" \\t\\r\\n\"));\n"
       << "            target.erase(target.find_last_not_of(\" \\t\\r\\n\") + 1);\n"
       << "            \n"
       << "            // Evaluate the value if it's an expression\n"
       << "            if (value.find_first_of(\"+-*/\") != std::string::npos) {\n"
       << "                auto result = evaluateExpression(value);\n"
       << "                std::string strResult;\n"
       << "                \n"
       << "                if (std::holds_alternative<int>(result)) {\n"
       << "                    strResult = std::to_string(std::get<int>(result));\n"
       << "                } else if (std::holds_alternative<float>(result)) {\n"
       << "                    strResult = std::to_string(std::get<float>(result));\n"
       << "                } else {\n"
       << "                    strResult = std::get<std::string>(result);\n"
       << "                }\n"
       << "                \n"
       << "                value = strResult;\n"
       << "            }\n"
       << "            \n"
       << "            outputs[target] = value;\n"
       << "        }\n"
       << "    }\n"
       << "}\n\n";
    
    // Evaluate transition
ss << "// Evaluate if a transition should be triggered\n"
   << "bool " << className << "::evaluateTransition(Transition* transition) {\n"
   << "    // For timeout transitions, check if the timeout is active\n"
   << "    if (transition->timeout > 0) {\n"
   << "        // Start the timer if not already started\n"
   << "        if (activeTimeouts.find(transition) == activeTimeouts.end()) {\n"
   << "            activeTimeouts[transition] = std::chrono::steady_clock::now();\n"
   << "            return false; // Wait for timeout\n"
   << "        }\n"
   << "    }\n"
   << "    \n"
   << "    std::string expr = transition->guardExpression;\n"
   << "    if (expr.empty()) {\n"
   << "        return false;\n"
   << "    }\n"
   << "    \n"
   << "    size_t pos = 0, end;\n"
   << "    bool result = false;\n"
   << "    \n"
   << "    while ((end = expr.find(';', pos)) != std::string::npos) {\n"
   << "        std::string condition = expr.substr(pos, end - pos);\n"
   << "        pos = end + 1;\n"
   << "        \n"
   << "        // Skip empty conditions\n"
   << "        if (condition.empty()) continue;\n"
   << "        \n"
   << "        // Handle boolean expressions (var == value or var != value)\n"
   << "        std::regex boolPattern(\"(.+) ?(==|!=) ?(.+)\");\n"
   << "        std::smatch boolMatch;\n"
   << "        \n"
   << "        if (std::regex_match(condition, boolMatch, boolPattern)) {\n"
   << "            std::string leftOp = boolMatch[1];\n"
   << "            std::string op = boolMatch[2];\n"
   << "            std::string rightOp = boolMatch[3];\n"
   << "            \n"
   << "            // Trim whitespace\n"
   << "            leftOp.erase(0, leftOp.find_first_not_of(\" \\t\\r\\n\"));\n"
   << "            leftOp.erase(leftOp.find_last_not_of(\" \\t\\r\\n\") + 1);\n"
   << "            rightOp.erase(0, rightOp.find_first_not_of(\" \\t\\r\\n\"));\n"
   << "            rightOp.erase(rightOp.find_last_not_of(\" \\t\\r\\n\") + 1);\n"
   << "            \n"
   << "            // Get left operand value\n"
   << "            auto leftIt = variables.find(leftOp);\n"
   << "            if (leftIt == variables.end()) {\n"
   << "                continue; // Skip this condition if variable not found\n"
   << "            }\n"
   << "            \n"
   << "            // Get right operand value\n"
   << "            VariableType rightValue;\n"
   << "            auto rightIt = variables.find(rightOp);\n"
   << "            \n"
   << "            if (rightIt != variables.end()) {\n"
   << "                rightValue = rightIt->second;\n"
   << "            } else {\n"
   << "                // Try to parse as literal based on left operand type\n"
   << "                if (std::holds_alternative<int>(leftIt->second)) {\n"
   << "                    try {\n"
   << "                        rightValue = std::stoi(rightOp);\n"
   << "                    } catch (...) {\n"
   << "                        continue; // Invalid conversion\n"
   << "                    }\n"
   << "                } else if (std::holds_alternative<float>(leftIt->second)) {\n"
   << "                    try {\n"
   << "                        rightValue = std::stof(rightOp);\n"
   << "                    } catch (...) {\n"
   << "                        continue; // Invalid conversion\n"
   << "                    }\n"
   << "                } else {\n"
   << "                    rightValue = rightOp; // String literal\n"
   << "                }\n"
   << "            }\n"
   << "            \n"
   << "            // Compare values\n"
   << "            bool conditionMet = false;\n"
   << "            \n"
   << "            if (op == \"==\") {\n"
   << "                conditionMet = (leftIt->second == rightValue);\n"
   << "            } else { // op == \"!=\"\n"
   << "                conditionMet = (leftIt->second != rightValue);\n"
   << "            }\n"
   << "            \n"
   << "            result = conditionMet;\n"
   << "            if (!result) {\n"
   << "                return false; // If any condition fails, the whole transition fails\n"
   << "            }\n"
   << "        }\n"
   << "        \n"
   << "        // Handle input conditions (got value from source)\n"
   << "        std::regex inputPattern(\"got (.+) from (.+)\");\n"
   << "        std::smatch inputMatch;\n"
   << "        \n"
   << "        if (std::regex_match(condition, inputMatch, inputPattern)) {\n"
   << "            std::string value = inputMatch[1];\n"
   << "            std::string source = inputMatch[2];\n"
   << "            \n"
   << "            // Trim whitespace\n"
   << "            value.erase(0, value.find_first_not_of(\" \\t\\r\\n\"));\n"
   << "            value.erase(value.find_last_not_of(\" \\t\\r\\n\") + 1);\n"
   << "            source.erase(0, source.find_first_not_of(\" \\t\\r\\n\"));\n"
   << "            source.erase(source.find_last_not_of(\" \\t\\r\\n\") + 1);\n"
   << "            \n"
   << "            // Check if input matches\n"
   << "            auto inputIt = inputs.find(source);\n"
   << "            if (inputIt != inputs.end() && inputIt->second == value) {\n"
   << "                result = true;\n"
   << "            } else {\n"
   << "                return false; // If any condition fails, the whole transition fails\n"
   << "            }\n"
   << "        }\n"
   << "    }\n"
   << "    \n"
   << "    return result;\n"
   << "}\n\n";

// Evaluate expression
ss << "// Evaluate a simple arithmetic expression\n"
   << className << "::VariableType " << className << "::evaluateExpression(const std::string& expression) {\n"
   << "    // Simple expression evaluation (no precedence)\n"
   << "    std::vector<std::string> tokens = tokenize(expression);\n"
   << "    \n"
   << "    if (tokens.empty()) {\n"
   << "        return std::string(\"\");\n"
   << "    }\n"
   << "    \n"
   << "    VariableType result;\n"
   << "    std::string currentOp = \"+\"; // Default operation\n"
   << "    bool firstToken = true;\n"
   << "    \n"
   << "    for (size_t i = 0; i < tokens.size(); i++) {\n"
   << "        const std::string& token = tokens[i];\n"
   << "        \n"
   << "        // Skip empty tokens\n"
   << "        if (token.empty()) continue;\n"
   << "        \n"
   << "        // Handle operators\n"
   << "        if (token == \"+\" || token == \"-\" || token == \"*\" || token == \"/\") {\n"
   << "            currentOp = token;\n"
   << "            continue;\n"
   << "        }\n"
   << "        \n"
   << "        // Get token value\n"
   << "        VariableType tokenValue;\n"
   << "        \n"
   << "        // Check if token is a variable\n"
   << "        auto varIt = variables.find(token);\n"
   << "        if (varIt != variables.end()) {\n"
   << "            tokenValue = varIt->second;\n"
   << "        }\n"
   << "        // Check if token is 'elapsed' keyword\n"
   << "        else if (token == \"elapsed\") {\n"
   << "            auto now = std::chrono::steady_clock::now();\n"
   << "            auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - stateEntryTime).count();\n"
   << "            tokenValue = static_cast<int>(elapsed);\n"
   << "        }\n"
   << "        // Try to parse as number\n"
   << "        else {\n"
   << "            try {\n"
   << "                // Check if it's a floating point number\n"
   << "                if (token.find('.') != std::string::npos) {\n"
   << "                    tokenValue = std::stof(token);\n"
   << "                } else {\n"
   << "                    tokenValue = std::stoi(token);\n"
   << "                }\n"
   << "            } catch (...) {\n"
   << "                // Default to string\n"
   << "                tokenValue = token;\n"
   << "            }\n"
   << "        }\n"
   << "        \n"
   << "        // Set initial result or apply operation\n"
   << "        if (firstToken) {\n"
   << "            result = tokenValue;\n"
   << "            firstToken = false;\n"
   << "        } else {\n"
   << "            // Integer operations\n"
   << "            if (std::holds_alternative<int>(result) && std::holds_alternative<int>(tokenValue)) {\n"
   << "                int val1 = std::get<int>(result);\n"
   << "                int val2 = std::get<int>(tokenValue);\n"
   << "                \n"
   << "                if (currentOp == \"+\") result = val1 + val2;\n"
   << "                else if (currentOp == \"-\") result = val1 - val2;\n"
   << "                else if (currentOp == \"*\") result = val1 * val2;\n"
   << "                else if (currentOp == \"/\") {\n"
   << "                    if (val2 == 0) result = 0; // Prevent division by zero\n"
   << "                    else result = val1 / val2;\n"
   << "                }\n"
   << "            }\n"
   << "            // Float operations (at least one operand is float)\n"
   << "            else if ((std::holds_alternative<float>(result) || std::holds_alternative<int>(result)) &&\n"
   << "                     (std::holds_alternative<float>(tokenValue) || std::holds_alternative<int>(tokenValue))) {\n"
   << "                \n"
   << "                float val1 = std::holds_alternative<float>(result) ?\n"
   << "                            std::get<float>(result) : static_cast<float>(std::get<int>(result));\n"
   << "                            \n"
   << "                float val2 = std::holds_alternative<float>(tokenValue) ?\n"
   << "                            std::get<float>(tokenValue) : static_cast<float>(std::get<int>(tokenValue));\n"
   << "                \n"
   << "                if (currentOp == \"+\") result = val1 + val2;\n"
   << "                else if (currentOp == \"-\") result = val1 - val2;\n"
   << "                else if (currentOp == \"*\") result = val1 * val2;\n"
   << "                else if (currentOp == \"/\") {\n"
   << "                    if (val2 == 0.0f) result = 0.0f; // Prevent division by zero\n"
   << "                    else result = val1 / val2;\n"
   << "                }\n"
   << "            }\n"
   << "            // String concatenation\n"
   << "            else if (currentOp == \"+\") {\n"
   << "                // Convert both values to strings\n"
   << "                std::string val1;\n"
   << "                if (std::holds_alternative<int>(result)) {\n"
   << "                    val1 = std::to_string(std::get<int>(result));\n"
   << "                } else if (std::holds_alternative<float>(result)) {\n"
   << "                    val1 = std::to_string(std::get<float>(result));\n"
   << "                } else {\n"
   << "                    val1 = std::get<std::string>(result);\n"
   << "                }\n"
   << "                \n"
   << "                std::string val2;\n"
   << "                if (std::holds_alternative<int>(tokenValue)) {\n"
   << "                    val2 = std::to_string(std::get<int>(tokenValue));\n"
   << "                } else if (std::holds_alternative<float>(tokenValue)) {\n"
   << "                    val2 = std::to_string(std::get<float>(tokenValue));\n"
   << "                } else {\n"
   << "                    val2 = std::get<std::string>(tokenValue);\n"
   << "                }\n"
   << "                \n"
   << "                result = val1 + val2;\n"
   << "            }\n"
   << "        }\n"
   << "    }\n"
   << "    \n"
   << "    return result;\n"
   << "}\n\n";

// Tokenize helper
ss << "// Split an expression into tokens\n"
   << "std::vector<std::string> " << className << "::tokenize(const std::string& expr) {\n"
   << "    std::vector<std::string> tokens;\n"
   << "    std::string currentToken;\n"
   << "    \n"
   << "    for (size_t i = 0; i < expr.length(); i++) {\n"
   << "        char c = expr[i];\n"
   << "        \n"
   << "        // Skip whitespace\n"
   << "        if (std::isspace(c)) {\n"
   << "            if (!currentToken.empty()) {\n"
   << "                tokens.push_back(currentToken);\n"
   << "                currentToken.clear();\n"
   << "            }\n"
   << "            continue;\n"
   << "        }\n"
   << "        \n"
   << "        // Handle operators\n"
   << "        if (c == '+' || c == '-' || c == '*' || c == '/') {\n"
   << "            if (!currentToken.empty()) {\n"
   << "                tokens.push_back(currentToken);\n"
   << "                currentToken.clear();\n"
   << "            }\n"
   << "            tokens.push_back(std::string(1, c));\n"
   << "            continue;\n"
   << "        }\n"
   << "        \n"
   << "        // Handle numbers and identifiers\n"
   << "        currentToken += c;\n"
   << "    }\n"
   << "    \n"
   << "    // Add the last token if any\n"
   << "    if (!currentToken.empty()) {\n"
   << "        tokens.push_back(currentToken);\n"
   << "    }\n"
   << "    \n"
   << "    return tokens;\n"
   << "}\n\n";

// Check timeouts
ss << "// Check for timeout transitions\n"
   << "bool " << className << "::checkTimeouts() {\n"
   << "    auto now = std::chrono::steady_clock::now();\n"
   << "    bool transitionOccurred = false;\n"
   << "    \n"
   << "    for (Transition* transition : transitions) {\n"
   << "        if (transition->fromState != currentState) {\n"
   << "            continue;\n"
   << "        }\n"
   << "        \n"
   << "        if (transition->timeout > 0) {\n"
   << "            auto it = activeTimeouts.find(transition);\n"
   << "            if (it != activeTimeouts.end()) {\n"
   << "                auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - it->second).count();\n"
   << "                \n"
   << "                if (elapsed >= transition->timeout) {\n"
   << "                    // Timeout triggered - transition to target state\n"
   << "                    State* oldState = currentState;\n"
   << "                    currentState = transition->toState;\n"
   << "                    \n"
   << "                    stateEntryTime = now;\n"
   << "                    activeTimeouts.clear();\n"
   << "                    \n"
   << "                    // Execute output actions\n"
   << "                    executeOutputActions();\n"
   << "                    transitionOccurred = true;\n"
   << "                    \n"
   << "                    break;\n"
   << "                }\n"
   << "            }\n"
   << "        }\n"
   << "    }\n"
   << "    \n"
   << "    return transitionOccurred;\n"
   << "}\n\n";

// Run with gotos - the main function for computed goto style
ss << "// Run the state machine using the GNU computed goto extension\n"
   << "void " << className << "::runWithGotos() {\n"
   << "#if USE_COMPUTED_GOTO\n"
   << "    if (!currentState) {\n"
   << "        reset();\n"
   << "        return;\n"
   << "    }\n"
   << "    \n"
   << "    // Dispatch based on current state\n"
   << "    StateId stateId = currentState->id;\n"
   << "    \n"
   << "    // Use direct access with check\n"
   << "    try {\n"
   << "        void* target = stateTargets.at(stateId);\n"
   << "        if (target != nullptr) {\n"
   << "            // Jump to the registered label\n"
   << "            goto *target;\n"
   << "            return;\n"
   << "        }\n"
   << "    } catch (const std::exception&) {\n"
   << "        // Handle missing state targets safely\n"
   << "    }\n"
   << "    \n"
   << "    // Default behavior if no target is registered or exception occurred\n"
   << "    tick();\n"
   << "#else\n"
   << "    // Fallback for compilers without computed goto support\n"
   << "    tick();\n"
   << "#endif\n"
   << "}\n";

   ss << "// Clear inputs that triggered a transition\n"
   << "void " << className << "::clearTriggeredInputs(Transition* transition) {\n"
   << "    if (!transition) return;\n"
   << "    \n"
   << "    // Iterate through all input conditions in the transition\n"
   << "    std::string expr = transition->guardExpression;\n"
   << "    size_t pos = 0, end;\n"
   << "    \n"
   << "    while ((end = expr.find(';', pos)) != std::string::npos) {\n"
   << "        std::string condition = expr.substr(pos, end - pos);\n"
   << "        pos = end + 1;\n"
   << "        \n"
   << "        // Skip empty conditions\n"
   << "        if (condition.empty()) continue;\n"
   << "        \n"
   << "        // Only clear non-boolean inputs (got X from Y conditions)\n"
   << "        std::regex inputPattern(\"got (.+) from (.+)\");\n"
   << "        std::smatch inputMatch;\n"
   << "        \n"
   << "        if (std::regex_match(condition, inputMatch, inputPattern)) {\n"
   << "            std::string value = inputMatch[1];\n"
   << "            std::string source = inputMatch[2];\n"
   << "            \n"
   << "            // Trim whitespace\n"
   << "            source.erase(0, source.find_first_not_of(\" \\t\\r\\n\"));\n"
   << "            source.erase(source.find_last_not_of(\" \\t\\r\\n\") + 1);\n"
   << "            \n"
   << "            // Clear the input that triggered this transition\n"
   << "            inputs[source] = \"\";\n"
   << "        }\n"
   << "    }\n"
   << "}\n\n";

   ss << "void* CounterFSM::getStateTarget(StateId id) const {\n"
      << "    auto it = stateTargets.find(id);\n"
      << "    if (it != stateTargets.end()) {\n"
      << "        return it->second;\n"
      << "    }\n"
      << "    return nullptr;\n"
      << "}\n\n";

   return ss.str();
}