/**
 * @file machine_file_handler.cpp
 * @brief Implementation of the MachineFileHandler class
 * @author Hugo Bohácsek (xbohach00)
 */

#include "../headers/machine_file_handler.h"
#include <fstream>
#include <sstream>
#include <algorithm>
#include <regex>
#include <stdexcept>
#include "../headers/string_utils.h"

/**
 * @brief Saves a machine to a file
 * 
 * Serializes the complete machine state including:
 * - Machine name
 * - Input and output alphabets
 * - Input and output pointers
 * - Variables
 * - States (with initial state first)
 * - Transitions
 * 
 * @param machine The machine to save
 * @param filename Path to the file where the machine will be saved
 * @return True if saving succeeded, false otherwise
 */
bool MachineFileHandler::saveToFile(const MooreMachine& machine, const std::string& filename) {
    std::ofstream file(filename);
    if (!file.is_open()) {
        return false;
    }
    
    // First line is mandatory
    file << "Name " << machine.getName() << std::endl;
    
    // Write input alphabet
    file << "Input alphabet is ";
    auto inputAlphabet = machine.getInputAlphabet();
    bool first = true;
    for (const auto& symbol : inputAlphabet) {
        if (!first) {
            file << ", ";
        }
        file << symbol;
        first = false;
    }
    file << std::endl;
    
    // Write output alphabet
    file << "Output alphabet is ";
    auto outputAlphabet = machine.getOutputAlphabet();
    first = true;
    for (const auto& symbol : outputAlphabet) {
        if (!first) {
            file << ", ";
        }
        file << symbol;
        first = false;
    }
    file << std::endl;
    
    // Write input pointers
    file << "Input pointers are ";
    auto inputPointers = machine.getInputPointers();
    first = true;
    for (const auto& pointer : inputPointers) {
        if (!first) {
            file << ", ";
        }
        file << pointer;
        first = false;
    }
    file << std::endl;
    
    // Write output pointers
    file << "Output pointers are ";
    auto outputPointers = machine.getOutputPointers();
    first = true;
    for (const auto& pointer : outputPointers) {
        if (!first) {
            file << ", ";
        }
        file << pointer;
        first = false;
    }
    file << std::endl;
    
    // Write variables
    file << "Variables are" << std::endl;
    auto variables = machine.getVariables();
    for (const auto& varPair : variables) {
        const auto& var = varPair.second;
        file << "\t" << typeToString(var.getType()) << " " << var.getName() 
             << " = " << var.getValueString() << std::endl;
    }
    
    // Write states - get initial state to write first
    file << "States are" << std::endl;
    State* initialState = const_cast<MooreMachine&>(machine).getInitialState();
    
    // Write initial state first
    if (initialState) {
        file << "\t" << initialState->getName() << ": ";
        
        // Get all outputs
        auto outputs = initialState->getOutputs();
        if (!outputs.empty()) {
            file << "[";
            
            bool firstOutput = true;
            for (const auto& output : outputs) {
                if (!firstOutput) {
                    file << "; ";
                }
                firstOutput = false;
                
                // Check if this is a variable expression or regular output
                if (output.value.find("=") != std::string::npos || 
                    output.value.find("+") != std::string::npos ||
                    output.value.find("-") != std::string::npos ||
                    output.value.find("*") != std::string::npos ||
                    output.value.find("/") != std::string::npos) {
                    file << output.value;
                } else {
                    file << "output " << output.value << " to " << output.target;
                }
                
                // Add condition if present
                if (output.hasCondition) {
                    file << " if " << output.inputPtr << " is defined";
                }
            }
            
            file << "]";
        }
        
        file << std::endl;
    }
    
    // Write the rest of the states
    for (State* state : const_cast<MooreMachine&>(machine).getAllStates()) {
        // Skip the initial state (already written)
        if (initialState && state->getId() == initialState->getId()) {
            continue;
        }
        
        file << "\t" << state->getName() << ": ";
        
        // Get all outputs
        auto outputs = state->getOutputs();
        if (!outputs.empty()) {
            file << "[";
            
            bool firstOutput = true;
            for (const auto& output : outputs) {
                if (!firstOutput) {
                    file << "; ";
                }
                firstOutput = false;
                
                // Check if this is a variable expression or regular output
                if (output.value.find("=") != std::string::npos || 
                    output.value.find("+") != std::string::npos ||
                    output.value.find("-") != std::string::npos ||
                    output.value.find("*") != std::string::npos ||
                    output.value.find("/") != std::string::npos) {
                    file << output.value;
                } else {
                    file << "output " << output.value << " to " << output.target;
                }
                
                // Add condition if present
                if (output.hasCondition) {
                    file << " if " << output.inputPtr << " is defined";
                }
            }
            
            file << "]";
        }
        
        file << std::endl;
    }
    
    // Write transitions
    file << "Transitions are" << std::endl;
    for (State* sourceState : const_cast<MooreMachine&>(machine).getAllStates()) {
        auto transitions = const_cast<MooreMachine&>(machine).getTransitionsFromState(sourceState->getId());
        
        // Group transitions by target state
        std::unordered_map<std::string, std::vector<Transition*>> transitionsByTarget;
        for (Transition* transition : transitions) {
            transitionsByTarget[transition->getTargetId()].push_back(transition);
        }
        
        // Write each target group
        for (const auto& targetGroup : transitionsByTarget) {
            State* targetState = const_cast<MooreMachine&>(machine).getState(targetGroup.first);
            
            file << "\t" << sourceState->getName() << "-[";
            
            bool firstCondition = true;
            for (Transition* transition : targetGroup.second) {
                // Write all input conditions
                for (const auto& inputCondition : transition->getInputConditions()) {
                    if (!firstCondition) {
                        file << "; ";
                    }
                    firstCondition = false;
                    
                    // Handle boolean expressions
                    if (inputCondition.isBooleanExpr) {
                        file << inputCondition.leftOperand << " " 
                            << inputCondition.operation << " " 
                            << inputCondition.rightOperand;
                    } else {
                        file << "got " << inputCondition.value << " from " << inputCondition.source;
                    }
                }
                
                // Write timeout if present
                if (transition->getTimeout() > 0) {
                    if (!firstCondition) {
                        file << "; ";
                    }
                    firstCondition = false;
                    
                    file << "timeout " << transition->getTimeout();
                }
            }
            
            file << "]->" << targetState->getName() << std::endl;
        }
    }
    
    // End file
    file << "Thanks" << std::endl;
    
    return file.good();
}

/**
 * @brief Loads a machine from a file
 * 
 * Parses a file containing a machine definition and constructs a MooreMachine object
 * 
 * @param filename Path to the file containing the machine definition
 * @return Loaded MooreMachine object
 * @throw std::runtime_error If loading fails or file format is invalid
 */
MooreMachine MachineFileHandler::loadFromFile(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        throw std::runtime_error("Cannot open file: " + filename);
    }
    
    std::string line;
    
    // First line must be the machine name
    if (!std::getline(file, line)) {
        throw std::runtime_error("Empty file");
    }
    
    std::regex nameRegex("Name\\s+(.+)");
    std::smatch nameMatch;
    if (!std::regex_search(line, nameMatch, nameRegex)) {
        throw std::runtime_error("Invalid file format: expected 'Name {Machine name}' on first line");
    }
    
    std::string machineName = nameMatch[1];
    MooreMachine machine(machineName);
    
    enum class ParseState {
        HEADER,
        VARIABLES,
        STATES,
        TRANSITIONS,
        END
    };
    
    ParseState state = ParseState::HEADER;
    std::string firstStateId; // Keep track of the first state
    
    while (std::getline(file, line)) {
        // Skip comments
        if (line.find("voices") == 0) {
            continue;
        }
        
        // Trim the line
        line = trim(line);
        
        // Skip empty lines
        if (line.empty()) {
            continue;
        }
        
        // Check for section markers
        if (line == "Variables are") {
            state = ParseState::VARIABLES;
            continue;
        } else if (line == "States are") {
            state = ParseState::STATES;
            continue;
        } else if (line == "Transitions are") {
            state = ParseState::TRANSITIONS;
            continue;
        } else if (line == "Thanks") {
            state = ParseState::END;
            break;
        }
        
        // Process section content
        switch (state) {
            case ParseState::HEADER:
            {
                if (line.find("Input alphabet is") == 0) {
                    std::string alphabet = line.substr(17); // Skip "Input alphabet is "
                    auto symbols = split(alphabet, ',');
                    for (auto& symbol : symbols) {
                        symbol = trim(symbol);
                        if (!symbol.empty()) {
                            machine.addInputSymbol(symbol);
                        }
                    }
                } else if (line.find("Output alphabet is") == 0) {
                    std::string alphabet = line.substr(18); // Skip "Output alphabet is "
                    auto symbols = split(alphabet, ',');
                    for (auto& symbol : symbols) {
                        symbol = trim(symbol);
                        if (!symbol.empty()) {
                            machine.addOutputSymbol(symbol);
                        }
                    }
                } else if (line.find("Input pointers are") == 0) {
                    std::string pointers = line.substr(18); // Skip "Input pointers are "
                    auto ptrList = split(pointers, ',');
                    for (auto& ptr : ptrList) {
                        ptr = trim(ptr);
                        if (!ptr.empty()) {
                            machine.addInputPointer(ptr);
                        }
                    }
                } else if (line.find("Output pointers are") == 0) {
                    std::string pointers = line.substr(19); // Skip "Output pointers are "
                    auto ptrList = split(pointers, ',');
                    for (auto& ptr : ptrList) {
                        ptr = trim(ptr);
                        if (!ptr.empty()) {
                            machine.addOutputPointer(ptr);
                        }
                    }
                }
                break;
            }
                
            case ParseState::VARIABLES:
            {
                // Parse variable definition
                // Format: {C type} {name} = {starting value}
                std::regex varRegex("([^\\s]+)\\s+([^\\s]+)\\s*=\\s*(.+)");
                std::smatch varMatch;
                if (std::regex_search(line, varMatch, varRegex)) {
                    std::string varType = varMatch[1];
                    std::string varName = varMatch[2];
                    std::string varValue = varMatch[3];
                    
                    // Add the variable
                    machine.addVariable(varType, varName, varValue);
                }
                break;
            }
                
            case ParseState::STATES:
            {
                // Split at the colon
                size_t colonPos = line.find(':');
                if (colonPos == std::string::npos) {
                    continue; // Invalid format, skip
                }
                
                std::string stateName = trim(line.substr(0, colonPos));
                std::string outputSection = trim(line.substr(colonPos + 1));
                
                // Create the state
                State state(stateName, stateName);
                
                // Set the first state as initial
                if (firstStateId.empty()) {
                    firstStateId = stateName;
                    state.setIsInitial(true);
                }
                
                // Parse outputs if present
                if (!outputSection.empty() && outputSection[0] == '[' && 
                    outputSection[outputSection.length() - 1] == ']') {
                    
                    // Extract content between brackets
                    std::string outputsContent = outputSection.substr(1, outputSection.length() - 2);
                    auto outputStatements = split(outputsContent, ';');
                    
                    for (auto& statement : outputStatements) {
                        statement = trim(statement);
                        
                        // Check for variable assignments (contains '=')
                        if (statement.find('=') != std::string::npos) {
                            // Just store the whole expression as is
                            OutputCondition outputCondition(statement, "expression");
                            state.addOutput(outputCondition);
                        }
                        // Parse the output statement - handle both formats:
                        // 1. "output VALUE to TARGET"
                        // 2. "output VALUE to TARGET if INPUT is defined"
                        else {
                            std::regex outputRegex(R"(output\s+(\S+)\s+to\s+(\S+)(?:\s+if\s+(\S+)\s+is\s+defined)?)");
                            std::smatch outputMatch;
                            
                            if (std::regex_search(statement, outputMatch, outputRegex)) {
                                std::string value = outputMatch[1];
                                std::string target = outputMatch[2];
                                bool hasCondition = outputMatch[3].matched;
                                std::string inputPtr = hasCondition ? outputMatch[3].str() : "";
                                
                                OutputCondition outputCondition(value, target, inputPtr, hasCondition);
                                state.addOutput(outputCondition);
                            }
                            // todo: remove
                            else if (statement.find("output ") == 0) {
                                std::string value = trim(statement.substr(7)); // Skip "output "
                                OutputCondition outputCondition(value, "default");
                                state.addOutput(outputCondition);
                            }
                        }
                    }
                }
                
                // Add the state to the machine
                machine.addState(state);
                break;
            }
                
            case ParseState::TRANSITIONS:
            {
                // Parse transition definition
                std::regex transitionRegex("([^-]+)-\\[(.+)\\]->(.+)");
                std::smatch transitionMatch;
                if (std::regex_search(line, transitionMatch, transitionRegex)) {
                    std::string sourceName = trim(transitionMatch[1]);
                    std::string conditionsStr = trim(transitionMatch[2]);
                    std::string targetName = trim(transitionMatch[3]);
                    
                    // Get the states
                    State* sourceState = machine.getStateByName(sourceName);
                    State* targetState = machine.getStateByName(targetName);
                    
                    if (!sourceState || !targetState) {
                        continue; // Skip if states not found
                    }
                    
                    // Parse conditions
                    auto conditions = split(conditionsStr, ';');
                    std::vector<InputCondition> inputs;
                    int timeout = 0;
                    
                    for (auto& condition : conditions) {
                        condition = trim(condition);
                        
                        // Parse timeout
                        if (condition.find("timeout ") == 0) {
                            std::string timeoutStr = trim(condition.substr(8)); // Skip "timeout "
                            try {
                                timeout = std::stoi(timeoutStr);
                            } catch (...) {
                                // Invalid timeout, ignore
                            }
                        }
                        // Parse boolean expression
                        else if (condition.find("==") != std::string::npos || condition.find("!=") != std::string::npos) {
                            std::string op;
                            size_t opPos = 0;
                            
                            if (condition.find("==") != std::string::npos) {
                                op = "==";
                                opPos = condition.find("==");
                            } else {
                                op = "!=";
                                opPos = condition.find("!=");
                            }
                            
                            std::string leftOp = trim(condition.substr(0, opPos));
                            std::string rightOp = trim(condition.substr(opPos + op.length()));
                            
                            // Add boolean expression condition
                            InputCondition boolCond(leftOp, op, rightOp);
                            boolCond.isBooleanExpr = true;
                            inputs.push_back(boolCond);
                        }
                        // Parse input condition - handle both formats:
                        // 1. "got VALUE from SOURCE"
                        // 2. "got VALUE" (legacy)
                        else if (condition.find("got ") == 0) {
                            std::string inputPart = trim(condition.substr(4)); // Skip "got "
                            
                            // Check for the "from" keyword
                            size_t fromPos = inputPart.find(" from ");
                            if (fromPos != std::string::npos) {
                                std::string value = trim(inputPart.substr(0, fromPos));
                                std::string source = trim(inputPart.substr(fromPos + 6)); // Skip " from "
                                inputs.emplace_back(value, source);
                            } else {
                                // Legacy format
                                inputs.emplace_back(inputPart, "default");
                            }
                        }
                    }
                    
                    // Create the transition
                    if (!inputs.empty() || timeout > 0) {
                        // Generate UID
                        std::string transitionId = sourceState->getId() + "->" + targetState->getId();
                        Transition transition(transitionId, sourceState->getId(), 
                                            targetState->getId(), inputs, timeout);
                        machine.addTransition(transition);
                    }
                }
                break;
            }
                
            case ParseState::END:
            {
                // Should not reach here
                break;
            }
        }
    }
    
    // Set the first state as the initial state
    if (!firstStateId.empty()) {
        State* firstState = machine.getStateByName(firstStateId);
        if (firstState) {
            machine.setInitialState(firstState->getId());
        }
    }

    // Double-check that we have an initial state
    bool hasInitialState = false; // todo: compact
    for (State* state : machine.getAllStates()) {
        if (state->getIsInitial()) {
            hasInitialState = true;
            break;
        }
    }

    // Set the first state as the initial state if no state is marked as initial
    if (!hasInitialState && !firstStateId.empty()) {
        State* firstState = machine.getStateByName(firstStateId);
        if (firstState) {
            machine.setInitialState(firstState->getId());
        }
    }
    
    return machine;
}

/**
 * @brief Splits a string by a delimiter
 * 
 * Helper method used during file parsing
 * 
 * @param str String to split
 * @param delimiter Character to split by
 * @return Vector of substrings
 */
std::vector<std::string> MachineFileHandler::split(const std::string& str, char delimiter) {
    std::vector<std::string> result;
    std::stringstream ss(str);
    std::string item;
    
    while (std::getline(ss, item, delimiter)) {
        result.push_back(item);
    }
    
    return result;
}