/**
 * @file fsm_bridge.cpp
 * @brief Implementation of the FSMBridge class
 * @author Hugo Bohácsek (xbohach00)
 */

#include "../headers/fsm_bridge.h"

/**
 * @brief Constructor
 * 
 * Initializes the bridge with null machine and simulator pointers
 * 
 * @param parent Parent QObject, used for memory management
 */
FSMBridge::FSMBridge(QObject *parent) : QObject(parent), machine(nullptr), simulator(nullptr), connector(nullptr), communicationsBridge(new CommBridge(this)), machineConnected(false) {
}

/**
 * @brief Destructor
 * 
 * Cleans up the machine and simulator objects
 */
FSMBridge::~FSMBridge() {
    delete simulator;
    delete machine;
    delete connector;
}

/**
 * @brief Creates a new empty machine with the given name
 * 
 * Sets up a new machine with default input/output pointers and a dummy state
 * 
 * @param name Name for the new machine
 */
void FSMBridge::createNewMachine(const QString &name) {
    // Clean up existing objects
    delete simulator;
    delete machine;
    delete connector;
    
    // Create a new machine with the given name
    machine = new MooreMachine(name.toStdString());

    // Create default input and output pointers
    machine->addInputPointer("default");
    machine->addOutputPointer("default");

    // only creates an placeholder before new state is created, the graphic scene is not defined here so its not visible
    addState("__dummy", false, false, 0, 0);

    // Create a simulator for the machine
    simulator = new MachineSimulator(machine);
}

/**
 * @brief Loads a machine from a file
 * 
 * Replaces the current machine with the loaded one and ensures it has an initial state
 * 
 * @param filePath Path to the file containing the machine definition
 * @return True if loading succeeded, false otherwise
 */
bool FSMBridge::loadMachineFromFile(const QString &filePath) {
    try {
        // Load machine from file
        MooreMachine loadedMachine = MachineFileHandler::loadFromFile(filePath.toStdString());

        // Replace the current machine
        delete simulator;
        delete machine;
        
        machine = new MooreMachine(loadedMachine);

        // Ensure there's an initial state
        State* initialState = machine->getInitialState();
        if (!initialState) {
            // If no initial state is set, set the first state as initial
            auto allStates = machine->getAllStates();
            if (!allStates.empty()) {
                initialState = allStates[0];
                machine->setInitialState(initialState->getId());
            }
        }

        if (machineConnected){
            connector->setNewMachine(machine);
        }

        simulator = new MachineSimulator(machine);
        
        return true;
    } catch (const std::exception &e) {
        return false;
    }
}

/**
 * @brief Connects to a running machine
 *
 * @return True if connecting succeeded, false otherwise
 */
bool FSMBridge::connectToRunningMachine() {
    delete connector;
    machineConnected = true;
    connector = new MachineConnector(machine, &communicationsBridge, this);
    connect(&communicationsBridge, &CommBridge::eventReceived, [this](const QString& msg){
        connector->handleReceivedMessage(msg);
    });
    if (!communicationsBridge.establishConnection()){
        return false;
    }
    return true;
}

/**
 * @brief Disconnects from a running machine
 */
void FSMBridge::disconnectFromRunningMachine(){
    machineConnected = false;
    communicationsBridge.goodbye();
}

/**
 * @brief Saves the current machine to a file
 * 
 * @param filePath Path to the file where the machine will be saved
 * @return True if saving succeeded, false otherwise
 */
bool FSMBridge::saveMachineToFile(const QString &filePath) {
    if (!machine) {
        return false;
    }
    
    try {
        return MachineFileHandler::saveToFile(*machine, filePath.toStdString());
    } catch (const std::exception &e) {
        return false;
    }
}

/**
 * @brief Saves the generated code of the current machine to a file
 *
 * @param filePath Path to the file where the code will be saved
 * @return True if saving succeeded, false otherwise
 */
bool FSMBridge::saveCodeToFile(const QString &filePath) {
    if (!machine) {
        return false;
    }

    try {
        return CodeGenarator::generateCode(*machine, filePath.toStdString());
    } catch (const std::exception &e) {
        return false;
    }
}

/**
 * @brief Adds a new state to the machine
 * 
 * Creates a state with the given properties and adds it to the machine
 * 
 * @param name Name of the state
 * @param isInitial Whether this is the initial state
 * @param isFinal Whether this is a final state
 * @param x X-coordinate for visual representation
 * @param y Y-coordinate for visual representation
 * @return The ID of the created state, or empty string if creation failed
 */
QString FSMBridge::addState(const QString &name, bool isInitial, bool isFinal, int x, int y) {
    if (!machine) {
        return QString();
    }
    
    // Create a new state with the given name
    std::string stateId = name.toStdString();
    State state(stateId, name.toStdString());
    
    // Set position and initial status
    state.setPosition(Point(x, y));
    state.setIsInitial(isInitial);
    
    // Add output for final states
    if (isFinal) {
        OutputCondition outputCondition("final", "default");
        state.addOutput(outputCondition);
    }
    
    // Add the state to the machine
    if (machine->addState(state)) {
        // If this is the initial state, set it as such
        if (isInitial) {
            machine->setInitialState(stateId);
        }
        
        return QString::fromStdString(stateId);
    }
    
    return QString();
}

/**
 * @brief Removes a state from the machine
 * 
 * @param stateId ID of the state to remove
 * @return True if removal succeeded, false otherwise
 */
bool FSMBridge::removeState(const QString &stateId) {
    if (!machine) {
        return false;
    }
    
    return machine->removeState(stateId.toStdString());
}

/**
 * @brief Sets a state as the initial state
 * 
 * @param stateId ID of the state to set as initial
 * @return True if setting succeeded, false otherwise
 */
bool FSMBridge::setInitialState(const QString &stateId) {
    if (!machine) {
        return false;
    }
    
    return machine->setInitialState(stateId.toStdString());
}

/**
 * @brief Adds a transition between states
 * 
 * Creates a transition with the given properties and adds it to the machine
 * 
 * @param sourceId ID of the source state
 * @param targetId ID of the target state
 * @param input Input condition for the transition
 * @return The ID of the created transition, or empty string if creation failed
 */
QString FSMBridge::addTransition(const QString &sourceId, const QString &targetId, const QList<QPair<QString, QString>> &inputConditions, const int timeout) {
    if (!machine) {
        return QString();
    }
    
    // Create a unique ID for the transition
    std::string transitionId = sourceId.toStdString() + "->" + targetId.toStdString();
    
    // Create input condition
    std::vector<InputCondition> inputs;

    for (const QPair<QString, QString> &condition : inputConditions) {
        const QString &symbol = condition.first;
        const QString &pointer = condition.second;

        if (!symbol.isEmpty()) {
            machine->addInputSymbol(symbol.toStdString());  // optional: add to input alphabet
            inputs.emplace_back(symbol.toStdString(), pointer.toStdString());
        }
    }
    
    // Create the transition
    Transition transition(transitionId, sourceId.toStdString(), targetId.toStdString(), inputs);

    // Add timeout (if set)
    if (timeout != 0) {
        transition.setTimeout(timeout);
    }
    
    // Add the transition to the machine
    if (machine->addTransition(transition)) {
        return QString::fromStdString(transitionId);
    }
    
    return QString();
}

/**
 * @brief Removes a transition from the machine
 * 
 * @param transitionId ID of the transition to remove
 * @return True if removal succeeded, false otherwise
 */
bool FSMBridge::removeTransition(const QString &transitionId) {
    if (!machine) {
        return false;
    }
    
    return machine->removeTransition(transitionId.toStdString());
}

/**
 * @brief Resets the simulation to initial state
 */
void FSMBridge::resetSimulation() {
    if (simulator) {
        simulator->reset();
    }
}

/**
 * @brief Processes an input through the machine
 * 
 * @param input Input symbol to process
 * @return The output produced by the machine
 */
QString FSMBridge::processInput(const QString &input) {
    if (!simulator) {
        return QString();
    }
    
    return QString::fromStdString(simulator->processSymbol(input.toStdString()));
}

/**
 * @brief Gets the name of the current machine
 * 
 * @return The name of the machine
 */
QString FSMBridge::getMachineName() const {
    if (!machine) {
        return QString();
    }
    
    return QString::fromStdString(machine->getName());
}

/**
 * @brief Gets all input pointers in the machine
 * @return QStringList containing all input pointers
 */
QStringList FSMBridge::getInputPointers() const {
    QStringList pointers;
    
    if (!machine) {
        return pointers;
    }
    
    for (const std::string &pointer : machine->getInputPointers()) {
        pointers.append(QString::fromStdString(pointer));
    }
    
    return pointers;
}

/**
 * @brief Gets all output pointers in the machine
 * @return QStringList containing all output pointers
 */
QStringList FSMBridge::getOutputPointers() const {
    QStringList pointers;
    
    if (!machine) {
        return pointers;
    }
    
    for (const std::string &pointer : machine->getOutputPointers()) {
        // Skip the special "expression" marker used internally
        if (pointer != "expression") {
            pointers.append(QString::fromStdString(pointer));
        }
    }
    
    return pointers;
}

/**
 * @brief Processes an input through the machine on a specific input pointer
 * @param input Input symbol to process
 * @param inputPtr Input pointer to use
 * @return The output produced by the machine
 */
QString FSMBridge::processInputOnPointer(const QString &input, const QString &inputPtr) {
    if (!simulator) {
        return QString();
    }

    if (machineConnected){
        connector->setInput(inputPtr.toStdString(), input.toStdString());
        return QString::fromStdString(connector->getOutput("default"));
    }
    
    // Set the input on the specific pointer
    simulator->setInput(inputPtr.toStdString(), input.toStdString());
    
    // Process the input
    simulator->processInputs();
    
    // Get the output from the default output pointer
    return QString::fromStdString(simulator->getOutput("default"));
}

/**
 * @brief Gets the current value of a specific output pointer
 * @param outputPtr Output pointer to get the value from
 * @return The value of the specified output pointer
 */
QString FSMBridge::getOutputFromPointer(const QString &outputPtr) {
    if (!simulator) {
        return QString();
    }

    if (machineConnected){
        return QString::fromStdString(connector->getOutput(outputPtr.toStdString()));
    }

    return QString::fromStdString(simulator->getOutput(outputPtr.toStdString()));
}

/**
 * @brief Adds a transition with a boolean expression condition
 * @param sourceId ID of the source state
 * @param targetId ID of the target state
 * @param leftOp Left operand (variable name)
 * @param op Operation (== or !=)
 * @param rightOp Right operand (variable name or literal)
 * @return The ID of the created transition, or empty string if creation failed
 */
QString FSMBridge::addBooleanTransition(const QString &sourceId, const QString &targetId, 
    const QString &leftOp, const QString &op, const QString &rightOp) {
    if (!machine) {
        return QString();
    }

    QString opSymbol;
    if (op == "eq") {
        opSymbol = "==";
    } else {
        opSymbol = "!=";
    }
    
    // Create a unique ID for the transition
    std::string transitionId = sourceId.toStdString() + "->" + targetId.toStdString();

    // Create the transition with empty inputs
    std::vector<InputCondition> inputs;
    Transition transition(transitionId, sourceId.toStdString(), targetId.toStdString(), inputs);

    // Add the boolean condition
    transition.addBooleanCondition(leftOp.toStdString(), opSymbol.toStdString(), rightOp.toStdString());

    // Add the transition to the machine
    if (machine->addTransition(transition)) {
    return QString::fromStdString(transitionId);
    }

    return QString();
}

/**
 * @brief Gets a list of all state IDs in the machine
 * 
 * @return QStringList containing all state IDs
 */
QStringList FSMBridge::getStateIds() const {
    QStringList ids;
    
    if (!machine) {
        return ids;
    }
    
    for (State* state : machine->getAllStates()) {
        ids.append(QString::fromStdString(state->getId()));
    }
    
    return ids;
}

/**
 * @brief Gets a list of all state names in the machine
 * 
 * @return QStringList containing all state names
 */
QStringList FSMBridge::getStateNames() const {
    QStringList names;
    
    if (!machine) {
        return names;
    }
    
    for (State* state : machine->getAllStates()) {
        names.append(QString::fromStdString(state->getName()));
    }
    
    return names;
}

/**
 * @brief Gets the positions of all states
 * 
 * @return QMap mapping state IDs to their positions
 */
QMap<QString, QPointF> FSMBridge::getStatePositions() const {
    QMap<QString, QPointF> positions;
    
    if (!machine) {
        return positions;
    }
    
    for (State* state : machine->getAllStates()) {
        QString id = QString::fromStdString(state->getId());
        Point point = state->getPosition();
        positions[id] = QPointF(point.x, point.y);
    }
    
    return positions;
}

/**
 * @brief Gets all transitions in the machine as source/target pairs
 * 
 * @return List of source/target state ID pairs
 */
QList<QPair<QString, QString>> FSMBridge::getTransitions() const {
    QList<QPair<QString, QString>> transitionPairs;
    
    if (!machine) {
        return transitionPairs;
    }
    
    for (State* state : machine->getAllStates()) {
        std::string sourceId = state->getId();
        
        for (Transition* transition : machine->getTransitionsFromState(sourceId)) {
            QString source = QString::fromStdString(sourceId);
            QString target = QString::fromStdString(transition->getTargetId());
            transitionPairs.append(qMakePair(source, target));
        }
    }
    
    return transitionPairs;
}

/**
 * @brief Gets a list of all transition IDs in the machine
 * @return QStringList containing all transition IDs
 */
QStringList FSMBridge::getTransitionIds() const {
    QStringList ids;

    if (!machine) {
        return ids;
    }

    QList<QPair<QString, QString>> transitionPairs = FSMBridge::getTransitions();

    for (const auto& pair : transitionPairs) {
        ids << QString("%1->%2").arg(pair.first, pair.second);
    }

    return ids;
}

/**
 * @brief Gets all input symbols in the machine's alphabet
 * 
 * @return QStringList containing all input symbols
 */
QStringList FSMBridge::getInputSymbols() const {
    QStringList symbols;
    
    if (!machine) {
        return symbols;
    }
    
    for (const std::string &symbol : machine->getInputAlphabet()) {
        symbols.append(QString::fromStdString(symbol));
    }
    
    return symbols;
}

/**
 * @brief Gets all output symbols in the machine's alphabet
 * 
 * @return QStringList containing all output symbols
 */
QStringList FSMBridge::getOutputSymbols() const {
    QStringList symbols;
    
    if (!machine) {
        return symbols;
    }
    
    for (const std::string &symbol : machine->getOutputAlphabet()) {
        // Filter out any expression-like symbols 
        if (symbol.find("=") == std::string::npos && 
            symbol.find("+") == std::string::npos &&
            symbol.find("-") == std::string::npos &&
            symbol.find("*") == std::string::npos &&
            symbol.find("/") == std::string::npos) {
            symbols.append(QString::fromStdString(symbol));
        }
    }
    
    return symbols;
}

/**
 * @brief Checks if the machine is valid
 * 
 * @return True if the machine is valid, false otherwise
 */
bool FSMBridge::isValid() const {
    if (!machine) {
        return false;
    }
    
    return machine->isValid();
}

/**
 * @brief Gets validation errors if the machine is invalid
 * 
 * @return QStringList containing error messages
 */
QStringList FSMBridge::getValidationErrors() const {
    QStringList errors;
    
    if (!machine) {
        errors.append("No machine exists");
        return errors;
    }
    
    for (const std::string &error : machine->getValidationErrors()) {
        errors.append(QString::fromStdString(error));
    }
    
    return errors;
}

/**
 * @brief Gets the ID of the current active state during simulation
 * 
 * @return The ID of the current state
 */
QString FSMBridge::getCurrentStateId() const {
    if (!simulator) {
        return QString();
    }
    
    State* currentState = simulator->getCurrentState();
    if (!currentState) {
        return QString();
    }

    if (machineConnected){
        return QString::fromStdString(connector->getCurrentStateId());
    }

    return QString::fromStdString(currentState->getId());
}

/**
 * @brief Processes next step based on the timeout
 */
void FSMBridge::stepSimulation() {
    if (simulator) {
        simulator->processInputs();  // This handles both timeouts and input-triggered transitions
    }
}

/**
 * @brief Converts a backend state ID to frontend representation
 * 
 * @param backendId State ID in backend format
 * @return State ID in frontend format
 */
QString FSMBridge::backendStateIdToFrontend(const std::string &backendId) const {
    return QString::fromStdString(backendId);
}

/**
 * @brief Converts a frontend state ID to backend representation
 * 
 * @param frontendId State ID in frontend format
 * @return State ID in backend format
 */
std::string FSMBridge::frontendStateIdToBackend(const QString &frontendId) const {
    return frontendId.toStdString();
}

/**
 * @brief Converts a Qt point to backend Point representation
 * 
 * @param qpoint Point in Qt format
 * @return Point in backend format
 */
Point FSMBridge::qpointToPoint(const QPointF &qpoint) const {
    return Point(qpoint.x(), qpoint.y());
}

/**
 * @brief Converts a backend Point to Qt point representation
 * 
 * @param point Point in backend format
 * @return Point in Qt format
 */
QPointF FSMBridge::pointToQPoint(const Point &point) const {
    return QPointF(point.x, point.y);
}

/**
 * @brief Returns the Transitions trigger condition a string
 * 
 * @param transition Trransition in backend format
 * @return String representation in Qt format
 */
QString FSMBridge::getInputConditionsString(const Transition *transition){

    if (!transition) return QString();

    QStringList condValues;
    const std::vector<InputCondition>& inputConditions = transition->getInputConditions();
    for (const InputCondition& cond : inputConditions) {
        if (!cond.isBooleanExpr && !cond.value.empty()) {
            condValues << QString::fromStdString(cond.value);
        }
    }

    // Optional: show timeout if it exists
    if (transition->getTimeout() > 0) {
        QString symbol = QChar(0x29D7);
        condValues << QString("%1 %2 s").arg(symbol, QString::number(transition->getTimeout() / 1000));
    }

    return condValues.join(", ");
}

/**
 * @brief Adds a symbol to the input alphabet
 * @param symbol Symbol to add
 * @return True if successful, false otherwise
 */
bool FSMBridge::addInputSymbol(const QString &symbol) {
    if (!machine) {
        return false;
    }
    
    return machine->addInputSymbol(symbol.toStdString());
}

/**
 * @brief Removes a symbol from the input alphabet
 * @param symbol Symbol to remove
 * @return True if successful, false otherwise
 */
bool FSMBridge::removeInputSymbol(const QString &symbol) {
    if (!machine) {
        return false;
    }
    
    return machine->removeInputSymbol(symbol.toStdString());
}

/**
 * @brief Adds a symbol to the output alphabet
 * @param symbol Symbol to add
 * @return True if successful, false otherwise
 */
bool FSMBridge::addOutputSymbol(const QString &symbol) {
    if (!machine) {
        return false;
    }
    
    return machine->addOutputSymbol(symbol.toStdString());
}

/**
 * @brief Removes a symbol from the output alphabet
 * @param symbol Symbol to remove
 * @return True if successful, false otherwise
 */
bool FSMBridge::removeOutputSymbol(const QString &symbol) {
    if (!machine) {
        return false;
    }
    
    return machine->removeOutputSymbol(symbol.toStdString());
}

/**
 * @brief Adds an input pointer
 * @param pointer Pointer to add
 * @return True if successful, false otherwise
 */
bool FSMBridge::addInputPointer(const QString &pointer) {
    if (!machine) {
        return false;
    }
    
    return machine->addInputPointer(pointer.toStdString());
}

/**
 * @brief Removes an input pointer
 * @param pointer Pointer to remove
 * @return True if successful, false otherwise
 */
bool FSMBridge::removeInputPointer(const QString &pointer) {
    if (!machine) {
        return false;
    }
    
    return machine->removeInputPointer(pointer.toStdString());
}

/**
 * @brief Adds an output pointer
 * @param pointer Pointer to add
 * @return True if successful, false otherwise
 */
bool FSMBridge::addOutputPointer(const QString &pointer) {
    if (!machine) {
        return false;
    }
    
    return machine->addOutputPointer(pointer.toStdString());
}

/**
 * @brief Removes an output pointer
 * @param pointer Pointer to remove
 * @return True if successful, false otherwise
 */
bool FSMBridge::removeOutputPointer(const QString &pointer) {
    if (!machine) {
        return false;
    }
    
    return machine->removeOutputPointer(pointer.toStdString());
}

/**
 * @brief Get machine variables names
 * @return list of names
 */
QStringList FSMBridge::getVariableNames() {
    QStringList names;

    if (!machine) {
        return names;
    }

    std::unordered_map<std::string, MachineVariable> vars = machine->getVariables();
    for (const auto& pair : vars) {
        names << QString::fromStdString(pair.first);
    }

    return names;
}

/**
 * @brief Create new machine variable
 * @param name variable name
 * @param type variable type
 * @param value variable value
 * @return True if successful, false otherwise
 */
bool FSMBridge::addVar(const QString &name, const QString &type, const QString &value) {
    if (!machine) {
        return false;
    }

    return machine->addVariable(type.toStdString(), name.toStdString(), value.toStdString());
}

/**
 * @brief Remove machine variable
 * @param name variable name
 * @return True if successful, false otherwise
 */
bool FSMBridge::removeVar(const QString &name) {
    if (!machine) {
        return false;
    }

    return machine->removeVariable(name.toStdString());
}

/**
 * @brief Removes a specific input condition from a transition
 * @param transitionId ID of the transition to modify
 * @param conditionIndex Index of the condition to remove
 * @return True if successful, false otherwise
 */
bool FSMBridge::removeCondition(const QString &transitionId, int conditionIndex)
{
    // Check if machine exists
    if (!machine) {
        return false;
    }
    
    // Get the transition
    Transition* transition = machine->getTransition(transitionId.toStdString());
    if (!transition) {
        return false;
    }
    
    // Get current conditions
    std::vector<InputCondition> currentConditions = transition->getInputConditions();
    
    // Check if condition index is valid
    if (conditionIndex < 0 || conditionIndex >= static_cast<int>(currentConditions.size())) {
        return false;
    }
    
    // Create a new vector with all conditions except the one to remove
    std::vector<InputCondition> newConditions;
    for (int i = 0; i < static_cast<int>(currentConditions.size()); i++) {
        if (i != conditionIndex) {
            newConditions.push_back(currentConditions[i]);
        }
    }
    
    // Update the transition with the new conditions
    transition->setInputConditions(newConditions);
    
    return true;
}

/**
 * @brief Update the timeout of a transition
 * @param transitionId ID of the transition to modify
 * @param timeout New timeout value in milliseconds
 * @return True if successful, false otherwise
 */
bool FSMBridge::updateTransitionTimeout(const QString &transitionId, int timeout)
{
    if (!machine) return false;

    Transition* transition = machine->getTransition(transitionId.toStdString());
    if (!transition) return false;

    transition->setTimeout(timeout);
    return true;
}

/**
 * @brief Replace all input conditions of a transition
 * @param transitionId ID of the transition to modify
 * @param conditions List of (inputSymbol, inputPointer) pairs
 * @return True if successful, false otherwise
 */
bool FSMBridge::updateTransitionConditions(const QString &transitionId, const QList<QPair<QString, QString>> &conditions)
{
    if (!machine) return false;

    Transition* transition = machine->getTransition(transitionId.toStdString());
    if (!transition) return false;

    std::vector<InputCondition> updated;
    for (const QPair<QString, QString> &pair : conditions) {
        if (!pair.first.isEmpty()) {
            machine->addInputSymbol(pair.first.toStdString());
            updated.emplace_back(pair.first.toStdString(), pair.second.toStdString());
        }
    }

    transition->setInputConditions(updated);
    return true;
}

/**
 * @brief Call backend function to generate includable file in given code style
 */
void FSMBridge::genIncludable(const QString &codeStyle) {
    if (!machine) return;

    if (codeStyle == "callback") {
        IncludableGenerator::generateCode(*machine, machine->getName(), "FSMAutomaton", CodeStyle::CALLBACK);
    } else if (codeStyle == "goto") {
        IncludableGenerator::generateCode(*machine, machine->getName(), "FSMAutomaton", CodeStyle::COMPUTED_GOTO);
    }
}
