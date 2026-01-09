#include "../headers/automatoneditor.h"
#include "ui_automatoneditor.h"

#include <QPainterPath>
#include <QInputDialog>
#include <QVBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QGroupBox>
#include <QComboBox>
#include <QTime>
#include <stdexcept>

/**
 * @brief Construct a new Automaton Editor object
 * 
 * This is the constructor for creating new automaton from scratch
 * 
 * @param automatonName The name of the automaton to create
 * @param parent Parent widget, default is nullptr
 */
AutomatonEditor::AutomatonEditor(const QString &automatonName, QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::AutomatonEditor),
    fsmBridge(new FSMBridge(this)),
    simulationTimer(new QTimer(this)),
    connectionTimer(new QTimer(this)),
    simulationActive(false),
    connectionActive(false)
{
    ui->setupUi(this);
    setupUi();
    ui->pushButton_add_transition->setEnabled(false);
    ui->pushButton_add_bool_transition->setEnabled(false);
    ui->pushButton_edit_transition->setEnabled(false);
    ui->pushButton_remove_state->setEnabled(false);
    ui->pushButton_remove_transition->setEnabled(false);
    ui->pushButton_start->setEnabled(false);

    initializeFSM(automatonName);

    setWindowTitle("Edit " + automatonName);
}

/**
 * @brief Construct a new Automaton Editor object
 * 
 * This is the constructor for loading automaton from file and creating visual representation for it
 * 
 * @param filePath File path to load the automaton from
 * @param loadFromFile If true, loads the machine from the given file
 * @param parent Parent widget, default is nullptr
 */
AutomatonEditor::AutomatonEditor(const QString& filePath, bool loadFromFile, QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::AutomatonEditor),
    fsmBridge(new FSMBridge(this)),
    simulationTimer(new QTimer(this)),
    connectionTimer(new QTimer(this)),
    simulationActive(false),
    connectionActive(false)
{
    ui->setupUi(this);
    setupUi();
    
    if (loadFromFile) {
        this->loadFromFile(filePath);
        setWindowTitle("Edit " + fsmBridge->getMachineName());
    } else {
        initializeFSM("New Automaton");
        setWindowTitle("Edit New Automaton");
    }
}

/**
 * @brief Destroy the Automaton Editor object
 */
AutomatonEditor::~AutomatonEditor()
{
    delete ui;
    delete fsmBridge;
}

/**
 * @brief Set up the core UI components (menu, graphic scene and simulation controls) for the AutomatonEditor window
 * 
 */
void AutomatonEditor::setupUi()
{
    // Change text size
    QFont font = ui->labelSimulationPanel->font();
    font.setPointSize(13);
    font.setBold(true);
    ui->labelSimulationPanel->setFont(font);

    // Initialize scene
    scene = new QGraphicsScene(this);
    ui->graphicsView->setScene(scene);
    
    // Add menu bar
    QMenu *fileMenu = menuBar()->addMenu("&File");
    
    QAction *saveAction = new QAction("&Save", this);
    saveAction->setShortcut(QKeySequence::Save);
    connect(saveAction, &QAction::triggered, this, &AutomatonEditor::on_actionSave_triggered);
    fileMenu->addAction(saveAction);
    
    QAction *loadAction = new QAction("&Load", this);
    loadAction->setShortcut(QKeySequence::Open);
    connect(loadAction, &QAction::triggered, this, &AutomatonEditor::on_actionLoad_triggered);
    fileMenu->addAction(loadAction);
    
    fileMenu->addSeparator();
    
    QAction *codeGenAction = new QAction("&Generate Code", this);
    connect(codeGenAction, &QAction::triggered, this, &AutomatonEditor::on_actionCodeGen_triggered);
    fileMenu->addAction(codeGenAction);

    QAction *genCallbackAction = new QAction("&Generate Callbacks", this);
    connect(codeGenAction, &QAction::triggered, this, &AutomatonEditor::on_actionGenCallback_triggered);
    fileMenu->addAction(genCallbackAction);

    QAction *genGotoAction = new QAction("&Generate Computed gotos", this);
    connect(codeGenAction, &QAction::triggered, this, &AutomatonEditor::on_actionGenGoto_triggered);
    fileMenu->addAction(genGotoAction);

    QAction *connectAction = new QAction("&Connect to a running automaton", this);
    connect(connectAction, &QAction::triggered, this, &AutomatonEditor::on_actionConnect_triggered);
    fileMenu->addAction(connectAction);

    fileMenu->addSeparator();

    QAction *exitAction = new QAction("E&xit", this);
    exitAction->setShortcut(QKeySequence::Quit);
    connect(exitAction, &QAction::triggered, this, &AutomatonEditor::on_actionExit_triggered);
    fileMenu->addAction(exitAction);
    
    // Set up simulation controls
    setupSimulationControls();
    
    // Connect the simulation timer
    connect(simulationTimer, &QTimer::timeout, this, &AutomatonEditor::simulationStep);

    // Connect the connection timer
    connect(connectionTimer, &QTimer::timeout, this, &AutomatonEditor::connectionStep);
}

/**
 * @brief Set up the simulation control panel UI
 */
void AutomatonEditor::setupSimulationControls()
{
    // Create a group box for simulation controls
    QGroupBox *simulationBox = new QGroupBox("Simulation Controls", this);
    simulationBox->setObjectName("simulationBox");
    QVBoxLayout *simulationLayout = new QVBoxLayout(simulationBox);
    
    // Add input pointer selection
    QLabel *pointerLabel = new QLabel("Input Pointer:", simulationBox);
    QComboBox *inputPointerCombo = new QComboBox(simulationBox);
    inputPointerCombo->setObjectName("inputPointerCombo");

    // Add input symbol field
    QLabel *inputLabel = new QLabel("Input Symbol:", simulationBox);
    QLineEdit *inputField = new QLineEdit(simulationBox);
    inputField->setObjectName("inputField");
    
    // Add send button
    QPushButton *sendButton = new QPushButton("Send Input", simulationBox);
    connect(sendButton, &QPushButton::clicked, this, &AutomatonEditor::onInputEntered);
    
    // Add status label
    QLabel *statusLabel = new QLabel("Status: Not Running", simulationBox);
    statusLabel->setObjectName("statusLabel");
    
    // Add to layout
    simulationLayout->addWidget(pointerLabel);
    simulationLayout->addWidget(inputPointerCombo);
    simulationLayout->addWidget(inputLabel);
    simulationLayout->addWidget(inputField);
    simulationLayout->addWidget(sendButton);
    simulationLayout->addWidget(statusLabel);
    
    // Add to main UI
    QVBoxLayout *controlLayout = new QVBoxLayout();
    controlLayout->addWidget(simulationBox);
    
    // Find a suitable place in the UI to add these controls
    QWidget *controlWidget = new QWidget(this);
    controlWidget->setLayout(controlLayout);
    controlWidget->setFixedWidth(200);
    controlWidget->setGeometry(5, 540, 120, 230);
    
    // Hide initially - will show when simulation starts
    simulationBox->hide();
}

/**
 * @brief Initialize a new finite state machine (FSM) with a given name
 * @param name Name for the new machine
 */
void AutomatonEditor::initializeFSM(const QString& name)
{
    fsmBridge->createNewMachine(name);
}

/**
 * @brief Create a new state and add it to the scene and machine
 * 
 * @param name State name
 * @param x X position
 * @param y Y position
 * @param isStart If the state is initiol
 * @param isFinal If the state is final
 */
void AutomatonEditor::createNewState(const QString& name, int x, int y, bool isStart, bool isFinal){
    // Add to FSM model first
    QString stateId = fsmBridge->addState(name, isStart, isFinal, x, y);

    if (stateId.isEmpty()) {
        QMessageBox::warning(this, "Error", "Failed to add state to the model");
        return;
    }

    // Add to visual representation
    state_ellipse_item* stateEllipse = new state_ellipse_item(QRectF(x, y, 60, 60));
    scene->addItem(stateEllipse);

    connect(stateEllipse, &state_ellipse_item::stateMoved, this, [=]() {
        updateTransitionsForState(stateId);
    });

    stateEllipse->setBrush(Qt::white);  // Fill the inside with white
    stateEllipse->setZValue(1);
    QGraphicsTextItem* label = scene->addText(name);
    label->setZValue(3);
    // Center label
    QRectF ellipseRect = stateEllipse->rect();
    QRectF textRect = label->boundingRect();

    qreal labelX = x + ellipseRect.width() / 2 - textRect.width() / 2;
    qreal labelY = y + ellipseRect.height() / 2 - textRect.height() / 2;
    label->setPos(labelX, labelY);

    if (isFinal) {
        QGraphicsEllipseItem* innerEllipse = new QGraphicsEllipseItem(x + 5, y + 5, 50, 50, stateEllipse); // Bind it to the stateEllipse
        scene->addItem(innerEllipse);
        innerEllipse->setZValue(2);
    }

    if (isStart) {
        QGraphicsLineItem* lineStart = new QGraphicsLineItem(x - 30, y + 30, x, y + 30, stateEllipse); // Line to the state
        scene->addItem(lineStart);
    }


    // Store data in QMap
    StateItem state;
    state.id = stateId;
    state.name = name;
    state.isStart = isStart;
    state.isFinal = isFinal;
    state.position = QPoint(x, y);
    state.ellipse = stateEllipse;
    state.label = label;

    states[stateId] = state;

    addLog(QString("State %1 added successfully").arg(name));

    // Enable another buttons
    ui->pushButton_add_transition->setEnabled(true);
    ui->pushButton_add_bool_transition->setEnabled(true);
    ui->pushButton_edit_transition->setEnabled(true);
    ui->pushButton_remove_state->setEnabled(true);
    ui->pushButton_remove_transition->setEnabled(true);

    // Add it to the list widget
    ui->listWidget_states->addItem(name);
}

/**
 * @brief Call the DialogAddState dialog and add new state
 */
void AutomatonEditor::on_pushButton_add_state_clicked()
{
    //if this is the first created state, remove the placeholder
    if (states.contains("__dummy")) {
        states.remove("__dummy");
    }

    bool isFirst = states.empty();

    while (true) {
        DialogAddState dialog(this);
        if (dialog.exec() != QDialog::Accepted)
            return;

        QString name = dialog.getStateName();
        int x = dialog.getPosX();
        int y = dialog.getPosY();
        bool requestedStart = dialog.isStartState();
        bool requestedFinal = dialog.isFinalState();

        if (name.isEmpty()) {
            QMessageBox::warning(this, "Empty Name", "Please enter a State Name");
            continue; // re-show dialog
        }
        bool nameExists = false;
        for (const StateItem& state : states) {
            if (state.name == name) {
                nameExists = true;
                break;
            }
        }

        if (nameExists) {
            QMessageBox::warning(this, "Duplicate Name", "A state with this name already exists. Please choose a different one.");
            continue; // re-show dialog
        }

        // Enforce only one initial state
        if (!isFirst && requestedStart && hasInitialState()) {
            QMessageBox::warning(this, "Initial State Exists", "Only one initial state is allowed.");
            continue;
        }

        // Enforce only one final state
        if (requestedFinal && hasFinalState()) {
            QMessageBox::warning(this, "Final State Exists", "Only one final state is allowed.");
            continue;
        }

        // If it's the first state, make it initial regardless
        createNewState(name, x, y, isFirst || requestedStart, requestedFinal);

        break; // valid, exit loop
    }
}

/**
 * @brief Call the DialogAddTransition dialog and add new transition
 * 
 * Create new transition, add its source and target state,
 * input conditions (optional) and timeout (optional)
 */
void AutomatonEditor::on_pushButton_add_transition_clicked()
{
    if (states.isEmpty()) {
        QMessageBox::information(this, "No States", "You need at least one state to create a transition.");
        return;
    }

    ui->pushButton_remove_transition->setEnabled(true);
    ui->pushButton_edit_transition->setEnabled(true);
    ui->pushButton_start->setEnabled(true);

    DialogAddTransition dialog(this);
    dialog.setAvailableStates(states.keys());
    dialog.setInputPointers(fsmBridge->getInputPointers());

    if (dialog.exec() != QDialog::Accepted)
        return;

    QString fromName = dialog.getFromState();
    QString toName = dialog.getToState();
    QList<QPair<QString, QString>> inputConditions = dialog.getInputConditions();
    int timeout = dialog.getTimeout();
    
    // Find state IDs by name
    QString fromId;
    QString toId;
    
    for (auto it = states.begin(); it != states.end(); ++it) {
        if (it.value().name == fromName) {
            fromId = it.key();
        }
        if (it.value().name == toName) {
            toId = it.key();
        }
    }
    
    if (fromId.isEmpty() || toId.isEmpty()) {
        QMessageBox::warning(this, "Error", "Could not find specified states");
        return;
    }

    // Add to FSM model first
    QString transitionId = fsmBridge->addTransition(fromId, toId, inputConditions, timeout);
    
    if (transitionId.isEmpty()) {
        QMessageBox::warning(this, "Error", "Failed to add transition to the model");
        return;
    }

    // Get states for visual representation
    StateItem* stateFrom = &states[fromId];
    StateItem* stateTo = &states[toId];

    // Get the transition from backend
    Transition* backendTransition = fsmBridge->getMachine()->getTransition(transitionId.toStdString());
    QString inputLabel = backendTransition ? fsmBridge->getInputConditionsString(backendTransition) : "(?)";

    // Create a visual representation of the transition
    TransitionItem transition;
    transition.id = transitionId;
    transition.stateFrom = stateFrom;
    transition.stateTo = stateTo;
    transition.inputValue = inputLabel;
    transition.isBoolean = false;

    // Add input symbols to the alphabet and update the tab widget list
    for (const QPair<QString, QString> &condition : inputConditions) {
        const QString &symbol = condition.first;

        if (!symbol.isEmpty()) {
            fsmBridge->addInputSymbol(symbol); // Optional: may already happen internally

            // Avoid duplicates in the list widget
            bool alreadyListed = false;
            for (int i = 0; i < ui->listWidget_input_alphabet->count(); ++i) {
                if (ui->listWidget_input_alphabet->item(i)->text() == symbol) {
                    alreadyListed = true;
                    break;
                }
            }
            if (!alreadyListed) {
                ui->listWidget_input_alphabet->addItem(symbol);
            }
        }
    }

    // Handle self-transition
    if (fromId == toId) {
        QPainterPath path;

        qreal radius = 20;
        qreal loopRadius = 60;
        qreal x = stateFrom->position.x() + loopRadius/2;
        qreal y = stateFrom->position.y() + radius/2;

        path.moveTo(x + radius, y); // Move
        path.cubicTo(x + loopRadius, y - loopRadius,
                     x - loopRadius, y - loopRadius,
                     x - radius, y); // Create a loop

        // Add to scene
        transition.pathItem = scene->addPath(path, QPen(Qt::black));
        transition.pathItem->setZValue(0); // make the line appear nehind the states
        transition.labelItem = scene->addText(inputLabel);
        transition.labelItem->setPos(x - 10, y - loopRadius - 10);
    }
    else { // Between two states
        QPointF p1 = stateFrom->ellipse->sceneBoundingRect().center();
        QPointF p2 = stateTo->ellipse->sceneBoundingRect().center();

        QPainterPath path;
        path.moveTo(p1);
        path.lineTo(p2);

        transition.pathItem = scene->addPath(path, QPen(Qt::black));
        transition.pathItem->setZValue(0);

        qreal midX = (p1.x() + p2.x()) / 2;
        qreal midY = (p1.y() + p2.y()) / 2;

        transition.labelItem = scene->addText(inputLabel);
        transition.labelItem->setPos(midX, midY);
    }

    transitions[transitionId] = transition;

    // Add it to the list widget
    ui->listWidget_transitions->addItem(transitionId);

    addLog(QString("Transition from %1 to %2 added successfully").arg(fromName, toName));
}

/**
 * @brief Call the DialogAddBooleanTransition dialog and add new boolean transition
 * 
 * Create new boolean transition, add its source and target state and input condition (optional)
 */
void AutomatonEditor::on_pushButton_add_bool_transition_clicked()
{
    if (states.isEmpty()) {
        QMessageBox::information(this, "No States", "You need at least one state to create a transition.");
        return;
    }

    ui->pushButton_remove_transition->setEnabled(true);
    ui->pushButton_edit_transition->setEnabled(true);
    ui->pushButton_start->setEnabled(true);

    DialogAddBooleanTransition dialog(this);
    dialog.setAvailableStates(states.keys());
    dialog.setAvailableVars(fsmBridge->getVariableNames());

    if (dialog.exec() != QDialog::Accepted)
        return;

    QString fromName = dialog.getFromState();
    QString toName = dialog.getToState();
    QString leftOp = dialog.getLeftOp();
    QString op = dialog.getOp();
    QString rightOp = dialog.getRightOp();

    // Find state IDs by name
    QString fromId;
    QString toId;

    for (auto it = states.begin(); it != states.end(); ++it) {
        if (it.value().name == fromName) {
            fromId = it.key();
        }
        if (it.value().name == toName) {
            toId = it.key();
        }
    }

    if (fromId.isEmpty() || toId.isEmpty()) {
        QMessageBox::warning(this, "Error", "Could not find specified states");
        return;
    }

    // Add to FSM model first
    QString transitionId = fsmBridge->addBooleanTransition(fromId, toId, leftOp, op, rightOp);

    if (transitionId.isEmpty()) {
        QMessageBox::warning(this, "Error", "Failed to add transition to the model");
        return;
    }

    // Get states for visual representation
    StateItem* stateFrom = &states[fromId];
    StateItem* stateTo = &states[toId];

    QString inputLabel = QString("%1%2%3").arg(leftOp, op, rightOp);

    // Create a visual representation of the transition
    TransitionItem transition;
    transition.id = transitionId;
    transition.stateFrom = stateFrom;
    transition.stateTo = stateTo;
    transition.inputValue = inputLabel;
    transition.isBoolean = true;

    // Handle self-transition
    if (fromId == toId) {
        QPainterPath path;

        qreal radius = 20;
        qreal loopRadius = 60;
        qreal x = stateFrom->position.x() + loopRadius/2;
        qreal y = stateFrom->position.y() + radius/2;

        path.moveTo(x + radius, y); // Move
        path.cubicTo(x + loopRadius, y - loopRadius,
                     x - loopRadius, y - loopRadius,
                     x - radius, y); // Create a loop

        // Add to scene
        transition.pathItem = scene->addPath(path, QPen(Qt::black));
        transition.pathItem->setZValue(0); // make the line appear nehind the states
        transition.labelItem = scene->addText(inputLabel);
        transition.labelItem->setPos(x - 10, y - loopRadius - 10);
    }
    else { // Between two states
        QPointF p1 = stateFrom->ellipse->sceneBoundingRect().center();
        QPointF p2 = stateTo->ellipse->sceneBoundingRect().center();

        QPainterPath path;
        path.moveTo(p1);
        path.lineTo(p2);

        transition.pathItem = scene->addPath(path, QPen(Qt::black));
        transition.pathItem->setZValue(0);

        qreal midX = (p1.x() + p2.x()) / 2;
        qreal midY = (p1.y() + p2.y()) / 2;

        transition.labelItem = scene->addText(inputLabel);
        transition.labelItem->setPos(midX, midY);
    }

    transitions[transitionId] = transition;

    // Add it to the list widget
    ui->listWidget_transitions->addItem(transitionId);

    addLog(QString("Boolean transition from %1 to %2 added successfully").arg(fromName, toName));

}

/**
 * @brief Edit selected transition from the list widget and edit its input conditions or timeout
 */
void AutomatonEditor::on_pushButton_edit_transition_clicked()
{
    QListWidgetItem* selectedTransition = ui->listWidget_transitions->currentItem();
    if (!selectedTransition) {
        QMessageBox::information(this, "No Selection", "Please select a transition to edit.");
        return;
    }

    // Get state names
    QString fromName;
    QString toName;
    QStringList parts = selectedTransition->text().split("->");
    if (parts.size() == 2) {
        fromName = parts[0].trimmed();
        toName = parts[1].trimmed();
    } else {
        QMessageBox::warning(this, "Error", "Invalid transition label format.");
        return;
    }

    // Find transition ID
    QString transitionId;
    for (auto it = transitions.constBegin(); it != transitions.constEnd(); ++it) {
        const TransitionItem& t = it.value();
        if (t.stateFrom->name == fromName && t.stateTo->name == toName) {
            transitionId = it.key();
            break;
        }
    }

    if (transitionId.isEmpty()) {
        QMessageBox::warning(this, "Error", "Transition not found.");
        return;
    }

    // Show edit dialog
    DialogEditTransition dialog(this);
    dialog.setInputPointers(fsmBridge->getInputPointers());

    // Set existing input conditions and timeout
    const Transition* transition = fsmBridge->getMachine()->getTransition(transitionId.toStdString());
    QList<QPair<QString, QString>> existingConditions;

    if (transition) {
        for (const InputCondition& cond : transition->getInputConditions()) {
            if (!cond.isBooleanExpr) {
                existingConditions.append(qMakePair(QString::fromStdString(cond.value),
                                                    QString::fromStdString(cond.source)));
            }
        }
        dialog.setInputConditions(existingConditions);
        dialog.setTimeout(transition->getTimeout());
    }

    if (dialog.exec() != QDialog::Accepted)
        return;

    // Update timeout
    if (!fsmBridge->updateTransitionTimeout(transitionId, dialog.getTimeout())) {
        QMessageBox::warning(this, "Error", "Failed to update transition timeout.");
        return;
    }

    // Replace all input conditions
    if (!fsmBridge->updateTransitionConditions(transitionId, dialog.getInputConditions())) {
        QMessageBox::warning(this, "Error", "Failed to update input conditions.");
        return;
    }

    // Update label in scene
    TransitionItem& t = transitions[transitionId];
    QString newLabel = fsmBridge->getInputConditionsString(
        fsmBridge->getMachine()->getTransition(transitionId.toStdString())
    );
    t.inputValue = newLabel;
    if (t.labelItem) {
        t.labelItem->setPlainText(newLabel);
    }

    addLog(QString("Transition from %1 to %2 updated").arg(fromName, toName));
}

/**
 * @brief Remove the selected state from the list widget (also remove its transitions)
 */
void AutomatonEditor::on_pushButton_remove_state_clicked()
{
    if (states.isEmpty()) {
        QMessageBox::information(this, "No States", "There are no states to remove.");
        return;
    }

    // Populate the list of state names
    QStringList stateNames;
    QMap<QString, QString> nameToId;
    for (auto it = states.constBegin(); it != states.constEnd(); ++it) {
        const QString& id = it.key();
        const StateItem& state = it.value();
        stateNames << state.name;
        nameToId[state.name] = id;
    }

    QListWidgetItem* selectedState = ui->listWidget_states->currentItem();
    if (!selectedState) {
        QMessageBox::information(this, "No Selection", "Please select a state to remove.");
        return;
    }

    QString stateId = nameToId[selectedState->text()];

    // Remove from the FSM model first
    if (!fsmBridge->removeState(stateId)) {
        QMessageBox::warning(this, "Error", "Failed to remove state from the model");
        return;
    }

    // Remove visual representation
    StateItem& state = states[stateId];
    if (state.ellipse) {
        scene->removeItem(state.ellipse);
        delete state.ellipse;
    }
    if (state.label) {
        scene->removeItem(state.label);
        delete state.label;
    }

    // Remove associated transitions
    QList<QString> transitionsToRemove;
    for (auto it = transitions.begin(); it != transitions.end(); ++it) {
        if (it.value().stateFrom->id == stateId || it.value().stateTo->id == stateId) {
            if (it.value().pathItem) {
                scene->removeItem(it.value().pathItem);
                delete it.value().pathItem;
            }
            if (it.value().labelItem) {
                scene->removeItem(it.value().labelItem);
                delete it.value().labelItem;
            }
            transitionsToRemove.append(it.key());
        }
    }

    // Remove transition items
    for (const QString& transId : transitionsToRemove) {
        transitions.remove(transId);
    }

    // Remove the state item
    states.remove(stateId);

    if (states.isEmpty()) {
        //disable another buttons
        ui->pushButton_add_transition->setEnabled(false);
        ui->pushButton_add_bool_transition->setEnabled(false);
        ui->pushButton_edit_transition->setEnabled(false);
        ui->pushButton_remove_state->setEnabled(false);
        ui->pushButton_remove_transition->setEnabled(false);
        ui->pushButton_start->setEnabled(false);
    }

    addLog(QString("State %1 removed successfully").arg(selectedState->text()));
    delete selectedState;
}

/**
 * @brief Remove selected transition from the list widget
 */
void AutomatonEditor::on_pushButton_remove_transition_clicked()
{
    if (transitions.isEmpty()) {
        QMessageBox::information(this, "No Transitions", "There are no transitions to remove.");
        return;
    }

    QListWidgetItem* selectedTransition = ui->listWidget_transitions->currentItem();
    if (!selectedTransition) {
        QMessageBox::information(this, "No Selection", "Please select a transition to remove.");
        return;
    }

    // Get state names
    QString fromName;
    QString toName;
    QStringList parts = selectedTransition->text().split("->");
    if (parts.size() == 2) {
        fromName = parts[0].trimmed();
        toName = parts[1].trimmed();
    } else {
        QMessageBox::warning(this, "Error", "Invalid transition label format.");
        return;
    }

    // Find transition(s) matching fromName → toName
    QStringList toDelete;

    for (auto it = transitions.constBegin(); it != transitions.constEnd(); ++it) {
        const TransitionItem& t = it.value();
        if (t.stateFrom->name == fromName && t.stateTo->name == toName) {
            toDelete << it.key();
        }
    }

    if (toDelete.isEmpty()) {
        QMessageBox::information(this, "No Match", "No transition found between selected states.");
        return;
    }

    for (const QString& transitionId : toDelete) {
        // Remove from FSM model
        fsmBridge->removeTransition(transitionId);

        // Remove graphics
        TransitionItem& t = transitions[transitionId];
        if (t.pathItem) {
            scene->removeItem(t.pathItem);
            delete t.pathItem;
        }
        if (t.labelItem) {
            scene->removeItem(t.labelItem);
            delete t.labelItem;
        }

        transitions.remove(transitionId);
    }

    delete selectedTransition;

    if (transitions.empty()) {
        ui->pushButton_edit_transition->setEnabled(false);
        ui->pushButton_remove_transition->setEnabled(false);
    }

    addLog(QString("Transition from %1 to %2 removed successfully").arg(fromName, toName));
}

/**
 * @brief Check if the machine already has initial state
 * @return True if the initial state exists, false otherwise
 */
bool AutomatonEditor::hasInitialState() const {
    for (const StateItem& state : states) {
        if (state.isStart)
            return true;
    }
    return false;
}

/**
 * @brief Check if the machine already has final state
 * @return True if the final state exists, false otherwise
 */
bool AutomatonEditor::hasFinalState() const {
    for (const StateItem& state : states) {
        if (state.isFinal)
            return true;
    }
    return false;
}

/**
 * @brief Start/Stop and toggle the simulation
 */
void AutomatonEditor::on_pushButton_start_clicked()
{
    if (!fsmBridge->isValid()) {
        QStringList errors = fsmBridge->getValidationErrors();
        QString errorMsg = "The automaton is not valid:\n" + errors.join("\n");
        QMessageBox::warning(this, "Invalid Automaton", errorMsg);
        return;
    }

    if (connectionActive){
        toggleConnection(false);
        return;
    }

    // Toggle simulation state
    toggleSimulation(!simulationActive);
}

/**
 * @brief Start/stop the simulation
 * @param active true to start simulation, false to end it
 */
void AutomatonEditor::toggleSimulation(bool active)
{
    simulationActive = active;
    
    if (simulationActive) {
        // Start simulation
        addLog(QString("Simulation started"));
        fsmBridge->resetSimulation();
        simulationTimer->start(100); // Check every 100 ms, fix later if needed
        
        // Update UI
        ui->pushButton_start->setText("Stop");
        ui->pushButton_add_state->setEnabled(false);
        ui->pushButton_add_transition->setEnabled(false);
        ui->pushButton_add_bool_transition->setEnabled(false);
        ui->pushButton_edit_transition->setEnabled(false);
        ui->pushButton_remove_state->setEnabled(false);
        ui->pushButton_remove_transition->setEnabled(false);
        
        // Show simulation controls
        QGroupBox *simulationBox = findChild<QGroupBox*>("simulationBox");
        if (simulationBox) {
            simulationBox->show();
        }
        
        // Highlight initial state
        QString currentState = fsmBridge->getCurrentStateId();
        if (!currentState.isEmpty()) {
            highlightState(currentState, true);
        }
        
        // Update status
        QLabel *statusLabel = findChild<QLabel*>("statusLabel");
        if (statusLabel) {
            statusLabel->setText("Status: Running");
        }
    } else {
        // Stop simulation
        addLog(QString("Simulation stopped"));
        simulationTimer->stop();
        
        // Update UI
        ui->pushButton_start->setText("Start");
        ui->pushButton_add_state->setEnabled(true);
        ui->pushButton_add_transition->setEnabled(true);
        ui->pushButton_add_bool_transition->setEnabled(true);
        ui->pushButton_edit_transition->setEnabled(true);
        ui->pushButton_remove_state->setEnabled(true);
        ui->pushButton_remove_transition->setEnabled(true);
        
        // Hide simulation controls
        QGroupBox *simulationBox = findChild<QGroupBox*>("simulationBox");
        if (simulationBox) {
            simulationBox->hide();
        }
        
        // Remove highlights
        QString currentState = fsmBridge->getCurrentStateId();
        if (!currentState.isEmpty()) {
            highlightState(currentState, false);
        }
        
        // Update status
        QLabel *statusLabel = findChild<QLabel*>("statusLabel");
        if (statusLabel) {
            statusLabel->setText("Status: Stopped");
        }
    }
}

/**
 * @brief Execute one step of the simulation cycle
 * 
 * Invoked periodically by the QTimer to evaluate timeout or process condition-based transitions,
 * update the visual state, and log output changes
 */
void AutomatonEditor::simulationStep()
{
    // Step the backend simulation (time-based logic)
    fsmBridge->stepSimulation();

    // This is called by the timer to update the simulation
    QString currentState = fsmBridge->getCurrentStateId();

    //addLog(QString("Current state is %1").arg(currentState));
    
    // Update visualization
    for (auto it = states.begin(); it != states.end(); ++it) {
        bool isCurrentState = (it.key() == currentState);
        highlightState(it.key(), isCurrentState);
    }

    // Log state transition
    if (currentState != lastLoggedState && !currentState.isEmpty()) {
        addLog(QString("Entered state: %1").arg(currentState));
        lastLoggedState = currentState;
    }

    // Log outputs
    QStringList outputPointers = fsmBridge->getOutputPointers();
    QMap<QString, QString> newOutputs;

    for (const QString& ptr : outputPointers) {
        QString value = fsmBridge->getOutputFromPointer(ptr);
        newOutputs[ptr] = value;
    }

    if (newOutputs != lastOutputs) {
        QStringList outputLogs;
        for (auto it = newOutputs.begin(); it != newOutputs.end(); ++it) {
            outputLogs << QString("Output on pointer %1 is %2").arg(it.key(), it.value());
        }
        addLog(QString("Outputs: %1").arg(outputLogs.join(", ")));
        lastOutputs = newOutputs;
    }
}

/**
 * @brief Handle given input symbol on pointer during simulation
 */
void AutomatonEditor::onInputEntered()
{
    if (!simulationActive && !connectionActive) {
        return;
    }


    // Get input from field
    QLineEdit *inputField = findChild<QLineEdit*>("inputField");
    QComboBox *inputPointerCombo = findChild<QComboBox*>("inputPointerCombo");
    if (!inputField || !inputPointerCombo) {
        return;
    }

    QString input = inputField->text().trimmed();
    QString inputPtr = inputPointerCombo->currentText();
    if (input.isEmpty() || inputPtr.isEmpty()) {
        return;
    }

    addLog(QString("Input %1 on input pointer %2 added").arg(input, inputPtr));
    
    // Process the input
    QString output = fsmBridge->processInputOnPointer(input, inputPtr);

    addLog(QString("Input %1 processed, output is: %2").arg(input, output));

    // Clear the input field
    inputField->clear();
    
    // Highlight the new current state
    QString previousState = fsmBridge->getCurrentStateId();
    if (!previousState.isEmpty()) {
        highlightState(previousState, false);
    }
    
    QString currentState = fsmBridge->getCurrentStateId();
    if (!currentState.isEmpty()) {
        highlightState(currentState, true);
    }
    
    // Show the output
    if (!output.isEmpty()) {
        QMessageBox::information(this, "Output", "The automaton produced output: " + output);
        addLog(QString("The automaton produced output:  %1").arg(output));
    }
}

/**
 * @brief Highlight/unhighlight the given state in the simulation
 * @param stateId ID of the state to highlight/unhighlight
 * @param highlight true to highlight, false to unhighlight
 */
void AutomatonEditor::highlightState(const QString& stateId, bool highlight)
{
    auto it = states.find(stateId);
    if (it == states.end()) {
        return;
    }
    
    StateItem& state = it.value();
    if (state.ellipse) {
        if (highlight) {
            state.ellipse->setPen(QPen(Qt::red, 2));
            state.ellipse->setBrush(QBrush(QColor(255, 200, 200)));
        } else {
            state.ellipse->setPen(QPen(Qt::black, 1));
            state.ellipse->setBrush(QBrush(Qt::white));
        }
    }
}

/**
 * @brief Connect/disconnect to/from a running automaton
 * @param active true to start the connection, false to end it
 */
void AutomatonEditor::toggleConnection(bool active)
{
    connectionActive = active;

    if (connectionActive) {
        // Clear existing visualization
        clearVisualization();

        // Load the FSM from running automaton
        if (!fsmBridge->connectToRunningMachine()) {
            QMessageBox::warning(this, "Error", "Failed to connect to running machine");
            return;
        }

        // Stop simulation (if running)
        if (simulationActive) {
            toggleSimulation(false);
        }

        // Update the window title
        setWindowTitle("Edit " + fsmBridge->getMachineName());

        // Recreate visualization based on the loaded machine
        updateVisualization();

        // Load available input pointers
        QComboBox *inputPointerCombo = findChild<QComboBox*>("inputPointerCombo");
        if (inputPointerCombo) {
            inputPointerCombo->clear();
            QStringList inputPointers = fsmBridge->getInputPointers();
            inputPointerCombo->addItems(inputPointers);
        }

        // Load I/O pointers
        QListWidget *inputPtrList = findChild<QListWidget*>("listWidget_input_pointers");
        if (inputPtrList) {
            inputPtrList->clear();
            QStringList inputPointers = fsmBridge->getInputPointers();
            inputPtrList->addItems(inputPointers);
        }

        QListWidget *outputPtrList = findChild<QListWidget*>("listWidget_output_pointers");
        if (outputPtrList) {
            outputPtrList->clear();
            QStringList outputPointers = fsmBridge->getOutputPointers();
            outputPtrList->addItems(outputPointers);
        }

        // Load I/O alphabet
        QListWidget *inputList = findChild<QListWidget*>("listWidget_input_alphabet");
        if (inputList) {
            inputList->clear();
            QStringList inputAlphabet = fsmBridge->getInputSymbols();
            inputList->addItems(inputAlphabet);
        }

        QListWidget *outputList = findChild<QListWidget*>("listWidget_output_alphabet");
        if (outputList) {
            outputList->clear();
            QStringList outputAlphabet = fsmBridge->getOutputSymbols();
            outputList->addItems(outputAlphabet);
        }

        // Load variables
        QListWidget *loadedVars = findChild<QListWidget*>("listWidget_vars");
        if (loadedVars) {
            loadedVars->clear();
            QStringList varNames = fsmBridge->getVariableNames();
            loadedVars->addItems(varNames);
        }

        // Load states
        QListWidget *loadedStates = findChild<QListWidget*>("listWidget_states");
        if (loadedStates) {
            loadedStates->clear();
            QStringList stateNames = fsmBridge->getStateNames();
            loadedStates->addItems(stateNames);
        }

        // Load transitions
        QListWidget *loadedTransitions = findChild<QListWidget*>("listWidget_transitions");
        if (loadedTransitions) {
            loadedTransitions->clear();
            QStringList transitionsIds = fsmBridge->getTransitionIds();
            loadedTransitions->addItems(transitionsIds);
        }

        addLog(QString("Automaton %1 loaded successfully").arg(fsmBridge->getMachineName()));

        addLog(QString("Connected to running automaton"));
        connectionTimer->start(100);

        // Update UI
        ui->pushButton_start->setText("Stop");
        ui->pushButton_start->setEnabled(true);
        ui->pushButton_add_state->setEnabled(false);
        ui->pushButton_add_transition->setEnabled(false);
        ui->pushButton_add_bool_transition->setEnabled(false);
        ui->pushButton_edit_transition->setEnabled(false);
        ui->pushButton_remove_state->setEnabled(false);
        ui->pushButton_remove_transition->setEnabled(false);

        // Show simulation controls
        QGroupBox *simulationBox = findChild<QGroupBox *>("simulationBox");
        if (simulationBox) {
            simulationBox->show();
        }

        // Highlight initial state
        QString currentState = fsmBridge->getCurrentStateId();
        if (!currentState.isEmpty()) {
            highlightState(currentState, true);
        }

        // Update status
        QLabel *statusLabel = findChild<QLabel *>("statusLabel");
        if (statusLabel) {
            statusLabel->setText("Status: Running");
        }
    }else{
        addLog(QString("Disconnected from running automaton"));
        connectionTimer->stop();

        // Update UI
        ui->pushButton_start->setText("Start");
        ui->pushButton_add_state->setEnabled(true);
        ui->pushButton_add_transition->setEnabled(true);
        ui->pushButton_add_bool_transition->setEnabled(true);
        ui->pushButton_edit_transition->setEnabled(true);
        ui->pushButton_remove_state->setEnabled(true);
        ui->pushButton_remove_transition->setEnabled(true);

        // Hide simulation controls
        QGroupBox *simulationBox = findChild<QGroupBox*>("simulationBox");
        if (simulationBox) {
            simulationBox->hide();
        }

        // Remove highlights
        QString currentState = fsmBridge->getCurrentStateId();
        if (!currentState.isEmpty()) {
            highlightState(currentState, false);
        }

        // Update status
        QLabel *statusLabel = findChild<QLabel*>("statusLabel");
        if (statusLabel) {
            statusLabel->setText("Status: Stopped");
        }

        fsmBridge->disconnectFromRunningMachine();
    }
}

/**
 * @brief Refresh data from remote running automaton
 *
 * Invoked periodically by the QTimer to update the visual state, and log outputs
 */
void AutomatonEditor::connectionStep()
{
    // This is called by the timer to update the simulation
    QString currentState = fsmBridge->getCurrentStateId();

    //addLog(QString("Current state is %1").arg(currentState));

    // Update visualization
    for (auto it = states.begin(); it != states.end(); ++it) {
        bool isCurrentState = (it.key() == currentState);
        highlightState(it.key(), isCurrentState);
    }

    // Log state transition
    if (currentState != lastLoggedState && !currentState.isEmpty()) {
        addLog(QString("Entered state: %1").arg(currentState));
        lastLoggedState = currentState;
    }

    // Log outputs
    QStringList outputPointers = fsmBridge->getOutputPointers();
    QMap<QString, QString> newOutputs;

    for (const QString& ptr : outputPointers) {
        QString value = fsmBridge->getOutputFromPointer(ptr);
        newOutputs[ptr] = value;
    }

    if (newOutputs != lastOutputs) {
        QStringList outputLogs;
        for (auto it = newOutputs.begin(); it != newOutputs.end(); ++it) {
            outputLogs << QString("Output on pointer %1 is %2").arg(it.key(), it.value());
        }
        addLog(QString("Outputs: %1").arg(outputLogs.join(", ")));
        lastOutputs = newOutputs;
    }
}

/**
 * @brief Save the automaton in editor to a file
 */
void AutomatonEditor::on_actionSave_triggered()
{
    QString filePath = QFileDialog::getSaveFileName(this, "Save Automaton", QString(), "Automaton Files (*.fsm);;All Files (*)");

    if (filePath.isEmpty()) {
        return;
    }
    
    saveToFile(filePath);
}

/**
 * @brief Load automaton from a file and replace the current one
 */
void AutomatonEditor::on_actionLoad_triggered()
{
    QString filePath = QFileDialog::getOpenFileName(this, "Load Automaton", QString(), "Automaton Files (*.fsm);;All Files (*)");

    if (filePath.isEmpty()) {
        return;
    }
    
    loadFromFile(filePath);
}

/**
 * @brief Generate C++ code of the current automaton and save it to a file
 */
void AutomatonEditor::on_actionCodeGen_triggered()
{
    QString filePath = QFileDialog::getSaveFileName(this, "Save Generated Code", QString(), "C++ Code Files (*.cpp);;All Files (*)");

    if (filePath.isEmpty()) {
        return;
    }

    saveCodeToFile(filePath);
}

/**
 * @brief Generate includable file in code style Callback
 */
void AutomatonEditor::on_actionGenCallback_triggered() {
    fsmBridge->genIncludable("callback");
}

/**
 * @brief Generate includable file in code style Computed goto
 */
void AutomatonEditor::on_actionGenGoto_triggered() {
    fsmBridge->genIncludable("goto");
}

/**
 * @brief Connect to a running automaton
 */
void AutomatonEditor::on_actionConnect_triggered() {
    toggleConnection(true);
}

/**
 * @brief Exit the app
 */
void AutomatonEditor::on_actionExit_triggered()
{
    close();
}

/**
 * @brief Save the current automaton to the file
 * @param filePath Destination for the file to save
 */
void AutomatonEditor::saveToFile(const QString& filePath)
{
    if (!fsmBridge->saveMachineToFile(filePath)) {
        QMessageBox::warning(this, "Error", "Failed to save automaton to file");
        return;
    }
    
    QMessageBox::information(this, "Success", "Automaton saved successfully");
}

/**
 * @brief Save the current automaton to the file
 * @param filePath Destination for the file to save
 */
void AutomatonEditor::saveCodeToFile(const QString& filePath)
{
    if (!fsmBridge->saveCodeToFile(filePath)) {
        QMessageBox::warning(this, "Error", "Failed to save automaton code to file");
        return;
    }

    QMessageBox::information(this, "Success", "Automaton code saved successfully");
}

/**
 * @brief Load automaton from a file and create its visual representation
 * @param filePath Path to the .fsm file to load
 */
void AutomatonEditor::loadFromFile(const QString& filePath)
{
    // Clear existing visualization
    clearVisualization();

    // Load the FSM from file
    if (!fsmBridge->loadMachineFromFile(filePath)) {
        QMessageBox::warning(this, "Error", "Failed to load automaton from file");
        return;
    }

    // Stop simulation (if running)
    toggleSimulation(false);

    // Stop connection (if connected)
    toggleConnection(false);
    
    // Update the window title
    setWindowTitle("Edit " + fsmBridge->getMachineName());
    
    // Recreate visualization based on the loaded machine
    updateVisualization();

    ui->pushButton_add_transition->setEnabled(true);
    ui->pushButton_add_bool_transition->setEnabled(true);
    ui->pushButton_edit_transition->setEnabled(true);
    ui->pushButton_remove_state->setEnabled(true);
    ui->pushButton_remove_transition->setEnabled(true);
    ui->pushButton_start->setEnabled(true);

    // Load available input pointers
    QComboBox *inputPointerCombo = findChild<QComboBox*>("inputPointerCombo");
    if (inputPointerCombo) {
        inputPointerCombo->clear();
        QStringList inputPointers = fsmBridge->getInputPointers();
        inputPointerCombo->addItems(inputPointers);
    }

    // Load I/O pointers
    QListWidget *inputPtrList = findChild<QListWidget*>("listWidget_input_pointers");
    if (inputPtrList) {
        inputPtrList->clear();
        QStringList inputPointers = fsmBridge->getInputPointers();
        inputPtrList->addItems(inputPointers);
    }

    QListWidget *outputPtrList = findChild<QListWidget*>("listWidget_output_pointers");
    if (outputPtrList) {
        outputPtrList->clear();
        QStringList outputPointers = fsmBridge->getOutputPointers();
        outputPtrList->addItems(outputPointers);
    }

    // Load I/O alphabet
    QListWidget *inputList = findChild<QListWidget*>("listWidget_input_alphabet");
    if (inputList) {
        inputList->clear();
        QStringList inputAlphabet = fsmBridge->getInputSymbols();
        inputList->addItems(inputAlphabet);
    }

    QListWidget *outputList = findChild<QListWidget*>("listWidget_output_alphabet");
    if (outputList) {
        outputList->clear();
        QStringList outputAlphabet = fsmBridge->getOutputSymbols();
        outputList->addItems(outputAlphabet);
    }

    // Load variables
    QListWidget *loadedVars = findChild<QListWidget*>("listWidget_vars");
    if (loadedVars) {
        loadedVars->clear();
        QStringList varNames = fsmBridge->getVariableNames();
        loadedVars->addItems(varNames);
    }

    // Load states
    QListWidget *loadedStates = findChild<QListWidget*>("listWidget_states");
    if (loadedStates) {
        loadedStates->clear();
        QStringList stateNames = fsmBridge->getStateNames();
        loadedStates->addItems(stateNames);
    }

    // Load transitions
    QListWidget *loadedTransitions = findChild<QListWidget*>("listWidget_transitions");
    if (loadedTransitions) {
        loadedTransitions->clear();
        QStringList transitionsIds = fsmBridge->getTransitionIds();
        loadedTransitions->addItems(transitionsIds);
    }

    addLog(QString("Automaton %1 loaded successfully").arg(fsmBridge->getMachineName()));
}

/**
 * @brief Clear the graphic scene for new visualisation
 */
void AutomatonEditor::clearVisualization()
{
    // Clear scene
    scene->clear();
    
    // Clear data structures
    states.clear();
    transitions.clear();
}

/**
 * @brief Update the whole graphic scene with new visualisation
 */
void AutomatonEditor::updateVisualization()
{
    // Create visual representations of all states
    MooreMachine* machine = fsmBridge->getMachine();
    if (!machine) {
        return;
    }
    
    // Get all states
    for (State* state : machine->getAllStates()) {
        std::string stateId = state->getId();
        std::string stateName = state->getName();
        bool isInitial = state->getIsInitial();
        Point position = state->getPosition();
        
        // Check if this is a final state (has a "final" output)
        bool isFinal = false;
        for (const auto& output : state->getOutputs()) {
            if (output.value == "final") {
                isFinal = true;
                break;
            }
        }

        // Create the visual representation
        int x = position.x;
        int y = position.y;

        state_ellipse_item* stateEllipse = new state_ellipse_item(QRectF(x, y, 60, 60));
        scene->addItem(stateEllipse);

        connect(stateEllipse, &state_ellipse_item::stateMoved, this, [=]() {
            updateTransitionsForState(QString::fromStdString(stateId));
        });

        stateEllipse->setBrush(Qt::white);  // Fill the inside with white
        stateEllipse->setZValue(1);

        QGraphicsTextItem* label = scene->addText(QString::fromStdString(stateName));
        label->setZValue(3);

        // Center label
        QRectF ellipseRect = stateEllipse->rect();
        QRectF textRect = label->boundingRect();

        qreal labelX = x + ellipseRect.width() / 2 - textRect.width() / 2;
        qreal labelY = y + ellipseRect.height() / 2 - textRect.height() / 2;
        label->setPos(labelX, labelY);

        if (isFinal) {
            QGraphicsEllipseItem* innerEllipse = new QGraphicsEllipseItem(x + 5, y + 5, 50, 50, stateEllipse);
            scene->addItem(innerEllipse);
            innerEllipse->setZValue(2);
        }

        if (isInitial) {
            QGraphicsLineItem* lineStart = new QGraphicsLineItem(x - 30, y + 30, x, y + 30, stateEllipse); // Line to the state
            scene->addItem(lineStart);
        }
        
        // Store data
        StateItem item;
        item.id = QString::fromStdString(stateId);
        item.name = QString::fromStdString(stateName);
        item.isStart = isInitial;
        item.isFinal = isFinal;
        item.position = QPoint(x, y);
        item.ellipse = stateEllipse;
        item.label = label;
        
        states[item.id] = item;
    }
    
    // Create visual representations of all transitions
    for (auto stateIt = states.begin(); stateIt != states.end(); ++stateIt) {
        std::string sourceId = stateIt.key().toStdString();
        
        // Get all transitions from this state
        std::vector<Transition*> outTransitions = machine->getTransitionsFromState(sourceId);
        
        for (Transition* transition : outTransitions) {
            std::string targetId = transition->getTargetId();
            QString qSourceId = QString::fromStdString(sourceId);
            QString qTargetId = QString::fromStdString(targetId);
            
            // Get input condition
            QString inputValue = fsmBridge->getInputConditionsString(transition);

            // Skip if source or target state not found in our map
            if (!states.contains(qSourceId) || !states.contains(qTargetId)) {
                continue;
            }
            
            // Get state items
            StateItem* sourceItem = &states[qSourceId];
            StateItem* targetItem = &states[qTargetId];
            
            // Create a visual representation
            TransitionItem item;
            item.id = QString::fromStdString(transition->getId());
            item.stateFrom = sourceItem;
            item.stateTo = targetItem;
            item.inputValue = inputValue;
            
            // Handle self-transition
            if (qSourceId == qTargetId) {
                QPainterPath path;
                
                qreal radius = 20;
                qreal loopRadius = 60;
                qreal x = sourceItem->position.x() + loopRadius/2;
                qreal y = sourceItem->position.y() + radius/2;
                
                path.moveTo(x + radius, y);
                path.cubicTo(x + loopRadius, y - loopRadius,
                             x - loopRadius, y - loopRadius,
                             x - radius, y);
                
                item.pathItem = scene->addPath(path, QPen(Qt::black));
                item.labelItem = scene->addText(item.inputValue);
                item.labelItem->setPos(x - 10, y - loopRadius - 10);
            }
            else {
                QPointF p1 = sourceItem->ellipse->sceneBoundingRect().center();
                QPointF p2 = targetItem->ellipse->sceneBoundingRect().center();
                
                QPainterPath path;
                path.moveTo(p1);
                path.lineTo(p2);
                
                item.pathItem = scene->addPath(path, QPen(Qt::black));
                
                qreal midX = (p1.x() + p2.x()) / 2;
                qreal midY = (p1.y() + p2.y()) / 2;
                
                item.labelItem = scene->addText(item.inputValue);
                item.labelItem->setPos(midX, midY);
            }
            
            transitions[item.id] = item;
        }
    }
}

/**
 * @brief Update the transition position and graphics if the state has been moved
 * 
 * If the state in the graphic scene was moved (dragged by a mouse),
 * upddate the transition item values
 * 
 * @param stateId ID of the moved state
 */
void AutomatonEditor::updateTransitionsForState(const QString& stateId)
{
    if (!states.contains(stateId)) return;
    StateItem& moved = states[stateId];

    // Update position
    QPointF newPos = moved.ellipse->sceneBoundingRect().topLeft();
    moved.position = newPos.toPoint();

    // Update label
    QRectF textRect = moved.label->boundingRect();
    qreal labelX = newPos.x() + 30 - textRect.width() / 2;
    qreal labelY = newPos.y() + 30 - textRect.height() / 2;
    moved.label->setPos(labelX, labelY);

    // Update transitions
    for (auto& t : transitions) {
        if (t.stateFrom->id != stateId && t.stateTo->id != stateId)
            continue;

        if (t.pathItem)
            scene->removeItem(t.pathItem);
        if (t.labelItem)
            scene->removeItem(t.labelItem);

        QPointF p1 = t.stateFrom->ellipse->sceneBoundingRect().center();
        QPointF p2 = t.stateTo->ellipse->sceneBoundingRect().center();

        QPainterPath path;
        if (t.stateFrom->id == t.stateTo->id) {
            qreal radius = 20;
            qreal loopRadius = 60;
            qreal x = moved.position.x() + loopRadius / 2;
            qreal y = moved.position.y() + radius / 2;

            path.moveTo(x + radius, y);
            path.cubicTo(x + loopRadius, y - loopRadius,
                         x - loopRadius, y - loopRadius,
                         x - radius, y);

            t.pathItem = scene->addPath(path, QPen(Qt::black));
            t.pathItem->setZValue(0);
            t.labelItem = scene->addText(t.inputValue);
            t.labelItem->setPos(x - 10, y - loopRadius - 10);
        } else {
            path.moveTo(p1);
            path.lineTo(p2);
            t.pathItem = scene->addPath(path, QPen(Qt::black));
            t.pathItem->setZValue(0);
            qreal midX = (p1.x() + p2.x()) / 2;
            qreal midY = (p1.y() + p2.y()) / 2;
            t.labelItem = scene->addText(t.inputValue);
            t.labelItem->setPos(midX, midY);
        }
    }
}

/**
 * @brief Add a formatted log to the log veiw with exact time
 * @param logMessage Message to display
 */
void AutomatonEditor::addLog(const QString &logMessage) {
    QTextEdit *logBox = ui->textEdit_log;
    QString timestamp = QTime::currentTime().toString("HH:mm:ss.zzz");
    logBox->append(QString("time: %1 - %2").arg(timestamp, logMessage));
}

/**
 * @brief Add new input pointer to the list of input pointers
 */
void AutomatonEditor::on_pushButton_add_input_pointer_clicked()
{
    QStringList inputPointers = fsmBridge->getInputPointers();
    QString text = ui->lineEdit_add_input_pointer->text().trimmed();
    if (!text.isEmpty() && !inputPointers.contains(text)) {
        inputPointers.append(text);
        ui->listWidget_input_pointers->addItem(text);
        ui->lineEdit_add_input_pointer->clear();
        fsmBridge->addInputPointer(text);
        addLog(QString("Input pointer %1 added successfully").arg(text));

        // Update input pointers in simulation
        QComboBox *inputPointerCombo = findChild<QComboBox*>("inputPointerCombo");
        if (inputPointerCombo) {
            QStringList inputPointers = fsmBridge->getInputPointers();
            inputPointerCombo->addItems(inputPointers);
        }
    }
}

/**
 * @brief Remove selected input pointer from list of input pointers
 */
void AutomatonEditor::on_pushButton_remove_input_pointer_clicked()
{
    QStringList inputPointers = fsmBridge->getInputPointers();
    QListWidgetItem* item = ui->listWidget_input_pointers->currentItem();
    if (item) {
        inputPointers.removeAll(item->text());
        fsmBridge->removeInputPointer(item->text());
        addLog(QString("Input pointer %1 removed successfully").arg(item->text()));
        delete item;
    }
}

/**
 * @brief Add new input symbol to the machine input alphabet
 */
void AutomatonEditor::on_pushButton_add_input_alphabet_clicked()
{
    QStringList inputAlphabet = fsmBridge->getInputSymbols();
    QString text = ui->lineEdit_add_input_alphabet->text().trimmed();
    if (!text.isEmpty() && !inputAlphabet.contains(text)) {
        inputAlphabet.append(text);
        ui->listWidget_input_alphabet->addItem(text);
        ui->lineEdit_add_input_alphabet->clear();
        fsmBridge->addInputSymbol(text);
        addLog(QString("Input symbol %1 added successfully").arg(text));
    }
}

/**
 * @brief Remove selected input symbol from the input alphabet
 */
void AutomatonEditor::on_pushButton_remove_input_alphabet_clicked()
{
    QStringList inputAlphabet = fsmBridge->getInputSymbols();
    QListWidgetItem* item = ui->listWidget_input_alphabet->currentItem();
    if (item) {
        inputAlphabet.removeAll(item->text());
        fsmBridge->removeInputSymbol(item->text());
        addLog(QString("Input symbol %1 removed successfully").arg(item->text()));
        delete item;
    }
}

/**
 * @brief Add new output pointer to the list of output pointers
 */
void AutomatonEditor::on_pushButton_add_output_pointer_clicked()
{
    QStringList outputPointers = fsmBridge->getOutputPointers();
    QString text = ui->lineEdit_add_output_pointer->text().trimmed();
    if (!text.isEmpty() && !outputPointers.contains(text)) {
        outputPointers.append(text);
        ui->listWidget_output_pointers->addItem(text);
        ui->lineEdit_add_output_pointer->clear();
        fsmBridge->addOutputPointer(text);
        addLog(QString("Output pointer %1 added successfully").arg(text));
    }
}

/**
 * @brief Remove selected output pointer from list of output pointers
 */
void AutomatonEditor::on_pushButton_remove_output_pointer_clicked()
{
    QStringList outputPointers = fsmBridge->getOutputPointers();
    QListWidgetItem* item = ui->listWidget_output_pointers->currentItem();
    if (item) {
        outputPointers.removeAll(item->text());
        fsmBridge->removeOutputPointer(item->text());
        addLog(QString("Output pointer %1 removed successfully").arg(item->text()));
        delete item;
    }
}

/**
 * @brief Add new output symbol to the machine output alphabet
 */
void AutomatonEditor::on_pushButton_add_output_alphabet_clicked()
{
    QStringList outputAlphabet = fsmBridge->getOutputSymbols();
    QString text = ui->lineEdit_add_output_alphabet->text().trimmed();
    if (!text.isEmpty() && !outputAlphabet.contains(text)) {
        outputAlphabet.append(text);
        ui->listWidget_output_alphabet->addItem(text);
        ui->lineEdit_add_output_alphabet->clear();
        fsmBridge->addOutputSymbol(text);
        addLog(QString("Output symbol %1 added successfully").arg(text));
    }
}

/**
 * @brief Remove selected output symbol from the output alphabet
 */
void AutomatonEditor::on_pushButton_remove_output_alphabet_clicked()
{
    QStringList outputAlphabet = fsmBridge->getOutputSymbols();
    QListWidgetItem* item = ui->listWidget_output_alphabet->currentItem();
    if (item) {
        outputAlphabet.removeAll(item->text());
        fsmBridge->removeOutputSymbol(item->text());
        addLog(QString("Output symbol %1 removed successfully").arg(item->text()));
        delete item;
    }
}

/**
 * @brief Create a new variable
 * 
 * @param name Variable name
 * @param type Variable type (one of string, int or float)
 * @param value Variable value
 */
void AutomatonEditor::createNewVar(const QString &name, const QString &type, const QString &value) {

    if (!fsmBridge->addVar(name, type, value)) {
        QMessageBox::warning(this, "Error", "Failed to add variable to the machine");
        return;
    }

    addLog(QString("Variable %1 added successfully").arg(name));

    // Add it to the list widget
    ui->listWidget_vars->addItem(name);
}

/**
 * @brief Call the DialogAddVariable dialog and create new variable
 * 
 * In the dialog add variable name, select its type and add its value
 */
void AutomatonEditor::on_pushButton_add_var_clicked() {
    while (true) {
        DialogAddVariable dialog(this);
        if (dialog.exec() != QDialog::Accepted)
            return;

        QString name = dialog.getVarName();
        QString type = dialog.getVarType();
        QString value = dialog.getVarValue();

        if (name.isEmpty()) {
            QMessageBox::warning(this, "Empty Name", "Please enter a Variable Name");
            continue; // re-show dialog
        }

        bool nameExists = false;
        if (fsmBridge->getVariableNames().contains(name)) {
            nameExists = true;
        }

        if (nameExists) {
            QMessageBox::warning(this, "Duplicate Name", "A variable with this name already exists. Please choose a different one.");
            continue; // re-show dialog
        }

        // If it's the first state, make it initial regardless
        createNewVar(name, type, value);

        break; // valid, exit loop
    }
}

/**
 * @brief Remove variable from the list of variables
 */
void AutomatonEditor::on_pushButton_remove_var_clicked()
{
    QListWidgetItem* selectedVar = ui->listWidget_vars->currentItem();
    if (!selectedVar) {
        QMessageBox::information(this, "No Selection", "Please select a variable to remove.");
        return;
    }

    // Remove from the FSM model first
    if (!fsmBridge->removeVar(selectedVar->text())) {
        QMessageBox::warning(this, "Error", "Failed to remove variable from the machine");
        return;
    }

    addLog(QString("Variable %1 removed successfully").arg(selectedVar->text()));
    delete selectedVar;
}
