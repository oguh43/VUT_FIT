/**
 * @author Filip Jenis (xjenisf00)
 */

#ifndef CODE_GENERATOR_H
#define CODE_GENERATOR_H

#include "moore_machine.h"
#include "machine_file_handler.h"
#include <fstream>
#include <iostream>
#include <sstream>
#include <QProcess>
#include <QDebug>

/**
 * @class CodeGenerator
 * @brief Generates and compiles executable Moore machine
 */
class CodeGenarator{
private:
    static std::string generatedFileHeader; /**< Static C++ header code for generated code */
    static std::string helperFunctions; /**< Static C++ code of helper functions */
    static std::string transitionFunctions; /**< Static C++ code of transition handling */
    static std::string automatonEngineFunctions; /**< Static C++ code of the main automaton engine */
    static std::string networkFunctions; /**< Static C++ code of UDP networking functions */

    /**
     * @brief Compiles the generated C++ code
     *
     * @param sourcePath Path of the generated C++ file
     * @param outputPath Path of the compiled executable
     */
    static void compileGeneratedCode(const QString& sourcePath, const QString& outputPath);

public:
    /**
     * @brief Generates C++ code from the internal representation of a Moore Machine
     *
     * @param machine Internal representation of a Moore Machine
     * @param filename Path for the generated code
     * @return True if generation is successful, false otherwise
     */
    static bool generateCode(const MooreMachine& machine, const std::string& filename);
};

#endif //CODE_GENERATOR_H
