from src.str_w_arrows import string_with_arrows
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
        error_ += '\n\n' + string_with_arrows(self.poss.file_text, self.poss, self.pose)
        return error_

"""
This class advice when an illegal character is intended to be processed
"""
class IllegalCharacter(Error):
    def __init__(self, pos_start, pos_end, details) -> None:
        super().__init__(pos_start, pos_end, 'Illegal Character Type', details)

"""
This class advice when an illegal grammar expresion is evaluated
"""
class InvalidSyntaxMistake(Error):
    def __init__(self, pos_start, pos_end, details) -> None:
        super().__init__(pos_start, pos_end, 'Invalid Syntax', details)