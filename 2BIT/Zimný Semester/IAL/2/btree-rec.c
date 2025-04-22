/*
 * Binární vyhledávací strom — rekurzivní varianta
 *
 * S využitím datových typů ze souboru btree.h a připravených koster funkcí
 * implementujte binární vyhledávací strom pomocí rekurze.
 */

#include "../btree.h"
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
 * Funkci implementujte rekurzivně bez použité vlastních pomocných funkcí.
 */
bool bst_search(bst_node_t *tree, char key, bst_node_content_t **value)
{
  
  // Overíme či má zmysel hľadať.
  if (tree == NULL) {
    return false;
  }

  // Hľadmáme uzol
  char curr_key = tree -> key;

  if (curr_key == key) {
    *value = &(tree ->content);
    return true;
  }

  // Rozhodneme sa, kde ďalej hľadať
  if (curr_key > key) {
    return bst_search(tree -> left, key, value);
  }

  if (curr_key < key) {
    return bst_search(tree -> right, key, value);
  }

  return !true;
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
 * Funkci implementujte rekurzivně bez použití vlastních pomocných funkcí.
 */
void bst_insert(bst_node_t **tree, char key, bst_node_content_t value)
{

  // Ak nemáme strom, vytvoríme nový uzol
  if (*tree == NULL) {
    bst_node_t *new = (bst_node_t *) malloc( sizeof( bst_node_t));
    if (new == NULL) {
      return;
    }
    new -> key = key;
    new -> content = value;
    new -> left = NULL;
    new -> right = NULL;

    *tree = new;
    return;
  }

  // Hľadáme pozíciu pre nový uzol
  char curr_key = (*tree) -> key;
  if (key == curr_key) {
    if ((*tree) -> content . value){
      free((*tree) -> content . value);
    }
    (*tree) -> content = value;
    return;
  }

  // Rozhodneme sa, kde vložiť uzol
  if (key < curr_key) {
    bst_insert(&(*tree) -> left, key, value);
  } else if (key > curr_key) {
    bst_insert(&(*tree) -> right, key, value);
  }
}

/*
 * Pomocná funkce která nahradí uzel nejpravějším potomkem.
 *
 * Klíč a hodnota uzlu target budou nahrazeny klíčem a hodnotou nejpravějšího
 * uzlu podstromu tree. Nejpravější potomek bude odstraněný. Funkce korektně
 * uvolní všechny alokované zdroje odstraněného uzlu.
 *
 * Funkce předpokládá, že hodnota tree není NULL.
 *
 * Tato pomocná funkce bude využitá při implementaci funkce bst_delete.
 *
 * Funkci implementujte rekurzivně bez použití vlastních pomocných funkcí.
 */
void bst_replace_by_rightmost(bst_node_t *target, bst_node_t **tree)
{

  // Ak nemáme strom, nemáme čo robiť
  if (*tree == NULL) {
    return;
  }

  // Hľadáme najpravší uzol
  if ((*tree) -> right == NULL) {
    target -> key = (*tree) -> key;
    if (target -> content . value){
      free (target -> content . value);
    }
    target -> content = (*tree) -> content;

    // Nahradíme uzol
    bst_node_t *tmp = *tree;
    *tree = (*tree) -> left;
    free(tmp);
    return;
  }

  bst_replace_by_rightmost(target, &(*tree) -> right);
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
 * Funkci implementujte rekurzivně pomocí bst_replace_by_rightmost a bez
 * použití vlastních pomocných funkcí.
 */
void bst_delete(bst_node_t **tree, char key)
{

  // Ak nemáme strom, nemáme čo mazať
  if (*tree == NULL) {
    return;
  }

  // Zistíme, či má uzol susedov
  char curr_key = (*tree) -> key;
  bool left = (*tree) -> left != NULL;
  bool right = (*tree) -> right != NULL;

  // Hľadáme uzol
  if (key == curr_key) {
    // Uzol nemá deti
    if (left == false && right == false) {
      if ((*tree) -> content . value) {
        free((*tree) -> content . value);
      }
      free (*tree);
      *tree = NULL;
      return;
    }

    // Uzol má jedno dieťa, musíme zistiť na ktorej strane
    if (left ^ right) {
      bst_node_t *tmp = *tree;
      if (left) {
        *tree = (*tree) -> left;
      } else {
        *tree = (*tree) -> right;
      }
      if (tmp -> content . value){
        free (tmp -> content . value);
      }
      free (tmp);
      return;
    }

    // Uzol má dve deti
    if (left && right) {
      bst_replace_by_rightmost(*tree, &(*tree) -> left);
      return;
    }
  }

  // Posúvame sa ďalej po strome
  if (key < curr_key && left == true) {
    bst_delete(&(*tree) -> left, key);
    return;
  }

  if (key > curr_key && right == true){
    bst_delete(&(*tree) -> right, key);
    return;
  }

}

/*
 * Zrušení celého stromu.
 *
 * Po zrušení se celý strom bude nacházet ve stejném stavu jako po
 * inicializaci. Funkce korektně uvolní všechny alokované zdroje rušených
 * uzlů.
 *
 * Funkci implementujte rekurzivně bez použití vlastních pomocných funkcí.
 */
void bst_dispose(bst_node_t **tree)
{

  // Ak nemáme strom, nemáme čo mazať
  if (*tree == NULL) {
    return;
  }

  // Prechádzame strom
  bst_dispose(&(*tree) -> left);
  bst_dispose(&(*tree) -> right);

  // Uvoľníme uzol
  if ((*tree) -> content . value){
    free ((*tree) -> content . value);
  }
  free ( *tree);

  *tree = NULL;
}

/*
 * Preorder průchod stromem.
 *
 * Pro aktuálně zpracovávaný uzel zavolejte funkci bst_add_node_to_items.
 *
 * Funkci implementujte rekurzivně bez použití vlastních pomocných funkcí.
 */
void bst_preorder(bst_node_t *tree, bst_items_t *items)
{

  // Ak nemáme strom, nemáme čo prechádzať
  if (tree == NULL) {
    return;
  }

  // Prechádzame stromom
  bst_add_node_to_items(tree, items);
  bst_preorder(tree -> left, items);
  bst_preorder(tree -> right, items);

}

/*
 * Inorder průchod stromem.
 *
 * Pro aktuálně zpracovávaný uzel zavolejte funkci bst_add_node_to_items.
 *
 * Funkci implementujte rekurzivně bez použití vlastních pomocných funkcí.
 */
void bst_inorder(bst_node_t *tree, bst_items_t *items)
{

  // Ak nemáme strom, nemáme čo prechádzať
  if (tree == NULL) {
    return;
  }

  // Prechádzame stromom
  bst_inorder(tree -> left, items);
  bst_add_node_to_items(tree, items);
  bst_inorder(tree -> right, items);
}

/*
 * Postorder průchod stromem.
 *
 * Pro aktuálně zpracovávaný uzel zavolejte funkci bst_add_node_to_items.
 *
 * Funkci implementujte rekurzivně bez použití vlastních pomocných funkcí.
 */
void bst_postorder(bst_node_t *tree, bst_items_t *items)
{

  // Ak nemáme strom, nemáme čo prechádzať
  if (tree == NULL) {
    return;
  }


  // Prechádzame stromom
  bst_postorder(tree -> left, items);
  bst_postorder(tree -> right, items);
  bst_add_node_to_items(tree, items);
}
