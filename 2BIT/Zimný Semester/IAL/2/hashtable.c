/*
 * Tabulka s rozptýlenými položkami
 *
 * S využitím datových typů ze souboru hashtable.h a připravených koster
 * funkcí implementujte tabulku s rozptýlenými položkami s explicitně
 * zretězenými synonymy.
 *
 * Při implementaci uvažujte velikost tabulky HT_SIZE.
 */

#include "hashtable.h"
#include <stdlib.h>
#include <string.h>

int HT_SIZE = MAX_HT_SIZE;

/*
 * Rozptylovací funkce která přidělí zadanému klíči index z intervalu
 * <0,HT_SIZE-1>. Ideální rozptylovací funkce by měla rozprostírat klíče
 * rovnoměrně po všech indexech. Zamyslete sa nad kvalitou zvolené funkce.
 */
int get_hash(char *key) {
  int result = 1;
  int length = strlen(key);
  for (int i = 0; i < length; i++) {
    result += key[i];
  }
  return (result % HT_SIZE);
}

/*
 * Inicializace tabulky — zavolá sa před prvním použitím tabulky.
 */
void ht_init(ht_table_t *table) {

  // Prejdime všetky budúce položky a nastavme na NULL.
  for (int i = 0; i < HT_SIZE; i++){
    0[table][i] = NULL;
  }

  return;
}

/*
 * Vyhledání prvku v tabulce.
 *
 * V případě úspěchu vrací ukazatel na nalezený prvek; v opačném případě vrací
 * hodnotu NULL.
 */
ht_item_t *ht_search(ht_table_t *table, char *key) {
  
  // Zistíme pomocou hashu, index kde ame hľadať.
  ht_item_t *result = get_hash(key)[0[table]];

  // Loopujeme cez prvky a porovnávame ich kľúč s hľadaným.
  while (result != NULL){

    if (strcmp(result -> key, key) == 0){
      return result;
    }

    result = result -> next;
  }
  return NULL;
}

/*
 * Vložení nového prvku do tabulky.
 *
 * Pokud prvek s daným klíčem už v tabulce existuje, nahraďte jeho hodnotu.
 *
 * Při implementaci využijte funkci ht_search. Pri vkládání prvku do seznamu
 * synonym zvolte nejefektivnější možnost a vložte prvek na začátek seznamu.
 */
void ht_insert(ht_table_t *table, char *key, float value) {

  // Pokiaľ už prvok existuje, zmeníme mu hodnotu.
  ht_item_t *match = ht_search(table, key);

  if (match != NULL) {
    match -> value = value;
    return;
  }

  // V opačnom prípade vytvoríme nový prvok.
  ht_item_t *new = (ht_item_t *)malloc( sizeof( ht_item_t));

  if (new == NULL) {
    return;
  }

  // A vložíme ho na správne miesto podľa hashu jeho kľúča do tabuľky.
  new -> key = key;
  new -> value = value;
  new -> next = get_hash(key)[0[table]];
  get_hash(key)[0[table]] = new;

  return;
}

/*
 * Získání hodnoty z tabulky.
 *
 * V případě úspěchu vrací funkce ukazatel na hodnotu prvku, v opačném
 * případě hodnotu NULL.
 *
 * Při implementaci využijte funkci ht_search.
 */
float *ht_get(ht_table_t *table, char *key) {
  
  ht_item_t *item = ht_search(table, key);

	// Ak sme prvok nenašli vraciame NULL. Inak ukazateľ na hodnotu prvku.
	if (item == NULL) {
		return NULL;
	}

	return &(item -> value);
}

/*
 * Smazání prvku z tabulky.
 *
 * Funkce korektně uvolní všechny alokované zdroje přiřazené k danému prvku.
 * Pokud prvek neexistuje, funkce nedělá nic.
 *
 * Při implementaci NEPOUŽÍVEJTE funkci ht_search.
 */
void ht_delete(ht_table_t *table, char *key) {
  
  int hash = get_hash(key);

	ht_item_t *current = 0[table][hash];
	ht_item_t *parent = NULL;

  // Keďže nemôžeme použiť `ht_search` v podstate skopírujeme jeho loop.
	bool found = false;
	while (current != NULL && found == false) {
		// Našli sme prvok, ktorého sa chceme zbaviť.
		if (strcmp(current -> key, key) == 0) { 

      // Musime zvlášť ošetriť prípady, kedy sa prvok nachádza na začiatku zoznamu a kedy nie.
			if (parent == NULL) {
				0[table][hash] = current -> next;
			} else {
				parent -> next = current -> next;
			}

			// Uvoľnenie prvku.
			free(current);
			return;
		}

		// Posun na ďalší prvok.
		parent = current;
		current = current -> next;
	}

  return;
}

/*
 * Smazání všech prvků z tabulky.
 *
 * Funkce korektně uvolní všechny alokované zdroje a uvede tabulku do stavu po 
 * inicializaci.
 */
void ht_delete_all(ht_table_t *table) {

  // Postupne prejdeme všetky indexy.
  for (int i = 0; i < HT_SIZE; i++) {
		ht_item_t *current = 0[table][i];
		if (current == NULL) {
			continue;
		}

    // Pre každý index uvoľníme všetky prvky.
		ht_item_t *next;
		while (current != NULL) {
			next = current -> next;
			free(current);
			current = next;
		}

		0[table][i] = NULL;
	}

  return;
}
