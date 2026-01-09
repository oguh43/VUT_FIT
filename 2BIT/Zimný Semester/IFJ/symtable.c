/**
 * FIT VUT - IFJ project 2024
 *
 * @file symtable.c
 *
 * @brief Symtable implementation (binary tree variant)
 *
 * @author Štefan Dubnička (xdubnis00)
 */

#include "symtable.h"
#include "ptr_registry.h"
#include "error.h"

#ifdef DEBUG
    #define DEBUG_PRINT(...) fprintf(stderr, __VA_ARGS__)
#else
    #define DEBUG_PRINT(...)
#endif

SymNode *symnode_create(char *id, DType type){
    SymNode *new_node = (SymNode *)malloc(sizeof(SymNode));
    if (new_node == NULL){
        fprintf(stderr, "[ERROR] Memory allocation failed\n");
        exit(99);
    }
    ptr_registry_add(new_node);
    new_node->id = id;
    new_node->ObjType = type;
    new_node->data = (STData *)malloc(sizeof(STData));
    if (new_node->data == NULL) {
        fprintf(stderr, "[ERROR] Memory allocation failed\n");
        exit(99);
    }
    ptr_registry_add(new_node->data);
    new_node->data->DataType = TYPE_UNDEFINED;
    new_node->data->nullable = false;
    new_node->data->slice = false;
    new_node->isDefined = false;
    new_node->params = NULL;
    new_node->parfunc = new_node;
    new_node->context_next = NULL;
    new_node->left = NULL;
    new_node->right = NULL;
    return new_node;
}

void symnode_set_data(SymNode *node, STData data){
    DEBUG_PRINT("\nSetting data for node: %s to %d %d %d\n\n", node->id, data.DataType, data.nullable, data.slice);
    if (node == NULL){
        fprintf(stderr, "[ERROR] Node is NULL\n");
        exit(99);
    }
    node->data->DataType = data.DataType;
    node->data->nullable = data.nullable;
    node->data->slice = data.slice;
}

void symnode_set_defined(SymNode *node){
    if (node == NULL){
        fprintf(stderr, "[ERROR] Node is NULL\n");
        exit(99);
    }
    node->isDefined = true;
}

void symnode_insert(SymNode *root, SymNode *node){
    if (root == NULL){
        fprintf(stderr, "[ERROR] Root is NULL\n");
        exit(99);
    }
    if (node == NULL){
        fprintf(stderr, "[ERROR] Node is NULL\n");
        exit(99);
    }
    if (strcmp(node->id, root->id) < 0){
        if (root->left == NULL){
            root->left = node;
        } else {
            symnode_insert(root->left, node);
        }
    } else if (strcmp(node->id, root->id) > 0){
        if (root->right == NULL){
            root->right = node;
        } else {
            symnode_insert(root->right, node);
        }
    } else {
        fprintf(stderr, "[ERROR] Node with this id already exists\n");
        exit(99);
    }
}

void symnode_add_param(SymNode *node, char *id, STData type){
    if (node == NULL){
        fprintf(stderr, "[ERROR] Node is NULL\n");
        exit(99);
    }
    if (node->ObjType != FUNCTION){
        fprintf(stderr, "[ERROR] Node is not a function\n");
        exit(99);
    }
    
    Param *new_param = (Param *)malloc(sizeof(Param));
    if (new_param == NULL){
        fprintf(stderr, "[ERROR] Memory allocation failed\n");
        exit(99);
    }
    ptr_registry_add(new_param);
    new_param->id = id;
    new_param->type = (STData *)malloc(sizeof(STData));
    if (new_param->type == NULL){
        fprintf(stderr, "[ERROR] Memory allocation failed\n");
        exit(99);
    }
    //ptr_registry_add(new_param->type);
    new_param->type->DataType = type.DataType;
    new_param->type->nullable = type.nullable;
    new_param->type->slice = type.slice;
    new_param->next = NULL;
    if (node->params == NULL){
        node->params = new_param;
    } else {
        Param *temp = node->params;
        while (temp->next != NULL){
            temp = temp->next;
        }
        temp->next = new_param;
    }
    SymNode * param_const = symnode_create(id, CONSTANT);
    symnode_set_data(param_const, type);
    symnode_add_to_context(param_const, node);
}

SymNode *symnode_find(SymNode *root, char *key){
    if (root == NULL){
        return NULL;
    }
    if (strcmp(key, root->id) < 0){
        return symnode_find(root->left, key);
    } else if (strcmp(key, root->id) > 0){
        return symnode_find(root->right, key);
    } else {
        return root;
    }
}

SymNode * symnode_search(SymNode *root, char *key){
    if (root == NULL){
        return NULL;
    }

    SymNode *tmp_c = root->context_next;
    int num_of_blocks = 0;
    while (tmp_c != NULL){
        if (strcmp(key, tmp_c->id) == 0 && num_of_blocks == 0){
            return tmp_c;
        }
        if (tmp_c->ObjType == BLOCK && strcmp(tmp_c->id, "end_block") != 0){
            num_of_blocks++;
        }
        if (tmp_c->ObjType == BLOCK && strcmp(tmp_c->id, "end_block") == 0){
            num_of_blocks--;
        }
        tmp_c = tmp_c->context_next;
    }

    return NULL;
}

void symnode_add_to_context(SymNode *node, SymNode *context){
    DEBUG_PRINT("Adding node to context : %s | %s\n", node->id, context->id);
    if (node == NULL){
        fprintf(stderr, "[ERROR] Node is NULL\n");
        exit(99);
    }
    if (context == NULL){
        fprintf(stderr, "[ERROR] Context is NULL\n");
        exit(99);
    }
    if (context->ObjType != FUNCTION && context->ObjType != BLOCK){
        fprintf(stderr, "[ERROR] Parent context is not a function or block, is type: %d\n", context->ObjType);
        exit(99);
    }
    if (symnode_search(context, node->id) != NULL && strcmp(node->id, "term_parameter_opt") != 0){
        if (strcmp(node->id, "end_block") == 0 || strcmp(node->id, "else") == 0 || strcmp(node->id, "while") == 0 || strcmp(node->id, "if") == 0){
            goto IsVynimka;
        }
        fprintf(stderr, "[ERROR] Node with this id already exists in context\n");
        error(SEMANTIC_ERROR_REDEFINE);
    }
    IsVynimka:

    SymNode * tmp = context;
    while (tmp->context_next != NULL){
        //printf("tmp: %s\n", tmp->id);
        tmp = tmp->context_next;
    }
    node->parfunc = context;
    tmp->context_next = node;
    //symnode_print(node);
    //symnode_print(context);
}

int symnode_get_param_count(SymNode *node){
    if (node == NULL){
        fprintf(stderr, "[ERROR] Node is NULL\n");
        exit(99);
    }
    if (node->ObjType != FUNCTION){
        fprintf(stderr, "[ERROR] Node is not a function\n");
        exit(99);
    }
    int count = 0;
    Param *tmp = node->params;
    if (tmp == NULL){
        return 0;
    }
    if (strcmp(tmp->id, "term_parameter_opt") == 0){
        count++;
    }
    while (tmp != NULL){
        if (strcmp(tmp->id, "term_parameter_opt") != 0){
            count++;
        }
        tmp = tmp->next;
    }
    return count;
}

Param *symnode_get_param(SymNode *node, int index){
    if (node == NULL){
        fprintf(stderr, "[ERROR] Node is NULL\n");
        exit(99);
    }
    if (node->ObjType != FUNCTION){
        fprintf(stderr, "[ERROR] Node is not a function\n");
        exit(99);
    }
    if (index < 0){
        fprintf(stderr, "[ERROR] Index is negative\n");
        exit(99);
    }
    Param *tmp = node->params;
    for (int i = 0; i < index; i++){
        if (tmp == NULL){
            return NULL;
        }
        tmp = tmp->next;
    }
    return tmp;
}

SymNode *symnode_min(SymNode *root){
    if (root == NULL){
        return NULL;
    }
    while (root->left != NULL){
        root = root->left;
    }
    return root;
}

void symnode_delete(SymNode *root, char *key){
    if (root == NULL){
        return;
    }
    if (strcmp(key, root->id) < 0){
        symnode_delete(root->left, key);
    } else if (strcmp(key, root->id) > 0){
        symnode_delete(root->right, key);
    } else {
        if (root->left == NULL){
            SymNode *temp = root->right;
            ptr_registry_remove(root);
            root = temp;
        } else if (root->right == NULL){
            SymNode *temp = root->left;
            ptr_registry_remove(root);
            root = temp;
        } else {
            SymNode *temp = symnode_min(root->right);
            root->id = temp->id;
            root->data = temp->data;
            symnode_delete(root->right, temp->id);
        }
    }
}

void symnode_free(SymNode *root){
    if (root == NULL){
        return;
    }
    symnode_free(root->left);
    symnode_free(root->right);
    ptr_registry_remove(root);
}

void iter_print(SymNode *root){
    static char Dtypes[4][10] = {"VARIABLE", "CONSTANT", "FUNCTION", "BLOCK"};
    static char STypes[6][10] = {"UNDEFINED", "I32", "F64", "U8", "SLITERAL", "FVOID"};
    if (root == NULL){
        return;
    }
    iter_print(root->left);
    DEBUG_PRINT("\n--------| Function: |--------\n");
    DEBUG_PRINT("| id: '%s'    | ObjType: %s   | isDefined: %d |\n", root->id, Dtypes[root->ObjType], root->isDefined);
    if (root->data != NULL){
        DEBUG_PRINT("->| DataType: %s    | Nullable: %d    | Slice: %d |\n", STypes[root->data->DataType], root->data->nullable, root->data->slice);
    }
    if (root->ObjType == FUNCTION){
        Param *tmp = root->params;
        while (tmp != NULL){
            DEBUG_PRINT("-->| Param: '%s' | Type: %s | Nullable: %d | Slice: %d |\n", tmp->id, STypes[tmp->type->DataType], tmp->type->nullable, tmp->type->slice);
            tmp = tmp->next;
        }
    }
    if (root->context_next != NULL){
        DEBUG_PRINT("--------\n| Context:\n--------\n");
        SymNode *tmp = root->context_next;
        while (tmp != NULL){
            DEBUG_PRINT("\t| id: '%s' | ObjType: %s | isDefined: %d | DataType: %s | Nullable: %d | Slice: %d | USED: %d |\n", tmp->id, Dtypes[tmp->ObjType], tmp->isDefined, STypes[tmp->data->DataType], tmp->data->nullable, tmp->data->slice, tmp->is_used);
            tmp = tmp->context_next;
        }
    }
    DEBUG_PRINT("\n");
    iter_print(root->right);
}

void symnode_print(SymNode *root){
    DEBUG_PRINT("\n\n--------Symtable:--------\n");
    iter_print(root);
}
