ICP project - Finite State Machine app

Authors:
Hugo Bohácsek (xbohach00)
Filip Jenis (xjenisf00)
Eliška Křeménková (xkremee00)

Program Overview
This application provides a visual editor and simulator for interpreted Moore-type finite state machines (FSMs) with extended capabilities, including input events, delayed transitions, and state actions. It is designed to support simulation, real-time monitoring, and code generation for integration into larger C++ applications.

Implemented functionality
GUI editor for creating new machine or loading the machine from existing .fsm file.
Editor allows:
    - adding/removing I/O pointers and alphabet
    - defining and managing automaton variables
    - creating, editing (only for transitions) and deleting states and transitions
    - running the simulation
    
Each state has a unique name, flag indicating if it's an initial or final state (only one of each for the machine) and its position.
Each transition has source and target state (can be the same for self-loops), timeout in millisecond and input conditions or a boolean condition. Transition timeout and input conditions can be later edited.

During simulation user can inject input symbols on a specific input pointer and generate output to log with timestamps.

The current machine can be saved to the .fsm file, exported as corresponding C++ code or an includeable format for use in other projects.

The implemented code generator creates and compiles C++ code, so that the machine can be executed outside of our editor. It contains all the functions needed for the full execution of a Moore Machine. Our editor can connect to the running generated machine via UDP and asynchronously infuse input values. The editor shows the current state of the machine as well as stores logs about all the machine actions.

Missing functionality
Output conditions can't be defined in the editor itself, but can be loaded from the machine definition in .fsm file.

Requirements
    - Qt 5.9.2
    - C++ 17
