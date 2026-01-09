#include <iostream>
#include <chrono>
#include <thread>
#include "counter_callback.h"

// Callback for state changes
void onStateChange(const std::string& newState) {
    std::cout << "State changed to: " << newState << std::endl;
}

// Callback for output changes
void onOutputChange(const std::string& outputName, const std::string& value) {
    std::cout << "Output " << outputName << " = " << value << std::endl;
}

// Callback for the Counting state
void onCountingState(const std::unordered_map<std::string, std::string>& outputs,
                   const std::unordered_map<std::string, CounterFSM::VariableType>& variables) {
    int count = 0;
    try {
        if (variables.find("count") != variables.end()) {
            count = std::get<int>(variables.at("count"));
        }
    } catch (...) {}
    
    std::cout << "In Counting state, count = " << count << std::endl;
}

int main() {
    std::cout << "==== Counter FSM Demo (Callback Style) ====\n\n";
    
    // Create an instance of the generated FSM
    CounterFSM fsm;
    
    // Set up callbacks
    fsm.setStateChangeCallback(onStateChange);
    fsm.setOutputChangeCallback(onOutputChange);
    fsm.setStateCallback("Counting", onCountingState);
    
    // Initialize variables
    fsm.setVariable("count", 0);
    fsm.setVariable("limit", 5);
    
    // Reset to initial state
    fsm.reset();
    std::cout << "Initial state: " << fsm.getCurrentStateName() << "\n";
    
    // Send "start" input
    std::cout << "Sending 'start' input...\n";
    fsm.processInput("start");
    
    // Run for several steps
    for (int i = 0; i < 10; i++) {
        fsm.tick();
        
        // Display current state and variables
        std::cout << "State: " << fsm.getCurrentStateName() 
                  << ", Count: " << fsm.getVariable<int>("count") 
                  << " / " << fsm.getVariable<int>("limit") << "\n";
                  
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
        
        // Break if we've reached the Done state
        if (fsm.getCurrentStateName() == "Done") {
            break;
        }
    }
    
    return 0;
}