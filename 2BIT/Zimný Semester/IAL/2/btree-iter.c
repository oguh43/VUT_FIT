/*
 * Binární vyhledávací strom — iterativní varianta
 *
 * S využitím datových typů ze souboru btree.h, zásobníku ze souboru stack.h
 * a připravených koster funkcí implementujte binární vyhledávací
 * strom bez použití rekurze.
 */

#include "../btree.h"
#include "stack.h"
#include <stdio.h>
#include <stdlib.h>

/*
 * Inicializace stromu.
 *
 * Uživatel musí zajistit, že inicializace se nebude opakovaně volat nad
 * inicializovaným stromem. V opačném případě může dojít k úniku paměti (memory
 * leak). Protože neinicializovaný ukazatel má nedefinovanou hodnotu, není
 * možné toto detekovat ve funkci.
 */
void bst_init(bst_node_t **tree)
{

  // Nastavíeme ukazateľ na NULL
  *tree = NULL;
}

/*
 * Vyhledání uzlu v stromu.
 *
 * V případě úspěchu vrátí funkce hodnotu true a do proměnné value zapíše
 * ukazatel na obsah daného uzlu. V opačném případě funkce vrátí hodnotu false a proměnná
 * value zůstává nezměněná.
 *
 * Funkci implementujte iterativně bez použité vlastních pomocných funkcí.
 */
bool bst_search(bst_node_t *tree, char key, bst_node_content_t **value)
{
  
  // Overíme či má zmysel hľadať.
  if (tree == NULL){
    return false;
  }


  bst_node_t *curr = tree;
  bool break_out = false;

  while ( break_out == false ) {
    
    // Pripravíme si, kde má zmysel hľadať.
    char current_key = curr -> key;
    bool left = curr -> left != NULL;
    bool right = curr -> right != NULL;

    if (key == current_key) {
      *value = &curr -> content;
      break_out = true;

        // Pokiaľ sme nenašli zhodu, posúvame sa po strome ďalej
    } else if (key < current_key && left) {
      curr = curr -> left;
    } else if (key > current_key && right) {
      curr = curr -> right;
    } else {
      break;
    }
  }
  
  return break_out;
}

/*
 * Vložení uzlu do stromu.
 *
 * Pokud uzel se zadaným klíče už ve stromu existuje, nahraďte jeho hodnotu.
 * Jinak vložte nový listový uzel.
 *
 * Výsledný strom musí splňovat podmínku vyhledávacího stromu — levý podstrom
 * uzlu obsahuje jenom menší klíče, pravý větší.
 *
 * Funkci implementujte iterativně bez použití vlastních pomocných funkcí.
 */
void bst_insert(bst_node_t **tree, char key, bst_node_content_t value)
{
  
  // Naalokujeme nový uzol
  bst_node_t *new = ( bst_node_t * ) malloc( sizeof( bst_node_t));
  if (new == NULL){
    return;
  }

  // Priradíme vyžadované hodnoty
  new -> key = key;
  new -> content = value;

  // Lístoček nemá sónsédov
  new -> left = NULL;
  new -> right = NULL;

  // Sme korienoq?
  if (*tree == NULL) {
    *tree = new;
    return;
  }

  bst_node_t *curr = *tree;
  bool break_out = false;

  // Hľadáme pozíciu pre nový uzol
  while (break_out == false){
    // Pripravíme si, kde má zmysel hľadať.
    char curr_key = curr -> key;
    bool left = curr -> left != NULL;
    bool right = curr -> right != NULL;

    // Ak sa nám podarilo nájsť uzol s rovnakým kľúčom, prepíšeme hodnotu
    if (key == curr_key){

      free(curr -> content . value);
      curr -> content = value;
      free(new);
      break_out = true;
    }

    // Ak je kľúč menší, posunieme sa do ľavého podstromu
    if (key < curr_key) {
      if (left) {
        curr = curr -> left;
      } else {
        curr -> left = new;
        break_out = true;
      }
    }

    // Ak je kľúč väčší, posunieme sa do pravého podstromu
    if (key > curr_key) {
      if (right) {
        curr = curr -> right;
      } else {
        curr -> right = new;
        break_out = true;
      }
    }
  }
}

/*
 * Pomocná funkce která nahradí uzel nejpravějším potomkem.
 *
 * Klíč a hodnota uzlu target budou nahrazené klíčem a hodnotou nejpravějšího
 * uzlu podstromu tree. Nejpravější potomek bude odstraněný. Funkce korektně
 * uvolní všechny alokované zdroje odstraněného uzlu.
 *
 * Funkce předpokládá, že hodnota tree není NULL.
 *
 * Tato pomocná funkce bude využita při implementaci funkce bst_delete.
 *
 * Funkci implementujte iterativně bez použití vlastních pomocných funkcí.
 */
void bst_replace_by_rightmost(bst_node_t *target, bst_node_t **tree)
{
  
  
  bst_node_t *curr = *tree;
  bst_node_t *parent = NULL;
  bool break_out = false;

  // Hľadáme najpravší uzol
  while (break_out == false) {
    // Ak sme našli najpravší uzol
    if (curr -> right == NULL) {
      // Nahradíme hodnoty
      target -> key = curr -> key;
      free(target -> content . value);
      target -> content = curr -> content;

      // Zistíme či sme novým koreňom
      if (parent == NULL) {
        *tree = curr -> left;
      } else {
        parent -> right = curr -> left;
      }

      free(curr);

      break_out = true;
    } else {
      parent = curr;
      curr = curr -> right;
    }
  }
}

/*
 * Odstranění uzlu ze stromu.
 *
 * Pokud uzel se zadaným klíčem neexistuje, funkce nic nedělá.
 * Pokud má odstraněný uzel jeden podstrom, zdědí ho rodič odstraněného uzlu.
 * Pokud má odstraněný uzel oba podstromy, je nahrazený nejpravějším uzlem
 * levého podstromu. Nejpravější uzel nemusí být listem.
 *
 * Funkce korektně uvolní všechny alokované zdroje odstraněného uzlu.
 *
 * Funkci implementujte iterativně pomocí bst_replace_by_rightmost a bez
 * použití vlastních pomocných funkcí.
 */
void bst_delete(bst_node_t **tree, char key)
{

  // Ak nemáme strom, nemáme čo mazať
  if (*tree == NULL) {
    return;
  }

  // Pripravíme si ukazateľ na koreň a rodiča
  bst_node_t *curr = *tree;
  bst_node_t *parent = NULL;
  bool break_out = false;

  // Hľadáme uzol na zmazanie
  while (break_out == false) {
    if (curr == NULL) {
      return;
    }

    // Pripravíme si, kde má zmysel hľadať.
    char curr_key = curr -> key;
    bool left = curr -> left != NULL;
    bool right = curr -> right != NULL;

    // Ak sme našli uzol na zmazanie
    if (key == curr_key) {
      
      // Kontrolujeme, kde sa nachádza nájdený uzol
      bool on_left = parent != NULL && parent -> left -> key == curr_key;
      bool on_right = parent != NULL && parent -> right -> key == curr_key;

      // Uzol na mazanie nemá deti
      if (left == false && right == false) {
        free (curr -> content . value);
        free (curr);
        if (on_left) {
          parent -> left = NULL;
        } else if (on_right) {
          parent -> right = NULL;
        } else {
          *tree = NULL;
        }
      }

      // Uzol na mazanie má jedno dieťa
      if (left ^ right) {
        bst_node_t *tmp;
        if (left) {
          tmp = curr -> left;
        } else {
          tmp = curr -> right;
        }

        free (curr -> content . value);
        free (curr);

        // Kontrolujeme, kde sa nachádza nájdený uzol
        if (on_left) {
          parent -> left = tmp;
        } else if (on_right) {
          parent -> right = tmp;
        } else {
          *tree = tmp;
        }
      }

      // Uzol na mazanie má dve deti
      if (left && right) {
        bst_replace_by_rightmost(curr, &(curr -> left));
      }
      break_out = true;
    }

    // Ak sme nenašli uzol na zmazanie, posúvame sa po strome ďalej
    if (key > curr_key) {
      parent = curr;
      curr = curr -> right;
    } else if (key < curr_key) {
      parent = curr;
      curr = curr -> left;
    }
  }
}

/*
 * Zrušení celého stromu.
 *
 * Po zrušení se celý strom bude nacházet ve stejném stavu jako po
 * inicializaci. Funkce korektně uvolní všechny alokované zdroje rušených
 * uzlů.
 *
 * Funkci implementujte iterativně s pomocí zásobníku a bez použití
 * vlastních pomocných funkcí.
 */
void bst_dispose(bst_node_t **tree)
{
  
  // Ak nemáme strom, nemáme čo mazať
  if (*tree == NULL) {
    return;
  }

  // Pripravíme si zásobník
  stack_bst_t *stack = (stack_bst_t *) malloc( sizeof( stack_bst_t));
  if (stack == NULL) {
    return;
  }

  stack_bst_init(stack);
  stack_bst_push(stack, *tree);

  // Postupné mazanie uzlov
  while (stack_bst_empty(stack) == false) {
    bst_node_t *curr = stack_bst_pop(stack);
    if (curr == NULL) {
      continue;
    }

    // Uložíme si deti uzla
    if (curr -> left != NULL) {
      stack_bst_push(stack, curr -> left);
    }

    if (curr -> right != NULL) {
      stack_bst_push(stack, curr -> right);
    }


    free (curr -> content . value);
    free (curr);
  }

  *tree = NULL;
  free( stack);
}

/*
 * Pomocná funkce pro iterativní preorder.
 *
 * Prochází po levé větvi k nejlevějšímu uzlu podstromu.
 * Nad zpracovanými uzly zavolá bst_add_node_to_items a uloží je do zásobníku uzlů.
 *
 * Funkci implementujte iterativně s pomocí zásobníku a bez použití
 * vlastních pomocných funkcí.
 */
void bst_leftmost_preorder(bst_node_t *tree, stack_bst_t *to_visit, bst_items_t *items)
{

  bst_node_t *curr = tree;

  // Prechádzame po ľavej vetve
  while (curr != NULL) {
    bst_add_node_to_items(curr, items);

    stack_bst_push(to_visit, curr);

    curr = curr -> left;
  }
}

/*
 * Preorder průchod stromem.
 *
 * Pro aktuálně zpracovávaný uzel zavolejte funkci bst_add_node_to_items.
 *
 * Funkci implementujte iterativně pomocí funkce bst_leftmost_preorder a
 * zásobníku uzlů a bez použití vlastních pomocných funkcí.
 */
void bst_preorder(bst_node_t *tree, bst_items_t *items)
{

  // Ak nemáme strom, nemáme čo prechádzať
  if (tree == NULL) {
    return;
  }

  // Pripravíme si zásobník
  stack_bst_t *stack = (stack_bst_t *) malloc( sizeof( stack_bst_t));
  if (stack == NULL) {
    return;
  }

  stack_bst_init(stack);

  bst_node_t *curr = tree;
  bst_leftmost_preorder(curr, stack, items);

  // Prechádzame zásobník
  while (stack_bst_empty(stack) == false) {
    curr = stack_bst_pop(stack);

    if (curr -> right != NULL) {
      bst_leftmost_preorder(curr -> right, stack, items);
    }
  }

  free (stack);
}

/*
 * Pomocná funkce pro iterativní inorder.
 *
 * Prochází po levé větvi k nejlevějšímu uzlu podstromu a ukládá uzly do
 * zásobníku uzlů.
 *
 * Funkci implementujte iterativně s pomocí zásobníku a bez použití
 * vlastních pomocných funkcí.
 */
void bst_leftmost_inorder(bst_node_t *tree, stack_bst_t *to_visit)
{

  bst_node_t *curr = tree;

  // Prechádzame po ľavej vetve
  while (curr != NULL) {
    stack_bst_push(to_visit, curr);
    curr = curr -> left;
  }
}

/*
 * Inorder průchod stromem.
 *
 * Pro aktuálně zpracovávaný uzel zavolejte funkci bst_add_node_to_items.
 *
 * Funkci implementujte iterativně pomocí funkce bst_leftmost_inorder a
 * zásobníku uzlů a bez použití vlastních pomocných funkcí.
 */
void bst_inorder(bst_node_t *tree, bst_items_t *items)
{

  // Ak nemáme strom, nemáme čo prechádzať
  if (tree == NULL) {
    return;
  }

  // Pripravíme si zásobník
  stack_bst_t *stack = (stack_bst_t *) malloc( sizeof( stack_bst_t));
  if (stack == NULL) {
    return;
  }

  stack_bst_init(stack);

  bst_node_t *curr = tree;
  bst_leftmost_inorder(curr, stack);

  // Prechádzame zásobník
  while (stack_bst_empty(stack) == false) {
    curr = stack_bst_pop(stack);
    bst_add_node_to_items(curr, items);

    if (curr -> right != NULL) {
      bst_leftmost_inorder(curr -> right, stack);
    }
  }

  free(stack);
}

/*
 * Pomocná funkce pro iterativní postorder.
 *
 * Prochází po levé větvi k nejlevějšímu uzlu podstromu a ukládá uzly do
 * zásobníku uzlů. Do zásobníku bool hodnot ukládá informaci, že uzel
 * byl navštíven poprvé.
 *
 * Funkci implementujte iterativně pomocí zásobníku uzlů a bool hodnot a bez použití
 * vlastních pomocných funkcí.
 */
void bst_leftmost_postorder(bst_node_t *tree, stack_bst_t *to_visit,
                            stack_bool_t *first_visit)
{

  bst_node_t *curr = tree;

  // Prechádzame po ľavej vetve
  while (curr != NULL) {
    stack_bst_push(to_visit, curr);
    stack_bool_push(first_visit, true);

    curr = curr -> left;
  }
}

/*
 * Postorder průchod stromem.
 *
 * Pro aktuálně zpracovávaný uzel zavolejte funkci bst_add_node_to_items.
 *
 * Funkci implementujte iterativně pomocí funkce bst_leftmost_postorder a
 * zásobníku uzlů a bool hodnot a bez použití vlastních pomocných funkcí.
 */
void bst_postorder(bst_node_t *tree, bst_items_t *items)
{

  // Ak nemáme strom, nemáme čo prechádzať
  if (tree == NULL) {
    return;
  }

  // Pripravíme si zásobníky
  stack_bst_t *stack = (stack_bst_t *) malloc( sizeof( stack_bst_t));
  if (stack == NULL) {
    return;
  }
  stack_bst_init(stack);

  stack_bool_t *bool_stack = (stack_bool_t *) malloc( sizeof( stack_bool_t));
  if (bool_stack == NULL) {
    return;
  }
  stack_bool_init(bool_stack);


  bst_node_t *curr = tree;
  bst_leftmost_postorder(curr, stack, bool_stack);

  // Prechádzame zásobník, pričom si dávame poozor na to, či už sme uzol navštívili
  while (stack_bst_empty(stack) == false) {
    curr = stack_bst_pop(stack);
    bool first = stack_bool_pop(bool_stack);

    if (first) {
      stack_bst_push(stack, curr);
      stack_bool_push(bool_stack, false);

      if (curr -> right != NULL) {
        bst_leftmost_postorder(curr -> right, stack, bool_stack);
      }
    } else {
      bst_add_node_to_items(curr, items);
    }
  }

  free(stack);
  free(bool_stack);
}
