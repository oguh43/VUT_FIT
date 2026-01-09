/**
* @author Filip Jenis (xjenisf00)
*/

#include "../headers/machine_connector.h"
#include "../headers/fsm_bridge.h"

/**
 * @brief Constructor
 * @param machine Pointer to the internal representation of a Moore Machine Object
 * @param comm Pointer to the UDP Communication bridge class
 * @param fsmBridge Pointer to the parent FSMBridge class
 */
MachineConnector::MachineConnector(MooreMachine* machine, CommBridge* comm, FSMBridge* fsmBridge) : machine(machine), comm(comm), fsmBridge(fsmBridge), currentStateId("") {}

/**
 * @brief Sets new pointer to the internal representation of a Moore Machine
 * @param machine Pointer to the internal representation of a Moore Machine Object
 */
void MachineConnector::setNewMachine(MooreMachine *machinePtr) {
    machine = machinePtr;
}

/**
 * @brief Sets the new value of an input and sends it to the generated executable automaton
 * @param inputPtr Name of the input
 * @param value New value of the input
 */
void MachineConnector::setInput(const std::string &inputPtr, const std::string &value) {
    inputValues[inputPtr] = value;
    std::string msg = {static_cast<char>(0x01)};
    msg += "\r\n";
    msg += inputPtr + "\r\n";
    msg += value + "\r\n";
    comm->send(QString::fromStdString(msg));
}

/**
 * @brief Retrieves the current value of an output
 * @param outputPtr Name of the output
 * @return Current value of the output
 */
std::string MachineConnector::getOutput(const std::string &outputPtr) const {
    auto it = outputValues.find(outputPtr);
    if (it != outputValues.end()) {
        return it->second;
    }
    return "";
}

/**
 * @brief Retrieves the object of the current state
 * @return Object of the current state
 */
State* MachineConnector::getCurrentState() const {
    if (!machine) {
        return nullptr;
    }
    return machine->getState(currentStateId);
}

/**
 * @brief Retrieves the id of the current state
 * @return ID of the current state
 */
std::string MachineConnector::getCurrentStateId() const {
    return currentStateId;
}

/**
 * @brief Handles received UDP message from generated executable automaton
 * @param msg Message from the generated executable automaton
 */
void MachineConnector::handleReceivedMessage(const QString& msg) {
    std::string msgStr = msg.toStdString();
    switch (msgStr[0]){
        // Changed state message
        case 0x03: {
            std::string newStateId = msgStr.substr(3, msgStr.find("\r\n", 2) - 3);
            currentStateId = newStateId;
            }
            break;
        // Changed variable value message
        case 0x04: {
            auto delimiter = msgStr.find("\r\n", 2);
            std::string variableName = msgStr.substr(3, delimiter - 3);
            auto valueEnd = msgStr.find("\r\n", delimiter + 2);
            std::string value = msgStr.substr(delimiter + 2, valueEnd - delimiter - 2);
            }
            break;
        // Changed output value message
        case 0x05: {
            auto delimiter = msgStr.find("\r\n", 2);
            std::string outputPtr = msgStr.substr(3, delimiter - 3);
            auto valueEnd = msgStr.find("\r\n", delimiter + 2);
            std::string value = msgStr.substr(delimiter + 2, valueEnd - delimiter - 2);
            outputValues[outputPtr] = value;
            }
            break;
        // Initial information message
        case 0x06: {
            // Retrieve path of the text representation of the generated automaton
            auto pathEnd = msgStr.find("\r\n", 2);
            std::string fsmDefinitionPath = msgStr.substr(3, pathEnd - 3);
            // Load automaton representation
            fsmBridge->loadMachineFromFile(QString::fromStdString(fsmDefinitionPath));
            // Retrieve information about the current state
            auto currentStateEnd = msgStr.find("\r\n", pathEnd + 2);
            std::string currentStateIdStr = msgStr.substr(pathEnd + 2, currentStateEnd - pathEnd - 2);
            currentStateId = currentStateIdStr;
            // Retrieve information about current input values
            auto inputsLenEnd = msgStr.find("\r\n", currentStateEnd + 2);
            int inputsLen = std::stoi(msgStr.substr(currentStateEnd + 2, inputsLenEnd - currentStateEnd - 2));
            std::string inputs = msgStr.substr(inputsLenEnd + 2, inputsLen);
            parseIncomingInputs(inputs);
            // Retrieve information about current output values
            auto outputsLenEnd = msgStr.find("\r\n", inputsLenEnd + 2 + inputsLen);
            int outputsLen = std::stoi(msgStr.substr(inputsLenEnd + 2 + inputsLen, outputsLenEnd - inputsLenEnd + inputsLen));
            std::string outputs = msgStr.substr(outputsLenEnd + 2, outputsLen);
            parseIncomingOutputs(outputs);
            // Retrieve information about current variable values
            auto variablesLenEnd = msgStr.find("\r\n", outputsLenEnd + 2 + outputsLen);
            int variablesLen = std::stoi(msgStr.substr(outputsLenEnd+ 2 + outputsLen, variablesLenEnd - outputsLenEnd + outputsLen));
            std::string variables = msgStr.substr(variablesLenEnd + 2, variablesLen);
            parseIncomingVariables(variables);
            }
            break;
    }
}

/**
 * @brief Parses values of all inputs from the initial information UDP message
 * @param inputs Inputs information section of the initial information UDP message
 */
void MachineConnector::parseIncomingInputs(std::string inputs) {
    size_t startpos = 0, endpos, valpos, valendpos;
    std::string inputPtr, value;
    while ((endpos = inputs.find("\r\n", startpos)) != std::string::npos){
        inputPtr = inputs.substr(startpos, endpos - startpos);
        valpos = endpos + 2;
        valendpos = inputs.find("\r\n", valpos);
        value = inputs.substr(valpos, valendpos - valpos);
        startpos = valendpos + 2;
        inputValues[inputPtr] = value;
    }
}

/**
 * @brief Parses values of all outputs from the initial information UDP message
 * @param outputs Outputs information section of the initial information UDP message
 */
void MachineConnector::parseIncomingOutputs(std::string outputs) {
    size_t startpos = 0, endpos, valpos, valendpos;
    std::string outputPtr, value;
    while ((endpos = outputs.find("\r\n", startpos)) != std::string::npos){
        outputPtr = outputs.substr(startpos, endpos - startpos);
        valpos = endpos + 2;
        valendpos = outputs.find("\r\n", valpos);
        value = outputs.substr(valpos, valendpos - valpos);
        startpos = valendpos + 2;
        outputValues[outputPtr] = value;
    }
}

/**
 * @brief Parses values of all variables from the initial information UDP message
 * @param outputs Variables information section of the initial information UDP message
 */
void MachineConnector::parseIncomingVariables(std::string variables) {
    size_t startpos = 0, endpos, valpos, valendpos;
    std::string variablePtr, value;
    while ((endpos = variables.find("\r\n", startpos)) != std::string::npos){
        variablePtr = variables.substr(startpos, endpos - startpos);
        valpos = endpos + 2;
        valendpos = variables.find("\r\n", valpos);
        value = variables.substr(valpos, valendpos - valpos);
        startpos = valendpos + 2;
        if (std::regex_match(value, std::regex("^[+-]?([0-9]*[.])[0-9]*$"))){
            machine->getVariable(variablePtr)->setValue(value);
        }else if (std::regex_match(value, std::regex("^[+-]?([0-9]*)$"))){
            machine->getVariable(variablePtr)->setValue(value);
        }else{
            machine->getVariable(variablePtr)->setValue(value);
        }
    }
}
