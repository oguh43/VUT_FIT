/**
 * @file state.h
 * @brief Declaration of the State class and OutputCondition structure
 * @author Hugo Bohácsek (xbohach00)
 */

#ifndef STATE_H
#define STATE_H

#include <string>
#include <vector>
#include "point.h"

/**
 * @struct OutputCondition
 * @brief Represents an output condition for a state
 * 
 * Outputs can be unconditional or conditional (based on an input).
 */
struct OutputCondition {
    std::string value;       /**< Output value */
    std::string target;      /**< Output pointer target */
    std::string inputPtr;    /**< Input pointer to check for condition */
    bool hasCondition;       /**< Whether this output has a condition */

    /**
     * @brief Constructor
     * 
     * @param value Output value
     * @param target Output pointer target
     * @param inputPtr Input pointer for condition (empty if no condition)
     * @param hasCondition Whether this output has a condition
     */
    OutputCondition(const std::string& value, const std::string& target, 
                const std::string& inputPtr = "", bool hasCondition = false)
        : value(value), target(target), inputPtr(inputPtr), hasCondition(hasCondition) {}
};

/**
 * @class State
 * @brief Represents a state in a finite state machine
 * 
 * States have an ID, name, position, and can be marked as initial.
 * Each state can produce outputs, either unconditionally or conditionally.
 */
class State {
private:
    std::string id;         /**< Unique identifier */
    std::string name;       /**< Display name */
    std::vector<OutputCondition> outputs; /**< Outputs associated with this state */
    bool isInitial;         /**< Whether this is the initial state */
    Point position;         /**< Position in the GUI (x,y coordinates) */
    
public:
    /**
     * @brief Default constructor
     * 
     * Creates a state with empty ID and name
     */
    State() : id(""), name(""), isInitial(false), position(0, 0) {}

    /**
     * @brief Constructor with ID and name
     * 
     * @param id Unique identifier
     * @param name Display name
     */
    State(const std::string& id, const std::string& name);
    
    /**
     * @brief Constructor with ID, name, and output
     * 
     * @param id Unique identifier
     * @param name Display name
     * @param output Output value
     */
    State(const std::string& id, const std::string& name, const std::string& output);
    
    /**
     * @brief Gets the state ID
     * 
     * @return The state ID
     */
    std::string getId() const;
    
    /**
     * @brief Gets the state name
     * 
     * @return The state name
     */
    std::string getName() const;
    
    /**
     * @brief Gets the state output (legacy)
     * 
     * @return The first output value, or empty string if none
     */
    std::string getOutput() const;
    
    /**
     * @brief Gets all outputs for the state
     * 
     * @return Vector of output conditions
     */
    std::vector<OutputCondition> getOutputs() const;
    
    /**
     * @brief Checks if this is the initial state
     * 
     * @return True if this is the initial state
     */
    bool getIsInitial() const;
    
    /**
     * @brief Gets the state position
     * 
     * @return Position as a Point
     */
    Point getPosition() const;
    
    /**
     * @brief Sets the state name
     * 
     * @param name New name for the state
     */
    void setName(const std::string& name);
    
    /**
     * @brief Sets the state output (legacy)
     * 
     * @param output Output value
     */
    void setOutput(const std::string& output);
    
    /**
     * @brief Adds an output condition
     * 
     * @param output Output condition to add
     */
    void addOutput(const OutputCondition& output);
    
    /**
     * @brief Clears all outputs
     */
    void clearOutputs();
    
    /**
     * @brief Sets whether this is the initial state
     * 
     * @param initial Whether this should be the initial state
     */
    void setIsInitial(bool initial);
    
    /**
     * @brief Sets the state position
     * 
     * @param position New position for the state
     */
    void setPosition(const Point& position);
    
    /**
     * @brief Evaluates outputs based on available inputs
     * 
     * @param availableInputs Vector of input pointer/value pairs
     * @return Vector of output pointer/value pairs
     */
    std::vector<std::pair<std::string, std::string>> evaluateOutputs(
        const std::vector<std::pair<std::string, std::string>>& availableInputs) const;
};

#endif // STATE_H