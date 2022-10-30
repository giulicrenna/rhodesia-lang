# CONSTANT TYPES
NUMBERS = '0123456789'

# POSITION

class Position():
    def __init__(self, index, row, column, file_name, file_text) -> None:
        self.index = index
        self.row = row
        self.column = column
        self.file_name = file_name
        self.file_text = file_text
    def next_char(self, curr_char):
        self.index += 1
        self.column += 1
        
        if curr_char == '\n':
            self.row += 1
            self.column = 0

        return self
    
    def copy(self):
        return Position(self.index, self.row, self.column, self.file_name, self.file_text)

# ERROR CLASS
class Error():
    def __init__(self, pos_start, pos_end, error_type, details) -> None:
        self.poss = pos_start
        self.pose = pos_end
        self.type = error_type
        self.det = details
    def error_string(self) -> str:
        error_ = f'{self.type} --> {self.det}'
        error_ += f'\nFile: {self.poss.file_name}, line {self.poss.row+1}'
        return error_

class IllegalCharacter(Error):
    def __init__(self, pos_start, pos_end, details) -> None:
        super().__init__(pos_start, pos_end, 'Illegal Character Type', details)

# TOKENS TYPES
_TT_INT = 'INT'
_TT_FLOAT = 'FLOAT'
_TT_SUBSTRACT = 'SUBSTRACT'
_TT_ADD = 'ADD'
_TT_MULT = 'MULT'
_TT_DIV = 'DIV'
_TT_LPARENTHESIS = 'LPAREN'
_TT_RPARENTHESIS = 'RPAREN'

class Token():
    def __init__(self, type_, value=None) -> None:
        self.type_ = type_
        self.value = value
    def __repr__(self) -> str:
        if self.value: return f'{self.type_}@{self.value}'
        return f'{self.type_}@None'

# RHODESIA LEXER ANALYSIS

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
    def create_number(self) -> str:
        number_str = ''
        dot_count = 0
        while self.current_character != None and self.current_character in NUMBERS + '.':
            if self.current_character == '.':
                if dot_count == 1: break
                dot_count += 1
                number_str += '.'
            else:
                number_str += self.current_character 
            self.next_character()
        if dot_count == 0:
            return Token(_TT_INT, int(number_str))
        else:
            return Token(_TT_FLOAT, float(number_str))
    def create_token(self):
        tokens = []

        while self.current_character != None:  
            if self.current_character in ' \t':
                self.next_character()
            elif self.current_character in NUMBERS:
                tokens.append(self.create_number())
            elif self.current_character == '+':
                tokens.append(Token(_TT_ADD))
                self.next_character()
            elif self.current_character == '-':
                tokens.append(Token(_TT_SUBSTRACT))
                self.next_character()
            elif self.current_character == '*':
                tokens.append(Token(_TT_MULT))
                self.next_character()
            elif self.current_character == '/':
                tokens.append(Token(_TT_DIV))
                self.next_character()
            elif self.current_character == ')':
                tokens.append(Token(_TT_RPARENTHESIS))
                self.next_character()
            elif self.current_character == '(':
                tokens.append(Token(_TT_LPARENTHESIS))
                self.next_character()
            else:
                pos_start = self.posn.copy()
                char_ = self.current_character
                self.next_character()
                return [], IllegalCharacter(pos_start, self.posn, "> " + char_ + " <")

        return tokens, None
        

def start(file_name, comm):
    lexing_engine = Lexer(file_name, comm)
    tokens, error_ = lexing_engine.create_token()

    return tokens, error_


