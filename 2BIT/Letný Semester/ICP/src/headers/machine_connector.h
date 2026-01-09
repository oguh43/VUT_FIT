/**
 * @author Filip Jenis (xjenisf00)
 */

#ifndef MACHINE_CONNECTOR_H
#define MACHINE_CONNECTOR_H

#include <string>
#include <unordered_map>
#include <regex>
#include <QString>
#include <QDebug>
#include "moore_machine.h"
#include "comm_bridge.h"

class FSMBridge;

/**
 * @class MachineConnector
 *
 * @brief Bridge class to connect the editor backend with the generated executable automaton
 */
class MachineConnector{
private:
    MooreMachine* machine; /**< The machine running */
    CommBridge* comm; /**< The communication bridge */
    FSMBridge* fsmBridge; /**< Pointer to the parent FSMBridge class */
    std::string currentStateId; /**< ID of the current state */
    std::unordered_map<std::string, std::string> inputValues; /**< Current input values by input pointer */
    std::unordered_map<std::string, std::string> outputValues; /**< Current output values by output pointer */

    /**
     * @brief Parses values of all inputs from the initial information UDP message
     * @param inputs Inputs information section of the initial information UDP message
     */
    void parseIncomingInputs(std::string inputs);
    /**
     * @brief Parses values of all outputs from the initial information UDP message
     * @param outputs Outputs information section of the initial information UDP message
     */
    void parseIncomingOutputs(std::string outputs);
    /**
     * @brief Parses values of all variables from the initial information UDP message
     * @param outputs Variables information section of the initial information UDP message
     */
    void parseIncomingVariables(std::string variables);

public:
    /**
     * @brief Constructor
     * @param machine Pointer to the internal representation of a Moore Machine Object
     * @param comm Pointer to the UDP Communication bridge class
     * @param fsmBridge Pointer to the parent FSMBridge class
     */
    MachineConnector(MooreMachine* machine, CommBridge* comm, FSMBridge* fsmBridge);

    /**
     * @brief Sets new pointer to the internal representation of a Moore Machine
     * @param machine Pointer to the internal representation of a Moore Machine Object
     */
    void setNewMachine(MooreMachine* machine);

    /**
     * @brief Sets the new value of an input and sends it to the generated executable automaton
     * @param inputPtr Name of the input
     * @param value New value of the input
     */
    void setInput(const std::string& inputPtr, const std::string& value);

    /**
     * @brief Retrieves the current value of an output
     * @param outputPtr Name of the output
     * @return Current value of the output
     */
    std::string getOutput(const std::string& outputPtr) const;

    /**
     * @brief Retrieves the object of the current state
     * @return Object of the current state
     */
    State* getCurrentState() const;

    /**
     * @brief Retrieves the id of the current state
     * @return ID of the current state
     */
    std::string getCurrentStateId() const;

    /**
     * @brief Handles received UDP message from generated executable automaton
     * @param msg Message from the generated executable automaton
     */
    void handleReceivedMessage(const QString& msg);
};

#endif //MACHINE_CONNECTOR_H
