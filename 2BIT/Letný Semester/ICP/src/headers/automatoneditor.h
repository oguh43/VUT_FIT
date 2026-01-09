/**
 * @file automatoneditor.h
 * @author Eliška Křeménková (xkremee00)
 * @brief Declaration of the AutomatonEditor class that handles creating, editing and running
 * the automaton
 * @date 2025-05-09
 */

#ifndef AUTOMATONEDITOR_H
#define AUTOMATONEDITOR_H

#include "dialogaddstate.h"
#include "dialogaddtransition.h"
#include "dialogaddbooleantransition.h"
#include "dialogedittransition.h"
#include "dialogaddvariable.h"
#include "state_ellipse_item.h"
#include "fsm_bridge.h"
#include "code_generator.h"
#include <QMainWindow>
#include <QGraphicsScene>
#include <QGraphicsEllipseItem>
#include <QGraphicsTextItem>
#include <QGraphicsPathItem>
#include <QString>
#include <QPoint>
#include <QMap>
#include <QVector>
#include <QMessageBox>
#include <QFileDialog>
#include <QTimer>

namespace Ui {
class AutomatonEditor;
}

/**
 * @struct StateItem
 * @brief Stores the state with all its attributes and graphics
 */
struct StateItem {
    QString id;                             /** Unique state identifier */
    QString name;                           /** State name */
    bool isStart = false;                   /** Whether this is the initial state*/
    bool isFinal = false;                   /** Whether this is a final state */
    QPoint position;                        /** Position of the state in the scene */
    state_ellipse_item* ellipse = nullptr;  /** Pointer to the ellipse item representing the state in the scene */
    QGraphicsTextItem* label = nullptr;     /** Pointer to the state name text item in the scene */
};

/**
 * @struct TransitionItem
 * @brief Stores the transition with all its attributes and graphics
 */
struct TransitionItem {
    QString id;                             /** Unique state identifier */
    StateItem* stateFrom = nullptr;         /** Pointer to the source State */
    StateItem* stateTo = nullptr;           /** Pointer to the target State */
    QString inputValue;                     /** String containing all of the input conditions */
    bool isBoolean = false;                 /** Whether the transition uses boolean input condition */
    QGraphicsPathItem* pathItem = nullptr;  /** Pointer to the path item (line representing the transition) in the scene */
    QGraphicsTextItem* labelItem = nullptr; /** Pointer to the transition label in the scene */
};

/**
 * @class AutomatonEditor
 * @brief Main class for editing and simulating a Moore machine automaton
 * 
 * Provides a graphical editor to add/remove/edit states and transitions,
 * manage input/output alphabets and pointers, run simulations, and visualize state transitions
 */
class AutomatonEditor : public QMainWindow
{
    Q_OBJECT

public:
    /**
     * @brief Construct a new Automaton Editor object
     * 
     * This is the constructor for creating new automaton from scratch
     * 
     * @param automatonName The name of the automaton to create
     * @param parent Parent widget, default is nullptr
     */
    explicit AutomatonEditor(const QString& automatonName, QWidget *parent = nullptr);
    
    /**
     * @brief Construct a new Automaton Editor object
     * 
     * This is the constructor for loading automaton from file and creating visual representation for it
     * 
     * @param filePath File path to load the automaton from
     * @param loadFromFile If true, loads the machine from the given file
     * @param parent Parent widget, default is nullptr
     */
    explicit AutomatonEditor(const QString& filePath, bool loadFromFile, QWidget *parent = nullptr);

    /**
     * @brief Destroy the Automaton Editor object
     */
    ~AutomatonEditor();

    /**
     * @brief Create a new state and add it to the scene and machine
     * 
     * @param name State name
     * @param x X position
     * @param y Y position
     * @param isStart If the state is initiol
     * @param isFinal If the state is final
     */
    void createNewState(const QString& name, int x, int y, bool isStart, bool isFinal);

private slots:

    /**
     * @brief Call the DialogAddState dialog and add new state
     */
    void on_pushButton_add_state_clicked();

    /**
     * @brief Call the DialogAddTransition dialog and add new transition
     * 
     * Create new transition, add its source and target state,
     * input conditions (optional) and timeout (optional)
     */
    void on_pushButton_add_transition_clicked();

    /**
     * @brief Call the DialogAddBooleanTransition dialog and add new boolean transition
     * 
     * Create new boolean transition, add its source and target state and input condition (optional)
     */
    void on_pushButton_add_bool_transition_clicked();

    /**
     * @brief Edit selected transition from the list widget and edit its input conditions or timeout
     */
    void on_pushButton_edit_transition_clicked();

    /**
     * @brief Remove the selected state from the list widget (also remove its transitions)
     */
    void on_pushButton_remove_state_clicked();

    /**
     * @brief Remove selected transition from the list widget
     */
    void on_pushButton_remove_transition_clicked();

    /**
     * @brief Add new input pointer to the list of input pointers
     */
    void on_pushButton_add_input_pointer_clicked();

    /**
     * @brief Remove selected input pointer from list of input pointers
     */
    void on_pushButton_remove_input_pointer_clicked();

    /**
     * @brief Add new input symbol to the machine input alphabet
     */
    void on_pushButton_add_input_alphabet_clicked();

    /**
     * @brief Remove selected input symbol from the input alphabet
     */
    void on_pushButton_remove_input_alphabet_clicked();

    /**
     * @brief Add new output pointer to the list of output pointers
     */
    void on_pushButton_add_output_pointer_clicked();

    /**
     * @brief Remove selected output pointer from list of output pointers
     */
    void on_pushButton_remove_output_pointer_clicked();

    /**
     * @brief Add new output symbol to the machine output alphabet
     */
    void on_pushButton_add_output_alphabet_clicked();

    /**
     * @brief Remove selected output symbol from the output alphabet
     */
    void on_pushButton_remove_output_alphabet_clicked();

    /**
     * @brief Call the DialogAddVariable dialog and create new variable
     * 
     * In the dialog add variable name, select its type and add its value
     */
    void on_pushButton_add_var_clicked();

    /**
     * @brief Remove variable from the list of variables
     */
    void on_pushButton_remove_var_clicked();

    /**
     * @brief Start/Stop and toggle the simulation
     */
    void on_pushButton_start_clicked();

    /**
     * @brief Save the automaton in editor to a file
     */
    void on_actionSave_triggered();

    /**
     * @brief Load automaton from a file and replace the current one
     */
    void on_actionLoad_triggered();

    /**
     * @brief Exit the app
     */
    void on_actionExit_triggered();

    /**
     * @brief Generate C++ code of the current automaton and save it to a file
     */
    void on_actionCodeGen_triggered();

    /**
     * @brief Generate includable file in code style Callback
     */
    void on_actionGenCallback_triggered();

    /**
     * @brief Generate includable file in code style Computed goto
     */
    void on_actionGenGoto_triggered();

    /**
     * @brief Connect to a running automaton
     */
    void on_actionConnect_triggered();

private:
    /**
     * @brief Pointer to the UI elements defined in the automatoneditor.ui file.
     * 
     * This is automatically generated by Qt's user interface compiler (uic) and provides
     * access to all widgets and layout elements created in the Qt Designer for AutomatonEditor.
     */
    Ui::AutomatonEditor *ui;

    /**
     * @brief Graphics scene for drawing the automaton
     */
    QGraphicsScene* scene;

    /**
     * @brief Map of all state items in the automaton, idexed by State ID
     */
    QMap<QString, StateItem> states;

    /**
     * @brief Map of all transition items in the automaton, idexed by Transition ID
     */
    QMap<QString, TransitionItem> transitions;

    /**
     * @brief Bridge connecting the frontend with the backend
     */
    FSMBridge* fsmBridge;

    /** 
     * @brief Timer used for the simulation
     */
    QTimer* simulationTimer;

    /**
     * @brief Timer used for the connection to running automaton
     */
    QTimer* connectionTimer;

    /** 
     * @brief Flag to see if the simulation is running
     */
    bool simulationActive;

    /**
     * @brief Flag to see if there is a remote automaton connected
     */
    bool connectionActive;

    /** 
     * @brief ID of the last highlighted state
     */
    QString lastLoggedState;

    /** 
     * @brief Map of last output values printed to the log
     */
    QMap<QString, QString> lastOutputs;
    
    /**
     * @brief Set up the core UI components (menu, graphic scene and simulation controls) for the AutomatonEditor window
     * 
     */
    void setupUi();

    /**
     * @brief Initialize a new finite state machine (FSM) with a given name
     * @param name Name for the new machine
     */
    void initializeFSM(const QString& name);

    /**
     * @brief Check if the machine already has initial state
     * @return True if the initial state exists, false otherwise
     */
    bool hasInitialState() const;

    /**
     * @brief Check if the machine already has final state
     * @return True if the final state exists, false otherwise
     */
    bool hasFinalState() const;

    /**
     * @brief Load automaton from a file and create its visual representation
     * @param filePath Path to the .fsm file to load
     */
    void loadFromFile(const QString& filePath);

    /**
     * @brief Save the current automaton to the file
     * @param filePath Destination for the file to save
     */
    void saveToFile(const QString& filePath);

    /**
     * @brief Save the current automaton to the file
     * @param filePath Destination for the file to save
     */
    void saveCodeToFile(const QString& filePath);

    /**
     * @brief Clear the graphic scene for new visualisation
     */
    void clearVisualization();

    /**
     * @brief Update the whole graphic scene with new visualisation
     */
    void updateVisualization();

    /**
     * @brief Highlight/unhighlight the given state in the simulation
     * @param stateId ID of the state to highlight/unhighlight
     * @param highlight true to highlight, false to unhighlight
     */
    void highlightState(const QString& stateId, bool highlight);

    /**
     * @brief Set up the simulation control panel UI
     */
    void setupSimulationControls();

    /**
     * @brief Start/stop the simulation
     * @param active true to start simulation, false to end it
     */
    void toggleSimulation(bool active);

    /**
     * @brief Update the transition position and graphics if the state has been moved
     * 
     * If the state in the graphic scene was moved (dragged by a mouse),
     * upddate the transition item values
     * 
     * @param stateId ID of the moved state
     */
    void updateTransitionsForState(const QString& stateId);

    /**
     * @brief Add a formatted log to the log veiw with exact time
     * @param logMessage Message to display
     */
    void addLog(const QString& logMessage);

    /**
     * @brief Create a new variable
     * 
     * @param name Variable name
     * @param type Variable type (one of string, int or float)
     * @param value Variable value
     */
    void createNewVar(const QString& name, const QString& type, const QString& value);

    /**
     * @brief Execute one step of the simulation cycle
     * 
     * Invoked periodically by the QTimer to evaluate timeout or process condition-based transitions,
     * update the visual state, and log output changes
     */
    void simulationStep();

    /**
     * @brief Handle given input symbol on pointer during simulation
     */
    void onInputEntered();

    /**
     * @brief Connect/disconnect to/from a running automaton
     * @param active true to start the connection, false to end it
     */
    void toggleConnection(bool active);

    /**
     * @brief Refresh data from remote running automaton
     *
     * Invoked periodically by the QTimer to update the visual state, and log outputs
     */
    void connectionStep();
};

#endif // AUTOMATONEDITOR_H
