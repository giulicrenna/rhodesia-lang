import src.global_var as gb
from src.errors import *
from src.ast import *
from src.positioning import *

"""
Token class generate tokens
"""
class Token():
    def __init__(self, type_, value=None, pos_start = None, pos_end = None) -> None:
        self.type_ = type_
        self.value = value
        #self.pos_start = pos_start
        #self.pos_end = pos_end

        if pos_start:
            self.pos_start = pos_start.copy()
            self.pos_end = pos_start.copy()
            self.pos_end.next_char()
        if pos_end:
            self.pos_end = pos_end.copy()
        
    def __repr__(self) -> str:
        if self.value: return f'{self.type_}@{self.value}'
        return f'{self.type_}'

""" 
RHODESIA LEXER ANALYSIS
This is the main class where all the grammar will be analysed
"""
class Lexer():
    def __init__(self, file_name, command) -> None:
        self.file_name = file_name
        self.command = command
        self.posn = Position(-1, 0, -1, file_name, command)
        self.current_character = None
        self.next_character()
    def next_character(self) -> None:
        self.posn.next_char(self.current_character)
        self.current_character = self.command[self.posn.index] if self.posn.index < len(self.command) else None
    def create_number(self):
        number_str = ''
        dot_count = 0
        pos_start = self.posn.copy()
        
        while self.current_character != None and self.current_character in gb.NUMBERS + '.':
            if self.current_character == '.':
                if dot_count == 1: break
                dot_count += 1
                number_str += '.'
            else:
                number_str += self.current_character 
            self.next_character()
        if dot_count == 0:
            return Token(gb._TT_INT, int(number_str), pos_start, self.posn)
        else:
            return Token(gb._TT_FLOAT, float(number_str), pos_start, self.posn)
    def create_token(self):
        tokens = []

        while self.current_character != None:  
            if self.current_character in ' \t':
                self.next_character()
            elif self.current_character in gb.NUMBERS:
                tokens.append(self.create_number())
            elif self.current_character == '+':
                tokens.append(Token(gb._TT_ADD, pos_start=self.posn))
                self.next_character()
            elif self.current_character == '-':
                tokens.append(Token(gb._TT_SUBSTRACT, pos_start=self.posn))
                self.next_character()
            elif self.current_character == '*':
                tokens.append(Token(gb._TT_MULT, pos_start=self.posn))
                self.next_character()
            elif self.current_character == '/':
                tokens.append(Token(gb._TT_DIV, pos_start=self.posn))
                self.next_character()
            elif self.current_character == ')':
                tokens.append(Token(gb._TT_RPARENTHESIS, pos_start=self.posn))
                self.next_character()
            elif self.current_character == '(':
                tokens.append(Token(gb._TT_LPARENTHESIS, pos_start=self.posn))
                self.next_character()
            else:
                pos_start = self.posn.copy()
                char_ = self.current_character
                self.next_character()
                return [], IllegalCharacter(pos_start, self.posn, "> " + char_ + " <")

        return tokens, None
        

def start(file_name, comm):
    # Generates tokens
    lexing_engine = Lexer(file_name, comm)
    tokens, error_ = lexing_engine.create_token()
    if error_: return None, error_
    # Generates Abstract syntactic tree
    parser = Parser(tokens)
    ast = parser.parse()

    return ast.node, ast.error


