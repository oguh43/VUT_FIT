/**
 * FIT VUT - IFJ project 2024
 *
 * @file generator.c
 *
 * @brief Code generator implementation for IFJ24
 *
 * @author Filip Jenis (xjenisf00)
 */

#include "generator.h"
#include "dynamic_string.h"
#include "label_list.h"
#include <stdio.h>

/// Macro for adding generated code concluded by a newline to the dynamic string buffer
#define OUT(output) dynamic_string_append(&generatorOutput, (output "\n"))
/// Macro for adding generated code with possible line continuation to the dynamic string buffer
#define OUT_CL(output) dynamic_string_append(&generatorOutput, output)

/** Built-in functions definitions */

/// pub fn ifj.readstr() ?[]u8
#define BUILTIN_READSTR "LABEL readstr\n" \
                        "PUSHFRAME\n"     \
                        "DEFVAR LF@%retval\n" \
                        "READ LF@%retval string\n" \
                        "POPFRAME\n"      \
                        "RETURN"

/// pub fn ifj.readi32() ?i32
#define BUILTIN_READI32 "LABEL readi32\n" \
                        "PUSHFRAME\n"     \
                        "DEFVAR LF@%retval\n" \
                        "READ LF@%retval int\n" \
                        "POPFRAME\n"      \
                        "RETURN"

/// pub fn ifj.readf64() ?f64
#define BUILTIN_READF64 "LABEL readf64\n" \
                        "PUSHFRAME\n"     \
                        "DEFVAR LF@%retval\n" \
                        "READ LF@%retval float\n" \
                        "POPFRAME\n"      \
                        "RETURN"

/// pub fn ifj.write(term) void
#define BUILTIN_WRITE "LABEL write\n" \
                      "PUSHFRAME\n"   \
                      "WRITE LF@%0\n" \
                      "POPFRAME\n"    \
                      "RETURN"

/// pub fn ifj.i2f(term ∶ i32) f64
#define BUILTIN_I2F "LABEL i2f\n" \
                    "PUSHFRAME\n" \
                    "DEFVAR LF@%retval\n" \
                    "INT2FLOAT LF@%retval LF@%0\n" \
                    "POPFRAME\n"  \
                    "RETURN"

/// pub fn ifj.f2i(term ∶ f64) i32
#define BUILTIN_F2I     "LABEL f2i\n" \
                        "PUSHFRAME\n" \
                        "DEFVAR LF@%retval\n" \
                        "FLOAT2INT LF@%retval LF@%0\n" \
                        "POPFRAME\n" \
                        "RETURN"

/// pub fn ifj.string(term) []u8
#define BUILTIN_STRING  "LABEL string\n" \
                        "PUSHFRAME\n"    \
                        "DEFVAR LF@%retval\n" \
                        "MOVE LF@%retval LF@%0\n" \
                        "POPFRAME\n"     \
                        "RETURN"

/// pub fn ifj.length(s : []u8) i32
#define BUILTIN_LENGTH  "LABEL length\n" \
                        "PUSHFRAME\n"    \
                        "DEFVAR LF@%retval\n" \
                        "STRLEN LF@%retval LF@%0\n" \
                        "POPFRAME\n"     \
                        "RETURN"

/// pub fn ifj.concat(s1 : []u8, s2 : []u8) []u8
#define BUILTIN_CONCAT  "LABEL concat\n" \
                        "PUSHFRAME\n"    \
                        "DEFVAR LF@%retval\n" \
                        "CONCAT LF@%retval LF@%0 LF@%1\n" \
                        "POPFRAME\n"     \
                        "RETURN"

/// pub fn ifj.substring(s : []u8, i : i32, i : i32) ?[]u8
#define BUILTIN_SUBSTRING   "LABEL substring\n" \
                            "PUSHFRAME\n"       \
                            "DEFVAR LF@%retval\n" \
                            "DEFVAR LF@null_condition\n" \
                            "LT LF@null_condition LF@%1 int@0\n" \
                            "JUMPIFEQ substring$nullreturn LF@null_condition bool@true\n" \
                            "LT LF@null_condition LF@%2 int@0\n" \
                            "JUMPIFEQ substring$nullreturn LF@null_condition bool@true\n" \
                            "GT LF@null_condition LF@%1 LF@%2\n" \
                            "JUMPIFEQ substring$nullreturn LF@null_condition bool@true\n" \
                            "DEFVAR LF@arg_length\n"     \
                            "CREATEFRAME\n"     \
                            "DEFVAR TF@%0\n"    \
                            "MOVE TF@%0 LF@%1\n"\
                            "CALL length\n"     \
                            "MOVE LF@arg_length TF@%retval\n"    \
                            "LT LF@null_condition LF@%1 LF@arg_length\n"                  \
                            "JUMPIFNEQ substring$nullreturn LF@null_condition bool@true\n"\
                            "GT LF@null_condition LF@%2 LF@arg_length\n"                  \
                            "JUMPIFEQ substring$nullreturn LF@null_condition bool@true\n" \
                            "DEFVAR LF@curr_char\n"      \
                            "DEFVAR LF@curr_index\n"     \
                            "MOVE LF@curr_index LF@%1\n" \
                            "GETCHAR LF@%retval LF@%0 LF@curr_index\n"                    \
                            "ADD LF@curr_index LF@curr_index int@1\n"                     \
                            "DEFVAR LF@end_condition\n"  \
                            "LT LF@end_condition LF@curr_index LF@%2\n"                   \
                            "JUMPIFNEQ substring$return LF@end_condition bool@true\n"     \
                            "DEFVAR LF@loop_condition\n"                    \
                            "LABEL substring$loop\n"     \
                            "GETCHAR LF@curr_char LF@%0 LF@curr_index\n"                  \
                            "CONCAT LF@%retval LF@%retval LF@curr_char\n"                 \
                            "ADD LF@curr_index LF@curr_index int@1\n"                     \
                            "LT LF@loop_condition LF@curr_index LF@%2\n"                  \
                            "JUMPIFEQ substring$loop LF@loop_condition bool@true\n"       \
                            "JUMP substring$return\n"    \
                            "LABEL substring$nullreturn\n"       \
                            "MOVE LF@%retval nil@nil\n"  \
                            "LABEL substring$return\n"   \
                            "POPFRAME\n"        \
                            "RETURN"

/// pub fn ifj.strcmp(s1 : []u8, s2 : []u8) i32
#define BUILTIN_STRCMP  "LABEL strcmp\n" \
                        "PUSHFRAME\n"    \
                        "DEFVAR LF@%retval\n" \
                        "DEFVAR LF@arg1_length\n" \
                        "DEFVAR LF@arg2_length\n" \
                        "DEFVAR LF@length_condition\n" \
                        "CREATEFRAME\n"  \
                        "DEFVAR TF@%0\n" \
                        "MOVE TF@%0 LF@%0\n"  \
                        "CALL length\n"  \
                        "MOVE LF@arg1_length TF@%retval\n" \
                        "CREATEFRAME\n"  \
                        "DEFVAR TF@%0\n" \
                        "MOVE TF@%0 LF@%1\n"  \
                        "CALL length\n"  \
                        "MOVE LF@arg2_length TF@%retval\n" \
                        "LT LF@length_condition LF@arg1_length LF@arg2_length\n" \
                        "JUMPIFEQ strcmp$arg1ret LF@length_condition bool@true\n"\
                        "LT LF@length_condition LF@arg2_length LF@arg1_length\n" \
                        "JUMPIFEQ strcmp$arg2ret LF@length_condition bool@true\n"\
                        "DEFVAR LF@arg1_char\n"   \
                        "DEFVAR LF@arg2_char\n"   \
                        "DEFVAR LF@loop_index\n"  \
                        "MOVE LF@loop_index int@0\n" \
                        "DEFVAR LF@loop_condition\n"   \
                        "LABEL strcmp$loop\n" \
                        "STRI2INT LF@arg1_char LF@%0 LF@loop_index\n"            \
                        "STRI2INT LF@arg2_char LF@%1 LF@loop_index\n"            \
                        "JUMPIFNEQ strcmp$noteq LF@arg1_char LF@arg2_char\n"     \
                        "ADD LF@loop_index LF@loop_index int@1\n"                \
                        "LT LF@loop_condition LF@loop_index LF@arg1_length\n"    \
                        "JUMPIFEQ strcmp$loop LF@loop_condition bool@true\n"     \
                        "MOVE LF@%retval int@0\n" \
                        "JUMP strcmp$return\n"\
                        "LABEL strcmp$noteq\n"  \
                        "DEFVAR LF@arg1ltarg2\n"  \
                        "LT LF@arg1ltarg2 LF@arg1_char LF@arg2_char\n"             \
                        "JUMPIFEQ strcmp$arg1ret LF@arg1ltarg2 bool@true\n"      \
                        "JUMP strcmp$arg2ret\n"   \
                        "LABEL strcmp$arg1ret\n"  \
                        "MOVE LF@%retval int@-1\n"\
                        "JUMP strcmp$return\n"\
                        "LABEL strcmp$arg2ret\n"  \
                        "MOVE LF@%retval int@1\n" \
                        "LABEL strcmp$return\n" \
                        "POPFRAME\n"     \
                        "RETURN"

/// pub fn ifj.ord(s : []u8, i : i32) i32
#define BUILTIN_ORD "LABEL ord\n" \
                    "PUSHFRAME\n" \
                    "DEFVAR LF@%retval\n" \
                    "MOVE LF@%retval int@0\n" \
                    "DEFVAR LF@index_in_boundaries\n" \
                    "LT LF@index_in_boundaries LF@%1 int@0\n" \
                    "JUMPIFEQ ord$return LF@index_in_boundries bool@true\n" \
                    "CREATEFRAME\n"       \
                    "DEFVAR TF@%0\n"      \
                    "MOVE TF@%0 LF@%0\n"  \
                    "CALL length\n"       \
                    "DEFVAR LF@arg_length\n"  \
                    "MOVE LF@arg_length TF@%retval\n" \
                    "SUB LF@arg_length LF@arg_length int@1\n" \
                    "GT LF@index_in_boundaries LF@%1 LF@%arg_length\n"      \
                    "JUMPIFEQ ord$return LF@index_in_boundaries bool@true\n"\
                    "STRI2INT LF@%retval LF@%0 LF@%1\n"       \
                    "LABEL ord$return\n" \
                    "POPFRAME\n"  \
                    "RETURN"

/// pub fn ifj.chr(i : i32) []u8
#define BUILTIN_CHR "LABEL chr\n" \
                    "PUSHFRAME\n" \
                    "DEFVAR LF@%retval\n" \
                    "INT2CHAR LF@%retval LF@%0\n" \
                    "POPFRAME\n"  \
                    "RETURN"

dstring_t generatorOutput;

const char *currFnIdentifier = "";
unsigned ifCounter = 0;
unsigned whileCounter = 0;
unsigned typeCheckCounter = 0;

label_list_t ifLabelList;
label_list_t whileLabelList;

void generator_init(){
    dynamic_string_init(&generatorOutput);
    OUT(".IFJcode24");
    OUT("DEFVAR GF@%exp_result");
    OUT("DEFVAR GF@%tmp1");
    OUT("DEFVAR GF@%tmp2");
    OUT("DEFVAR GF@%type_check");
    OUT("JUMP $main");
    generate_builtin_functions();
    label_list_init(&ifLabelList);
    label_list_init(&whileLabelList);
}

void generate_builtin_functions(){
    OUT("# BUILTIN functions definitions");
    OUT(BUILTIN_READSTR);
    OUT(BUILTIN_READI32);
    OUT(BUILTIN_READF64);
    OUT(BUILTIN_WRITE);
    OUT(BUILTIN_I2F);
    OUT(BUILTIN_F2I);
    OUT(BUILTIN_STRING);
    OUT(BUILTIN_LENGTH);
    OUT(BUILTIN_CONCAT);
    OUT(BUILTIN_SUBSTRING);
    OUT(BUILTIN_STRCMP);
    OUT(BUILTIN_ORD);
    OUT(BUILTIN_CHR);
    OUT("# CODE section");
}

void generate_main_function_head(){
    OUT("# MAIN function definition");
    OUT("LABEL $main");
    OUT("CREATEFRAME");
    OUT("PUSHFRAME");
    currFnIdentifier = "main";
    ifCounter = 0;
    whileCounter = 0;
}

void generate_main_function_end(){
    OUT("POPFRAME");
    OUT("CLEARS");
    OUT("EXIT int@0");
    currFnIdentifier = "";
    label_list_dispose(&ifLabelList);
    label_list_dispose(&whileLabelList);
}

void generate_function_head(const char *identifier){
    OUT_CL("# ");
    OUT_CL(identifier);
    OUT(" definition");
    OUT_CL("LABEL $");
    OUT_CL(identifier);
    OUT(); //newline
    OUT("PUSHFRAME");
    currFnIdentifier = identifier;
    ifCounter = 0;
    whileCounter = 0;
}

void generate_function_end(const char *identifier){
    OUT_CL("LABEL $");
    OUT_CL(identifier);
    OUT_CL("$return");
    OUT(); //newline
    OUT("POPFRAME");
    OUT("RETURN");
    currFnIdentifier = "";
    label_list_dispose(&ifLabelList);
    label_list_dispose(&whileLabelList);
}

void generate_function_call_param_head(){
    OUT("CREATEFRAME");
}

void generate_function_call_param(unsigned index, Token *token){
    char indexStr[100];
    sprintf(indexStr, "%d", index);
    OUT_CL("DEFVAR TF@%");
    OUT_CL(indexStr);
    OUT(); //newline
    OUT_CL("MOVE TF@%");
    OUT_CL(indexStr);
    OUT_CL(" ");
    generate_term(token);
    OUT(); //newline
}

void generate_function_call(const char *identifier){
    OUT_CL("CALL $");
    OUT_CL(identifier);
    OUT(); //newline
}

void generate_builtin_function_call(const char *identifier){
    OUT_CL("CALL ");
    OUT_CL(identifier);
    OUT(); //newline
}

void generate_function_param(const char *identifier, unsigned index){
    char indexStr[100];
    sprintf(indexStr, "%d", index);
    OUT_CL("DEFVAR LF@");
    OUT_CL(identifier);
    OUT(); //newline
    OUT_CL("MOVE LF@");
    OUT_CL(identifier);
    OUT_CL(" LF@%");
    OUT_CL(indexStr);
    OUT(); //newline
}

void generate_function_return(const char *identifier){
    OUT_CL("JUMP $");
    OUT_CL(identifier);
    OUT("$return");
}

void generate_function_return_value(){
    OUT("DEFVAR LF@%retval");
}

void generate_function_return_value_assign(){
    OUT("MOVE LF@%retval GF@%exp_result");
}

void generate_function_return_value_push(){
    OUT("PUSHS TF@%retval");
}

void generate_variable_declaration(const char *identifier){
    OUT_CL("DEFVAR LF@");
    OUT_CL(identifier);
    OUT(); //newline
}

void generate_variable_definition(const char *identifier){
    OUT_CL("MOVE LF@");
    OUT_CL(identifier);
    OUT(" GF@%exp_result");
}

void generate_term(Token *token){
    int character;
    char tmpBuffer[4];
    switch (token->type) {
        case TOKEN_TP_FLOAT:
            OUT_CL("float@");
            char floatBuffer[100];
            sprintf(floatBuffer, "%a", token->typed_value.float_val);
            OUT_CL(floatBuffer);
            break;
        case TOKEN_TP_INT:
            OUT_CL("int@");
            OUT_CL(token->value);
            break;
        case TOKEN_KW_NULL:
            OUT_CL("nil@nil");
            break;
        case TOKEN_TP_STRING:
            OUT_CL("string@");
            dstring_t tmpString;
            dynamic_string_init(&tmpString);
            for (int index = 0; (character = (int) token->value[index]) != '\0'; index++){
                if (character <= 32 || character == 35 || character == 92){
                    dynamic_string_append(&tmpString,"\\");
                    sprintf(tmpBuffer, "%03d", character);
                    dynamic_string_append(&tmpString, tmpBuffer);
                }else{
                    dynamic_string_append_char(&tmpString, (char) character);
                }
            }
            OUT_CL(tmpString.string);
            dynamic_string_dispose(&tmpString);
            break;
        case TOKEN_ID:
            OUT_CL("LF@");
            OUT_CL(token->value);
            break;
        default:
            break;
    }
}

void generate_stack_push(Token *token){
    OUT_CL("PUSHS ");
    generate_term(token);
    OUT(); //newline
}

void generate_stack_operation(Token *token){
    generate_stack_operation_type_check();
    switch (token->type) {
        case TOKEN_OPE_ADD:
            OUT("ADDS");
            break;
        case TOKEN_OPE_SUB:
            OUT("SUBS");
            break;
        case TOKEN_OPE_MUL:
            OUT("MULS");
            break;
        case TOKEN_OPE_DIV:
            OUT("DIVS");
            break;
        case TOKEN_OPE_EQ:
            OUT("EQS");
            break;
        case TOKEN_OPE_NEQ:
            OUT("EQS");
            OUT("NOTS");
            break;
        case TOKEN_OPE_LT:
            OUT("LTS");
            break;
        case TOKEN_OPE_GT:
            OUT("GTS");
            break;
        case TOKEN_OPE_LTE:
            OUT("GTS");
            OUT("NOTS");
            break;
        case TOKEN_OPE_GTE:
            OUT("LTS");
            OUT("NOTS");
            break;
        default:
            break;
    }
}

void generate_stack_operation_type_check(){
    OUT("POPS GF@%tmp1\n" \
        "POPS GF@%tmp2\n" \
        "TYPE GF@%type_check GF@%tmp1"
        );
    char label[120] = "$$typeCheck$";
    char tmpBuffer[100];
    sprintf(tmpBuffer, "%d", typeCheckCounter);
    strcat(label,tmpBuffer);
    OUT_CL("JUMPIFEQ ");
    OUT_CL(label);
    OUT("$1 GF@%type_check string@float");
    OUT("TYPE GF@%type_check GF@%tmp2");
    OUT_CL("JUMPIFEQ ");
    OUT_CL(label);
    OUT("$2 GF@%type_check string@float");
    OUT_CL("JUMP ");
    OUT_CL(label);
    OUT("$end");
    OUT_CL("LABEL ");
    OUT_CL(label);
    OUT("$1");
    OUT("TYPE GF@%type_check GF@%tmp2");
    OUT_CL("JUMPIFEQ ");
    OUT_CL(label);
    OUT("$end GF@%type_check string@float");
    OUT("INT2FLOAT GF@%tmp2 GF@%tmp2");
    OUT_CL("JUMP ");
    OUT_CL(label);
    OUT("$end");
    OUT_CL("LABEL ");
    OUT_CL(label);
    OUT("$2");
    OUT("INT2FLOAT GF@%tmp1 GF@%tmp1");
    OUT_CL("LABEL ");
    OUT_CL(label);
    OUT("$end");
    OUT("PUSHS GF@%tmp2");
    OUT("PUSHS GF@%tmp1");
    typeCheckCounter++;
}

void generate_stack_operation_result(){
    OUT("POPS GF@%exp_result");
}

void generate_if_head(bool pipe){
    OUT_CL("LABEL ");
    generate_label(LABEL_IF);
    OUT(); //newline
    if (!pipe) {
        OUT_CL("JUMPIFNEQ ");
        get_label(LABEL_IF);
        OUT_CL("$else GF@%exp_result bool@true");
        OUT(); //newline
    } else{
        OUT_CL("JUMPIFEQ ");
        get_label(LABEL_IF);
        OUT_CL("$else GF@%exp_result nil@nil");
        OUT();
    }
}

void generate_if_end(){
    OUT_CL("JUMP ");
    get_label(LABEL_IF);
    OUT("$end");
}

void generate_if_pipe(const char *identifier){
    OUT_CL("DEFVAR LF@");
    OUT_CL(identifier);
    OUT(); //newline
    OUT_CL("MOVE LF@");
    OUT_CL(identifier);
    OUT(" GF@%exp_result");
}

void generate_else(){
    OUT_CL("LABEL ");
    get_label(LABEL_IF);
    OUT_CL("$else");
    OUT(); //newline
}

void generate_else_end(){
    OUT_CL("LABEL ");
    get_label_pop(LABEL_IF);
    OUT_CL("$end");
    OUT(); //newline
}

void generate_while_head(){
    OUT_CL("LABEL ");
    generate_label(LABEL_WHILE);
    whileCounter++;
    OUT(); //newline
}

void generate_while_condition(bool pipe){
    if (!pipe) {
        OUT_CL("JUMPIFNEQ ");
        get_label(LABEL_WHILE);
        OUT_CL("$end");
        OUT_CL(" GF@%exp_result bool@true");
        OUT(); //newline
    }else{
        OUT_CL("JUMPIFEQ ");
        get_label(LABEL_WHILE);
        OUT_CL("$end");
        OUT_CL(" GF@%exp_result nil@nil");
        OUT();
    }
}

void generate_while_end(){
    OUT_CL("JUMP ");
    get_label(LABEL_WHILE);
    OUT(); //newline
    OUT_CL("LABEL ");
    get_label_pop(LABEL_WHILE);
    OUT_CL("$end");
    OUT(); //newline
}

void generate_while_pipe(const char *identifier){
    OUT_CL("DEFVAR LF@");
    OUT_CL(identifier);
    OUT();
}

void generate_while_pipe_definition(const char *identifier){
    OUT_CL("MOVE LF@");
    OUT_CL(identifier);
    OUT(" GF@%exp_result");
}

void generate_label(label_t type) {
    dstring_t tmpLabelString;
    dynamic_string_init(&tmpLabelString);
    dynamic_string_append(&tmpLabelString,"$");
    dynamic_string_append(&tmpLabelString, currFnIdentifier);
    char tmpBuffer[100];
    switch (type) {
        case LABEL_WHILE:
            dynamic_string_append(&tmpLabelString,"$while$");
            sprintf(tmpBuffer, "%d", whileCounter++);
            break;
        case LABEL_IF:
            dynamic_string_append(&tmpLabelString, "$if$");
            sprintf(tmpBuffer, "%d", ifCounter++);
            break;
    }
    dynamic_string_append(&tmpLabelString, tmpBuffer);
    OUT_CL(tmpLabelString.string);
    switch (type) {
        case LABEL_WHILE:
            label_list_push(&whileLabelList, &tmpLabelString);
            break;
        case LABEL_IF:
            label_list_push(&ifLabelList, &tmpLabelString);
            break;
    }
    dynamic_string_dispose(&tmpLabelString);
}

void get_label(label_t type){
    dstring_t *stringPtr;
    switch (type) {
        case LABEL_WHILE:
            stringPtr = label_list_top(&whileLabelList);
            break;
        case LABEL_IF:
            stringPtr = label_list_top(&ifLabelList);
            break;
    }
    if (stringPtr == NULL){
        OUT_CL("$labelRetrieveFail");
    }else{
        OUT_CL(stringPtr->string);
    }
}

void get_label_pop(label_t type){
    dstring_t *stringPtr;
    switch (type) {
        case LABEL_WHILE:
            stringPtr = label_list_top(&whileLabelList);
            break;
        case LABEL_IF:
            stringPtr = label_list_top(&ifLabelList);
            break;
    }
    if (stringPtr == NULL){
        OUT_CL("$labelRetrieveAndPopFail");
    }else{
        OUT_CL(stringPtr->string);
        switch (type) {
            case LABEL_WHILE:
                label_list_pop(&whileLabelList);
                break;
            case LABEL_IF:
                label_list_pop(&ifLabelList);
                break;
        }
    }
}

void generator_flush(){
    fputs(generatorOutput.string, stdout);
    generator_dispose();
}

void generator_dispose(){
    dynamic_string_dispose(&generatorOutput);
    label_list_dispose(&ifLabelList);
    label_list_dispose(&whileLabelList);
}