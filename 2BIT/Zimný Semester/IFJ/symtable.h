/**
 * FIT VUT - IFJ project 2024
 *
 * @file symtable.h
 *
 * @brief Symtable implementation (binary tree variant)
 *
 * @author Štefan Dubnička (xdubnis00)
 */

// symtable implementation using a self balancing binary search tree.

#ifndef SYMTABLE_H
#define SYMTABLE_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "scanner.h"

typedef enum DType {
    VARIABLE,
    CONSTANT,
    FUNCTION,
    BLOCK
} DType;

typedef struct Param {
    char *id;
    STData *type;
    struct Param *next;
} Param;

typedef struct SymNode {
    char *id;
    DType ObjType;

    STData *data;

    bool isDefined;

    int is_used;

    Param * params;

    struct SymNode *parfunc;
    struct SymNode *context_next;

    struct SymNode *left;
    struct SymNode *right;
} SymNode;


SymNode *symnode_create(char *id, DType type);
void symnode_set_data(SymNode *node, STData data);
void symnode_set_defined(SymNode *node);
void symnode_insert(SymNode *root, SymNode *node);
void symnode_delete(SymNode *root, char *key);
void symnode_print(SymNode *root);
void symnode_free(SymNode *root);
void symnode_add_to_context(SymNode *node, SymNode *context);
void symnode_add_param(SymNode *node, char *id, STData type);
int symnode_get_param_count(SymNode *node);
Param *symnode_get_param(SymNode *node, int index);
SymNode *symnode_find(SymNode *root, char *key);
SymNode * symnode_search(SymNode *root, char *key);

#endif