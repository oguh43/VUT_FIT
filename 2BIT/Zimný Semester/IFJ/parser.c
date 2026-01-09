/**
 * FIT VUT - IFJ project 2024
 *
 * @file parser.c
 *
 * @brief Parser implementation for IFJ24
 *
 * @author Štefan Dubnička (xdubnis00)
 * @author Hugo Bohácsek (xbohach00)
 */
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>

#include "parser.h"
#include "scanner.h"
#include "ptr_registry.h"
#include "symtable.h"
#include "generator.h"


#ifdef DEBUG
    #define DEBUG_PRINT(...) fprintf(stderr, __VA_ARGS__)
#else
    #define DEBUG_PRINT(...)
#endif

#define TABLE_SIZE 15

Stack input_stack;
Stack backup_stack;

//semamntic

char* sem_4_func_id;
bool sem_4_is_assigned = false;
int sem_4_arg_cnt = 0;
void inc_in_all_contexts(SymNode *parent_func, char* id);
void sem_check_is_redefine_or_undefine_question_mark(SymNode *find_me, SymNode *context,  bool defining);
bool sem_6_has_return = false;
char* sem_6_func_id;

Token curr_token;
FILE *input;

Stack PrecedenceInputStack;

SymNode * GlobalSymTable;
SymNode *CurrentContext;

// Check vars
bool incoming_while_pipe = true;
Token incoming_while_pipe_var;

bool has_main = false;

bool has_nullable = false;
bool has_slice = false;

const char *types[] = {"TOKEN_KW_CONST", "TOKEN_KW_ELSE", "TOKEN_KW_FN", "TOKEN_KW_IF", "TOKEN_KW_I32", "TOKEN_KW_F64", "TOKEN_KW_NULL", "TOKEN_KW_PUB", "TOKEN_KW_RETURN", "TOKEN_KW_U8", "TOKEN_KW_VAR", "TOKEN_KW_VOID", "TOKEN_KW_WHILE", "TOKEN_KW_FOR", "TOKEN_KW_IMPORT", "TOKEN_ID", "TOKEN_TP_INT", "TOKEN_TP_FLOAT", "TOKEN_TP_STRING", "TOKEN_OPE_ADD", "TOKEN_OPE_SUB", "TOKEN_OPE_MUL", "TOKEN_OPE_DIV", "TOKEN_OPE_LPAREN", "TOKEN_OPE_RPAREN", "TOKEN_OPE_LBRACE", "TOKEN_OPE_RBRACE", "TOKEN_OPE_LBRACKET", "TOKEN_OPE_RBRACKET", "TOKEN_OPE_SEMICOLON", "TOKEN_OPE_AT", "TOKEN_OPE_COLON", "TOKEN_OPE_DOT", "TOKEN_OPE_COMMA", "TOKEN_NULLABLE", "TOKEN_PIPE", "TOKEN_OPE_ASSIGN", "TOKEN_OPE_EQ", "TOKEN_OPE_NEQ", "TOKEN_OPE_LT", "TOKEN_OPE_GT", "TOKEN_OPE_LTE", "TOKEN_OPE_GTE", "TOKEN_EOF", "TOKEN_ERROR", "TOKEN_BUILTIN"};

int precedence_table[TABLE_SIZE][TABLE_SIZE] = {
  // New      // +    -    *    /    (    )   id    $   EPS    EQUAL   NEQUAL   LESS   GREATER   LTE   GTE
/* Stack   + */ {R,   R,   S,   S,   S,   R,   S,   R,  S,     R,      R,      R,      R,      R,    R},
        /* - */ {R,   R,   S,   S,   S,   R,   S,   R,  S,     R,      R,      R,      R,      R,    R},
        /* * */ {R,   R,   R,   R,   S,   R,   S,   R,  S,     R,      R,      R,      R,      R,    R},
        /* / */ {R,   R,   R,   R,   S,   R,   S,   R,  S,     R,      R,      R,      R,      R,    R},
        /* ( */ {S,   S,   S,   S,   S,   R,   S,   E,  S,     S,      S,      S,      S,      S,    S},
        /* ) */ {R,   R,   R,   R,   E,   R,   E,   R,  R,     S,      S,      S,      S,      S,    S},
       /* id */ {R,   R,   R,   R,   E,   R,   E,   R,  E,     R,      R,      R,      R,      R,    R},
        /* $ */ {S,   S,   S,   S,   S,   E,   S,   E,  E,     R,      R,      R,      R,      R,    R},
      /* EPS */ {S,   S,   S,   S,   E,   R,   E,   R,  E,     S,      S,      S,      S,      S,    S},
    /* EQUAL */ {S,   S,   S,   S,   S,   R,   S,   R,  R,     E,      E,      E,      E,      E,    E},
   /* NEQUAL */ {S,   S,   S,   S,   S,   R,   S,   R,  R,     E,      E,      E,      E,      E,    E},
     /* LESS */ {S,   S,   S,   S,   S,   R,   S,   R,  R,     E,      E,      E,      E,      E,    E},
  /* GREATER */ {S,   S,   S,   S,   S,   R,   S,   R,  R,     E,      E,      E,      E,      E,    E},
      /* LTE */ {S,   S,   S,   S,   S,   R,   S,   R,  R,     E,      E,      E,      E,      E,    E},
      /* GTE */ {S,   S,   S,   S,   S,   R,   S,   R,  R,     E,      E,      E,      E,      E,    E}
};



char* concat_char(const char* str1, const char* str2){
    // TODO: Free str1 and str2?????
    char* result = (char*)malloc(strlen(str1) + strlen(str2) + 1); // +1 for the null-terminator \0
    if (result == NULL){
        fprintf(stderr, "[ERROR] Memory allocation failed\n");
        error(INTERNAL_ERROR);
    }
    ptr_registry_add(result);
    strcpy(result, str1);
    strcat(result, str2);
    return result;
}


/** 
 * Pushes token to the stack
 * @param stack Pointer to the stack
 * @param token Token to be pushed
**/
void push(Stack *stack, Token token){
    StackNode *new_node = (StackNode *)malloc(sizeof(StackNode));
    if (new_node == NULL){
        fprintf(stderr, "[ERROR] Memory allocation failed\n");
        error(INTERNAL_ERROR);
    }
    //ptr_registry_add(new_node);
    new_node->token = token;
    new_node->next = stack->top;
    stack->top = new_node;
    /*
    fprintf(stderr, "PUSHING: %s\n", token.value);
    StackNode *tmp = stack->top;
    int i=0;
    while(tmp != NULL){
        fprintf(stderr, "STACK %d: %s\n",i++, tmp->token.value);
        tmp = tmp->next;
    }
    */
}
void reorder_stack(Stack *stack);
/**
 * Reverts the stack from the backup stack
 * @param dest Pointer to the destination stack
 * @return Pointer to the destination stack
 */
Stack* revert_stack(Stack* dest){
    StackNode *tmp = backup_stack.top;
    while (tmp != NULL){
        push(dest, tmp->token);
        tmp = tmp->next;
    }
    reorder_stack(dest);
    return dest;
}

void init_stack(Stack *stack){
    stack->top = NULL;
}


/**
 * Pops the top element from the stack
 * @param stack Pointer to the stack
 * @return void
 */
void pop(Stack *stack){
    if (stack->top == NULL){
        fprintf(stderr, "[ERROR] Stack is empty\n");
        error(INTERNAL_ERROR);
    }
    StackNode *tmp = stack->top;
    stack->top = stack->top->next;
    free(tmp);
}

/**
 * Returns the top element from the stack
 * @param stack Pointer to the stack
 * @return Pointer to the top element
 */
StackNode * top(Stack *stack){
    if (stack->top == NULL){
        fprintf(stderr, "[ERROR] Stack is empty\n");
        return NULL;
        
        error(INTERNAL_ERROR);
    }
    return stack->top;
}

/**
 * Disposes the stack
 * @param stack Pointer to the stack
 * @return void
 */
void dispose_stack(Stack *stack){
    StackNode *tmp;
    while (stack->top != NULL){
        tmp = stack->top;
        stack->top = stack->top->next;
        ptr_registry_remove(tmp);
    }
    init_stack(stack);
}

/**
 * Returns the precedence action from the precedence table
 * @param stack_token Token from the stack
 * @param new_token Token from the input
 * @return PrecedenceAction
 */
PrecedenceAction get_precedence_action(Token stack_token, Token new_token){
    DEBUG_PRINT("Precedence ------------ stack_token: %s\t new_token: %s\n", types[stack_token.type], types[new_token.type]);
    int stack_index = -1;
    int new_index = -1;
    switch (stack_token.type){
        case TOKEN_OPE_ADD:
            stack_index = ADD;
            break;
        case TOKEN_OPE_SUB:
            stack_index = SUB;
            break;
        case TOKEN_OPE_MUL:
            stack_index = MUL;
            break;
        case TOKEN_OPE_DIV:
            stack_index = DIV;
            break;
        case TOKEN_OPE_LPAREN:
            stack_index = LPAREN;
            break;
        case TOKEN_OPE_RPAREN:
            stack_index = RPAREN;
            break;
        case TOKEN_ID:
        case TOKEN_KW_FN:
            if (stack_token.is_e){
                stack_index = EPS;
            }
            else {
                stack_index = ID;
            }
            break;
        case TOKEN_TP_INT:
        case TOKEN_TP_FLOAT:
        case TOKEN_KW_NULL:
        case TOKEN_KW_VOID:
            if (stack_token.is_e){
                stack_index = EPS;
            }
            else {
                stack_index = ID;
            }
            break;
        case TOKEN_EOF:
            stack_index = DOLLAR;
            break;
        case TOKEN_OPE_EQ:
            stack_index = EQUAL;
            break;
        case TOKEN_OPE_NEQ:
            stack_index = NEQUAL;
            break;
        case TOKEN_OPE_LT:
            stack_index = LESS;
            break;
        case TOKEN_OPE_GT:
            stack_index = GREATER;
            break;
        case TOKEN_OPE_LTE:
            stack_index = LTE;
            break;
        case TOKEN_OPE_GTE:
            stack_index = GTE;
            break;
        default:
            fprintf(stderr, "[ERROR] Invalid token in stack\n");
            error(7); // ???
    }
    switch (new_token.type){
        case TOKEN_OPE_ADD:
            new_index = ADD;
            break;
        case TOKEN_OPE_SUB:
            new_index = SUB;
            break;
        case TOKEN_OPE_MUL:
            new_index = MUL;
            break;
        case TOKEN_OPE_DIV:
            new_index = DIV;
            break;
        case TOKEN_OPE_LPAREN:
            new_index = LPAREN;
            break;
        case TOKEN_OPE_RPAREN:
            new_index = RPAREN;
            break;
        case TOKEN_ID:
        case TOKEN_KW_FN:
            new_index = ID;
            break;
        case TOKEN_TP_INT:
        case TOKEN_TP_FLOAT:
        case TOKEN_KW_NULL:
        case TOKEN_KW_VOID:
            new_index = ID;
            break;
        case TOKEN_EOF:
            new_index = DOLLAR;
            break;
        case TOKEN_OPE_EQ:
            new_index = EQUAL;
            break;
        case TOKEN_OPE_NEQ:
            new_index = NEQUAL;
            break;
        case TOKEN_OPE_LT:
            new_index = LESS;
            break;
        case TOKEN_OPE_GT:
            new_index = GREATER;
            break;
        case TOKEN_OPE_LTE:
            new_index = LTE;
            break;
        case TOKEN_OPE_GTE:
            new_index = GTE;
            break;
        default:
            fprintf(stderr, "[ERROR] Invalid token in stack\n");
            error(7); // ???
    }
    return precedence_table[stack_index][new_index];
}

/**
 * Orders the stack in reverse order
 * @param stack Pointer to the stack
 * @return void
 */
void reorder_stack(Stack *stack){
    StackNode * prev = NULL;
    StackNode * current = stack->top;
    StackNode * next = NULL;

    while (current != NULL) {
        next = current->next;  // Save the next node
        current->next = prev;  // Reverse the link
        prev = current;        // Move prev forward
        current = next;        // Move current forward
    }

    stack->top = prev;
}

/**
 * Returns the next token from the stack
 * @param stack Pointer to the stack
 * @return Token
 */
Token get_next_token_from_stack(Stack *stack){
    if (stack->top == NULL){
        Token token;
        token.type = TOKEN_EOF;
        token.value = "EOF";
        return token;
    }
    Token token = stack->top->token;
    //DEBUG_PRINT("-----------\nNext token from stack: %s\n-----------\n", token.value);
    pop(stack);
    return token;
}

/**
 * Applies the rule from the precedence table
 * @param stack Pointer to the stack
 * @param next_token Token from the input
 * @return Token
 */
Token precedent_apply_rule(Stack *stack, Token next_token){
    static char * datatypes[] = {"UNDEFINED", "I32", "F64", "U8", "SLITERAL", "FVOID"};
    // Reducing Grammar
    // E -> id
    // E -> E + E
    // E -> E - E
    // E -> E * E
    // E -> E / E
    // E -> E == E
    // E -> E != E
    // E -> E < E
    // E -> E > E
    // E -> E <= E
    // E -> E >= E
    // E -> (E)
    DEBUG_PRINT("\nAPPLYING RULES\n");
    Token new_token;
    
    if (((top(stack)->token.type == TOKEN_ID) ||  (top(stack)->token.type == TOKEN_KW_FN) || (top(stack)->token.type == TOKEN_TP_INT) || (top(stack)->token.type == TOKEN_TP_FLOAT) || (top(stack)->token.type == TOKEN_KW_NULL)) && !(top(stack)->token.is_e)) {
        DEBUG_PRINT("RULE 1:  E -> id\n");
        new_token = top(stack)->token;
        new_token.is_e = true;

        if (top(stack)->token.type != TOKEN_KW_FN) {
            generate_stack_push(&top(stack)->token);
        }

        pop(stack);
        DEBUG_PRINT("NEW TOKEN VALUE: %s | TokenType: %s\n | DataType: %d\n", new_token.value, types[new_token.type], new_token.data.DataType);
        push(stack, new_token);
    }
    else {
        Token rule_token0;
        Token rule_token1;
        Token rule_token2;
        rule_token1 = top(stack)->next->token;
        switch (rule_token1.type){
            case TOKEN_OPE_ADD:
            case TOKEN_OPE_SUB:
            case TOKEN_OPE_MUL:
                rule_token0 = top(stack)->token;
                rule_token2 = top(stack)->next->next->token;

                //DEBUG_PRINT("RULE TOKEN0: %s | TokenType: %s\n | DataType: %s\n", rule_token0.value, types[rule_token0.type], datatypes[rule_token0.data.DataType]);
                //DEBUG_PRINT("RULE TOKEN1: %s | TokenType: %s\n | DataType: %s\n", rule_token1.value, types[rule_token1.type], datatypes[rule_token1.data.DataType]);
                //DEBUG_PRINT("RULE TOKEN2: %s | TokenType: %s\n | DataType: %s\n", rule_token2.value, types[rule_token2.type], datatypes[rule_token2.data.DataType]); 

                DEBUG_PRINT("RULES 2-4:  E -> E {+-*} E\n");

                if (rule_token0.data.DataType == TYPE_UNDEFINED || rule_token2.data.DataType == TYPE_UNDEFINED){
                    fprintf(stderr, "[SEMANTIC ERROR] Result of comparator in arithmetic operation\n");
                    error(7);
                }

                if (rule_token0.data.nullable || rule_token2.data.nullable){
                    if ((rule_token0.type != TOKEN_TP_INT && rule_token0.type != TOKEN_TP_FLOAT) && (rule_token2.type != TOKEN_TP_INT && rule_token2.type != TOKEN_TP_FLOAT)){
                        fprintf(stderr, "[SEMANTIC ERROR] Nullable type in arithmetic operation\n");
                        error(7);
                    }
                }

                if (rule_token0.data.slice || rule_token2.data.slice){
                    fprintf(stderr, "[SEMANTIC ERROR] Slice type in arithmetic operation\n");
                    error(7);
                }

                switch (rule_token0.data.DataType) {
                    case TYPE_I32:
                        if (rule_token2.data.DataType == TYPE_I32){
                            DEBUG_PRINT("I FELL HERE\n\n");
                            pop(stack);
                            pop(stack);
                            pop(stack);

                            new_token = rule_token0;
                            new_token.is_e = true;
                            push(stack, new_token);
                        }
                        else if (rule_token2.data.DataType == TYPE_F64){
                            if (rule_token0.type == TOKEN_TP_INT) {
                                pop(stack);
                                pop(stack);
                                pop(stack);

                                new_token = rule_token2;
                                new_token.is_e = true;
                                push(stack, new_token);
                            }
                            else {
                                fprintf(stderr, "[SEMANTIC ERROR] Implicit type conversion from FUNCALL|VAR : I32 to F64\n");
                                error(7);
                            }
                        }
                        else {
                            fprintf(stderr, "[SEMANTIC ERROR] Incompatible types in arithmetic operation\n");
                            error(7);
                        }
                        break;
                    case TYPE_F64:
                        if (rule_token2.data.DataType == TYPE_F64){
                            pop(stack);
                            pop(stack);
                            pop(stack);

                            new_token = rule_token0;
                            new_token.is_e = true;
                            push(stack, new_token);
                        }
                        else if (rule_token2.data.DataType == TYPE_I32){
                            if (rule_token0.type == TOKEN_TP_FLOAT && (rule_token0.typed_value.float_val == (int)rule_token0.typed_value.float_val)) {
                                pop(stack);
                                pop(stack);
                                pop(stack);

                                new_token = rule_token2;
                                new_token.is_e = true;
                                push(stack, new_token);
                            }
                            else {
                                fprintf(stderr, "[SEMANTIC ERROR] Implicit type conversion from F64 to I32\n");
                                error(7);
                            }
                        }
                        else {
                            fprintf(stderr, "[SEMANTIC ERROR] Incompatible types in arithmetic operation\n");
                            error(7);
                        }
                        break;
                    default:
                        fprintf(stderr, "[SEMANTIC ERROR] Incompatible types in arithmetic operation\n");
                        error(7);
                }

                generate_stack_operation(&rule_token1);

                break;
            case TOKEN_OPE_EQ:
            case TOKEN_OPE_NEQ:
                DEBUG_PRINT("RULES 6-7:  E -> E {==,!=} E\n");
                rule_token0 = top(stack)->token;
                rule_token2 = top(stack)->next->next->token;

                if (rule_token0.data.DataType == TYPE_UNDEFINED || rule_token2.data.DataType == TYPE_UNDEFINED){
                    fprintf(stderr, "[SEMANTIC ERROR] Result of comparator used in expression\n");
                    error(7);
                }

                if (rule_token0.data.slice || rule_token2.data.slice){
                    fprintf(stderr, "[SEMANTIC ERROR] Slice type in comparator\n");
                    error(7);
                }

                if (rule_token0.data.DataType == TYPE_FVOID){
                    if (!rule_token2.data.nullable) {
                        fprintf(stderr, "[SEMANTIC ERROR] Non nullable type in comparison with 'null'\n");
                        error(7);
                    }
                    else {
                        pop(stack);
                        pop(stack);
                        pop(stack);

                        new_token.type = TOKEN_KW_VOID;
                        new_token.value = "True|False";
                        new_token.data.DataType = TYPE_UNDEFINED;
                        new_token.data.nullable = false;
                        new_token.data.slice = false;
                        new_token.is_e = true;
                        push(stack, new_token);
                    }
                }
                else if (rule_token2.data.DataType == TYPE_FVOID){
                    if (!rule_token0.data.nullable) {
                        fprintf(stderr, "[SEMANTIC ERROR] Non nullable type in comparison with 'null'\n");
                        error(7);
                    }
                    else {
                        pop(stack);
                        pop(stack);
                        pop(stack);

                        new_token.type = TOKEN_KW_VOID;
                        new_token.value = "True|False";
                        new_token.data.DataType = TYPE_UNDEFINED;
                        new_token.data.nullable = false;
                        new_token.data.slice = false;
                        new_token.is_e = true;
                        push(stack, new_token);
                    }
                }
                else {
                    if (rule_token0.data.DataType == rule_token2.data.DataType){
                        pop(stack);
                        pop(stack);
                        pop(stack);

                        new_token.type = TOKEN_KW_VOID;
                        new_token.value = "True|False";
                        new_token.data.DataType = TYPE_UNDEFINED;
                        new_token.data.nullable = false;
                        new_token.data.slice = false;
                        new_token.is_e = true;
                        push(stack, new_token);
                    }
                    else {
                        fprintf(stderr, "[SEMANTIC ERROR] Incompatible types in comparison\n");
                        error(7);
                    }
                }
                generate_stack_operation(&rule_token1);

                break;
            case TOKEN_OPE_DIV:
                rule_token0 = top(stack)->token;
                rule_token2 = top(stack)->next->next->token;
                if (rule_token0.data.DataType == TYPE_UNDEFINED || rule_token2.data.DataType == TYPE_UNDEFINED){
                    fprintf(stderr, "[SEMANTIC ERROR] Result of comparator in arithmetic operation\n");
                    error(7);
                }
                if (rule_token0.data.DataType == TYPE_F64 && rule_token2.data.DataType == TYPE_F64){
                    pop(stack);
                    pop(stack);
                    pop(stack);

                    new_token = rule_token0;
                    new_token.is_e = true;
                    push(stack, new_token);
                }
                else if (rule_token0.data.DataType == TYPE_I32 && rule_token2.data.DataType == TYPE_I32){
                    pop(stack);
                    pop(stack);
                    pop(stack);

                    new_token = rule_token0;
                    new_token.is_e = true;
                    push(stack, new_token);
                }
                else if (rule_token0.data.DataType == TYPE_F64 && rule_token2.data.DataType == TYPE_I32){
                    if (rule_token0.type == TOKEN_TP_FLOAT && (rule_token0.typed_value.float_val == (int)rule_token0.typed_value.float_val)) {
                        pop(stack);
                        pop(stack);
                        pop(stack);

                        new_token = rule_token0;
                        new_token.is_e = true;
                        push(stack, new_token);
                    }
                    else {
                        fprintf(stderr, "[SEMANTIC ERROR] Implicit type conversion from F64 to I32\n");
                        error(7);
                    }
                }
                else if (rule_token0.data.DataType == TYPE_I32 && rule_token2.data.DataType == TYPE_F64){
                    if (rule_token2.type == TOKEN_TP_FLOAT && (rule_token2.typed_value.float_val == (int)rule_token2.typed_value.float_val)) {
                        pop(stack);
                        pop(stack);
                        pop(stack);

                        new_token = rule_token2;
                        new_token.is_e = true;
                        push(stack, new_token);
                    }
                    else {
                        fprintf(stderr, "[SEMANTIC ERROR] Implicit type conversion from I32 to F64\n");
                        error(7);
                    }
                }
                else {
                    fprintf(stderr, "[SEMANTIC ERROR] Incompatible types in arithmetic operation\n");
                    error(7);
                }
                generate_stack_operation(&rule_token1);
                break;

            case TOKEN_OPE_LT:
            case TOKEN_OPE_GT:
            case TOKEN_OPE_LTE:
            case TOKEN_OPE_GTE:
                DEBUG_PRINT("RULES 8-11:  E -> E {operator} E\n");
                rule_token0 = top(stack)->token;
                rule_token2 = top(stack)->next->next->token;


                if (rule_token0.data.nullable || rule_token2.data.nullable){
                    fprintf(stderr, "[SEMANTIC ERROR] Nullable type in arithmetic operation\n");
                    error(7);
                }

                if (rule_token0.data.DataType == TYPE_UNDEFINED || rule_token2.data.DataType == TYPE_UNDEFINED){
                    fprintf(stderr, "[SEMANTIC ERROR] Result of comparator used in expression\n");
                    error(7);
                }

                if (rule_token0.data.slice || rule_token2.data.slice){
                    fprintf(stderr, "[SEMANTIC ERROR] Slice type in comparator\n");
                    error(7);
                }

                if (rule_token2.data.DataType == TYPE_F64 && rule_token0.data.DataType == TYPE_I32){
                    pop(stack);
                    pop(stack);
                    pop(stack);

                    new_token.type = TOKEN_KW_VOID;
                    new_token.value = "True|False";
                    new_token.data.DataType = TYPE_UNDEFINED;
                    new_token.data.nullable = false;
                    new_token.data.slice = false;
                    new_token.is_e = true;
                    push(stack, new_token);
                }
                else if (rule_token2.data.DataType == TYPE_I32 && rule_token0.data.DataType == TYPE_F64 && rule_token2.type == TOKEN_TP_INT){
                    pop(stack);
                    pop(stack);
                    pop(stack);

                    new_token.type = TOKEN_KW_VOID;
                    new_token.value = "True|False";
                    new_token.data.DataType = TYPE_UNDEFINED;
                    new_token.data.nullable = false;
                    new_token.data.slice = false;
                    new_token.is_e = true;
                    push(stack, new_token);
                }
                else if (rule_token0.data.DataType == rule_token2.data.DataType){
                    pop(stack);
                    pop(stack);
                    pop(stack);

                    new_token.type = TOKEN_KW_VOID;
                    new_token.value = "True|False";
                    new_token.data.DataType = TYPE_UNDEFINED;
                    new_token.data.nullable = false;
                    new_token.data.slice = false;
                    new_token.is_e = true;
                    push(stack, new_token);
                }
                else {
                    fprintf(stderr, "[SEMANTIC ERROR] Incompatible types in comparison\n");
                    error(7);
                } 

                generate_stack_operation(&rule_token1);

                break;
            case TOKEN_OPE_LPAREN:
                if (next_token.type == TOKEN_OPE_RPAREN){
                    push(stack, next_token);
                    next_token = get_next_token_from_stack(&PrecedenceInputStack);
                    DEBUG_PRINT("RULE 12:  E -> (E)\n");
                    pop(stack);
                    new_token = top(stack)->token;
                    new_token.is_e = true;
                    pop(stack);
                    pop(stack);
                    push(stack, new_token);
                }
                else {
                    break;
                }
            case TOKEN_EOF:
                DEBUG_PRINT("SEM SPADNEM :(\n");
                break;
            default:
                DEBUG_PRINT("-------------- rule_token1: %s\n", types[rule_token1.type]);
                fprintf(stderr, "[SEMANTIC ERROR] Invalid expression\n");
                error(7);
        }

    }

    return next_token;
}

/**
 * Get the last operator from the stack
 * @param stack Pointer to the stack
 * @return Pointer to the last operator
 */
StackNode * get_last_operator(Stack *stack){
    StackNode *top = stack->top;
    StackNode *tmp = stack->top;
    while (tmp->next != NULL){
        switch (tmp->token.type){
            case TOKEN_OPE_ADD:
            case TOKEN_OPE_SUB:
            case TOKEN_OPE_MUL:
            case TOKEN_OPE_DIV:
            case TOKEN_OPE_LPAREN:
            case TOKEN_OPE_EQ:
            case TOKEN_OPE_NEQ:
            case TOKEN_OPE_LT:
            case TOKEN_OPE_GT:
            case TOKEN_OPE_LTE:
            case TOKEN_OPE_GTE:
                return tmp;
            default:
                break;
        }
        tmp = tmp->next;
    }
    return top;
}

/**
 * Parses the input stack using the precedent parser
 * @return STData
 * @see STData
 * @see PrecedenceAction
 * @see get_precedence_action
 */
STData precedent_parser_parse(){
    if (PrecedenceInputStack.top == NULL){
        dispose_stack(&PrecedenceInputStack);
        STData ret = {TYPE_UNDEFINED, false, false};
        return ret;
    }

    static char * datatypes[] = {"UNDEFINED", "I32", "F64", "U8", "SLITERAL", "FVOID"};

    DEBUG_PRINT("\n");
    DEBUG_PRINT("---Precedent parser---\nInput Stack: | ");
    for (StackNode *tmp = PrecedenceInputStack.top; tmp != NULL; tmp = tmp->next){
        //DEBUG_PRINT("%s  -  value: ", types[tmp->token.type]);
        DEBUG_PRINT("%s | ", tmp->token.value);
    }
    DEBUG_PRINT("\n\n");
    //Prepare precedence stack - reverse order
    /*
    for (StackNode *tmp = PrecedenceInputStack.top; tmp != NULL; tmp = tmp->next){
        DEBUG_PRINT("stack: %s  -  value: %s\n", types[tmp->token.type], tmp->token.value);
    }
    DEBUG_PRINT("\n\n");
    */
    reorder_stack(&PrecedenceInputStack);
    /*
    for (StackNode *tmp = PrecedenceInputStack.top; tmp != NULL; tmp = tmp->next){
        DEBUG_PRINT("stack: %s  -  value: %s\n", types[tmp->token.type], tmp->token.value);
    }
    */

    Stack stack;
    init_stack(&stack);

    STData expr_datatype = {TYPE_UNDEFINED, false, false};
    
    Token new_token;
    Token stack_token;
    Token dollar_token;
    dollar_token.type = TOKEN_EOF;
    dollar_token.value = "EOF";
    push(&stack, dollar_token);
    new_token = get_next_token_from_stack(&PrecedenceInputStack);
    PrecedenceAction action;
    while (1){
        stack_token = top(&stack)->token;
        
        if (get_last_operator(&stack) == stack.top){
            //DEBUG_PRINT("THERE IS NO OPERATOR ----- stack_token: %s\t new_token: %s\n", types[stack_token.type], types[new_token.type]);
            action = get_precedence_action(stack_token, new_token);
        }
        else {
            action = get_precedence_action(get_last_operator(&stack)->token, new_token);
        }
        //DEBUG_PRINT("action: %d\n", action);
        //DEBUG_PRINT("stack_token: %s\t new_token: %s\n", types[stack_token.type], types[new_token.type]);
        switch (action){
            case S:
                push(&stack, new_token);
                new_token = get_next_token_from_stack(&PrecedenceInputStack);
                break;
            case Q:
                push(&stack, new_token);
                new_token = get_next_token_from_stack(&PrecedenceInputStack);
                break;
            case R:
                if ((stack.top->token.is_e) && stack.top->next->token.type == TOKEN_EOF && new_token.type == TOKEN_EOF){
                    expr_datatype = stack.top->token.data;
                    DEBUG_PRINT("\n-----\n$ == $\nExpr DataType: %s | nullable: %d | slice: %d \n-----\n", datatypes[expr_datatype.DataType], expr_datatype.nullable, expr_datatype.slice);
                    pop(&stack);
                    break;
                }
                new_token = precedent_apply_rule(&stack, new_token);
                break;
            case E:
                fprintf(stderr, "[SYNTAX ERROR] Invalid token in expression\n");
                error(2);
        }
        if (stack.top->token.type == TOKEN_EOF && new_token.type == TOKEN_EOF){
            break;
        }
        DEBUG_PRINT("Stack: | ");
        for (StackNode *tmp = stack.top; tmp != NULL; tmp = tmp->next){
            //DEBUG_PRINT("%s  -  value: ", types[tmp->token.type]); 
            if (tmp->token.is_e){
                DEBUG_PRINT("E | ");
            }
            else {
                DEBUG_PRINT("%s | ", tmp->token.value);
            }
        }
        DEBUG_PRINT("\n");
    }
    DEBUG_PRINT("VALID\n\n");

    //Generate the value of the expression result
    generate_stack_operation_result();

    return expr_datatype;
}

/**
 * Initializes the parser
 * @return void
 */
void parser_init(){
    ptr_registry_init();
}

/**
 * Cleans up after parser
 * @return void
 */
void parser_cleanup(){
    ptr_registry_cleanup();
}

/**
 * Gets the next token from the input stack
 * @return void
 */
void next(){
    //DEBUG_PRINT("curr_token value: '%s' | type: %s  ->  ", curr_token.value, types[curr_token.type]);
    
    /*DEBUG_PRINT("Stack: | ");
    for (StackNode *tmp = input_stack.top; tmp != NULL; tmp = tmp->next){
        //DEBUG_PRINT("%s  -  value: ", types[tmp->token.type]); 
        DEBUG_PRINT("%s | ", tmp->token.value);
    }
    DEBUG_PRINT("\nPRINT DONE\n");*/
    
    curr_token = top(&input_stack)->token;
    pop(&input_stack);
    
    if (incoming_while_pipe){return;}


    StackNode *next_on_stack = top(&input_stack);
 
    if (next_on_stack == NULL){
        return;
    }




    if (next_on_stack->token.type == TOKEN_KW_WHILE){

        
        next_on_stack = next_on_stack->next;
        
        if (next_on_stack == NULL){
            return;
        }

        while (next_on_stack != NULL && next_on_stack->token.type != TOKEN_OPE_LBRACE && next_on_stack->token.type != TOKEN_EOF){

            if (next_on_stack->token.type == TOKEN_PIPE){
                
                next_on_stack = next_on_stack->next;
                if (next_on_stack == NULL) {
                    return;
                }
                incoming_while_pipe = true;
                incoming_while_pipe_var = next_on_stack->token;
                generate_while_pipe(incoming_while_pipe_var.value);

                return;
            }

            next_on_stack = next_on_stack->next;

            if (next_on_stack == NULL){
                return;
            }
        }
    }

    //DEBUG_PRINT("next_token value: '%s' | type: %s\n", curr_token.value, types[curr_token.type]);
}

/**
 * Matches the current token with the expected token
 * @param type Expected token type
 * @return void
 */
void match(TokenType type){
    if (type == TOKEN_OPE_ASSIGN){
        sem_4_is_assigned = true;
    } else if (type == TOKEN_OPE_SEMICOLON){
        sem_4_is_assigned = false;
    }
    //DEBUG_PRINT("AT MATCH - curr_token value: %s\t curr_token type: %s\n", curr_token.value, types[curr_token.type]);
    //DEBUG_PRINT("AT MATCH - expected type: %s\n", types[type]);
    
    if (curr_token.type == type){
        next();
        //DEBUG_PRINT("SURVIVED MATCH\n");
    } else {
        // error(2);
        fprintf(stderr, "[SYNTAX ERROR] [AT MATCH] Unexpected token - value: '%s' | type: %s\nExpected - type: %s\n", curr_token.value, types[curr_token.type], types[type]);
        error(2);
    }
}

/**
 * Gets symtable data type from the token
 * @param token Token
 * @return DataType
 */
DataType get_datatype_from_token(Token token) {
    switch (token.type)
    {
        case TOKEN_KW_I32:
            return TYPE_I32;
        case TOKEN_KW_F64:
            return TYPE_F64;
        case TOKEN_KW_U8:
            return TYPE_U8;
        case TOKEN_KW_VOID:
            return TYPE_FVOID;
        case TOKEN_TP_STRING:
            return TYPE_SLITERAL;
        default:
            DEBUG_PRINT("[ERROR] Token isn't a data type!");
            error(99);
    }
}

/**
 * Parses the type
 * @param type Pointer to the STData
 * @return void
 */
void parse_type(STData * type) {
    switch (curr_token.type) {
        case TOKEN_KW_I32:
        case TOKEN_KW_F64:
        case TOKEN_KW_U8:
        case TOKEN_KW_VOID:
            type->DataType = get_datatype_from_token(curr_token);
            next();
            break;
        case TOKEN_OPE_LBRACKET:
            if (!has_slice){
                match(TOKEN_OPE_LBRACKET);
                match(TOKEN_OPE_RBRACKET);
                has_slice = true;
                type->slice = true;
                parse_type(type);
                break;
            }
            else {
                fprintf(stderr, "[SYNTAX ERROR] Type already has 'slice' modifier\n");
            }
        case TOKEN_NULLABLE:
            if (!has_nullable){
                match(TOKEN_NULLABLE);
                has_nullable = true;
                type->nullable = true;
                parse_type(type);
                break;
            }
            else {
                fprintf(stderr, "[SYNTAX ERROR] Type already has 'nullable' modifier\n");
            }
        default:
            fprintf(stderr, "[SYNTAX ERROR] Unexpected token in type: value: '%s' | type: %s, expected: ([])i32 | ([])f64 | ([])u8 | void\n", curr_token.value, types[curr_token.type]);
            error(2);
    }
}

/**
 * Parses the function parameters
 * @param node Pointer to the SymNode
 * @param skip Skip the adding of parameters to the symtable
 * @return void
 */
void params_n(SymNode * node, bool skip, int index){
    char * parameter_id;
    if (curr_token.type == TOKEN_ID) {
        parameter_id = curr_token.value;
        next();
    }
    match(TOKEN_OPE_COLON);

    STData parameter_type = {TYPE_UNDEFINED, false, false};

    has_nullable = false;
    has_slice = false;
    parse_type(&parameter_type);

    index++;

    if (!skip){
        symnode_add_param(node, parameter_id, parameter_type);
    }
    if (curr_token.type == TOKEN_OPE_COMMA){
        match(TOKEN_OPE_COMMA);
        params_n(node, skip, index);
    }
    else if (curr_token.type != TOKEN_OPE_RPAREN){
        // error(2);
        fprintf(stderr, "[SYNTAX ERROR] Unexpected token in function parameters: value: '%s' | type: %s\n", curr_token.value, types[curr_token.type]);
        error(2);
    }

    generate_function_param(parameter_id, index);
    
}

/**
 * Parses the function parameters
 * @param node Pointer to the SymNode
 * @param skip Skip the adding of parameters to the symtable
 * @return void
 */
void params(SymNode * node, bool skip){
    int index = 0;
    if (curr_token.type == TOKEN_OPE_RPAREN) {
        return;
    }
    params_n(node, skip, index);
}

void load_expression(Stack *precstack);

int resolve_enums(int src){
    switch (src)
    {
    case TOKEN_TP_INT:
        return TYPE_I32;
        break;
    case TOKEN_TP_FLOAT:
        return TYPE_F64;
        break;
    case TOKEN_TP_STRING:
        return TYPE_SLITERAL;
        break;
    default:
        return -1;
        break;
    }
}

/**
 * Parses the function call parameters
 * @return void
 * @see SymNode
 */
void parse_function_call_params(){
    DEBUG_PRINT("---------------- In parse_function_call_params, curr_token: %s | %s\n", curr_token.value, types[curr_token.type]); 
    if (curr_token.type == TOKEN_OPE_RPAREN){

        SymNode *tmp = symnode_find(GlobalSymTable, sem_4_func_id);
        if (symnode_get_param_count(tmp) != sem_4_arg_cnt){
            DEBUG_PRINT("SEMANTIC ERROR 4 - FUNC ARG COUNT\n");
            error(SEMANTIC_ERROR_FUNC);
        }
        if (sem_4_is_assigned == false && tmp->data->DataType != TYPE_FVOID){
            DEBUG_PRINT("SEMANTIC ERROR 4 - FUNC RETURN TRASHED\n");
            error(SEMANTIC_ERROR_FUNC);
        } else if (sem_4_is_assigned == true && tmp->data->DataType == TYPE_FVOID){
            DEBUG_PRINT("SEMANTIC ERROR 4 - VIOD FUNC RETURN SAVED\n");
            error(SEMANTIC_ERROR_FUNC);
        }
        sem_4_arg_cnt = 0;
        sem_4_is_assigned = false;
        return;
    }
    if (strcmp(curr_token.value, "vysl_i32")==0){
        DEBUG_PRINT("VYSL_I32 PROSIIM : %d\n", curr_token.type);

    }
    if (curr_token.type != TOKEN_OPE_COMMA){
        if (curr_token.type == TOKEN_ID){
            SymNode *tmp = symnode_find(GlobalSymTable, sem_4_func_id);
            DEBUG_PRINT("ARGS-PARSE: %s %s\n", CurrentContext->id, curr_token.value);
            inc_in_all_contexts(CurrentContext, curr_token.value);
            if (strcmp(curr_token.value, "vysl_i32")==0){
                DEBUG_PRINT("INC VYSL_I32 PROSIIM : %d %s\n", curr_token.type, CurrentContext->parfunc->id);

            }
            //DEBUG_PRINT("bb %d bb\n", tmp==NULL);
            DEBUG_PRINT("%s %s %d - %d\n",sem_4_func_id,curr_token.value,symnode_get_param(tmp, sem_4_arg_cnt)==NULL,symnode_search(CurrentContext, curr_token.value)==NULL);

            if (curr_token.type == TOKEN_TP_STRING){

            } else {
                if (symnode_get_param(tmp, sem_4_arg_cnt)->type->DataType != symnode_search(CurrentContext, curr_token.value)->data->DataType){
                    DEBUG_PRINT("SEMANTIC ERROR 4 - FUNC TYPE MISSMATCH\n");
                    //error(SEMANTIC_ERROR_FUNC);
                }
            }
        }

        generate_function_call_param(sem_4_arg_cnt, &curr_token);

        sem_4_arg_cnt++;
    }

    
    next();
    parse_function_call_params();
}

/**
 * Loads expression to precedence stack
 * @param precstack Pointer to the stack
 * @return void
 * @see Stack
 */
void load_expression(Stack *precstack) {
    //char *identifier = '\0';
    DEBUG_PRINT("Loading Tokens for Precedent Parsing: curr_token: %s | %s\n", curr_token.value, types[curr_token.type]); 
    Token new_prec_stack_token;
    switch (curr_token.type) {
        case TOKEN_ID:
            if (strcmp(curr_token.value, "ifj") == 0){
                
                //identifier = concat_char(identifier, "ifj");
                next();//ifj.dwad <-> 

                match(TOKEN_OPE_DOT);
                //identifier = concat_char(identifier, ".");
                if (curr_token.type == TOKEN_BUILTIN){
                    new_prec_stack_token.type = TOKEN_KW_FN;
                    new_prec_stack_token.value = curr_token.value;
                    SymNode *tmp = symnode_find(GlobalSymTable, curr_token.value);
                    new_prec_stack_token.data.DataType = tmp->data->DataType;
                    new_prec_stack_token.data.nullable = tmp->data->nullable;
                    new_prec_stack_token.data.slice = tmp->data->slice;
                    new_prec_stack_token.is_e = false;
                    sem_6_func_id = curr_token.value;
                    //sem_4_func_id = curr_token.value;

                    sem_4_func_id = curr_token.value;

                    push(precstack, new_prec_stack_token);
                    next();
                }
                else {
                    fprintf(stderr, "[SYNTAX ERROR] Unexpected token in builtin function call - value: '%s' | type: %s\n", curr_token.value, types[curr_token.type]);
                    error(2);
                }

                match(TOKEN_OPE_LPAREN);

                generate_function_call_param_head();

                parse_function_call_params();

                generate_builtin_function_call(new_prec_stack_token.value);
                generate_function_return_value_push();

                match(TOKEN_OPE_RPAREN);
                
            }
            else {
                char * var_id = curr_token.value;
                bool is_var = true;

                sem_6_func_id = curr_token.value;
                sem_4_func_id = curr_token.value;
                next();
                if (curr_token.type == TOKEN_OPE_LPAREN){

                    DEBUG_PRINT("TOKEN TYPE %d\n", curr_token.type);

                    match(TOKEN_OPE_LPAREN);

                    generate_function_call_param_head();

                    parse_function_call_params();

                    generate_function_call(sem_4_func_id);
                    generate_function_return_value_push();

                    match(TOKEN_OPE_RPAREN);

                    is_var = false;
                }
                
                if (is_var) {
                    new_prec_stack_token.type = TOKEN_ID;
                    new_prec_stack_token.value = var_id;
                    
                    DEBUG_PRINT("\nSearching for variable '%s' in context '%s'\n\n", var_id, CurrentContext->id);


                    SymNode *tmp = symnode_search(CurrentContext, var_id);

                    if (tmp == NULL){
                        fprintf(stderr, "1 [SEMANTIC ERROR] Variable '%s' is undefined\n", var_id);
                        error(SEMANTIC_ERROR_UNDEFINED);
                    }
                    tmp->is_used++;
                    new_prec_stack_token.data.DataType = tmp->data->DataType;
                    new_prec_stack_token.data.nullable = tmp->data->nullable;
                    new_prec_stack_token.data.slice = tmp->data->slice;
                    new_prec_stack_token.is_e = false;
                    //DEBUG_PRINT("PUSHING VARIABLE '%s' TO PRECSTACK\n", var_id);
                    push(precstack, new_prec_stack_token);
                    //DEBUG_PRINT("PUSHED VARIABLE '%s' TO PRECSTACK\n", precstack->top->token.value);
                }
                else {
                    new_prec_stack_token.type = TOKEN_KW_FN;
                    new_prec_stack_token.value = var_id;
                    
                    SymNode *tmp = symnode_find(GlobalSymTable, var_id);

                    if (tmp == NULL){
                        fprintf(stderr, "[SEMANTIC ERROR] Function '%s' is undefined\n", var_id);
                        error(SEMANTIC_ERROR_UNDEFINED);
                    }

                    new_prec_stack_token.data.DataType = tmp->data->DataType;
                    new_prec_stack_token.data.nullable = tmp->data->nullable;
                    new_prec_stack_token.data.slice = tmp->data->slice;
                    new_prec_stack_token.is_e = false;
                    push(precstack, new_prec_stack_token);
                }
            }
            load_expression(&PrecedenceInputStack);
            break;
        case TOKEN_OPE_LPAREN:
            new_prec_stack_token = curr_token;
            new_prec_stack_token.is_e = false;
            push(precstack, new_prec_stack_token);
            next();
            while (curr_token.type != TOKEN_OPE_RPAREN) {
                load_expression(&PrecedenceInputStack);
            }

            if (curr_token.type == TOKEN_OPE_RPAREN){
                new_prec_stack_token = curr_token;
                new_prec_stack_token.is_e = false;
                push(precstack, new_prec_stack_token);
                next();
            }
            break;
        case TOKEN_OPE_ADD:
        case TOKEN_OPE_SUB:
            new_prec_stack_token = curr_token;
            new_prec_stack_token.is_e = false;
            push(precstack, new_prec_stack_token);
            next();
            load_expression(&PrecedenceInputStack);
            break;
        case TOKEN_OPE_MUL:
        case TOKEN_OPE_DIV:
            new_prec_stack_token = curr_token;
            new_prec_stack_token.is_e = false;
            push(precstack, new_prec_stack_token);
            next();
            load_expression(&PrecedenceInputStack);
            break;
        case TOKEN_OPE_EQ:
        case TOKEN_OPE_NEQ:
        case TOKEN_OPE_LT:
        case TOKEN_OPE_GT:
        case TOKEN_OPE_LTE:
        case TOKEN_OPE_GTE:
            new_prec_stack_token = curr_token;
            new_prec_stack_token.is_e = false;
            push(precstack, new_prec_stack_token);
            next();
            load_expression(&PrecedenceInputStack);
            break;
        case TOKEN_OPE_ASSIGN: //skus ma dat do prec :)
            sem_4_is_assigned = true;
            next();
            load_expression(&PrecedenceInputStack);
            break;
        case TOKEN_KW_NULL:
            new_prec_stack_token.type = TOKEN_KW_NULL;
            new_prec_stack_token.value = "null";
            new_prec_stack_token.data.DataType = TYPE_FVOID;
            new_prec_stack_token.data.nullable = true;
            new_prec_stack_token.data.slice = false;
            push(precstack, new_prec_stack_token);
            next();
            load_expression(&PrecedenceInputStack);
            break;
        case TOKEN_TP_INT:
            new_prec_stack_token = curr_token;
            new_prec_stack_token.data.DataType = TYPE_I32;
            new_prec_stack_token.data.nullable = false;
            new_prec_stack_token.data.slice = false;
            new_prec_stack_token.is_e = false;
            push(precstack, new_prec_stack_token);
            next();
            load_expression(&PrecedenceInputStack);
            break;
        case TOKEN_TP_FLOAT:
            new_prec_stack_token = curr_token;
            new_prec_stack_token.data.DataType = TYPE_F64;
            new_prec_stack_token.data.nullable = false;
            new_prec_stack_token.data.slice = false;
            new_prec_stack_token.is_e = false;
            push(precstack, new_prec_stack_token);
            next();
            load_expression(&PrecedenceInputStack);
            break;
        case TOKEN_OPE_SEMICOLON:
        case TOKEN_OPE_COMMA:
        case TOKEN_OPE_LBRACE:
        case TOKEN_OPE_RPAREN:
            /*if (sem_4_func_id != NULL){
                SymNode *tmp = symnode_search(GlobalSymTable, sem_4_func_id);
                if (tmp == NULL){
                    return;
                }
                if (tmp -> data -> DataType != TYPE_FVOID){
                    if (!sem_4_is_assigned){
                        DEBUG_PRINT("SEMANTIC ERROR 4 - FUNC RETURN NOT ASSIGNED");
                        error(SEMANTIC_ERROR_FUNC);
                    }
                }
                sem_4_func_id = NULL;
                sem_4_is_assigned = false;
            }
            sem_4_is_assigned = false;*/
            return;
        default:
            // error(SYNTAX_ERROR);
            fprintf(stderr, "[SYNTAX ERROR] Unexpected token in expression - value: '%s' | type: %s\n", curr_token.value, types[curr_token.type]);
            error(2);
    }
}

/**
 * Parses line of code
 * @return void
 * @see STData
 * @see precedent_parser_parse
 */
void statement(){
    DEBUG_PRINT("Parsing statement: curr_token: %s | %s\n", curr_token.value, types[curr_token.type]);
    if (curr_token.type == TOKEN_OPE_RBRACE){
        DEBUG_PRINT("\n\nRETURNING FROM STATEMENT\n\n");
        match(TOKEN_OPE_RBRACE);

        return;
    }
    switch (curr_token.type){
        case TOKEN_ID: {
            char *id = curr_token.value;
            if (strcmp(curr_token.value, "ifj") == 0){
                next();

                char *builtin_id;

                match(TOKEN_OPE_DOT);
                if (curr_token.type == TOKEN_BUILTIN){
                    builtin_id = curr_token.value;
                    next();
                }
                else {
                    fprintf(stderr, "[SYNTAX ERROR] Unexpected token in builtin function call - value: '%s' | type: %s\n", curr_token.value, types[curr_token.type]);
                    error(2);
                }
                // Tuto sa to ma cheknut 
                match(TOKEN_OPE_LPAREN);
                generate_function_call_param_head();
                sem_4_func_id = builtin_id;
                while (curr_token.type != TOKEN_OPE_RPAREN){
                    parse_function_call_params();
                }
                generate_builtin_function_call(builtin_id);
                match(TOKEN_OPE_RPAREN);
                match(TOKEN_OPE_SEMICOLON);
                break;
            }
            else {
                Token id_token = curr_token;
                sem_4_func_id = id_token.value;
                next(); 
                if (curr_token.type == TOKEN_OPE_LPAREN) {
                    match(TOKEN_OPE_LPAREN);
                    generate_function_call_param_head();
                    parse_function_call_params();
                    generate_function_call(id_token.value);
                    match(TOKEN_OPE_RPAREN);
                    match(TOKEN_OPE_SEMICOLON);
                }
                else if (curr_token.type == TOKEN_OPE_ASSIGN){
                    if (strcmp(id_token.value, "_") != 0){
                        SymNode *tmp = symnode_create(id_token.value, BLOCK);
                        sem_check_is_redefine_or_undefine_question_mark(tmp, CurrentContext, false);
                    }
                    //sem_4_is_assigned = true;

                    match(TOKEN_OPE_ASSIGN);
                    load_expression(&PrecedenceInputStack);
                    STData expr_data_type = precedent_parser_parse();

                    if (strcmp(id_token.value, "_") != 0){
                        SymNode * find_var = symnode_search(CurrentContext, id_token.value);

                        if (find_var == NULL){
                            fprintf(stderr, "[SEMANTIC ERROR] Variable '%s' is undefined\n", id_token.value);
                            error(3);
                        }
                        inc_in_all_contexts(find_var->parfunc, find_var->id);
                        DEBUG_PRINT("ID: %s | DataType: %d | Nullable: %d | Slice: %d\n", find_var->id, find_var->data->DataType, find_var->data->nullable, find_var->data->slice);
                        if (expr_data_type.DataType != TYPE_FVOID){
                            if ((find_var->data->DataType != expr_data_type.DataType) || find_var->data->nullable != expr_data_type.nullable || find_var->data->slice != expr_data_type.slice){
                                fprintf(stderr, "[SEMANTIC ERROR] Variable type doesn't match expression type\n");
                                error(7);
                            }
                            symnode_set_data(symnode_search(CurrentContext->parfunc, id_token.value), expr_data_type);
                        }
                        else {
                            if (find_var->data->nullable == false){
                                fprintf(stderr, "[SEMANTIC ERROR] Variable '%s' is not nullable and is assigned 'null'\n", id_token.value);
                                error(7);
                            }
                        }
                        
                    }

                    generate_variable_definition(id);

                    dispose_stack(&PrecedenceInputStack);
                    match(TOKEN_OPE_SEMICOLON);
                    break;
                }
                else {
                    fprintf(stderr, "[SYNTAX ERROR] Unexpected token in statement - value: '%s' | type: %s\n", curr_token.value, types[curr_token.type]);
                    error(2);
                }
            }
            break;
        }
        case TOKEN_KW_CONST:
        case TOKEN_KW_VAR: {
            Token var_dtype = curr_token;
            char *var_id;
            //sem_4_is_assigned = true;
            next();

            SymNode * variable;
            variable = symnode_create(curr_token.value, var_dtype.type == TOKEN_KW_CONST ? CONSTANT : VARIABLE);

            if (strcmp(curr_token.value, "_") == 0){
                fprintf(stderr, "[SEMANTIC ERROR] Variable cannot be named '_'\n");
                error(10);
            }

            //Generate the declaration of the variable
            generate_variable_declaration(curr_token.value);
            var_id = curr_token.value;

            match(TOKEN_ID);

            STData var_type = {TYPE_UNDEFINED, false, false};

            if (curr_token.type == TOKEN_OPE_COLON){
                next();
                has_nullable = false;
                has_slice = false;
                parse_type(&var_type);
            }

            symnode_set_data(variable, var_type);

            sem_check_is_redefine_or_undefine_question_mark(variable, CurrentContext, true);

            symnode_add_to_context(variable, CurrentContext);

            match(TOKEN_OPE_ASSIGN);
            load_expression(&PrecedenceInputStack);
            STData expr_data_type = precedent_parser_parse();

            if (var_type.DataType != TYPE_UNDEFINED && expr_data_type.DataType != TYPE_FVOID) {
                if (var_type.DataType != expr_data_type.DataType ||  var_type.slice != expr_data_type.slice || var_type.nullable != expr_data_type.nullable){
                    if (var_type.nullable == true && (expr_data_type.DataType == TYPE_F64 || expr_data_type.DataType == TYPE_I32)){
                        goto IsANumConstant;
                    }
                    fprintf(stderr, "[SEMANTIC ERROR] Variable type doesn't match expression type\n");
                    error(7);
                }
            }

            IsANumConstant:

            if (var_type.nullable == false && expr_data_type.DataType == TYPE_FVOID){
                fprintf(stderr, "[SEMANTIC ERROR] Variable '%s' is not nullable and is assigned 'null'\n", var_id);
                error(7);
            }
            
            if (expr_data_type.DataType == TYPE_UNDEFINED){
                fprintf(stderr, "[SEMANTIC ERROR] Variable cannot be assigned bool or couldn't infer type from expression\n");
                error(8);
            }

            if (var_type.DataType == TYPE_UNDEFINED){
                if (expr_data_type.DataType == TYPE_FVOID){
                    fprintf(stderr, "[SEMANTIC ERROR] Variable type couldn't be inferred from expression\n");
                    error(8);
                }
                symnode_set_data(variable, expr_data_type);
            }
            dispose_stack(&PrecedenceInputStack);

            //Generate the definition (assigment) of the variable
            generate_variable_definition(var_id);

            match(TOKEN_OPE_SEMICOLON);
            break;
        }
        case TOKEN_KW_IF: {
            int is_expression = 1;
            SymNode * var_with_null;
            SymNode * if_block = symnode_create("if", BLOCK);
            int in_block = 0;
            for (SymNode *tmp = CurrentContext->context_next; tmp != NULL; tmp = tmp->context_next){
                if (tmp->ObjType != BLOCK && in_block == 0){
                    SymNode * tmptmp = symnode_create(tmp->id, tmp->ObjType);
                    symnode_set_data(tmptmp, *tmp->data);
                    symnode_add_to_context(tmptmp, if_block);
                }
                else {
                    if (strcmp(tmp->id, "end_block") != 0) {
                        in_block++;
                    }
                    else {
                        in_block--;
                    }
                }
            }
            symnode_add_to_context(if_block, CurrentContext);
            CurrentContext = if_block;
            next();
            if (curr_token.type == TOKEN_OPE_LPAREN){
                match(TOKEN_OPE_LPAREN);
                //DEBUG_PRINT("CURR_TOKEN: %s\n", curr_token.value);
                load_expression(&PrecedenceInputStack);
                //DEBUG_PRINT("curr_token: %s\n", curr_token.value);
                match(TOKEN_OPE_RPAREN);
                //DEBUG_PRINT("PRECEDECE INPUT STACK TOP: %d\n", top(&PrecedenceInputStack) == NULL);
                if ((&PrecedenceInputStack)->top->token.type == TOKEN_ID){
                    is_expression = 0;
                    var_with_null = symnode_search(CurrentContext->parfunc, PrecedenceInputStack.top->token.value);
                    if (var_with_null == NULL){
                        fprintf(stderr, "2 [SEMANTIC ERROR] Variable '%s' is undefined\n", PrecedenceInputStack.top->token.value);
                        error(3);
                    }
                    if (var_with_null->data->nullable == false){
                        fprintf(stderr, "[SEMANTIC ERROR] Variable '%s' is not nullable and is the only argument in if statement\n", PrecedenceInputStack.top->token.value);
                        error(3);
                    }
                    inc_in_all_contexts(var_with_null->parfunc, var_with_null->id);
                }
            
                precedent_parser_parse();
                dispose_stack(&PrecedenceInputStack);

                bool ifPipe = false;

                if (curr_token.type == TOKEN_PIPE && !is_expression){
                    match(TOKEN_PIPE);
                    SymNode *tmp = symnode_search(CurrentContext->parfunc, curr_token.value);
                    if (tmp != NULL){
                        fprintf(stderr, "[SEMANTIC ERROR] Variable |%s| is already defined\n", curr_token.value);
                        error(5);
                    }
                    else {
                        SymNode * variable = symnode_create(curr_token.value, VARIABLE);
                        symnode_set_data(variable, *var_with_null->data);
                        variable->data->nullable = false;
                        symnode_add_to_context(variable, CurrentContext);
                    }
                    generate_if_pipe(curr_token.value);
                    ifPipe = true;
                    match(TOKEN_ID);
                    match(TOKEN_PIPE);
                }

                //Generate the head of the if
                generate_if_head(ifPipe);

                match(TOKEN_OPE_LBRACE);
                statement();
                //Generate the end of the true branch of the if
                generate_if_end();
                CurrentContext = CurrentContext->parfunc;
                symnode_add_to_context(symnode_create("end_block", BLOCK), CurrentContext);
                DEBUG_PRINT("RETURNING FROM IF BLOCK TO %s\n", CurrentContext->id);
            }
            else {
                fprintf(stderr, "[SYNTAX ERROR] 2 Unexpected token in if statement - value: '%s' | type: %s\n", curr_token.value, types[curr_token.type]);
                error(2);
            }

            //Generate the beginning of the false branch of the if
            generate_else();

            if (curr_token.type == TOKEN_KW_ELSE){
                SymNode * else_block = symnode_create("else", BLOCK);
                int in_block = 0;
                for (SymNode *tmp = CurrentContext->context_next; tmp != NULL; tmp = tmp->context_next){
                    if (tmp->ObjType != BLOCK && in_block == 0){
                        SymNode * tmptmp = symnode_create(tmp->id, tmp->ObjType);
                        symnode_set_data(tmptmp, *tmp->data);
                        symnode_add_to_context(tmptmp, else_block);
                    }
                    else {
                        if (strcmp(tmp->id, "end_block") != 0) {
                            in_block++;
                        }
                        else {
                            in_block--;
                        }
                    }
                }
                next();
                match(TOKEN_OPE_LBRACE);
                symnode_add_to_context(else_block, CurrentContext);
                CurrentContext = else_block;
                statement();
                CurrentContext = CurrentContext->parfunc;
                symnode_add_to_context(symnode_create("end_block", BLOCK), CurrentContext);
            }

            //Generate the end of the whole if
            generate_else_end();

            break;
        }
        case TOKEN_KW_WHILE: {
            //Generate the head of the while
            generate_while_head();
            int is_expression = 1;
            SymNode * var_with_null;
            SymNode * while_block = symnode_create("while", BLOCK);
            int in_block = 0;
            for (SymNode *tmp = CurrentContext->context_next; tmp != NULL; tmp = tmp->context_next){
                if (tmp->ObjType != BLOCK && in_block == 0){
                    SymNode * tmptmp = symnode_create(tmp->id, tmp->ObjType);
                    symnode_set_data(tmptmp, *tmp->data);
                    symnode_add_to_context(tmptmp, while_block);
                }
                else {
                    if (strcmp(tmp->id, "end_block") != 0) {
                        in_block++;
                    }
                    else {
                        in_block--;
                    }
                }
            }
            symnode_add_to_context(while_block, CurrentContext);
            CurrentContext = while_block;
            next();
            if (curr_token.type == TOKEN_OPE_LPAREN){
                match(TOKEN_OPE_LPAREN);
                load_expression(&PrecedenceInputStack);
                DEBUG_PRINT("WHILE EXPRESSION: %s\n", PrecedenceInputStack.top->token.value);
                match(TOKEN_OPE_RPAREN);
                if (PrecedenceInputStack.top->token.type == TOKEN_ID){
                    is_expression = 0;
                    var_with_null = symnode_search(CurrentContext->parfunc, PrecedenceInputStack.top->token.value);
                    if (var_with_null == NULL){
                        fprintf(stderr, "3 [SEMANTIC ERROR] Variable '%s' is undefined\n", PrecedenceInputStack.top->token.value);
                        error(3);
                    }
                    if (var_with_null->data->nullable == false){
                        fprintf(stderr, "[SEMANTIC ERROR] Variable '%s' is not nullable and is the only argument in while statement\n", PrecedenceInputStack.top->token.value);
                        error(3);
                    }
                    inc_in_all_contexts(var_with_null->parfunc, var_with_null->id);
                }
                precedent_parser_parse();
                dispose_stack(&PrecedenceInputStack);

                if (incoming_while_pipe){
                    generate_while_pipe_definition(incoming_while_pipe_var.value);
                }

                //Generate the condition check for the while
                generate_while_condition(incoming_while_pipe);
                incoming_while_pipe = false;
                if (curr_token.type == TOKEN_PIPE && !is_expression){
                    DEBUG_PRINT("curr_token: %s\n", curr_token.value);
                    match(TOKEN_PIPE);
                    SymNode *tmp = symnode_search(CurrentContext->parfunc, curr_token.value);
                    if (tmp != NULL){
                        fprintf(stderr, "[SEMANTIC ERROR] Variable |%s| is already defined\n", curr_token.value);
                        error(5);
                    }
                    else {
                        SymNode * variable = symnode_create(curr_token.value, VARIABLE);
                        symnode_set_data(variable, *var_with_null->data);
                        variable->data->nullable = false;
                        symnode_add_to_context(variable, CurrentContext);
                    }
                    match(TOKEN_ID);
                    match(TOKEN_PIPE);
                }

                match(TOKEN_OPE_LBRACE);
                statement();
                CurrentContext = CurrentContext->parfunc;
                symnode_add_to_context(symnode_create("end_block", BLOCK), CurrentContext);
                //Generate the end of the while
                generate_while_end();
            }
            else {
                fprintf(stderr, "[SYNTAX ERROR] 2 Unexpected token in if statement - value: '%s' | type: %s\n", curr_token.value, types[curr_token.type]);
                error(2);
            }
            break;
        }
        case TOKEN_KW_RETURN: {
            next();
            load_expression(&PrecedenceInputStack);
            //DEBUG_PRINT("SEMANTIC 6 - FUNC ID: %s\n", sem_6_func_id);
            //DEBUG_PRINT("STACK TOP: %d\n", PrecedenceInputStack.top == NULL);
            if (PrecedenceInputStack.top!=NULL){
                //if (symnode_search(GlobalSymTable, sem_6_func_id)->data->DataType == TYPE_FVOID){
                    sem_6_has_return = true;

                //}

                STData return_data_type = precedent_parser_parse();

                if (!(CurrentContext->parfunc->data->DataType == return_data_type.DataType && CurrentContext->parfunc->data->nullable == return_data_type.nullable && CurrentContext->parfunc->data->slice == return_data_type.slice)){
                    fprintf(stderr, "[SEMANTIC ERROR] Return type different from function declaration\n");
                    error(10);
                }

                if (CurrentContext->data->DataType != TYPE_FVOID){
                    generate_function_return_value_assign();
                }

                generate_function_return(CurrentContext->parfunc->id);

                dispose_stack(&PrecedenceInputStack);
            }
            match(TOKEN_OPE_SEMICOLON);
            break;
        }
        default:
            fprintf(stderr, "[SYNTAX ERROR] Unexpected token in block - value: '%s' | type: %s\n", curr_token.value, types[curr_token.type]);
            error(2);
    }
    statement();
}

/**
 * Parses global scope
 * @return void
 */
void prepare_symtable(){
    DEBUG_PRINT("PREPARE token: %s\n", curr_token.value);
    if (curr_token.type == TOKEN_EOF){
        return;
    }
    match(TOKEN_KW_PUB);
    match(TOKEN_KW_FN);
    SymNode * function_def;
    if (curr_token.type == TOKEN_ID) {
         function_def = symnode_create(curr_token.value, FUNCTION);
         next();
    }
    match(TOKEN_OPE_LPAREN);
    params(function_def, false);
    
    match(TOKEN_OPE_RPAREN);


    STData function_returns = { TYPE_UNDEFINED, false, false };
    has_nullable = false;
    has_slice = false;
    parse_type(&function_returns);

    symnode_set_data(function_def, function_returns);

    if (strcmp(function_def->id, "main") == 0){
        has_main = true;
        if (function_def->params != NULL){
            fprintf(stderr, "[SEMANTIC ERROR] Main function cannot have parameters\n");
            error(10);
        }
        if (function_returns.DataType != TYPE_FVOID){
            fprintf(stderr, "[SEMANTIC ERROR] Main function must return void\n");
            error(10);
        }
    }

    symnode_insert(GlobalSymTable, function_def);
    match(TOKEN_OPE_LBRACE);
    int lbc = 1;
    while (lbc != 0){
        if (curr_token.type == TOKEN_OPE_LBRACE){
            lbc++;
        }
        if (curr_token.type == TOKEN_OPE_RBRACE){
            lbc--;
        }
        if (curr_token.type == TOKEN_EOF){
            fprintf(stderr, "[SYNTAX ERROR] Unexpected EOF in function definition\n");
            error(2);
        }
        next();
    }

    //while (curr_token.type != TOKEN_KW_PUB || curr_token.type != TOKEN_EOF) {
    //    next();
    //} ?????????????????????????????????????????????????????

    prepare_symtable();
}



/**
 * Parses function definition
 * @return void
 */
void funcdef(){
    if (curr_token.type == TOKEN_EOF){
        return;
    }
    match(TOKEN_KW_PUB);
    match(TOKEN_KW_FN);
    SymNode * function_def = symnode_find(GlobalSymTable, curr_token.value);
    DEBUG_PRINT("[FUNCDEF] curr token: %s\n", curr_token.value);
    DEBUG_PRINT("[FUNCDEF] token: %s\n", function_def->id);
    match(TOKEN_ID);
    sem_6_func_id = function_def->id;
    sem_6_has_return = false;
    //Generate the head of the function
    if (strcmp(function_def->id, "main") == 0){
        generate_main_function_head();
    }else{
        generate_function_head(function_def->id);
    }

    match(TOKEN_OPE_LPAREN);
    params(function_def, true);

    match(TOKEN_OPE_RPAREN);


    STData function_returns = { TYPE_UNDEFINED, false, false };
    has_nullable = false;
    has_slice = false;
    parse_type(&function_returns);

    symnode_set_data(function_def, function_returns);

    if (strcmp(function_def->id, "main") == 0){
        has_main = true;
        if (function_def->params != NULL){
            fprintf(stderr, "[SEMANTIC ERROR] Main function cannot have parameters\n");
            error(10);
        }
        if (function_returns.DataType != TYPE_FVOID){
            fprintf(stderr, "[SEMANTIC ERROR] Main function must return void\n");
            error(10);
        }
    }

    if (function_returns.DataType != TYPE_FVOID){
        generate_function_return_value();
    }

    //symnode_insert(GlobalSymTable, function_def);
    CurrentContext = function_def;
    match(TOKEN_OPE_LBRACE);
    statement();

    sem_6_func_id = CurrentContext->parfunc->id;
    
    DEBUG_PRINT("SEMANTIC 6 - FUNC ID 2: %s\n", sem_6_func_id);
    SymNode *tmp = symnode_find(GlobalSymTable, sem_6_func_id);
    if (tmp != NULL){
        if (tmp -> data -> DataType == TYPE_FVOID){
            if (sem_6_has_return){
                DEBUG_PRINT("SEMANTIC ERROR 6, VOID HAS RETURN");
                fprintf(stderr, "[SEMANTIC ERROR] Void function cannot return a value\n");
                error(SEMANTIC_ERROR_MISSING);
            }
        } else {
            if (!sem_6_has_return){
                DEBUG_PRINT("SEMANTIC ERROR 6, NON-VOID NO RETURN");
                fprintf(stderr, "[SEMANTIC ERROR] Non-void function must return a value\n");
                error(SEMANTIC_ERROR_MISSING);
            }
        }
    }

    //Generate the end of the function
    if (strcmp(function_def->id, "main") == 0){
        generate_main_function_end();
    }else{
        generate_function_end(function_def->id);
    }

    funcdef();
}

/**
 * Parses prolouge
 * @return void
 */
void prol(){

    next();
    match(TOKEN_KW_CONST);
    if (strcmp(curr_token.value, "ifj") == 0){
        next();
    }
    else {
        fprintf(stderr, "[SYNTAX ERROR] Constant other than 'ifj' in global scope: '%s'\n", curr_token.value);
        error(2);
    }
    match(TOKEN_OPE_ASSIGN);
    match(TOKEN_OPE_AT);
    match(TOKEN_KW_IMPORT);
    match(TOKEN_OPE_LPAREN);
    if ((curr_token.type == TOKEN_TP_STRING) && (strcmp(curr_token.value, "ifj24.zig") == 0)){
        next();
    }
    else {
        fprintf(stderr, "[SYNTAX ERROR] Invalid import, expected 'ifj24.zig', found '%s'\n", curr_token.value);
        //error(2);
        error(2);
    }
    match(TOKEN_OPE_RPAREN);
    match(TOKEN_OPE_SEMICOLON);
}

/**
 * Creates global symbol table with built-in functions
 * @return SymNode
 */
SymNode create_symtable_with_builtins(){

    SymNode * builtin = symnode_create("readstr", FUNCTION);
    STData data;
    data.DataType = TYPE_U8;
    data.nullable = true;
    data.slice = true;
    symnode_set_data(builtin, data);

    GlobalSymTable = builtin;



    builtin = symnode_create("readi32", FUNCTION);
    
    data.DataType = TYPE_I32;
    data.nullable = true;
    data.slice = false;
    symnode_set_data(builtin, data);

    symnode_insert(GlobalSymTable, builtin);



    builtin = symnode_create("readf64", FUNCTION);
    
    data.DataType = TYPE_F64;
    data.nullable = true;
    data.slice = false;
    symnode_set_data(builtin, data);

    symnode_insert(GlobalSymTable, builtin);
    


    builtin = symnode_create("write", FUNCTION);
    
    data.DataType = TYPE_FVOID;
    data.nullable = true;
    data.slice = false;
    symnode_set_data(builtin, data);

    STData param;
    param.DataType = TYPE_I32;
    param.nullable = true;
    param.slice = false;

    symnode_add_param(builtin, "term_parameter_opt", param);

    
    param.DataType = TYPE_F64;
    param.nullable = true;
    param.slice = false;

    symnode_add_param(builtin, "term_parameter_opt", param);

    symnode_insert(GlobalSymTable, builtin);



    builtin = symnode_create("i2f", FUNCTION);
    
    data.DataType = TYPE_F64;
    data.nullable = false;
    data.slice = false;
    symnode_set_data(builtin, data);

    
    param.DataType = TYPE_I32;
    param.nullable = false;
    param.slice = false;

    symnode_add_param(builtin, "i32", param);

    symnode_insert(GlobalSymTable, builtin);



    builtin = symnode_create("f2i", FUNCTION);
    
    data.DataType = TYPE_I32;
    data.nullable = false;
    data.slice = false;
    symnode_set_data(builtin, data);

    
    param.DataType = TYPE_F64;
    param.nullable = false;
    param.slice = false;

    symnode_add_param(builtin, "f64", param);

    symnode_insert(GlobalSymTable, builtin);



    builtin = symnode_create("string", FUNCTION);
    
    data.DataType = TYPE_U8;
    data.nullable = false;
    data.slice = true;
    symnode_set_data(builtin, data);

    
    param.DataType = TYPE_SLITERAL;
    param.nullable = false;
    param.slice = false;

    symnode_add_param(builtin, "term_parameter_opt", param);

    
    param.DataType = TYPE_U8;
    param.nullable = false;
    param.slice = true;

    symnode_add_param(builtin, "term_parameter_opt", param);

    symnode_insert(GlobalSymTable, builtin);



    builtin = symnode_create("length", FUNCTION);
    
    data.DataType = TYPE_I32;
    data.nullable = false;
    data.slice = false;
    symnode_set_data(builtin, data);

    
    param.DataType = TYPE_U8;
    param.nullable = false;
    param.slice = true;

    symnode_add_param(builtin, "s", param);

    symnode_insert(GlobalSymTable, builtin);



    builtin = symnode_create("concat", FUNCTION);
    
    data.DataType = TYPE_U8;
    data.nullable = false;
    data.slice = true;
    symnode_set_data(builtin, data);

    
    param.DataType = TYPE_U8;
    param.nullable = false;
    param.slice = true;

    symnode_add_param(builtin, "s1", param);

    
    param.DataType = TYPE_U8;
    param.nullable = false;
    param.slice = true;

    symnode_add_param(builtin, "s2", param);

    symnode_insert(GlobalSymTable, builtin);



    builtin = symnode_create("substring", FUNCTION);
    
    data.DataType = TYPE_U8;
    data.nullable = true;
    data.slice = true;
    symnode_set_data(builtin, data);

    
    param.DataType = TYPE_U8;
    param.nullable = false;
    param.slice = true;

    symnode_add_param(builtin, "s", param);

    
    param.DataType = TYPE_I32;
    param.nullable = false;
    param.slice = false;

    symnode_add_param(builtin, "i", param);

    
    param.DataType = TYPE_I32;
    param.nullable = false;
    param.slice = false;

    symnode_add_param(builtin, "j", param);

    symnode_insert(GlobalSymTable, builtin);



    builtin = symnode_create("strcmp", FUNCTION);
    
    data.DataType = TYPE_I32;
    data.nullable = false;
    data.slice = false;
    symnode_set_data(builtin, data);

    
    param.DataType = TYPE_U8;
    param.nullable = false;
    param.slice = true;

    symnode_add_param(builtin, "s1", param);

    
    param.DataType = TYPE_U8;
    param.nullable = false;
    param.slice = true;

    symnode_add_param(builtin, "s2", param);

    symnode_insert(GlobalSymTable, builtin);



    builtin = symnode_create("ord", FUNCTION);
    
    data.DataType = TYPE_I32;
    data.nullable = false;
    data.slice = false;
    symnode_set_data(builtin, data);

    
    param.DataType = TYPE_U8;
    param.nullable = false;
    param.slice = true;

    symnode_add_param(builtin, "s", param);

    
    param.DataType = TYPE_I32;
    param.nullable = false;
    param.slice = false;

    symnode_add_param(builtin, "i", param);

    symnode_insert(GlobalSymTable, builtin);



    builtin = symnode_create("chr", FUNCTION);
    
    data.DataType = TYPE_U8;
    data.nullable = false;
    data.slice = true;
    symnode_set_data(builtin, data);
    param.nullable = false;
    param.slice = false;

    symnode_add_param(builtin, "i", param);

    symnode_insert(GlobalSymTable, builtin);
}

void find_unused_variables(SymNode *symtable, bool inc);

/**
 * Parses the source code
 * @param FILE *source
 */
void parser_parse(FILE *source){
    input = source;

    //Initialize the code generator
    generator_init();

    Token tmp = get_next_token(input);
    while (tmp.type != TOKEN_EOF) {
        push(&input_stack, tmp);
        push(&backup_stack, tmp);
        tmp = get_next_token(input);
    }

    push(&input_stack, tmp);
    push(&backup_stack, tmp);

    reorder_stack(&input_stack);
    reorder_stack(&backup_stack);

    DEBUG_PRINT("Stack: | ");
    for (StackNode *tmp = input_stack.top; tmp != NULL; tmp = tmp->next){
        //DEBUG_PRINT("%s  -  value: ", types[tmp->token.type]); 
        DEBUG_PRINT("%s | ", tmp->token.value);
    }
    DEBUG_PRINT("\nPRINT DONE\n");

    parser_init();
    init_stack(&PrecedenceInputStack);
    
    create_symtable_with_builtins();

    prol();
    prepare_symtable();
    DEBUG_PRINT("SYMTABLE PREPARED\n");
    
    symnode_print(GlobalSymTable);
    dispose_stack(&input_stack);
    init_stack(&input_stack);
    revert_stack(&input_stack);
    incoming_while_pipe = false; // enable while parsing
    
    prol();
    funcdef();
    DEBUG_PRINT("[PARSER] SOURCE CODE VALID\n");

    


    find_unused_variables(GlobalSymTable, true);
    symnode_print(GlobalSymTable);
    find_unused_variables(GlobalSymTable, false);

    

    parser_cleanup();
    DEBUG_PRINT("Konec zvonec!\n");

    //Flush the generated code to stdout
    generator_flush();

}

/**
 * Finds unused variables in the symbol table
 * @param SymNode *symtable
 * @param bool inc
 */
void find_unused_variables(SymNode *symtable, bool inc){
    const char *sc_builtin[] = {"readstr", "readi32", "readf64", "write", "i2f", "f2i", "string", "length", "concat", "substring", "strcmp", "ord", "chr"};
    if (symtable == NULL){
        return;
    }
    bool is_built_in = false;
    for (int i = 0; i < 13; i++){
        if (strcmp(symtable->id, sc_builtin[i]) == 0){
            is_built_in = true;
            break;
        }
    }
    if (!is_built_in){
        if (!inc){
            if (symtable->context_next != NULL){
                SymNode *tmp = symtable->context_next;
                while (tmp != NULL){
                    if (tmp->ObjType == VARIABLE || tmp -> ObjType == CONSTANT){
                        if (tmp->is_used == 0){
                            fprintf(stderr, "[SEMANTIC ERROR] Variable '%s' is unused\n", tmp->id);
                            error(9);
                        }
                    }
                    tmp = tmp->context_next;
                }
            }
        }else {
            if (symtable->context_next != NULL){
                SymNode *tmp = symtable->context_next;
                while (tmp != NULL){
                    if (tmp->ObjType == VARIABLE || tmp -> ObjType == CONSTANT){
                        if (tmp->is_used != 0){
                            inc_in_all_contexts(symtable, tmp -> id);
                        }
                    }
                    tmp = tmp->context_next;
                }
            }
        }
    }
    find_unused_variables(symtable->right, inc);
    find_unused_variables(symtable->left, inc);
}

// 3 a 5
/**
 * Checks if the variable is redefined or undefined
 * @param SymNode *find_me
 * @param SymNode *context
 * @param bool defining
 */
void sem_check_is_redefine_or_undefine_question_mark(SymNode *find_me, SymNode *context,  bool defining){
    //symnode_find - 'foo', Global ()-> global context returne ptr na nájdenú
    //symnode_search - node -> hladaj v kontexte
    
    
    // snažíme sa ju definovať
    //      var 

    bool exists = symnode_search(context, find_me -> id) != NULL;

    //sémantická chyba v programu – redefinice proměnné nebo funkce; přiřazení do nemodifikovatelné proměnné.
    if (defining){
        if (exists){

            fprintf(stderr, "[SEMANTIC ERROR] Variable '%s' is already defined\n", find_me -> id);
            error(SEMANTIC_ERROR_REDEFINE);
        }
        find_me -> isDefined = true;
        return;
    }
    
    if (strcmp(find_me->id, "_") == 0){
        return;
    }

    //3 - sémantická chyba v programu – nedefinovaná funkce či proměnná.

    if (symnode_find(GlobalSymTable, find_me -> id) != NULL){
        return;
    }

    if (!exists){

        fprintf(stderr, "4 [SEMANTIC ERROR] Variable '%s' is undefined\n", find_me -> id);
        error(SEMANTIC_ERROR_UNDEFINED);
    }

    // přiřazení do nemodifikovatelné proměnné.
    if (symnode_search(context, find_me -> id) -> ObjType == CONSTANT) {

        fprintf(stderr, "[SEMANTIC ERROR] Constant redefinition - variable '%s' is a constant\n", find_me -> id);
        error(SEMANTIC_ERROR_REDEFINE);
    }

    inc_in_all_contexts(find_me->parfunc, find_me->id);
}

/**
 * Increments the is_used variable in all contexts
 * @param SymNode *parent_func
 * @param char* id
 */
void inc_in_all_contexts(SymNode *parent_func, char* id){
    if (parent_func == NULL){
        return;
    }
    if (strcmp(id, "vysl_i32")==0){

    }
    if (parent_func -> parfunc-> ObjType != FUNCTION){
        if (parent_func -> parfunc == parent_func -> parfunc -> parfunc){
            goto skp;
            return;
        }
        //DEBUG_PRINT("INC IN ALL CONTEXTS GOT %s\n", parent_func->parfunc->parfunc->id);
        for (SymNode *tmp = parent_func->parfunc; tmp->ObjType != NULL; tmp = tmp->parfunc){
            if (tmp -> ObjType == FUNCTION){
                parent_func = tmp;
                break;
            }
        }
    }
    skp:
    if (strcmp(id, "vysl_i32")==0){

    }
    for (SymNode *tmp = parent_func->context_next; tmp != NULL; tmp = tmp->context_next){
        if (strcmp(id, "vysl_i32")==0){

        }
        if (strcmp(tmp -> id, id)==0) {
            //DEBUG_PRINT("ADDEDDDDDD");
            tmp -> is_used++;
        }
    }
}
