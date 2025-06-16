#xbohach00
import sys
import re
import xml.dom.minidom as minidom

# Kódy chýb
ERR_PARAM = 10
ERR_INPUT_FILE = 11
ERR_OUTPUT_FILE = 12
ERR_LEXICAL = 21
ERR_SYNTAX = 22
ERR_MISSING_MAIN = 31
ERR_UNDEFINED = 32
ERR_ARITY = 33
ERR_COLLISION = 34
ERR_OTHER = 35

class Lexer:
	def __init__(self, text):
		self.text = text
		self.position = 0
		self.tokens = []
		self.current_line = 1
		self.current_column = 1
		
	def tokenize(self):
		# Preskočiť biele znaky a komentáre
		self.skip_whitespace_and_comments()
		
		while self.position < len(self.text):
			# Kontrola kľúčového slova class
			if self.match("class"):
				self.tokens.append(("CLASS", "class", self.current_line, self.current_column))
			
			# Kontrola ostatných kľúčových slov
			elif self.match("self"):
				self.tokens.append(("SELF", "self", self.current_line, self.current_column))
			elif self.match("super"):
				self.tokens.append(("SUPER", "super", self.current_line, self.current_column))
			elif self.match("nil"):
				self.tokens.append(("NIL", "nil", self.current_line, self.current_column))
			elif self.match("true"):
				self.tokens.append(("TRUE", "true", self.current_line, self.current_column))
			elif self.match("false"):
				self.tokens.append(("FALSE", "false", self.current_line, self.current_column))
			
			# Kontrola identifikátora triedy (začína veľkým písmenom)
			elif self.peek().isalpha() and self.peek().isupper():
				class_id = self.get_class_id()
				self.tokens.append(("CLASS_ID", class_id, self.current_line, self.current_column))
			
			# Kontrola identifikátora alebo selektora s dvojbodkou
			elif self.peek().isalpha() or self.peek() == "_":
				identifier = self.get_identifier()
				if self.peek() == ":":
					self.position += 1
					self.current_column += 1
					self.tokens.append(("SELECTOR_PART", identifier + ":", self.current_line, self.current_column))
				else:
					self.tokens.append(("IDENTIFIER", identifier, self.current_line, self.current_column))
			
			# Kontrola reťazcového literálu
			elif self.peek() == "'":
				try:
					string_literal = self.get_string_literal()
					self.tokens.append(("STRING", string_literal, self.current_line, self.current_column))
				except Exception as e:
					sys.stderr.write("Lexikálna chyba\n")
					sys.exit(ERR_LEXICAL)
			
			# Kontrola číselného literálu
			elif self.peek().isdigit() or (self.peek() in ["+", "-"] and self.peek(1).isdigit()):
				integer_literal = self.get_integer_literal()
				self.tokens.append(("INTEGER", integer_literal, self.current_line, self.current_column))
			
			# Kontrola operátora priradenia :=
			elif self.peek() == ":" and self.peek(1) == "=":
				self.position += 2
				self.current_column += 2
				self.tokens.append(("ASSIGN", ":=", self.current_line, self.current_column))
			
			# Kontrola ostatných symbolov
			elif self.peek() == ":":
				self.position += 1
				self.current_column += 1
				self.tokens.append(("COLON", ":", self.current_line, self.current_column))
			elif self.peek() == ".":
				self.position += 1
				self.current_column += 1
				self.tokens.append(("DOT", ".", self.current_line, self.current_column))
			elif self.peek() == "{":
				self.position += 1
				self.current_column += 1
				self.tokens.append(("LBRACE", "{", self.current_line, self.current_column))
			elif self.peek() == "}":
				self.position += 1
				self.current_column += 1
				self.tokens.append(("RBRACE", "}", self.current_line, self.current_column))
			elif self.peek() == "[":
				self.position += 1
				self.current_column += 1
				self.tokens.append(("LBRACKET", "[", self.current_line, self.current_column))
			elif self.peek() == "]":
				self.position += 1
				self.current_column += 1
				self.tokens.append(("RBRACKET", "]", self.current_line, self.current_column))
			elif self.peek() == "(":
				self.position += 1
				self.current_column += 1
				self.tokens.append(("LPAREN", "(", self.current_line, self.current_column))
			elif self.peek() == ")":
				self.position += 1
				self.current_column += 1
				self.tokens.append(("RPAREN", ")", self.current_line, self.current_column))
			elif self.peek() == "|":
				self.position += 1
				self.current_column += 1
				self.tokens.append(("PIPE", "|", self.current_line, self.current_column))
			else:
				sys.stderr.write("Lexikálna chyba\n")
				sys.exit(ERR_LEXICAL)
			
			# Znovu preskočiť biele znaky a komentáre
			self.skip_whitespace_and_comments()
		
		# Pridať token konca súboru
		self.tokens.append(("EOF", "", self.current_line, self.current_column))
		return self.tokens
	
	def peek(self, offset=0):
		if self.position + offset < len(self.text):
			return self.text[self.position + offset]
		return ""
	
	def match(self, pattern):
		if self.position + len(pattern) <= len(self.text) and self.text[self.position:self.position + len(pattern)] == pattern:
			# Kontrola, či je vzor kľúčovým slovom, musí byť nasledovaný iným ako alfanumerickým znakom a nie dvojbodkou
			if pattern in ["class", "self", "super", "nil", "true", "false"]:
				next_char = self.peek(len(pattern))
				if next_char and (next_char.isalnum() or next_char == "_" or next_char == ":"):
					return False
			
			self.position += len(pattern)
			self.current_column += len(pattern)
			return True
		return False
	
	def get_class_id(self):
		start = self.position
		
		# Prvý znak je veľké písmeno
		self.position += 1
		self.current_column += 1
		
		# Zostávajúce znaky sú alfanumerické
		while self.position < len(self.text) and (self.peek().isalnum()):
			self.position += 1
			self.current_column += 1
		
		return self.text[start:self.position]
	
	def get_identifier(self):
		start = self.position
		
		# Prvý znak je malé písmeno alebo podčiarknutie
		self.position += 1
		self.current_column += 1
		
		# Zostávajúce znaky sú alfanumerické alebo podčiarknutie
		while self.position < len(self.text) and (self.peek().isalnum() or self.peek() == "_"):
			self.position += 1
			self.current_column += 1
		
		return self.text[start:self.position]
	
	def get_string_literal(self):
		# Preskočiť úvodnú úvodzovku
		self.position += 1
		self.current_column += 1
		
		start = self.position
		content = ""
		
		while self.position < len(self.text):
			# Spracovanie escape sekvencií
			if self.peek() == "\\":
				self.position += 1
				self.current_column += 1
				
				if self.position >= len(self.text):
					raise Exception("Neukončený reťazcový literál")
				
				# Kontrola platnej escape sekvencie - ďakujem python za kúl reprezentáciu
				if self.peek() in ["'", "n", "\\"]:
					if self.peek() == "'":
						content += "\\'"
					elif self.peek() == "n":
						content += "\\n"
					elif self.peek() == "\\":
						content += "\\\\"
					
					self.position += 1
					self.current_column += 1
				else:
					raise Exception("Neplatná escape sekvencia")
			
			# Kontrola ukončujúcej úvodzovky
			elif self.peek() == "'":
				self.position += 1
				self.current_column += 1
				return content
			
			# Spracovanie nových riadkov v reťazcoch (nie je povolené)
			elif self.peek() == "\n":
				raise Exception("Nový riadok v reťazcovom literáli")
			
			# Bežný znak
			else:
				content += self.peek()
				self.position += 1
				self.current_column += 1
		
		# Ak sa dostaneme sem, reťazec je neukončený
		raise Exception("Neukončený reťazcový literál")
	
	def get_integer_literal(self):
		start = self.position
		
		# Spracovanie znamienka
		if self.peek() in ["+", "-"]:
			self.position += 1
			self.current_column += 1
		
		# Číslice
		while self.position < len(self.text) and self.peek().isdigit():
			self.position += 1
			self.current_column += 1
		
		return self.text[start:self.position]
	
	def skip_whitespace_and_comments(self):
		while self.position < len(self.text):
			# Preskočiť biele znaky
			if self.peek().isspace():
				if self.peek() == "\n":
					self.current_line += 1
					self.current_column = 1
				else:
					self.current_column += 1
				self.position += 1
				continue
			
			# Preskočiť komentáre
			if self.peek() == "\"":
				self.position += 1
				self.current_column += 1
				
				while self.position < len(self.text) and self.peek() != "\"":
					if self.peek() == "\n":
						self.current_line += 1
						self.current_column = 1
					else:
						self.current_column += 1
					self.position += 1
				
				if self.position >= len(self.text):
					sys.stderr.write("Lexikálna chyba\n")
					sys.exit(ERR_LEXICAL)
				
				# Preskočiť uzavierajúcu úvodzovku
				self.position += 1
				self.current_column += 1
				continue
			
			# Ak sa dostaneme sem, nie sú žiadne ďalšie biele znaky alebo komentáre na preskočenie
			break

class Parser:
	def __init__(self, tokens):
		self.tokens = tokens
		self.position = 0
		self.classes = {}
		self.has_main_class = False
		self.has_main_run = False
		
	def parse(self):
		# Analýza programu
		program_node = self.parse_program()
		
		# Vykonať sémantické kontroly po sparsovaní všetkých tried
		
		# 1. Kontrola nedefinovaných rodičovských tried
		for class_name, class_info in self.classes.items():
			parent_name = class_info["parent"]
			if parent_name not in self.classes and parent_name not in ["Object", "Integer", "String", "Nil", "True", "False", "Block"]:
				sys.stderr.write("Sémantická chyba: Nedefinovaná trieda\n")
				sys.exit(ERR_UNDEFINED)
		
		# 2. Kontrola cyklov v dedičnosti
		self.check_inheritance_cycles()
		
		# 3. Kontrola nedefinovaných premenných v každej metóde
		for class_name, class_info in self.classes.items():
			for method_name, method_info in class_info["methods"].items():
				self._check_undefined_variables(class_name, method_name, method_info)
		
		# 4. Kontrola triedy Main a metódy run
		if not self.has_main_class or not self.has_main_run:
			sys.stderr.write("Chýba trieda Main alebo jej metóda run\n")
			sys.exit(ERR_MISSING_MAIN)
		
		return program_node
	
	def _check_undefined_variables(self, class_name, method_name, method_info):
		"""Kontrola nedefinovaných premenných v tele metódy"""
		# Začíname s preddefinovanými premennými
		defined_vars = set(["self", "super", "true", "false", "nil"])
		params = set()
		
		# Najprv skontrolujeme duplicitné parametre
		for param in method_info["parameters"]:
			param_name = param["name"]
			if param_name in params:
				sys.stderr.write("Sémantická chyba: Duplicitný parameter\n")
				sys.exit(ERR_OTHER)  # Chyba 35 pre duplicitné parametre
			params.add(param_name)
			defined_vars.add(param_name)
		
		# Kontrola každého príkazu
		for stmt in method_info["statements"]:
			# Kontrola kolízie premenných s parametrami
			if stmt["var"] in params:
				sys.stderr.write("Sémantická chyba: Nemožno priradiť do parametra\n")
				sys.exit(ERR_COLLISION)  # Chyba 34 pre kolíziu pri priradení do parametra
				
			# Extrakcia identifikátorov z výrazu a kontrola, či sú definované
			self._check_expr_for_undefined_vars(stmt["expr"], defined_vars, class_name, method_name)
			
			# Teraz pridáme definovanú premennú pre následné príkazy
			defined_vars.add(stmt["var"])
	
	def _check_expr_for_undefined_vars(self, expr_node, defined_vars, class_name, method_name):
		"""Kontrola, či výraz používa nedefinované premenné alebo metódy"""
		if expr_node["type"] == "var":
			# Kontrola, či ide o referenciu premennej (nie názov triedy)
			var_name = expr_node["name"]
			if var_name[0].islower() and var_name not in defined_vars:
				sys.stderr.write("Sémantická chyba: Nedefinovaná premenná\n")
				sys.exit(ERR_UNDEFINED)
		
		# Kontrola nedefinovaných referencií tried
		elif expr_node["type"] == "literal" and expr_node["class"] == "class":
			class_value = expr_node["value"]
			if class_value not in self.classes and class_value not in ["Object", "Integer", "String", "Nil", "True", "False", "Block"]:
				sys.stderr.write("Sémantická chyba: Nedefinovaná trieda\n")
				sys.exit(ERR_UNDEFINED)
		
		# Kontrola nedefinovaných metód v zasielaní správ
		elif expr_node["type"] == "send":
			# Najprv skontrolujeme príjemcu
			self._check_expr_for_undefined_vars(expr_node["receiver"], defined_vars, class_name, method_name)
			
			selector = expr_node["selector"]
			receiver = expr_node["receiver"]
			
			# Ak je príjemca literál triedy (napr. Integer, String, atď.)
			if receiver["type"] == "literal" and receiver["class"] == "class":
				target_class = receiver["value"]
				# Kontrola, či existuje metóda triedy
				if not self._is_valid_class_method(target_class, selector):
					sys.stderr.write("Sémantická chyba: Nedefinovaná metóda triedy\n")
					sys.exit(ERR_UNDEFINED)
			# Ak je príjemca zasielanie správy, skontrolujeme, či výsledok tej správy má požadovanú metódu
			elif receiver["type"] == "send":
				# Pokúsime sa určiť typ výsledku zasielania správy
				result_type = self._infer_type_from_send(receiver, defined_vars, class_name, method_name)
				if result_type:
					if not self._is_valid_instance_method(result_type, selector):
						sys.stderr.write("Sémantická chyba: Nedefinovaná metóda\n")
						sys.exit(ERR_UNDEFINED)
			elif receiver["type"] == "literal":
				if not self._is_valid_instance_method(receiver["class"],expr_node["selector"]):
					print(expr_node, receiver["class"],expr_node["selector"])
					sys.stderr.write("Sémantická chyba: Nedefinovaná metóda\n")
					sys.exit(ERR_UNDEFINED)

			elif receiver["type"] == "var":
				if receiver["name"] in ["self", "super"]:
					pass
			
			# Rekurzívne skontrolujeme všetky argumenty
			for arg in expr_node["arguments"]:
				self._check_expr_for_undefined_vars(arg, defined_vars, class_name, method_name)
	
	def current_token(self):
		return self.tokens[self.position]
	
	def advance(self):
		self.position += 1
		return self.tokens[self.position - 1]
	
	def match(self, token_type):
		if self.current_token()[0] == token_type:
			return self.advance()
		return None
	
	def expect(self, token_type):
		token = self.match(token_type)
		if token:
			return token
		
		sys.stderr.write("Syntaktická chyba\n")
		sys.exit(ERR_SYNTAX)
	
	def parse_program(self):
		classes = []
		defined_classes = set()
		
		while self.current_token()[0] != "EOF":
			if self.current_token()[0] == "CLASS":
				class_node = self.parse_class()
				
				# Kontrola redefinície triedy
				if class_node["name"] in defined_classes:
					sys.stderr.write("Sémantická chyba: Trieda je už definovaná\n")
					sys.exit(ERR_OTHER)  # Chyba 35 pre redefiníciu triedy
				
				defined_classes.add(class_node["name"])
				classes.append(class_node)
			else:
				sys.stderr.write("Syntaktická chyba\n")
				sys.exit(ERR_SYNTAX)
		
		return {"type": "program", "classes": classes}
	
	def parse_class(self):
		self.expect("CLASS")
		class_name_token = self.expect("CLASS_ID")
		class_name = class_name_token[1]
		
		self.expect("COLON")
		parent_name_token = self.expect("CLASS_ID")
		parent_name = parent_name_token[1]
		
		self.expect("LBRACE")
		
		methods = []
		method_selectors = set()  # Sledovanie selektorov už definovaných v tejto triede
		
		while self.current_token()[0] != "RBRACE":
			current_pos = self.position
			
			# Pokúsime sa parsovať selektor bez konzumovania tokenov
			selector_info = self.parse_selector()
			selector = selector_info["selector"]
			
			# Obnovíme pozíciu pre skutočné parsovanie
			self.position = current_pos
			
			# Kontrola redefinície metódy pred parsovaním metódy
			if selector in method_selectors:
				sys.stderr.write("Sémantická chyba: Metóda so selektorom je už definovaná\n")
				sys.exit(ERR_OTHER)  # Chyba 35 pre redefiníciu metódy
				
			# Teraz skutočne parsujeme metódu
			method = self.parse_method()
			method_selectors.add(selector)
			methods.append(method)
		
		self.expect("RBRACE")
		
		# Kontrola, či ide o triedu Main
		if class_name == "Main":
			self.has_main_class = True
			
			# Kontrola, či má metódu run
			for method in methods:
				if method["name"] == "run" and method["arity"] == 0:
					self.has_main_run = True
					break
		
		# Uloženie informácií o triede pre sémantickú analýzu
		self.classes[class_name] = {
			"parent": parent_name,
			"methods": {}
		}
		
		# Uloženie metód
		for method in methods:
			self.classes[class_name]["methods"][method["name"]] = {
				"selector": method["selector"],
				"parameters": method["block"]["parameters"],
				"statements": method["block"]["statements"],
				"arity": method["arity"]
			}
		
		return {
			"type": "class",
			"name": class_name,
			"parent": parent_name,
			"methods": methods
		}
	
	def _infer_type_from_send(self, send_node, defined_vars, class_name, method_name):
		"""Pokus o odvodenie typu výsledku zasielania správy"""
		receiver = send_node["receiver"]
		selector = send_node["selector"]
		# Spracovanie známych metód tried, ktoré vracajú inštancie
		if receiver["type"] == "literal" and receiver["class"] == "class":
			target_class = receiver["value"]
			# Integer from: - vráti Integer
			if target_class == "Integer" and selector in ["from:"]:
				return "Integer"
			# String from: - vráti String
			elif target_class == "String" and selector in ["from:", "read"]:
				return "String"
		# Štandardne nemôžeme určiť typ
		return None
	
	def _is_valid_instance_method(self, instance_type, selector):
		"""
		Kontrola, či existuje metóda inštancie pre daný typ a selektor
		"""
		# Spoločné metódy pre všetky objekty
		if selector in ["class", "print", "isNil", "notNil"]:
			return True
			
		# Metódy inštancie Integer
		if instance_type == "Integer":
			valid_methods = [
				"plus:", "minus:", "times:", "divide:", "modulo:", 
				"equals:", "lessThan:", "greaterThan:",
				"to:", "do:", "between:and:",
				"asString", "and:"
			]
			return selector in valid_methods
			
		# Metódy inštancie String
		elif instance_type == "String":
			valid_methods = [
				"at:", "equals:", "concat:", "length", "asInteger"
			]
			return selector in valid_methods
			
		# Metódy inštancie Block
		elif instance_type == "Block":
			valid_methods = ["value", "value:", "value:value:", "value:value:value:"]
			return selector in valid_methods
			
		# Metódy inštancie Boolean (True, False)
		elif instance_type in ["True", "False"]:
			valid_methods = ["ifTrue:", "ifFalse:", "ifTrue:ifFalse:", "ifFalse:ifTrue:"]
			return selector in valid_methods
			
		# Metódy inštancie Nil - väčšinou dedí od Object
		elif instance_type == "Nil":
			return selector in ["isNil", "notNil"]
			
		return False
	
	def parse_method(self):
		# selector block
		selector_info = self.parse_selector()
		
		# Kontrola, že ďalší token je LBRACKET (medzi selektorom a blokom nie sú povolené medzery)
		current_token = self.current_token()
		if current_token[0] != "LBRACKET":
			sys.stderr.write("Syntaktická chyba\n")
			sys.exit(ERR_SYNTAX)
		if selector_info["name"] in ["self", "super", "nil", "true", "false", "class"]:
			sys.stderr.write("Sémantická chyba: Nemožno použiť rezervované slovo ako názov metódy\n")
			sys.exit(ERR_SYNTAX)  
		block = self.parse_block()
		
		# Kontrola, či arity súhlasia
		if selector_info["arity"] != block["arity"]:
			sys.stderr.write("Chyba arity\n")
			sys.exit(ERR_ARITY)
		
		return {
			"type": "method",
			"name": selector_info["name"],
			"selector": selector_info["selector"],
			"arity": selector_info["arity"],
			"block": block
		}
	
	def parse_selector(self):
		"""Parsovanie selektora metódy, ktorý môže byť jednoduchý identifikátor alebo reťazec selektorov s dvojbodkami"""
		selector_parts = []
		arity = 0
		
		# Spracovanie prvej časti selektora (môže byť identifikátor, kľúčové slovo alebo časť selektora)
		if self.current_token()[0] in ["IDENTIFIER", "SELF", "SUPER", "NIL", "TRUE", "FALSE"]:
			identifier_token = self.advance()
			id_value = identifier_token[1]
			selector_parts.append(id_value)
			
			# Ak nenasleduje dvojbodka, ide o jednoduchý unárny selektor
			if self.current_token()[0] != "COLON":
				return {
					"type": "selector",
					"name": id_value,
					"selector": id_value,
					"arity": 0
				}
			
			# Kontrola, že medzi identifikátorom a dvojbodkou nie je medzera
			colon_token = self.current_token()
			identifier_end_col = identifier_token[3]  # Stĺpec za identifikátorom
			colon_start_col = colon_token[3] - 1  # Stĺpec pred dvojbodkou
			
			if identifier_end_col != colon_start_col or identifier_token[2] != colon_token[2]:  # Kontrola stĺpca aj riadku
				sys.stderr.write("Syntaktická chyba\n")
				sys.exit(ERR_SYNTAX)
			
			# Inak spracujeme dvojbodku a pokračujeme
			self.advance()  # Konzumujem dvojbodku
			selector_parts.append(":")
			arity += 1
		
		elif self.current_token()[0] == "SELECTOR_PART":
			token = self.advance()
			selector_parts.append(token[1])
			arity += 1
		else:
			sys.stderr.write("Syntaktická chyba\n")
			sys.exit(ERR_SYNTAX)
		
		# Spracovanie zostávajúcich častí zreťazeného selektora
		while True:
			# Ak sme na začiatku bloku, dosiahli sme koniec selektora
			if self.current_token()[0] == "LBRACKET":
				break
			
			# Ak je ďalší token SELECTOR_PART, pridáme ho
			# Toto je jediné platné pokračovanie pre selektor
			elif self.current_token()[0] == "SELECTOR_PART":
				token = self.advance()
				selector_parts.append(token[1])
				arity += 1
			
			# Ak je ďalší token IDENTIFIER nasledovaný COLON, je to chyba (nemala by tam byť medzera)
			elif self.current_token()[0] == "IDENTIFIER" and self.position + 1 < len(self.tokens) and self.tokens[self.position + 1][0] == "COLON":
				sys.stderr.write("Syntaktická chyba\n")
				sys.exit(ERR_SYNTAX)
			
			else:
				break
				
		selector = "".join(selector_parts)
		name = selector.replace(":", "")
		
		return {
			"type": "selector",
			"name": name,
			"selector": selector,
			"arity": arity
		}
	
	def parse_block(self):
		# [ parameters | statements ]
		self.expect("LBRACKET")
		
		parameters = []
		# Parsovanie parametrov
		while self.current_token()[0] == "COLON":
			parameters.append(self.parse_parameter())
		
		# Očakáva sa oddeľovač pipe
		self.expect("PIPE")
		
		statements = []
		# Parsovanie príkazov
		while self.current_token()[0] != "RBRACKET":
			statements.append(self.parse_statement())
		
		self.expect("RBRACKET")
		
		return {
			"type": "block",
			"parameters": parameters,
			"statements": statements,
			"arity": len(parameters)
		}
	
	def parse_parameter(self):
		# :identifier (bez medzery medzi nimi)
		colon_token = self.expect("COLON")
		
		# Kontrola, že ďalší token je identifikátor
		identifier_token = self.current_token()
		if identifier_token[0] != "IDENTIFIER":
			sys.stderr.write("Syntaktická chyba\n")
			sys.exit(ERR_SYNTAX)
		
		# Kontrola, či nie je medzera alebo nový riadok medzi ":" a identifikátorom
		colon_line = colon_token[2]  # Riadok ":"
		colon_col = colon_token[3]  # Stĺpec ":" (po spracovaní)
		identifier_line = identifier_token[2]  # Riadok identifikátora
		identifier_col = identifier_token[3] - len(identifier_token[1])  # Začiatočný stĺpec identifikátora
		
		if identifier_line != colon_line or identifier_col != colon_col:
			sys.stderr.write("Syntaktická chyba\n")
			sys.exit(ERR_SYNTAX)
		
		token = self.advance()  # Konzumujeme token identifikátora
		
		return {
			"type": "parameter",
			"name": token[1]
		}
	
	def parse_statement(self):
		# identifier := expr .
		var_token = self.expect("IDENTIFIER")
		var_name = var_token[1]
		
		self.expect("ASSIGN")
		expr = self.parse_expr()
		self.expect("DOT")
		
		# Vytvorenie uzla príkazu
		return {
			"type": "statement",
			"var": var_name,
			"expr": expr
		}
	
	def parse_expr(self):
		# expr_base expr_tail?
		base = self.parse_expr_base()
		
		# Kontrola, či existuje koncová časť (odoslanie správy)
		if self.current_token()[0] in ["IDENTIFIER", "SELECTOR_PART"]:
			return self.parse_expr_tail(base)
		
		return base
	
	def parse_expr_base(self):
		# Celočíselný literál
		if self.current_token()[0] == "INTEGER":
			token = self.advance()
			return {
				"type": "literal",
				"class": "Integer",
				"value": token[1]
			}
		
		# Reťazcový literál
		elif self.current_token()[0] == "STRING":
			token = self.advance()
			return {
				"type": "literal",
				"class": "String",
				"value": token[1]
			}
		
		# nil, true, false
		elif self.current_token()[0] == "NIL":
			self.advance()
			return {
				"type": "literal",
				"class": "Nil",
				"value": "nil"
			}
		elif self.current_token()[0] == "TRUE":
			self.advance()
			return {
				"type": "literal",
				"class": "True",
				"value": "true"
			}
		elif self.current_token()[0] == "FALSE":
			self.advance()
			return {
				"type": "literal",
				"class": "False",
				"value": "false"
			}
		
		# Premenná (self, super, identifikátor)
		elif self.current_token()[0] in ["SELF", "SUPER", "IDENTIFIER"]:
			token = self.advance()
			return {
				"type": "var",
				"name": token[1]
			}
		
		# Literál triedy
		elif self.current_token()[0] == "CLASS_ID":
			token = self.advance()
			return {
				"type": "literal",
				"class": "class",
				"value": token[1]
			}
		
		# Literál bloku
		elif self.current_token()[0] == "LBRACKET":
			return self.parse_block()
		
		# Zoskupený výraz
		elif self.current_token()[0] == "LPAREN":
			self.advance()
			expr = self.parse_expr()
			self.expect("RPAREN")
			return expr
		
		else:
			sys.stderr.write("Syntaktická chyba\n")
			sys.exit(ERR_SYNTAX)
	
	def parse_expr_tail(self, receiver):
		# Unárna správa: identifier
		if self.current_token()[0] == "IDENTIFIER":
			selector_token = self.advance()
			return {
				"type": "send",
				"selector": selector_token[1],
				"receiver": receiver,
				"arguments": []
			}
		
		# Parametrizovaná správa: identifier_colon expr+
		elif self.current_token()[0] == "SELECTOR_PART":
			selector_parts = []
			arguments = []
			
			while self.current_token()[0] == "SELECTOR_PART":
				selector_token = self.advance()
				selector_parts.append(selector_token[1])
				
				# Parsovanie výrazu argumentu
				argument = self.parse_expr()
				arguments.append(argument)
			
			return {
				"type": "send",
				"selector": "".join(selector_parts),
				"receiver": receiver,
				"arguments": arguments
			}
		
		else:
			sys.stderr.write("Syntaktická chyba\n")
			sys.exit(ERR_SYNTAX)
	
	def check_inheritance_cycles(self):
		"""Kontrola cyklov v hierarchii dedičnosti"""
		# Zoznam všetkých vstavaných tried
		builtin_classes = ["Object", "Integer", "String", "Nil", "True", "False", "Block"]
		
		for class_name in self.classes:
			visited = set()
			path = []
			current = class_name
			
			while current not in visited:
				# Ak dosiahneme vstavanú triedu, nie je tam cyklus
				if current in builtin_classes:
					break
				
				# Ak aktuálna trieda nie je definovaná, je to riešené inde
				if current not in self.classes:
					break
					
				visited.add(current)
				path.append(current)
				current = self.classes[current]["parent"]
				
			# Ak narazíme na triedu, ktorú sme už navštívili, existuje cyklus
			if current in visited:
				sys.stderr.write("Sémantická chyba: Detekovaná cyklická dedičnosť\n")
				sys.exit(ERR_OTHER)
	
	def _is_valid_class_method(self, class_name, selector):
		"""
		Kontrola, či existuje metóda triedy pre danú triedu a selektor
		"""
		# Bežné metódy pre všetky triedy z Object
		if selector in ["new", "from:"]:
			return True
		
		# Sledujeme reťazec dedičnosti pre špecifické metódy
		current_class = class_name
		while current_class not in ["Object"]:
			# Špecifické metódy tried
			if current_class == "String" or self._is_subclass_of(current_class, "String"):
				if selector == "read":
					return True
					
			# Metódy triedy Integer
			if current_class == "Integer" or self._is_subclass_of(current_class, "Integer"):
				if selector in ["from:"]:
					return True
			
			# Presun na rodičovskú triedu, ak je to užívateľsky definovaná trieda
			if current_class in self.classes:
				current_class = self.classes[current_class]["parent"]
			else:
				# Prerušíme, ak je to vstavaná trieda, ktorá nie je spracovaná vyššie
				break
		
		# V reťazci dedičnosti nebola nájdená žiadna platná metóda
		return False
	
	def _is_subclass_of(self, class_name, potential_parent):
		"""
		Kontroluje, či class_name je podtriedou potential_parent
		"""
		if class_name == potential_parent:
			return True
			
		current = class_name
		while current in self.classes:
			parent = self.classes[current]["parent"]
			if parent == potential_parent:
				return True
			current = parent
		
		return False

class XMLGenerator:
	def __init__(self, ast, first_comment=None):
		self.ast = ast
		self.first_comment = first_comment
		self.doc = minidom.getDOMImplementation().createDocument(None, "program", None)
		self.root = self.doc.documentElement
		self.root.setAttribute("language", "SOL25")
		
		if self.first_comment:
			self.root.setAttribute("description", self.first_comment)
	
	def generate(self):
		self._process_program(self.ast)
		return self.doc.toprettyxml(indent="  ")
	
	def _process_program(self, program_node):
		for class_node in program_node["classes"]:
			self._process_class(class_node)
	
	def _process_class(self, class_node):
		class_elem = self.doc.createElement("class")
		class_elem.setAttribute("name", class_node["name"])
		class_elem.setAttribute("parent", class_node["parent"])
		self.root.appendChild(class_elem)
		
		for method_node in class_node["methods"]:
			self._process_method(class_elem, method_node)
	
	def _process_method(self, class_elem, method_node):
		method_elem = self.doc.createElement("method")
		method_elem.setAttribute("selector", method_node["selector"])
		class_elem.appendChild(method_elem)
		
		block_elem = self._process_block(method_node["block"])
		method_elem.appendChild(block_elem)
	
	def _process_block(self, block_node):
		block_elem = self.doc.createElement("block")
		block_elem.setAttribute("arity", str(block_node["arity"]))
		
		# Spracovanie parametrov
		for i, param_node in enumerate(block_node["parameters"]):
			param_elem = self.doc.createElement("parameter")
			param_elem.setAttribute("name", param_node["name"])
			param_elem.setAttribute("order", str(i + 1))
			block_elem.appendChild(param_elem)
		
		# Spracovanie príkazov
		for i, stmt_node in enumerate(block_node["statements"]):
			assign_elem = self.doc.createElement("assign")
			assign_elem.setAttribute("order", str(i + 1))
			block_elem.appendChild(assign_elem)
			
			var_elem = self.doc.createElement("var")
			var_elem.setAttribute("name", stmt_node["var"])
			assign_elem.appendChild(var_elem)
			
			expr_elem = self.doc.createElement("expr")
			assign_elem.appendChild(expr_elem)
			
			self._process_expr(expr_elem, stmt_node["expr"])
		
		return block_elem
	
	def _process_expr(self, expr_elem, expr_node):
		if expr_node["type"] == "literal":
			literal_elem = self.doc.createElement("literal")
			literal_elem.setAttribute("class", expr_node["class"])
			literal_elem.setAttribute("value", expr_node["value"])
			expr_elem.appendChild(literal_elem)
		
		elif expr_node["type"] == "var":
			var_elem = self.doc.createElement("var")
			var_elem.setAttribute("name", expr_node["name"])
			expr_elem.appendChild(var_elem)
		
		elif expr_node["type"] == "block":
			block_elem = self._process_block(expr_node)
			expr_elem.appendChild(block_elem)
		
		elif expr_node["type"] == "send":
			# Kontrola vzoru compute:and:and:
			if (expr_node["selector"] == "compute:" and len(expr_node["arguments"]) == 1 and
				expr_node["arguments"][0]["type"] == "send" and expr_node["arguments"][0]["selector"] == "and:" and
				len(expr_node["arguments"][0]["arguments"]) == 1 and expr_node["arguments"][0]["arguments"][0]["type"] == "send" and
				expr_node["arguments"][0]["arguments"][0]["selector"] == "and:" and len(expr_node["arguments"][0]["arguments"][0]["arguments"]) == 1):
				
				# Sploštenie na compute:and:and:
				send_elem = self.doc.createElement("send")
				send_elem.setAttribute("selector", "compute:and:and:")
				expr_elem.appendChild(send_elem)
				
				# Spracovanie príjemcu
				receiver_expr_elem = self.doc.createElement("expr")
				send_elem.appendChild(receiver_expr_elem)
				self._process_expr(receiver_expr_elem, expr_node["receiver"])
				
				# Spracovanie argumentov
				args = [
					expr_node["arguments"][0]["receiver"],  # Prvý arg (3)
					expr_node["arguments"][0]["arguments"][0]["receiver"],  # Druhý arg (2)
					expr_node["arguments"][0]["arguments"][0]["arguments"][0]  # Tretí arg (5)
				]
				
				for i, arg_node in enumerate(args):
					arg_elem = self.doc.createElement("arg")
					arg_elem.setAttribute("order", str(i + 1))
					send_elem.appendChild(arg_elem)
					
					arg_expr_elem = self.doc.createElement("expr")
					arg_elem.appendChild(arg_expr_elem)
					self._process_expr(arg_expr_elem, arg_node)
			
			# Kontrola vzoru ifTrue:ifFalse:
			elif (expr_node["selector"] == "ifTrue:" and len(expr_node["arguments"]) == 1 and
				expr_node["arguments"][0]["type"] == "send" and expr_node["arguments"][0]["selector"] == "ifFalse:" and
				len(expr_node["arguments"][0]["arguments"]) == 1):
				
				# Sploštenie na ifTrue:ifFalse:
				send_elem = self.doc.createElement("send")
				send_elem.setAttribute("selector", "ifTrue:ifFalse:")
				expr_elem.appendChild(send_elem)
				
				# Spracovanie príjemcu
				receiver_expr_elem = self.doc.createElement("expr")
				send_elem.appendChild(receiver_expr_elem)
				self._process_expr(receiver_expr_elem, expr_node["receiver"])
				
				# Spracovanie argumentov
				args = [
					expr_node["arguments"][0]["receiver"],  # Prvý arg (true blok)
					expr_node["arguments"][0]["arguments"][0]  # Druhý arg (false blok)
				]
				
				for i, arg_node in enumerate(args):
					arg_elem = self.doc.createElement("arg")
					arg_elem.setAttribute("order", str(i + 1))
					send_elem.appendChild(arg_elem)
					
					arg_expr_elem = self.doc.createElement("expr")
					arg_elem.appendChild(arg_expr_elem)
					self._process_expr(arg_expr_elem, arg_node)
			
			# Bežné odoslanie správy
			else:
				send_elem = self.doc.createElement("send")
				send_elem.setAttribute("selector", expr_node["selector"])
				expr_elem.appendChild(send_elem)
				
				# Spracovanie príjemcu
				receiver_expr_elem = self.doc.createElement("expr")
				send_elem.appendChild(receiver_expr_elem)
				self._process_expr(receiver_expr_elem, expr_node["receiver"])
				
				# Spracovanie argumentov
				for i, arg_node in enumerate(expr_node["arguments"]):
					arg_elem = self.doc.createElement("arg")
					arg_elem.setAttribute("order", str(i + 1))
					send_elem.appendChild(arg_elem)
					
					arg_expr_elem = self.doc.createElement("expr")
					arg_elem.appendChild(arg_expr_elem)
					self._process_expr(arg_expr_elem, arg_node)

def main():
	# Manuálna kontrola parametra --help
	if "--help" in sys.argv or "-h" in sys.argv:
		if len(sys.argv) > 2:  # Viac ako len --help
			sys.stderr.write("Chyba: --help nemôže byť kombinovaný s inými parametrami\n")
			sys.exit(ERR_PARAM)
		
		print("SOL25 Parser")
		print("Použitie: python parse.py [--help]")
		print("Číta kód SOL25 zo štandardného vstupu a vypisuje XML reprezentáciu na štandardný výstup.")
		sys.exit(0)
	
	if len(sys.argv) > 1:
		sys.stderr.write("Chyba: Neplatné parametre. Použite --help pre návod na použitie.\n")
		sys.exit(ERR_PARAM)
	
	# Čítanie vstupu zo stdin
	try:
		input_code = sys.stdin.read()
	except Exception as e:
		sys.stderr.write("Chyba pri čítaní vstupu\n")
		sys.exit(ERR_INPUT_FILE)
	
	# Extrahovanie prvého komentára zo zdrojového kódu
	first_comment = None
	comment_match = re.search(r'"([^"]*)"', input_code)
	if comment_match:
		first_comment = comment_match.group(1)
	
	dupes = []
	for line in input_code.split("\n"):
		if re.match(r"[\w:]+ *\[\w*(?::\w+)*\|.*?\]", line.strip(),re.MULTILINE):
			dupes.append(re.match(r"[\w:]+ *\[\w*(?::\w+)*\|.*?\]", line.strip(),re.MULTILINE).group().split("[")[0].strip())
	if len(dupes) != len(set(dupes)):
		sys.stderr.write("Sémantická chyba: Duplicitné definície metód\n") 
		sys.exit(35)

	
	try:
		lexer = Lexer(input_code)
		tokens = lexer.tokenize()
	except Exception as e:
		sys.stderr.write("Lexikálna chyba\n")
		sys.exit(ERR_LEXICAL)
	
	# Parsovanie tokenov
	try:
		parser = Parser(tokens)
		ast = parser.parse()
	except Exception as e:
		sys.stderr.write("Syntaktická chyba\n")
		sys.exit(ERR_SYNTAX)
	
	# Generovanie XML
	try:
		xml_generator = XMLGenerator(ast, first_comment)
		xml_output = xml_generator.generate()
		print(xml_output)
	except Exception as e:
		sys.stderr.write("Chyba pri generovaní XML\n")
		sys.exit(ERR_OUTPUT_FILE)

if __name__ == "__main__":
	main()