## IPK Projekt 2: Klient pre chatovací server využívajúci protokol IPK25-CHAT

#### Implementovaná funkcionalita

- **Podpora protokolu**: Implementované varianty protokolu IPK25-CHAT cez TCP aj UDP  
- **Príkazové rozhranie**: Pridaná podpora pre požadované príkazy klienta  
  - `/auth` – Autentifikácia pomocou používateľského mena, hesla a prezývky 
  - `/join` – Pripojenie k špecifickým kanálom  
  - `/rename` – Lokálna zmena prezývky
  - `/help` – Zobrazenie nápovedy o dostupných príkazoch  
- **Spracovanie správ**:  
  - TCP: Spracovanie textových správ s ukončením `\r\n`  
  - UDP: Implementácia binárneho formátu správ s 3-bajtovými hlavičkami  
- **Mechanizmy spoľahlivosti pre UDP**:  
  - Systém potvrdenia prijatia správy  
  - Opätovné odoslanie na základe časového limitu  
  - Dynamická práca s portom servera po autentifikácii  
  - Detekcia duplicitných správ  
- **Správa stavov**: Kompletná implementácia FSM klienta so stavmi:  
  - START – Počiatočný stav  
  - AUTH – Autentifikácia  
  - OPEN – Úspešne autentifikovaný  
  - END – Ukončenie spojenia  
- **Spracovanie signálov**: Korektné ukončenie pri signáloch SIGINT a SIGTERM  
- **Debug logovanie**: Implementovaný flexibilný systém debug logovania pomocou makier  

#### Známe obmedzenia

1. **Súlad s C99 pre variadické makrá**: Implementácia debug makra bola upravená, aby vyhovovala štandardu C99, ktorý vyžaduje aspoň jeden argument pre variadické makrá. Problém bol vyriešený použitím korektného makra `printf_debug`.

2. **Správa pamäte**: Aktuálna implementácia by mohla byť robustnejšia pri správe pamäte, najmä pri zlyhaní alokácií.

3. **Spracovanie bufferu pre TCP správy**: Veľké správy v TCP variante by mohli mať úžitok z efektívnejšej správy bufferov.

4. **Obnova po chybách**: Niektoré hraničné prípady obnovy po chybách nemusia byť úplne ošetrené, najmä pri neočakávanom správaní serveru.
