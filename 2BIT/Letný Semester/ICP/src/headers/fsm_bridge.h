/**
 * @file fsm_bridge.h
 * @brief Declaration of the FSMBridge class connecting FSM backend with Qt GUI
 * @author Hugo Bohácsek (xbohach00)
 */

#ifndef FSM_BRIDGE_H
#define FSM_BRIDGE_H

#include <QObject>
#include <QString>
#include <QMap>
#include <QPointF>
#include "moore_machine.h"
#include "machine_simulator.h"
#include "machine_file_handler.h"
#include "code_generator.h"
#include "comm_bridge.h"
#include "machine_connector.h"
#include "includable_generator.h"

/**
 * @class FSMBridge
 * @brief Bridge class to connect the FSM backend with the Qt GUI
 * 
 * This class provides an interface between the Qt GUI and the underlying
 * Finite State Machine (FSM) implementation. It handles operations such as:
 * - Creating and managing machines
 * - Adding and removing states and transitions
 * - Simulating machine execution
 * - Converting between frontend and backend data representations
 */
class FSMBridge : public QObject {
    Q_OBJECT

public:
    /**
     * @brief Constructor
     * @param parent Parent QObject, used for memory management
     */
    explicit FSMBridge(QObject *parent = nullptr);
    
    /**
     * @brief Destructor
     * 
     * Cleans up machine and simulator objects
     */
    ~FSMBridge();

    /**
     * @brief Creates a new empty machine with the given name
     * @param name Name for the new machine
     */
    void createNewMachine(const QString &name);
    
    /**
     * @brief Loads a machine from a file
     * @param filePath Path to the file containing the machine definition
     * @return True if loading succeeded, false otherwise
     */
    bool loadMachineFromFile(const QString &filePath);

    /**
     * @brief Connects to a running machine
     * @return True if connecting succeeded, false otherwise
     */
    bool connectToRunningMachine();

    /**
     * @brief Disconnects from a running machine
     */
    void disconnectFromRunningMachine();

    /**
     * @brief Saves the current machine to a file
     * @param filePath Path to the file where the machine will be saved
     * @return True if saving succeeded, false otherwise
     */
    bool saveMachineToFile(const QString &filePath);

    /**
     * @brief Saves the generated code of the current machine to a file
     *
     * @param filePath Path to the file where the code will be saved
     * @return True if saving succeeded, false otherwise
     */
    bool saveCodeToFile(const QString &filePath);
    
    /**
     * @brief Adds a new state to the machine
     * @param name Name of the state
     * @param isInitial Whether this is the initial state
     * @param isFinal Whether this is a final state
     * @param x X-coordinate for visual representation
     * @param y Y-coordinate for visual representation
     * @return The ID of the created state, or empty string if creation failed
     */
    QString addState(const QString &name, bool isInitial, bool isFinal, int x, int y);
    
    /**
     * @brief Removes a state from the machine
     * @param stateId ID of the state to remove
     * @return True if removal succeeded, false otherwise
     */
    bool removeState(const QString &stateId);
    
    /**
     * @brief Sets a state as the initial state
     * @param stateId ID of the state to set as initial
     * @return True if setting succeeded, false otherwise
     */
    bool setInitialState(const QString &stateId);
    
    /**
     * @brief Adds a transition between states
     * @param sourceId ID of the source state
     * @param targetId ID of the target state
     * @param input Input condition for the transition
     * @return The ID of the created transition, or empty string if creation failed
     */
    QString addTransition(const QString &sourceId, const QString &targetId, const QList<QPair<QString, QString> > &inputConditions, const int timeout);

    /**
     * @brief Update the timeout of a transition
     * @param transitionId ID of the transition to modify
     * @param timeout New timeout value in milliseconds
     * @return True if successful, false otherwise
     */
    bool updateTransitionTimeout(const QString &transitionId, int timeout);

    /**
     * @brief Replace all input conditions of a transition
     * @param transitionId ID of the transition to modify
     * @param conditions List of (inputSymbol, inputPointer) pairs
     * @return True if successful, false otherwise
     */
    bool updateTransitionConditions(const QString &transitionId, const QList<QPair<QString, QString>> &conditions);
    
    /**
     * @brief Removes a transition from the machine
     * @param transitionId ID of the transition to remove
     * @return True if removal succeeded, false otherwise
     */
    bool removeTransition(const QString &transitionId);
    
    /**
     * @brief Resets the simulation to initial state
     */
    void resetSimulation();
    
    /**
     * @brief Processes an input through the machine
     * @param input Input symbol to process
     * @return The output produced by the machine
     */
    QString processInput(const QString &input);
    
    /**
     * @brief Gets the name of the current machine
     * @return The name of the machine
     */
    QString getMachineName() const;
    
    /**
     * @brief Gets a list of all state IDs in the machine
     * @return QStringList containing all state IDs
     */
    QStringList getStateIds() const;
    
    /**
     * @brief Gets a list of all state names in the machine
     * @return QStringList containing all state names
     */
    QStringList getStateNames() const;
    
    /**
     * @brief Gets the positions of all states
     * @return QMap mapping state IDs to their positions
     */
    QMap<QString, QPointF> getStatePositions() const;
    
    /**
     * @brief Gets all transitions in the machine as source/target pairs
     * @return List of source/target state ID pairs
     */
    QList<QPair<QString, QString>> getTransitions() const;
    
    /**
     * @brief Gets a list of all transition IDs in the machine
     * @return QStringList containing all transition IDs
     */
    QStringList getTransitionIds() const;

    /**
     * @brief Gets all input symbols in the machine's alphabet
     * @return QStringList containing all input symbols
     */
    QStringList getInputSymbols() const;
    
    /**
     * @brief Gets all output symbols in the machine's alphabet
     * @return QStringList containing all output symbols
     */
    QStringList getOutputSymbols() const;
    
    /**
     * @brief Checks if the machine is valid
     * @return True if the machine is valid, false otherwise
     */
    bool isValid() const;
    
    /**
     * @brief Gets validation errors if the machine is invalid
     * @return QStringList containing error messages
     */
    QStringList getValidationErrors() const;
    
    /**
     * @brief Gets the ID of the current active state during simulation
     * @return The ID of the current state
     */
    QString getCurrentStateId() const;
    
    /**
     * @brief Gets the backend machine for direct access if needed
     * @return Pointer to the MooreMachine object
     */
    MooreMachine* getMachine() { return machine; }
    
    /**
     * @brief Gets all input pointers in the machine
     * @return QStringList containing all input pointers
     */
    QStringList getInputPointers() const;

    /**
     * @brief Gets all output pointers in the machine
     * @return QStringList containing all output pointers
     */
    QStringList getOutputPointers() const;

    /**
     * @brief Processes an input through the machine on a specific input pointer
     * @param input Input symbol to process
     * @param inputPtr Input pointer to use
     * @return The output produced by the machine
     */
    QString processInputOnPointer(const QString &input, const QString &inputPtr);

    /**
     * @brief Gets the current value of a specific output pointer
     * @param outputPtr Output pointer to get the value from
     * @return The value of the specified output pointer
     */
    QString getOutputFromPointer(const QString &outputPtr);

    /**
     * @brief Adds a transition with a boolean expression condition
     * @param sourceId ID of the source state
     * @param targetId ID of the target state
     * @param leftOp Left operand (variable name)
     * @param op Operation (== or !=)
     * @param rightOp Right operand (variable name or literal)
     * @return The ID of the created transition, or empty string if creation failed
     */
    QString addBooleanTransition(const QString &sourceId, const QString &targetId, 
                                const QString &leftOp, const QString &op, const QString &rightOp);

    /**
     * @brief Processes next step based on the timeout
     */
    void stepSimulation();

    /**
     * @brief Returns the Transitions trigger condition a string
     * 
     * @param transition Trransition in backend format
     * @return String representation in Qt format
     */
    QString getInputConditionsString(const Transition* transition);

        /**
     * @brief Adds a symbol to the input alphabet
     * @param symbol Symbol to add
     * @return True if successful, false otherwise
     */
    bool addInputSymbol(const QString &symbol);

    /**
     * @brief Removes a symbol from the input alphabet
     * @param symbol Symbol to remove
     * @return True if successful, false otherwise
     */
    bool removeInputSymbol(const QString &symbol);

    /**
     * @brief Adds a symbol to the output alphabet
     * @param symbol Symbol to add
     * @return True if successful, false otherwise
     */
    bool addOutputSymbol(const QString &symbol);

    /**
     * @brief Removes a symbol from the output alphabet
     * @param symbol Symbol to remove
     * @return True if successful, false otherwise
     */
    bool removeOutputSymbol(const QString &symbol);

    /**
     * @brief Adds an input pointer
     * @param pointer Pointer to add
     * @return True if successful, false otherwise
     */
    bool addInputPointer(const QString &pointer);

    /**
     * @brief Removes an input pointer
     * @param pointer Pointer to remove
     * @return True if successful, false otherwise
     */
    bool removeInputPointer(const QString &pointer);

    /**
     * @brief Adds an output pointer
     * @param pointer Pointer to add
     * @return True if successful, false otherwise
     */
    bool addOutputPointer(const QString &pointer);

    /**
     * @brief Removes an output pointer
     * @param pointer Pointer to remove
     * @return True if successful, false otherwise
     */
    bool removeOutputPointer(const QString &pointer);

    /**
     * @brief Get machine variables names
     * @return list of names
     */
    QStringList getVariableNames();

    /**
     * @brief Add new machine variable
     * @param name variable name
     * @param type variable type
     * @param value variable value
     * @return True if successful, false otherwise
     */
    bool addVar(const QString &name, const QString &type, const QString &value);

    /**
     * @brief Remove machine variable
     * @param name variable name
     * @return True if successful, false otherwise
     */
    bool removeVar(const QString &name);

    /**
     * @brief Removes a specific input condition from a transition
     * @param transitionId ID of the transition to modify
     * @param conditionIndex Index of the condition to remove
     * @return True if successful, false otherwise
     */
    bool removeCondition(const QString &transitionId, int conditionIndex);

    /**
     * @brief Call backend function to generate includable file in given code style
     */
    void genIncludable(const QString &codeStyle);

private:
    MooreMachine* machine;       /**< The underlying Moore machine */
    MachineSimulator* simulator; /**< Simulator for the machine */
    MachineConnector* connector; /**< Connector for the machine */
    CommBridge communicationsBridge; /**< Communication bridge between GUI and running machine */
    bool machineConnected; /**< Indicates whether there is a remote machine connected */

    /**
     * @brief Converts a backend state ID to frontend representation
     * @param backendId State ID in backend format
     * @return State ID in frontend format
     */
    QString backendStateIdToFrontend(const std::string &backendId) const;
    
    /**
     * @brief Converts a frontend state ID to backend representation
     * @param frontendId State ID in frontend format
     * @return State ID in backend format
     */
    std::string frontendStateIdToBackend(const QString &frontendId) const;
    
    /**
     * @brief Converts a Qt point to backend Point representation
     * @param qpoint Point in Qt format
     * @return Point in backend format
     */
    Point qpointToPoint(const QPointF &qpoint) const;
    
    /**
     * @brief Converts a backend Point to Qt point representation
     * @param point Point in backend format
     * @return Point in Qt format
     */
    QPointF pointToQPoint(const Point &point) const;
};

#endif // FSM_BRIDGE_H
