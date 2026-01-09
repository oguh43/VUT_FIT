#include <iostream>
#include <chrono>
#include <thread>
#include "counter_goto.h"

int main() {
    std::cout << "==== Counter FSM Demo (Computed Goto Style) ====\n\n";

    CounterFSM fsm;

    // Initialize variables
    fsm.setVariable("count", 0);
    fsm.setVariable("limit", 5);

    // Reset to initial state
    fsm.reset();
    std::cout << "Initial state: " << fsm.getCurrentStateName() << "\n";

    // Set up state targets
    fsm.setStateTarget(CounterFSM::STATE_Init, &&state_init);
    fsm.setStateTarget(CounterFSM::STATE_Counting, &&state_counting);
    fsm.setStateTarget(CounterFSM::STATE_Done, &&state_done);

    // Start FSM loop using computed goto
    goto *fsm.getStateTarget(fsm.getCurrentStateId());

state_init:
    std::cout << "In Init state\n";
    fsm.processInput("start");
    fsm.tick();
    goto *fsm.getStateTarget(fsm.getCurrentStateId());

state_counting: {
    int count = fsm.getVariable<int>("count");
    int limit = fsm.getVariable<int>("limit");
    std::cout << "In Counting state, count = " << count << " / " << limit << "\n";

    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    fsm.tick();

    // Check if we've transitioned to Done
    if (fsm.getCurrentStateId() == CounterFSM::STATE_Done)
        goto state_done;

    // Otherwise continue in Counting
    goto state_counting;
}

state_done:
    std::cout << "In Done state, final count = " << fsm.getVariable<int>("count") << "\n";
    return 0;
}
