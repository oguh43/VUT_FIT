# Implementačná dokumentácia k 2. úlohe do IPP 2024/2025
Meno a priezvisko: Hugo Bohácsek
Login: xbohach00

## Stručný popis
Projekt implementuje interpret pre objektovo orientovaný jazyk SOL25. Vstupom je XML reprezentácia abstraktného syntaktického stromu (AST) programu, ktorý interpret spracuje a vykoná. Implementácia využíva objektovo orientovaný prístup s dôrazom na oddelenie zodpovedností, znovupoužiteľnosť kódu a návrhové vzory.

## Architektúra a kľúčové časti

Implementácia je rozdelená do niekoľkých základných modulov:

### 1. AST (Abstraktný syntaktický strom)
Reprezentuje syntaktický strom XML vstupu pomocou hierarchie tried, ktoré implementujú rozhranie `Node`.

- **Node Interface**: Definuje základné rozhranie pre všetky uzly AST  
- **AbstractNode**: Základná implementácia a spoločná funkcionalita pre všetky uzly  
- **Špecifické uzly**: ProgramNode, ClassNode, MethodNode, BlockNode, AssignNode, ExprNode, LiteralNode, SendNode, VarNode  

Na prechádzanie AST je implementovaný návrhový vzor **Visitor**:  
- **NodeVisitor Interface**: Definuje metódy pre navštívenie každého typu uzla  
- **InterpreterVisitor**: Implementácia visitora, ktorý interpretuje AST

### 2. Runtime hodnoty

Všetky hodnoty SOL25 implementujú jednotné rozhranie `ValueInterface` a využívajú spoločnú základnú triedu `AbstractValue`:

- **ValueInterface**: Definuje spoločné rozhranie pre všetky hodnoty SOL25  
- **AbstractValue**: Implementuje všeobecnú funkcionalitu pre všetky hodnoty  
- **Singletony**: TrueValue, FalseValue, NilValue  
- **Bežné hodnoty**: IntegerValue, StringValue, BlockValue  
- **Špeciálne hodnoty**: ObjectValue, ClassValue

### 3. Správa tried

Systém na správu tried a ich metód:

- **ClassInfo**: Abstraktná trieda pre uchovávanie informácií o triedach  
- **BuiltinClassInfo**: Implementácia pre vstavané triedy  
- **UserClassInfo**: Implementácia pre používateľsky definované triedy  
- **DefinedClasses**: Register všetkých dostupných tried a pomocné metódy

### 4. Symboly a premenné

- **SymbolTable**: Tabuľka symbolov s podporou hierarchických rozsahov platnosti

### 5. Spracovanie chýb

Hierarchia výnimiek pre rôzne typy chýb:

- **DoesNotUnderstandException**: Objekt nerozumie zaslanej správe  
- **InvalidArgValueException**: Neplatná hodnota argumentu  
- **OtherRuntimeException**: Ostatné runtime chyby  
- **InvalidXmlStructureException**: Neplatná štruktúra XML  
- **NoMainRunException**: Chýbajúca trieda Main alebo metóda run
- **TypeErrorException**: Chyba nekompatibilných typov
- **UndefinedSymbolException**: Chyba použitia nedefinovaných symbolov

## Návrhové vzory a OOP princípy

### Použité návrhové vzory
1. **Visitor Pattern**: Oddelí algoritmus od dátovej štruktúry, na ktorej operuje. Implementovaný pre prechádzanie a interpretáciu AST.  
2. **Singleton Pattern**: Použitý pre implementáciu jedinečných inštancií TrueValue, FalseValue a NilValue.  
3. **Template Method Pattern**: Použitý v AbstractValue, kde základná implementácia môže byť prepísaná v potomkoch.  
4. **Factory Method Pattern**: Použitý v ClassValue pre vytváranie inštancií tried.  
5. **Composite Pattern**: AST implementuje tento vzor, kde každý uzol môže obsahovať ďalšie uzly.

### OOP princípy
1. **Zapuzdrenie (Encapsulation)**: Všetky triedy skrývajú svoje interné detaily a poskytujú jasné verejné rozhranie.  
2. **Dedičnosť**: Využívaná pre zdieľanie kódu a rozširovanie funkcionality.  
3. **Polymorfizmus**: Používaný naprieč celým kódom, najmä v AST a hodnotách SOL25.  
4. **Abstrakcia**: Abstraktné triedy a rozhrania definujú spoločné správanie.  
5. **Kompozícia nad dedičnosť**: Používaná pre flexibilnejší dizajn.

## Diagram tried
![Class Diagram](UML.png)

## Proces interpretácie

1. **Načítanie XML**: Vstupné XML je spracované do DOM.  
2. **Vytvorenie AST**: Z DOM sa vytvorí abstraktný syntaktický strom.  
3. **Validácia štruktúry**: Kontrola správnosti štruktúry AST.  
4. **Registrácia tried**: Vstavané a používateľské triedy sa zaregistrujú v DefinedClasses.  
5. **Interpretácia**: InterpreterVisitor prechádza AST a vykonáva program.  
   - Hľadá triedu Main a metódu run  
   - Vytvára inštanciu Main  
   - Spracováva bloky kódu a príkazy  
   - Vyhodnocuje výrazy a posiela správy  
   - Zapisuje výstup

## Optimalizácia a výkonnosť

1. **Efektívne posielanie správ**: Spoločný kód pre posielanie správ v AbstractValue.  
2. **Hierarchické vyhľadávanie metód**: Efektívne vyhľadávanie metód v hierarchii tried.  
3. **Opätovné použitie singletonov**: TrueValue, FalseValue a NilValue sú implementované ako singletony.  
4. **Odložené vyhodnocovanie blokov**: Bloky sú vyhodnotené až keď je to potrebné.

## Možné rozšírenia do budúcna

1. **Implementácia super**: Momentálne chýba podpora pre pseudopremennú `super`.  
2. **Optimalizácia vyhľadávania**: Ďalšia optimalizácia vyhľadávania v tabuľke symbolov a definovaných triedach.  

Implementácia spĺňa požiadavky PHP_CodeSniffer pre PSR-1 a PSR-12 a požiadavky PHPStan na úroveň 6 + skoro úroveň 9.
