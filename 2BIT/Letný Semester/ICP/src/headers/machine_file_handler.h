/**
 * @file machine_file_handler.h
 * @brief Declaration of the MachineFileHandler class for saving/loading machines
 * @author Hugo Bohácsek (xbohach00)
 */

#ifndef MACHINE_FILE_HANDLER_H
#define MACHINE_FILE_HANDLER_H

#include <string>
#include "moore_machine.h"

/**
 * @class MachineFileHandler
 * @brief Handles saving and loading Moore machines to/from files
 * 
 * This class provides static methods to serialize a Moore machine to a file
 * and deserialize it from a file.
 */
class MachineFileHandler {
public:
    /**
     * @brief Saves a machine to a file
     * 
     * @param machine The machine to save
     * @param filename Path to the file where the machine will be saved
     * @return True if saving succeeded, false otherwise
     */
    static bool saveToFile(const MooreMachine& machine, const std::string& filename);
    
    /**
     * @brief Loads a machine from a file
     * 
     * @param filename Path to the file containing the machine definition
     * @return Loaded MooreMachine object
     * @throw std::runtime_error If loading fails or file format is invalid
     */
    static MooreMachine loadFromFile(const std::string& filename);
    
private:
    /**
     * @brief Splits a string by a delimiter
     * 
     * Helper method used during file parsing
     * 
     * @param str String to split
     * @param delimiter Character to split by
     * @return Vector of substrings
     */
    static std::vector<std::string> split(const std::string& str, char delimiter);
};

#endif // MACHINE_FILE_HANDLER_H