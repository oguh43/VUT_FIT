/**
 * FIT VUT - IFJ project 2024
 *
 * @file ptr_registry.h
 *
 * @brief Registry of all pointers to be freed at the end of execution
 *
 * @author Hugo Bohácsek (xbohach00)
 */

#ifndef PTR_REGISTRY_H
#define PTR_REGISTRY_H

#include <stdlib.h>


/**
 * @brief Inicializuje registry.
*/

void ptr_registry_init();


/**
 * @brief Pridá ukazateľ do registry.
 * 
 * @param ptr Ukazateľ na pridanie.
*/
void ptr_registry_add(void *ptr);

/**
 * @brief Realokuje ukazateľ v registry.
 * 
 * @param ptr Ukazateľ na realokáciu.
 * @param size Nová veľkosť.
 */
void *ptr_registry_realloc(void *ptr, size_t size);

/**
 * @brief Odstráni ukazateľ z registry.
 * 
 * @param ptr Ukazateľ na odstránenie.
 */
void ptr_registry_remove(void *ptr);

/**
 * @brief Uvoľní všetky ukazatele z registry.
 */
void ptr_registry_cleanup();

#endif // PTR_REGISTRY_H
