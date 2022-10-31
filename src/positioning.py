# POSITION
class Position():
    def __init__(self, index, row, column, file_name, file_text) -> None:
        self.index = index
        self.row = row
        self.column = column
        self.file_name = file_name
        self.file_text = file_text
    def next_char(self, curr_char=None):
        self.index += 1
        self.column += 1
        
        if curr_char == '\n':
            self.row += 1
            self.column = 0

        return self
    
    def copy(self):
        return Position(self.index, self.row, self.column, self.file_name, self.file_text)
