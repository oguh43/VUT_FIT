/*
 *  Předmět: Algoritmy (IAL) - FIT VUT v Brně
 *  Rozšíření pro příklad c206.c (Dvousměrně vázaný lineární seznam)
 *  Vytvořil: Daniel Dolejška, září 2024
 */

// Makro, ktoré nájde zoznam s prioritou paketov korešpondujúcim k zadanému paketu.
#define MATCH_LIST(packetLists, packet) ({ \
    DLLElementPtr curr = (packetLists != NULL) ? (packetLists -> firstElement) : NULL; \
    QosPacketListPtr result = NULL; \
    while (curr) { \
        if (((QosPacketListPtr)curr -> data) -> priority == (packet) -> priority) { \
            result = (QosPacketListPtr)curr -> data; \
            break; \
        } \
        curr = curr -> nextElement; \
    } \
    result; \
})

// Makro, ktoré zráta počet paketov vo všetkých zoznamoch.
#define TOTAL_PACKETS(packetLists) ({ \
    int total = 0; \
    DLLElementPtr curr = (packetLists != NULL && (packetLists) -> firstElement != NULL) ? (packetLists) -> firstElement : NULL; \
    while (curr) { \
        total += ((QosPacketListPtr)(curr -> data)) -> list -> currentLength; \
        curr = curr -> nextElement; \
    } \
    total; \
})

// Makro, ktoré vráti zoznam s najvyššou prioritou paketov.
#define TOP_PRIORITY(packetLists) ({ \
    QosPacketListPtr highest = NULL; \
    if (packetLists != NULL && (packetLists) -> firstElement != NULL) { \
        highest = (QosPacketListPtr)(packetLists) -> firstElement -> data; \
        for (DLLElementPtr curr = (packetLists) -> firstElement; curr; curr = curr -> nextElement) { \
            QosPacketListPtr currList = (QosPacketListPtr)curr -> data; \
            if (currList -> priority > highest -> priority) highest = currList; \
        } \
    } \
    highest; \
})

#include "c206-ext.h"

bool error_flag;
bool solved;

/**
 * Tato metoda simuluje příjem síťových paketů s určenou úrovní priority.
 * Přijaté pakety jsou zařazeny do odpovídajících front dle jejich priorit.
 * "Fronty" jsou v tomto cvičení reprezentovány dvousměrně vázanými seznamy
 * - ty totiž umožňují snazší úpravy pro již zařazené položky.
 * 
 * Parametr `packetLists` obsahuje jednotlivé seznamy paketů (`QosPacketListPtr`).
 * Pokud fronta s odpovídající prioritou neexistuje, tato metoda ji alokuje
 * a inicializuje. Za jejich korektní uvolnení odpovídá volající.
 * 
 * V případě, že by po zařazení paketu do seznamu počet prvků v cílovém seznamu
 * překročil stanovený MAX_PACKET_COUNT, dojde nejdříve k promazání položek seznamu.
 * V takovémto případě bude každá druhá položka ze seznamu zahozena nehledě
 * na její vlastní prioritu ovšem v pořadí přijetí.
 * 
 * @param packetLists Ukazatel na inicializovanou strukturu dvousměrně vázaného seznamu
 * @param packet Ukazatel na strukturu přijatého paketu
 */
void receive_packet( DLList *packetLists, PacketPtr packet ) {
	
	// Nájdime zoznam, do ktorého patrí paket danej priority.
	QosPacketListPtr packetList = MATCH_LIST(packetLists, packet);

	// Pokiaľ sme taký nenašli, musíme si ho vytvoriť.
	if(packetList == NULL){
		packetList = ( QosPacketListPtr ) malloc( sizeof( QosPacketList));
		packetList -> list = ( DLList * ) malloc( sizeof( DLList));
		packetList -> priority = packet -> priority;

		DLL_Init(packetList -> list);
		DLL_InsertLast(packetLists, (long)packetList);
	}
	
	// Vložíme paket.
	DLL_InsertLast(packetList -> list, (long)packet);

	// Pokiaľ sme prekročili limit, je čas mazať.
	DLL_First(packetList -> list);
	if(packetList -> list -> currentLength > MAX_PACKET_COUNT){
		while(packetList -> list -> activeElement != NULL){
			DLL_DeleteAfter(packetList -> list);
			packetList -> list -> activeElement = packetList -> list -> activeElement -> nextElement;
		}
	}

	return;
}



/**
 * Tato metoda simuluje výběr síťových paketů k odeslání. Výběr respektuje
 * relativní priority paketů mezi sebou, kde pakety s nejvyšší prioritou
 * jsou vždy odeslány nejdříve. Odesílání dále respektuje pořadí, ve kterém
 * byly pakety přijaty metodou `receive_packet`.
 * 
 * Odeslané pakety jsou ze zdrojového seznamu při odeslání odstraněny.
 * 
 * Parametr `packetLists` obsahuje ukazatele na jednotlivé seznamy paketů (`QosPacketListPtr`).
 * Parametr `outputPacketList` obsahuje ukazatele na odeslané pakety (`PacketPtr`).
 * 
 * @param packetLists Ukazatel na inicializovanou strukturu dvousměrně vázaného seznamu
 * @param outputPacketList Ukazatel na seznam paketů k odeslání
 * @param maxPacketCount Maximální počet paketů k odeslání
 */
void send_packets(DLList *packetLists, DLList *outputPacketList, int maxPacketCount) {

    int packetsSent = 0;
    int totalPackets = TOTAL_PACKETS(packetLists); // Počet paketov naprieč všetkými zoznamami.

	// Máme viac paketov ako maximum?
    if (totalPackets > maxPacketCount) {
        totalPackets = maxPacketCount;
    }

	//Ideme od najvyššej priority.
    QosPacketListPtr highestPrioPacketList = TOP_PRIORITY(packetLists);
	if (highestPrioPacketList == NULL) {
		return;
	}

    while (packetsSent < totalPackets) {

		// Odosielame paket.
        PacketPtr packetToSend = ( PacketPtr ) highestPrioPacketList -> list -> firstElement -> data;
        if (packetToSend == NULL) {
            break;
        }

        DLL_InsertLast(outputPacketList, ( long ) packetToSend);
        DLL_DeleteFirst(highestPrioPacketList -> list);

		// Ak sme vyčerpali pakety v zozname, presúvame sa na iný.
		if (highestPrioPacketList -> list -> currentLength == 0) {
			highestPrioPacketList -> priority = '\0';
            highestPrioPacketList = TOP_PRIORITY(packetLists);
        }

		// Už nie je čo posielať :(
		if (highestPrioPacketList == NULL) {
            break;  
        }
        packetsSent++;
	}

	return;
}
