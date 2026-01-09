/**
 * @file includable_generator.h
 * @brief Declaration of the IncludableGenerator class that creates FSM code for importing into other projects
 * @author Hugo Bohácsek (xbohach00)
 */

#ifndef INCLUDABLE_GENERATOR_H
#define INCLUDABLE_GENERATOR_H

#include "moore_machine.h"
#include <string>

/**
 * @enum CodeStyle
 * @brief Defines the types of code style determined via compilers
 */
enum class CodeStyle{
    CALLBACK,
    COMPUTED_GOTO
};

/**
 * @class IncludableGenerator
 * @brief Generates includable C++ code from a Moore machine
 * 
 * This class creates C++ header and implementation files that can be
 * included in other projects, providing a reusable FSM implementation.
 */
class IncludableGenerator {
private:
    static std::string generateHeaderContent(const MooreMachine& machine, const std::string& className, CodeStyle style);
    static std::string generateSourceContent(const MooreMachine& machine, const std::string& className, const std::string& headerName, CodeStyle style);
    static std::string generateGotoImplementation(const MooreMachine& machine, const std::string& className, const std::string& headerName);
public:
    /**
     * @brief Generates includable C++ code from a Moore machine
     * 
     * @param machine The Moore machine to convert to code
     * @param baseName Base name for the generated files (without extension)
     * @param className Name of the generated C++ class
     * @return True if code generation succeeded, false otherwise
     */
    static bool generateCode(const MooreMachine& machine, 
                             const std::string& baseName, 
                             const std::string& className = "FSMAutomaton",
                             CodeStyle style = CodeStyle::CALLBACK);
};

#endif // INCLUDABLE_GENERATOR_H