/**
 * FIT VUT - IFJ project 2024
 *
 * @file generator.h
 *
 * @brief Code generator implementation for IFJ24
 *
 * @author Filip Jenis (xjenisf00)
 */

#ifndef GENERATOR_H
#define GENERATOR_H

#include "scanner.h"

typedef enum{LABEL_WHILE, LABEL_IF} label_t;

/**
 * Initialization of the code generator
 */
void generator_init();

/**
 * Generates output code definitions of builtin functions
 */
void generate_builtin_functions();

/**
 * Generates the beginning of main scope (main function)
 */
void generate_main_function_head();

/**
 * Generates the beginning of a function scope
 * @param identifier Identifier of the function
 */
void generate_function_head(const char *identifier);

/**
 * Generates the end of a function scope
 * @param identifier Identifier of the function
 */
void generate_function_end(const char *identifier);

/**
 * Generates frame for function call parameters
 */
void generate_function_call_param_head();

/**
 * Generates parameter pass for function call
 * @param index Index of the parameter
 * @param token Pointer to the parameter token
 */
void generate_function_call_param(unsigned index, Token *token);

/**
 * Generates a function call
 * @param identifier Identifier of the function
 */
void generate_function_call(const char *identifier);

/**
 * Generates function call parameter
 * @param identifier Identifier of the parameter
 * @param index Index of the parameter
 */
void generate_function_param(const char *identifier, unsigned index);

/**
 * Generates return from a function
 * @param identifier Identifier of the function
 */
void generate_function_return(const char *identifier);

/**
 * Generates function return value
 */
void generate_function_return_value();

/**
 * Generates the end of main scope
 */
void generate_main_function_end();

/**
 * Generates declaration of a variable
 * @param identifier Identifier of the variable
 */
void generate_variable_declaration(const char *identifier);

/**
 * Generates definition (assigment) of a variable
 * @param identifier Identifier of the variable
 */
void generate_variable_definition(const char *identifier);

/**
 * Generates value of a term
 * @param token Pointer to the token of the term
 */
void generate_term(Token *token);

/**
 * Generates push of a term to stack
 * @param token Pointer to the token of the term
 */
void generate_stack_push(Token *token);

/**
 * Generates operation on stack
 * @param token Pointer to the token of the operation
 */
void generate_stack_operation(Token *token);

/**
 * Generates type check for operands and change of type if they are incompatible
 */
void generate_stack_operation_type_check();

/**
 * Generates result of stack operation
 */
void generate_stack_operation_result();

/**
 * Generates the head of an if
 *
 * @param pipe Boolean if the if has a pipe
 */
void generate_if_head(bool pipe);

/**
 * Generates the end of the true branch of the if
 */
void generate_if_end();

/**
 * Generates the pipe for if expression result
 * @param identifier Identifier of the pipe
 */
void generate_if_pipe(const char *identifier);

/**
 * Generates the head of the false (else) branch of the if
 */
void generate_else();

/**
 * Generates the end of the whole if
 */
void generate_else_end();

/**
 * Generates the head of a while
 */
void generate_while_head();

/**
 * Generates the condition check for the while
 *
 * @param pipe Boolean if the while has a pipe
 */
void generate_while_condition(bool pipe);

/**
 * Generates the end of the while
 */
void generate_while_end();

/**
 * Generates the pipe declaration for while expression result
 * @param identifier Identifier of the pipe
 */
void generate_while_pipe(const char *identifier);

/**
 * Generates the pipe definition (assigment) for while expression result
 * @param identifier Identifier of the pipe
 */
void generate_while_pipe_definition(const char *identifier);

/**
 * Helper function for if/while label generating
 * @param type Type of the label - if/while
 */
void generate_label(label_t type);

/**
 * Helper function for getting current if/while label
 * @param type Type of the label - if/while
 */
void get_label(label_t type);

/**
 * Helper function for clearing the current if/while label
 * @param type Type of the label - if/while
 */
void get_label_pop(label_t type);

/**
 * Generates the assigment from function return value
 */
void generate_function_return_value_assign();

/**
 * Generates push of the function return value to stack
 */
void generate_function_return_value_push();

/**
 * Generates the call of a builtin function
 * @param identifier Identifier of the builtin function
 */
void generate_builtin_function_call(const char *identifier);

/**
 * Flushes the generated code to standard output and frees all resources allocated for code generator
 */
void generator_flush();

/**
 * Frees all resources allocated for code generator
 */
void generator_dispose();

#endif //GENERATOR_H
