// xbohach00
#include <iostream>
#include <string>
#include <stdexcept>

#include "../../src/headers/machine_file_handler.h"
#include "../../src/headers/includable_generator.h"
#include "../../src/headers/moore_machine.h"

int main(int argc, char* argv[]) {
    if (argc < 5) {
        std::cerr << "Usage: " << argv[0] << " <fsm_file> <class_name> <callback_base> <goto_base>" << std::endl;
        return 1;
    }
    
    std::string fsmFile = argv[1];
    std::string className = argv[2];
    std::string callbackBase = argv[3];
    std::string gotoBase = argv[4];
    
    try {
        // Load the FSM from file
        std::cout << "Loading FSM from " << fsmFile << "..." << std::endl;
        MooreMachine machine = MachineFileHandler::loadFromFile(fsmFile);
        
        // Generate callback style
        std::cout << "Generating callback style code..." << std::endl;
        if (IncludableGenerator::generateCode(machine, callbackBase, className, CodeStyle::CALLBACK)) {
            std::cout << "Successfully generated " << callbackBase << ".h and " << callbackBase << ".cpp" << std::endl;
        } else {
            std::cerr << "Failed to generate callback style code!" << std::endl;
            return 1;
        }
        
        // Generate computed goto style
        std::cout << "Generating computed goto style code..." << std::endl;
        if (IncludableGenerator::generateCode(machine, gotoBase, className, CodeStyle::COMPUTED_GOTO)) {
            std::cout << "Successfully generated " << gotoBase << ".h and " << gotoBase << ".cpp" << std::endl;
        } else {
            std::cerr << "Failed to generate computed goto style code!" << std::endl;
            return 1;
        }
        
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
}