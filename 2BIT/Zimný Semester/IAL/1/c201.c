/* c201.c **********************************************************************
** Téma: Jednosměrný lineární seznam
**
**                     Návrh a referenční implementace: Petr Přikryl, říjen 1994
**                                          Úpravy: Andrea Němcová listopad 1996
**                                                   Petr Přikryl, listopad 1997
**                                Přepracované zadání: Petr Přikryl, březen 1998
**                                  Přepis do jazyka C: Martin Tuček, říjen 2004
**                                              Úpravy: Kamil Jeřábek, září 2020
**                                                    Daniel Dolejška, září 2021
**                                                    Daniel Dolejška, září 2022
**
** Implementujte abstraktní datový typ jednosměrný lineární seznam.
** Užitečným obsahem prvku seznamu je celé číslo typu int.
** Seznam bude jako datová abstrakce reprezentován proměnnou typu List.
** Definici konstant a typů naleznete v hlavičkovém souboru c201.h.
**
** Vaším úkolem je implementovat následující operace, které spolu s výše
** uvedenou datovou částí abstrakce tvoří abstraktní datový typ List:
**
**      List_Dispose ....... zrušení všech prvků seznamu,
**      List_Init .......... inicializace seznamu před prvním použitím,
**      List_InsertFirst ... vložení prvku na začátek seznamu,
**      List_First ......... nastavení aktivity na první prvek,
**      List_GetFirst ...... vrací hodnotu prvního prvku,
**      List_DeleteFirst ... zruší první prvek seznamu,
**      List_DeleteAfter ... ruší prvek za aktivním prvkem,
**      List_InsertAfter ... vloží nový prvek za aktivní prvek seznamu,
**      List_GetValue ...... vrací hodnotu aktivního prvku,
**      List_SetValue ...... přepíše obsah aktivního prvku novou hodnotou,
**      List_Next .......... posune aktivitu na další prvek seznamu,
**      List_IsActive ...... zjišťuje aktivitu seznamu.
**
** Nemusíte ošetřovat situaci, kdy místo legálního ukazatele na seznam předá
** někdo jako parametr hodnotu NULL.
**
** Svou implementaci vhodně komentujte!
**/

#include "c201.h"

#include <stdio.h> // printf
#include <stdlib.h> // malloc, free

bool error_flag;
bool solved;

/**
 * Vytiskne upozornění na to, že došlo k chybě. Nastaví error_flag na logickou 1.
 * Tato funkce bude volána z některých dále implementovaných operací.
 */
void List_Error(void) {
	printf("*ERROR* The program has performed an illegal operation.\n");
	error_flag = true;
}

/**
 * Provede inicializaci seznamu list před jeho prvním použitím (tzn. žádná
 * z následujících funkcí nebude volána nad neinicializovaným seznamem).
 * Tato inicializace se nikdy nebude provádět nad již inicializovaným
 * seznamem, a proto tuto možnost neošetřujte. Vždy předpokládejte,
 * že neinicializované proměnné mají nedefinovanou hodnotu.
 *
 * @param list Ukazatel na strukturu jednosměrně vázaného seznamu
 */
void List_Init( List *list ) {
	list -> firstElement = NULL;
	list -> activeElement = NULL;
	return;
}

/**
 * Zruší všechny prvky seznamu list a uvede seznam list do stavu, v jakém se nacházel
 * po inicializaci. Veškerá paměť používaná prvky seznamu list bude korektně
 * uvolněna voláním operace free.
 *
 * @param list Ukazatel na inicializovanou strukturu jednosměrně vázaného seznamu
 **/
void List_Dispose( List *list ) {
	
	// List nesmie byť aktívny.
	list -> activeElement = NULL;

	// Dookola priraďujeme 0. prvok do dočasnej premennej a posúvame začiatok listu. Nakoniec premennú `tmp` uvoľníme.
	ListElementPtr tmp;
	while (list -> firstElement != NULL) {
		tmp = list -> firstElement;
		list -> firstElement = tmp -> nextElement;
		free(tmp);
	};
	return;
}

/**
 * Vloží prvek s hodnotou data na začátek seznamu list.
 * V případě, že není dostatek paměti pro nový prvek při operaci malloc,
 * volá funkci List_Error().
 *
 * @param list Ukazatel na inicializovanou strukturu jednosměrně vázaného seznamu
 * @param data Hodnota k vložení na začátek seznamu
 */
void List_InsertFirst( List *list, int data ) {
	
	// Alokácia štruktúry nového prvku zoznamu.
	ListElementPtr newEl = (ListElementPtr) malloc( sizeof( struct ListElement));
	if ( newEl == NULL ){
		List_Error();
		return;
	}

	// Priradenie dát.
	newEl->data = data;

	// Vloženie prvku na 0. miesto.
	newEl -> nextElement = list -> firstElement;
	list -> firstElement = newEl;

	return;
}

/**
 * Nastaví aktivitu seznamu list na jeho první prvek.
 * Funkci implementujte jako jediný příkaz, aniž byste testovali,
 * zda je seznam list prázdný.
 * @param list Ukazatel na inicializovanou strukturu jednosměrně vázaného seznamu
 */
void List_First( List *list ) {
	list -> activeElement = list -> firstElement; return;
}

/**
 * Prostřednictvím parametru dataPtr vrátí hodnotu prvního prvku seznamu list.
 * Pokud je seznam list prázdný, volá funkci List_Error().
 *
 * @param list Ukazatel na inicializovanou strukturu jednosměrně vázaného seznamu
 * @param dataPtr Ukazatel na cílovou proměnnou
 */
void List_GetFirst( List *list, int *dataPtr ) {
	
	// Kontrola prázdnosti zoznamu.
	if ( list -> firstElement == NULL ){
		List_Error();
		return;
	};

	// Priradenie prvku do ukazateľa.
	*dataPtr = list -> firstElement -> data;
	return;
}

/**
 * Zruší první prvek seznamu list a uvolní jím používanou paměť.
 * Pokud byl rušený prvek aktivní, aktivita seznamu se ztrácí.
 * Pokud byl seznam list prázdný, nic se neděje.
 *
 * @param list Ukazatel na inicializovanou strukturu jednosměrně vázaného seznamu
 */
void List_DeleteFirst( List *list ) {
	
	// Kontorla prázdnosti zoznamu.
	if (list -> firstElement == NULL) {
		return;
	}

	// Vyberieme prvok, ktorý chceme vymazať.
	ListElementPtr del = list -> firstElement;

	// Kontrola aktivity, a jej prípadné nulovanie.
	if (list -> activeElement == del) {
		list -> activeElement = NULL;
	}

	// Posunutie zoznamu a uvoľnenie.
	list -> firstElement = del -> nextElement;
	free(del);

	return;
}

/**
 * Zruší prvek seznamu list za aktivním prvkem a uvolní jím používanou paměť.
 * Pokud není seznam list aktivní nebo pokud je aktivní poslední prvek seznamu list,
 * nic se neděje.
 *
 * @param list Ukazatel na inicializovanou strukturu jednosměrně vázaného seznamu
 */
void List_DeleteAfter( List *list ) {
	
	// Kontrola či existuje aktívny prvok, a či nie je posledný. Kontrola prázdnosti zoznamu môže byť vynechaná, keďže by sa zoznam nemal nikdy dostať do stavu, že je prázdny a má aktívny prvok.
	if (list -> firstElement == NULL || list -> activeElement == NULL || list -> activeElement -> nextElement == NULL) {
		return;
	}

	// Vyberieme prvok, ktorý chceme vymazať.
	ListElementPtr del = list -> activeElement -> nextElement;

	// Posun prvkov v zozname a uvoľnenie.
	list -> activeElement -> nextElement = del -> nextElement;
	free(del);

	return;
}

/**
 * Vloží prvek s hodnotou data za aktivní prvek seznamu list.
 * Pokud nebyl seznam list aktivní, nic se neděje!
 * V případě, že není dostatek paměti pro nový prvek při operaci malloc,
 * zavolá funkci List_Error().
 *
 * @param list Ukazatel na inicializovanou strukturu jednosměrně vázaného seznamu
 * @param data Hodnota k vložení do seznamu za právě aktivní prvek
 */
void List_InsertAfter( List *list, int data ) {
	
	// Kontrola či existuje aktívny prvok.
	if (list -> activeElement == NULL){
		return;
	}

	// Alokácia nového prvku.
	ListElementPtr newEl = ( ListElementPtr ) malloc( sizeof( struct ListElement));
	if ( newEl == NULL ){
		List_Error();
		return;
	}

	// Priradenie dát novému prvku a jeho vsunutie do zoznamu.
	newEl -> data = data;
	newEl -> nextElement = list -> activeElement -> nextElement;
	list -> activeElement -> nextElement = newEl;

	return;
}

/**
 * Prostřednictvím parametru dataPtr vrátí hodnotu aktivního prvku seznamu list.
 * Pokud seznam není aktivní, zavolá funkci List_Error().
 *
 * @param list Ukazatel na inicializovanou strukturu jednosměrně vázaného seznamu
 * @param dataPtr Ukazatel na cílovou proměnnou
 */
void List_GetValue( List *list, int *dataPtr ) {
	
	// Kontrola či existuje aktívny prvok.
	if (list -> activeElement == NULL) {
		List_Error();
		return;
	}

	// Vloženie hodnoty aktívneho prvku do ukazateľa.
	*dataPtr = list -> activeElement -> data;
	
	return;
}

/**
 * Přepíše data aktivního prvku seznamu list hodnotou data.
 * Pokud seznam list není aktivní, nedělá nic!
 *
 * @param list Ukazatel na inicializovanou strukturu jednosměrně vázaného seznamu
 * @param data Nová hodnota právě aktivního prvku
 */
void List_SetValue( List *list, int data ) {
	
	// Kontrola či existuje aktívny prvok.
	if (list -> activeElement == NULL) {
		return;
	}

	// Prepíšeme dáta aktívneho prvku.
	list -> activeElement -> data = data;

	return;
}

/**
 * Posune aktivitu na následující prvek seznamu list.
 * Všimněte si, že touto operací se může aktivní seznam stát neaktivním.
 * Pokud není předaný seznam list aktivní, nedělá funkce nic.
 *
 * @param list Ukazatel na inicializovanou strukturu jednosměrně vázaného seznamu
 */
void List_Next( List *list ) {
	
	// Kontrola či existuje aktívny prvok.
	if (list -> activeElement == NULL) {
		return;
	}

	// Posunutie aktivity na následujúci prvok.
	list -> activeElement = list -> activeElement -> nextElement;

	return;
}

/**
 * Je-li seznam list aktivní, vrací nenulovou hodnotu, jinak vrací 0.
 * Tuto funkci je vhodné implementovat jedním příkazem return.
 *
 * @param list Ukazatel na inicializovanou strukturu jednosměrně vázaného seznamu
 */
int List_IsActive( List *list ) {
	return list -> activeElement != NULL;
}

/* Konec c201.c */
