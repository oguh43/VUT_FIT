/* ******************************* c206.c *********************************** */
/*  Předmět: Algoritmy (IAL) - FIT VUT v Brně                                 */
/*  Úkol: c206 - Dvousměrně vázaný lineární seznam                            */
/*  Návrh a referenční implementace: Bohuslav Křena, říjen 2001               */
/*  Vytvořil: Martin Tuček, říjen 2004                                        */
/*  Upravil: Kamil Jeřábek, září 2020                                         */
/*           Daniel Dolejška, září 2021                                       */
/*           Daniel Dolejška, září 2022                                       */
/* ************************************************************************** */
/*
** Implementujte abstraktní datový typ dvousměrně vázaný lineární seznam.
** Užitečným obsahem prvku seznamu je hodnota typu int. Seznam bude jako datová
** abstrakce reprezentován proměnnou typu DLList (DL znamená Doubly-Linked
** a slouží pro odlišení jmen konstant, typů a funkcí od jmen u jednosměrně
** vázaného lineárního seznamu). Definici konstant a typů naleznete
** v hlavičkovém souboru c206.h.
**
** Vaším úkolem je implementovat následující operace, které spolu s výše
** uvedenou datovou částí abstrakce tvoří abstraktní datový typ obousměrně
** vázaný lineární seznam:
**
**      DLL_Init ........... inicializace seznamu před prvním použitím,
**      DLL_Dispose ........ zrušení všech prvků seznamu,
**      DLL_InsertFirst .... vložení prvku na začátek seznamu,
**      DLL_InsertLast ..... vložení prvku na konec seznamu,
**      DLL_First .......... nastavení aktivity na první prvek,
**      DLL_Last ........... nastavení aktivity na poslední prvek,
**      DLL_GetFirst ....... vrací hodnotu prvního prvku,
**      DLL_GetLast ........ vrací hodnotu posledního prvku,
**      DLL_DeleteFirst .... zruší první prvek seznamu,
**      DLL_DeleteLast ..... zruší poslední prvek seznamu,
**      DLL_DeleteAfter .... ruší prvek za aktivním prvkem,
**      DLL_DeleteBefore ... ruší prvek před aktivním prvkem,
**      DLL_InsertAfter .... vloží nový prvek za aktivní prvek seznamu,
**      DLL_InsertBefore ... vloží nový prvek před aktivní prvek seznamu,
**      DLL_GetValue ....... vrací hodnotu aktivního prvku,
**      DLL_SetValue ....... přepíše obsah aktivního prvku novou hodnotou,
**      DLL_Previous ....... posune aktivitu na předchozí prvek seznamu,
**      DLL_Next ........... posune aktivitu na další prvek seznamu,
**      DLL_IsActive ....... zjišťuje aktivitu seznamu.
**
** Při implementaci jednotlivých funkcí nevolejte žádnou z funkcí
** implementovaných v rámci tohoto příkladu, není-li u funkce explicitně
** uvedeno něco jiného.
**
** Nemusíte ošetřovat situaci, kdy místo legálního ukazatele na seznam
** předá někdo jako parametr hodnotu NULL.
**
** Svou implementaci vhodně komentujte!
**
** Terminologická poznámka: Jazyk C nepoužívá pojem procedura.
** Proto zde používáme pojem funkce i pro operace, které by byly
** v algoritmickém jazyce Pascalovského typu implemenovány jako procedury
** (v jazyce C procedurám odpovídají funkce vracející typ void).
**
**/

#include "c206.h"

bool error_flag;
bool solved;

/**
 * Vytiskne upozornění na to, že došlo k chybě.
 * Tato funkce bude volána z některých dále implementovaných operací.
 */
void DLL_Error(void) {
	printf("*ERROR* The program has performed an illegal operation.\n");
	error_flag = true;
}

/**
 * Provede inicializaci seznamu list před jeho prvním použitím (tzn. žádná
 * z následujících funkcí nebude volána nad neinicializovaným seznamem).
 * Tato inicializace se nikdy nebude provádět nad již inicializovaným seznamem,
 * a proto tuto možnost neošetřujte.
 * Vždy předpokládejte, že neinicializované proměnné mají nedefinovanou hodnotu.
 *
 * @param list Ukazatel na strukturu dvousměrně vázaného seznamu
 */
void DLL_Init( DLList *list ) {
	list -> firstElement = NULL;
	list -> lastElement = NULL;
	list -> activeElement = NULL;
	list -> currentLength = 0;
}

/**
 * Zruší všechny prvky seznamu list a uvede seznam do stavu, v jakém se nacházel
 * po inicializaci.
 * Rušené prvky seznamu budou korektně uvolněny voláním operace free.
 *
 * @param list Ukazatel na inicializovanou strukturu dvousměrně vázaného seznamu
 */
void DLL_Dispose( DLList *list ) {

	// Postupné mazanie pomocou posúvania prvkov na prvú pozíciu.
	DLLElementPtr tmp;
	while(list -> firstElement != NULL){
		tmp = list -> firstElement;
		list -> firstElement = list -> firstElement -> nextElement;
		free(tmp); 
	}

	// Vynulujeme zoznam, aby bol ako bo volaní `DLL_Init`.
	list -> firstElement = NULL;
    list -> lastElement = NULL;
    list -> activeElement = NULL;
    list -> currentLength = 0;

	return;
}

/**
 * Vloží nový prvek na začátek seznamu list.
 * V případě, že není dostatek paměti pro nový prvek při operaci malloc,
 * volá funkci DLL_Error().
 *
 * @param list Ukazatel na inicializovanou strukturu dvousměrně vázaného seznamu
 * @param data Hodnota k vložení na začátek seznamu
 */
void DLL_InsertFirst( DLList *list, long data ) {

	if(list == NULL){
		DLL_Error();
		return;
	}
	
	DLLElementPtr new = ( DLLElementPtr ) malloc( sizeof( struct DLLElement));
	if(new == NULL){
		DLL_Error();
		return;
	}

	list -> currentLength++;

	// Vkladanie novej hodnoty. `previousElement` musíme rozhodnúť samostatne pre prázdny a neprázdny zoznam.
	new -> data = data;
	new -> nextElement = list -> firstElement;
	new -> previousElement = NULL;

	if(list -> firstElement != NULL){
		list -> firstElement -> previousElement = new;
	} else{
		list -> lastElement = new;
	}
	list -> firstElement = new;
	
	return;
}

/**
 * Vloží nový prvek na konec seznamu list (symetrická operace k DLL_InsertFirst).
 * V případě, že není dostatek paměti pro nový prvek při operaci malloc,
 * volá funkci DLL_Error().
 *
 * @param list Ukazatel na inicializovanou strukturu dvousměrně vázaného seznamu
 * @param data Hodnota k vložení na konec seznamu
 */
void DLL_InsertLast( DLList *list, long data ) {

	// To isté ako `DLL_InsertFirst`, ale pomocou `if` rozhodujeme o hodnote `nextElement`.
	if(list == NULL){
		DLL_Error();
		return;
	}
	
	DLLElementPtr new = ( DLLElementPtr ) malloc( sizeof( struct DLLElement));
	if(new == NULL){
		DLL_Error();
		return;
	}

	list -> currentLength++;

	new -> data = data;
    new -> nextElement = NULL;
	new -> previousElement = list -> lastElement;

	if(list -> lastElement != NULL){
		list -> lastElement -> nextElement = new;
	} else{
		list -> firstElement = new;
	}
	list -> lastElement = new;

	return;
}

/**
 * Nastaví první prvek seznamu list jako aktivní.
 * Funkci implementujte jako jediný příkaz, aniž byste testovali,
 * zda je seznam list prázdný.
 *
 * @param list Ukazatel na inicializovanou strukturu dvousměrně vázaného seznamu
 */
void DLL_First( DLList *list ) {
	list -> activeElement = list -> firstElement;
}

/**
 * Nastaví poslední prvek seznamu list jako aktivní.
 * Funkci implementujte jako jediný příkaz, aniž byste testovali,
 * zda je seznam list prázdný.
 *
 * @param list Ukazatel na inicializovanou strukturu dvousměrně vázaného seznamu
 */
void DLL_Last( DLList *list ) {
	list -> activeElement = list -> lastElement;
}

/**
 * Prostřednictvím parametru dataPtr vrátí hodnotu prvního prvku seznamu list.
 * Pokud je seznam list prázdný, volá funkci DLL_Error().
 *
 * @param list Ukazatel na inicializovanou strukturu dvousměrně vázaného seznamu
 * @param dataPtr Ukazatel na cílovou proměnnou
 */
void DLL_GetFirst( DLList *list, long *dataPtr ) {
	
	// Treba ošetriť prázdny zoznam.
	if( list -> currentLength == 0 ){
		DLL_Error();
		return;
	}

	*dataPtr = list -> firstElement -> data;

	return;
}

/**
 * Prostřednictvím parametru dataPtr vrátí hodnotu posledního prvku seznamu list.
 * Pokud je seznam list prázdný, volá funkci DLL_Error().
 *
 * @param list Ukazatel na inicializovanou strukturu dvousměrně vázaného seznamu
 * @param dataPtr Ukazatel na cílovou proměnnou
 */
void DLL_GetLast( DLList *list, long *dataPtr ) {

	// Treba ošetriť prázdny zoznam.
	if( list -> currentLength == 0 ){
		DLL_Error();
		return;
	}
	
	*dataPtr = list -> lastElement -> data;

	return;
}

/**
 * Zruší první prvek seznamu list.
 * Pokud byl první prvek aktivní, aktivita se ztrácí.
 * Pokud byl seznam list prázdný, nic se neděje.
 *
 * @param list Ukazatel na inicializovanou strukturu dvousměrně vázaného seznamu
 */
void DLL_DeleteFirst( DLList *list ) {

	if (list -> currentLength == 0){
		return;
	}
	
	// Strata aktivity.
	if (list -> activeElement == list -> firstElement){
		list -> activeElement = NULL;
	}
	
	// Mazanie s ošetrením prípadu, kedy je len 1 prvok v zozname.
	DLLElementPtr del = list -> firstElement;

	if (list -> firstElement == list -> lastElement){
		list -> lastElement = NULL;
		list -> firstElement = NULL;
		list -> currentLength--;
	} else {
		list -> firstElement = list -> firstElement -> nextElement;
		list -> firstElement -> previousElement = NULL;
		list -> currentLength--;
	}

	free(del);

	return;
}

/**
 * Zruší poslední prvek seznamu list.
 * Pokud byl poslední prvek aktivní, aktivita seznamu se ztrácí.
 * Pokud byl seznam list prázdný, nic se neděje.
 *
 * @param list Ukazatel na inicializovanou strukturu dvousměrně vázaného seznamu
 */
void DLL_DeleteLast( DLList *list ) {
	
	// Úplne to isté len naopak.
	if (list -> currentLength == 0){
		return;
	}
	
	// Strata aktivity.
	if (list -> activeElement == list -> lastElement){ 
		list -> activeElement = NULL; 
	}

	DLLElementPtr tmp = list -> lastElement;
		
	if (list -> firstElement == list -> lastElement){
		list -> lastElement = NULL;
		list -> firstElement = NULL;
		list -> currentLength--;
	} else {
		list -> lastElement = list -> lastElement -> previousElement; 
		list -> lastElement -> nextElement = NULL; 
		list -> currentLength--;
	}

	free(tmp);

	return;
}

/**
 * Zruší prvek seznamu list za aktivním prvkem.
 * Pokud je seznam list neaktivní nebo pokud je aktivní prvek
 * posledním prvkem seznamu, nic se neděje.
 *
 * @param list Ukazatel na inicializovanou strukturu dvousměrně vázaného seznamu
 */
void DLL_DeleteAfter( DLList *list ) {

	// Najskôr skontrolujeme či ideme vôbec mazať. Je to trochu problematické robiť cez `||` keďže by sme v `list->activeElement->nextElement` mohli dostať `activeElement = NULL`, preto je to také zložité.
	if (list->activeElement == NULL || (list->activeElement != NULL && list->activeElement->nextElement == NULL)){
		return;
	}

	list -> currentLength--;
	
	DLLElementPtr del = list -> activeElement -> nextElement;

	// Vykonáme posun / preskočenie mazaného prvku v zozname.
	list -> activeElement -> nextElement = del -> nextElement;

	// Musíme si dať pozor, či nerušíme posledný prvok.
	if(del == list -> lastElement){
		list -> lastElement = list -> activeElement;
	} else{
		del -> nextElement -> previousElement = list -> activeElement;
	}

	free(del);
	
	return;
}

/**
 * Zruší prvek před aktivním prvkem seznamu list .
 * Pokud je seznam list neaktivní nebo pokud je aktivní prvek
 * prvním prvkem seznamu, nic se neděje.
 *
 * @param list Ukazatel na inicializovanou strukturu dvousměrně vázaného seznamu
 */
void DLL_DeleteBefore( DLList *list ) {
	
	// To isté ako `DLL_DeleteAfter` ale naopak.
	if (list->activeElement == NULL || (list->activeElement != NULL && list->activeElement->previousElement == NULL)){
		return;
	}

	list -> currentLength--;

	DLLElementPtr tmp = list -> activeElement -> previousElement;
	
	list -> activeElement -> previousElement = tmp -> previousElement;
	
	if(tmp == list -> firstElement){
		list -> firstElement = list -> activeElement;
	} else{
		tmp -> previousElement -> nextElement = list -> activeElement;
	}
	
	free(tmp);
	
	return;	
}

/**
 * Vloží prvek za aktivní prvek seznamu list.
 * Pokud nebyl seznam list aktivní, nic se neděje.
 * V případě, že není dostatek paměti pro nový prvek při operaci malloc,
 * volá funkci DLL_Error().
 *
 * @param list Ukazatel na inicializovanou strukturu dvousměrně vázaného seznamu
 * @param data Hodnota k vložení do seznamu za právě aktivní prvek
 */
void DLL_InsertAfter( DLList *list, long data ) {
	
	if (list -> activeElement == NULL) {
		return;
	}

	DLLElementPtr new = ( DLLElementPtr ) malloc( sizeof( struct DLLElement));
	if(new == NULL){
		DLL_Error();
		return;
	}

	list -> currentLength++;

	// Inicializácia nového prvku.
	new -> data = data;
	new -> nextElement = list -> activeElement -> nextElement;
	new -> previousElement = list -> activeElement;

	// Napájanie "susedov", dáme si pozor, či je nový prvok posledný.
	list -> activeElement -> nextElement = new;
	if(list -> activeElement == list -> lastElement){
		list -> lastElement = new;
	} else{
		new -> nextElement -> previousElement = new;
	}
	
	return;
}

/**
 * Vloží prvek před aktivní prvek seznamu list.
 * Pokud nebyl seznam list aktivní, nic se neděje.
 * V případě, že není dostatek paměti pro nový prvek při operaci malloc,
 * volá funkci DLL_Error().
 *
 * @param list Ukazatel na inicializovanou strukturu dvousměrně vázaného seznamu
 * @param data Hodnota k vložení do seznamu před právě aktivní prvek
 */
void DLL_InsertBefore( DLList *list, long data ) {
	
	// To isté čo `DLL_InsertAfter` ale naopak napájame a kontrolujeme či sme prvý prvok :)
	if (list -> activeElement == NULL) {
		return;
	}

	DLLElementPtr new = ( DLLElementPtr ) malloc( sizeof( struct DLLElement));
	if(new == NULL){
		DLL_Error();
		return;
	}

	list -> currentLength++;

	new -> data = data;
	new -> previousElement = list -> activeElement -> previousElement;
	new -> nextElement = list -> activeElement;

	list -> activeElement -> previousElement = new;
	if(list -> activeElement == list -> firstElement){
		list -> firstElement = new;
	} else{
		new -> previousElement -> nextElement = new;
	}
	
	return;
}

/**
 * Prostřednictvím parametru dataPtr vrátí hodnotu aktivního prvku seznamu list.
 * Pokud seznam list není aktivní, volá funkci DLL_Error ().
 *
 * @param list Ukazatel na inicializovanou strukturu dvousměrně vázaného seznamu
 * @param dataPtr Ukazatel na cílovou proměnnou
 */
void DLL_GetValue( DLList *list, long *dataPtr ) {

	// Len klasická kontorla aktívnosti a priradenie návratovej hodnoty.
	if(list -> activeElement == NULL){
		DLL_Error();
		return;
	}

	*dataPtr = list -> activeElement -> data;

	return;
}

/**
 * Přepíše obsah aktivního prvku seznamu list.
 * Pokud seznam list není aktivní, nedělá nic.
 *
 * @param list Ukazatel na inicializovanou strukturu dvousměrně vázaného seznamu
 * @param data Nová hodnota právě aktivního prvku
 */
void DLL_SetValue( DLList *list, long data ) {

	// Kontrola aktivity a zápis.
	if(list -> activeElement == NULL){
		return;
	}

	list -> activeElement -> data = data;

	return;
}

/**
 * Posune aktivitu na následující prvek seznamu list.
 * Není-li seznam aktivní, nedělá nic.
 * Všimněte si, že při aktivitě na posledním prvku se seznam stane neaktivním.
 *
 * @param list Ukazatel na inicializovanou strukturu dvousměrně vázaného seznamu
 */
void DLL_Next( DLList *list ) {

	// Kontrola aktivity.
	if(list -> activeElement == NULL){
		return;
	}

	// Predávame aktivitu následujúcemu prvku. Pokiaľ toto spravíme na poslednom prvku, stratíme aktivitu.
	list -> activeElement = list -> activeElement -> nextElement;

	return;
}


/**
 * Posune aktivitu na předchozí prvek seznamu list.
 * Není-li seznam aktivní, nedělá nic.
 * Všimněte si, že při aktivitě na prvním prvku se seznam stane neaktivním.
 *
 * @param list Ukazatel na inicializovanou strukturu dvousměrně vázaného seznamu
 */
void DLL_Previous( DLList *list ) {

	// Kontrola aktivity.
	if(list -> activeElement == NULL){
		return;
	}
	
	// Predávame aktivitu predchádzajúcemu prvku. Pokiaľ toto spravíme na prvom prvku, stratíme aktivitu.
	list -> activeElement = list -> activeElement -> previousElement;
	
	return;
}

/**
 * Je-li seznam list aktivní, vrací nenulovou hodnotu, jinak vrací 0.
 * Funkci je vhodné implementovat jedním příkazem return.
 *
 * @param list Ukazatel na inicializovanou strukturu dvousměrně vázaného seznamu
 *
 * @returns Nenulovou hodnotu v případě aktivity prvku seznamu, jinak nulu
 */
bool DLL_IsActive( DLList *list ) {
	return list -> activeElement != NULL;
}

/* Konec c206.c */