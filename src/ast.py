import src.global_var as gb 
import src.errors

"""
TREE NODES
"""
class NumberNode:
    def __init__(self, number_token) -> None:
        self.ntoken = number_token
    def __repr__(self) -> str:
        return f'{self.ntoken}'

class BinaryOperationNode:
    def __init__(self, left_node, operation, right_node) -> None:
        self.left_node = left_node
        self.operation = operation
        self.right_node = right_node
    def __repr__(self) -> str:
        return f'({self.left_node}, {self.operation}, {self.right_node})'

"""
PARSE RESULT
This checks if there is any error
"""
class ParseResult:
    def __init__(self) -> None:
        self.error = None
        self.node = None
    def register(self, res):
        if isinstance(res, ParseResult):
            if res.error: self.error = self.error
            return res.node
        return res
    def success(self, node):
        self.node = node
        return self
    def failure(self, error):
        self.error = error
        return self

"""
PARSING CLASS
"""
class Parser:
    def __init__(self, tokens) -> None:
        self.tokens = tokens
        self.t_index = -1
        self.advance()
    def parse(self):
        res = self.expr()
        if not res.error and self.curr_token.type_ != gb._TT_EOF:
            return res.failure(src.errors.InvalidSyntaxMistake(self.curr_token.pos_start,
                self.curr_token.pos_end, "Expected +, -, * or /"))
        return res
    def advance(self):
        self.t_index += 1
        if self.t_index < len(self.tokens):
            self.curr_token = self.tokens[self.t_index]
        return self.curr_token
    def factor(self):
        res = ParseResult()
        token = self.curr_token
        if token.type_ in (gb._TT_INT, gb._TT_FLOAT):
            res.register(self.advance())
            return res.success(NumberNode(token))
        return res.failure(src.errors.InvalidSyntaxMistake(token.pos_start,
        token.pos_end, "Expected Integer or floating number"))
    def term(self):
        return self.binary_operation(self.factor, (gb._TT_MULT, gb._TT_DIV))
    def expr(self):
        return self.binary_operation(self.term, (gb._TT_ADD, gb._TT_SUBSTRACT))
    def binary_operation(self, func, operations):
        res = ParseResult()
        left = res.register(func())
        if res.error: return res
        while self.curr_token.type_ in operations: #(gb._TT_MULT, gb._TT_DIV):
            operation_token = self.curr_token
            res.register(self.advance())
            right = res.register(func())
            if res.error: return res
            left = BinaryOperationNode(left, operation_token, right)
        return res.success(left)
