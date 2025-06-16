Implementačná dokumentácia k 1. úlohe do IPP 2024/2025
Meno a priezvisko: Hugo Bohácsek
Login: xbohach00

## Úvod

Tento skript `parse.py` implementuje lexikálnu, syntaktickú a sémantickú analýzu zdrojového kódu v jazyku SOL25. Skript načíta vstupný kód zo štandardného vstupu, vykoná analýzu a vygeneruje XML reprezentáciu abstraktného syntaktického stromu na štandardný výstup.

## Implementácia

Riešenie je implementované objektovo-orientovaným prístupom, rozdelené do troch hlavných tried:

### Lexikálny analyzátor (Lexer)

Trieda `Lexer` je zodpovedná za tokenizáciu vstupného textu. Implementuje tieto hlavné funkcie:
- `tokenize()` - rozdelí vstupný text na tokeny
- `peek()` - bezpečne získa nasledujúci znak bez konzumovania
- `match()` - pokúsi sa zistiť zhodu s daným vzorom
- `get_class_id()`, `get_identifier()`, `get_string_literal()`, `get_integer_literal()` - metódy na extrakciu konkrétnych tokenov
- `skip_whitespace_and_comments()` - preskočenie bielych znakov a komentárov

Lexikálny analyzátor správne rozpoznáva všetky tokeny jazyka SOL25 vrátane identifikátorov, kľúčových slov, literálov a operátorov.

### Syntaktický analyzátor (Parser)

Trieda `Parser` implementuje rekurzívny zostupný parser, ktorý spracováva tokeny a vytvára abstraktný syntaktický strom. Kľúčové metódy:
- `parse()` - hlavná metóda na analýzu programu
- `parse_program()`, `parse_class()`, `parse_method()` - metódy na parsovanie hlavných konštrukcií jazyka
- `parse_selector()`, `parse_block()`, `parse_parameter()` - metódy na parsovanie menších syntaktických jednotiek
- `parse_statement()`, `parse_expr()`, `parse_expr_base()`, `parse_expr_tail()` - metódy na parsovanie výrazov a príkazov

Parser implementuje pravidlá gramatiky jazyka SOL25 a vykonáva aj základné sémantické kontroly počas analýzy.

### Sémantická analýza

Sémantické kontroly sú implementované v rámci triedy `Parser` a zahŕňajú:
- Kontrolu existencie triedy Main a jej metódy run
- Kontrolu použitia nedefinovaných tried, metód a premenných
- Kontrolu správnej arity metód a blokov
- Kontrolu kolízií mien premenných a parametrov
- Kontrolu cyklov v hierarchii dedičnosti

### XML generátor (XMLGenerator)

Trieda `XMLGenerator` prevádza abstraktný syntaktický strom do XML formátu podľa špecifikácie. Hlavné metódy:
- `generate()` - hlavná metóda pre generovanie XML
- `_process_program()`, `_process_class()`, `_process_method()` - spracovanie hlavných prvkov programu
- `_process_block()`, `_process_expr()` - spracovanie blokov a výrazov

## Návrhový vzor

Program využíva návrhový vzor Visitor, kde trieda `XMLGenerator` implementuje návštevníka, ktorý prechádza uzlami abstraktného syntaktického stromu a generuje príslušné XML elementy. Tento vzor umožňuje separáciu logiky spracovania stromovej štruktúry od jej reprezentácie, čo zlepšuje modularitu a rozšíriteľnosť kódu.
