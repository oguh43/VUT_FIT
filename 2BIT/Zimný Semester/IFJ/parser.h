/**
 * FIT VUT - IFJ project 2024
 *
 * @file parser.h
 *
 * @brief Parser implementation for IFJ24
 *
 * @author Štefan Dubnička (xdubnis00)
 * @author Hugo Bohácsek (xbohach00)
 */

#ifndef PARSER_H
#define PARSER_H

#include "scanner.h"
#include "error.h"

typedef enum {
    S, // < SHIFT - Lower precedence - push to stack
    Q, // = EQUAL - Equal precedence - go to next token
    R, // > REDUCE - Higher precedence - reduce
    E // ERROR - Error
} PrecedenceAction;

enum {
    ADD,
    SUB,
    MUL,
    DIV,
    LPAREN,
    RPAREN,
    ID,
    DOLLAR,
    EPS,
    EQUAL,
    NEQUAL,
    LESS,
    GREATER,
    LTE,
    GTE
};

typedef struct PrecedenceStackNode {
    Token token;
    struct PrecedenceStackNode *next;
} StackNode;

typedef struct Stack {
    StackNode *top;
} Stack;


char* concat_char(const char* str1, const char* str2);

void push(Stack *stack, Token token);

void reorder_stack(Stack *stack);

Stack* revert_stack(Stack* dest);

void init_stack(Stack *stack);

void pop(Stack *stack);

StackNode * top(Stack *stack);

void dispose_stack(Stack *stack);

PrecedenceAction get_precedence_action(Token stack_token, Token new_token);

Token get_next_token_from_stack(Stack *stack);

Token precedent_apply_rule(Stack *stack, Token next_token);

StackNode * get_last_operator(Stack *stack);

STData precedent_parser_parse();

void parser_init();

void parser_parse(FILE *source);

void parser_cleanup();

#endif