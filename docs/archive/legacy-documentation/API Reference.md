# Referencia de API Técnica de Rhodesia

## Introducción

Esta referencia técnica documenta la arquitectura interna del lenguaje Rhodesia, incluyendo las clases principales, estructuras de datos y algoritmos de implementación. Está destinada a desarrolladores que necesitan entender o extender el lenguaje.

## Arquitectura General

Rhodesia sigue una arquitectura de compilador/interprete típica:

```
Fuente → Lexer → Parser → AST → Evaluator → Resultado
```

- **Lexer**: Análisis léxico, tokenización
- **Parser**: Análisis sintáctico, construcción del AST
- **AST**: Árbol de sintaxis abstracta
- **Evaluator**: Intérprete que ejecuta el AST
- **Symbol Table**: Gestión de variables y funciones

## Sistema de Tipos (RhoValue)

### Clase RhoValue

```cpp
using RhoValue = std::variant<
    int64_t,        // Enteros
    double,         // Float64
    std::string,    // Cadenas
    Eigen::VectorXd,// Vectores
    Eigen::MatrixXd // Matrices
>;
```

### Funciones Utilitarias

#### getValueType(const RhoValue&)

Retorna el tipo runtime de un valor.

```cpp
RhoType getValueType(const RhoValue& value) {
    return std::visit([](auto&& arg) -> RhoType {
        using T = std::decay_t<decltype(arg)>;
        if constexpr (std::is_same_v<T, int64_t>) return RhoType::Int;
        // ... otros tipos
    }, value);
}
```

#### Conversión de Tipos

```cpp
double toDouble(const RhoValue& value);    // Convierte a double
int64_t toInt(const RhoValue& value);       // Convierte a int64_t
bool isScalar(const RhoValue& value);       // Verifica si es escalar
bool isNumeric(const RhoValue& value);      // Verifica si es numérico
```

#### Formateo de Valores

```cpp
std::string valueToString(const RhoValue& value, int precision = 6);
```

Esta función formatea valores para display, con manejo especial para matrices grandes (truncamiento automático).

## Tokens y Lexing

### Enum TokenType

Define todos los tipos de token reconocidos por el lexer:

```cpp
enum class TokenType {
    // Literales
    IntLiteral, FloatLiteral, StringLiteral,

    // Identificadores y Keywords
    Identifier,

    // Keywords de tipo
    KwInt, KwFloat64, KwVec, KwMat, KwVoid,

    // Keywords de control
    KwFun, KwReturn, KwFor, KwIn, KwIf, KwElse,
    KwWhile, KwBreak, KwContinue,

    // Keywords lógicos
    KwAnd, KwOr, KwNot, KwTrue, KwFalse,

    // Operadores aritméticos
    Plus, Minus, Star, Slash, Percent,

    // Operadores de comparación
    Equal, NotEqual, Less, Greater, LessEqual, GreaterEqual,

    // Operadores de asignación
    Assign,

    // Delimitadores
    LParen, RParen, LBracket, RBracket, LBrace, RBrace,
    Comma, Colon, Semicolon, Arrow,

    // Especiales
    Newline, Eof, Invalid
};
```

### Estructura Token

```cpp
struct Token {
    TokenType type;
    std::string value;
    SourceLocation location;

    Token(TokenType t, std::string v, SourceLocation loc)
        : type(t), value(std::move(v)), location(loc) {}

    bool is(TokenType t) const { return type == t; }
    bool isOneOf(std::initializer_list<TokenType> types) const;
    bool isKeyword() const;
    bool isTypeKeyword() const;
    bool isOperator() const;
    bool isComparisonOperator() const;
    bool isArithmeticOperator() const;
};
```

### Clase Lexer

```cpp
class Lexer {
public:
    explicit Lexer(std::string source);

    Token nextToken();
    const Token& peekToken() const;
    void consumeToken();

private:
    std::string source_;
    size_t position_ = 0;
    size_t line_ = 1;
    size_t column_ = 1;
    Token current_token_;

    void skipWhitespaceAndComments();
    Token readNumber();
    Token readIdentifier();
    Token readOperatorOrDelimiter();
    Token readString();

    char peek() const;
    char consume();
    bool isAtEnd() const;
};
```

#### Métodos del Lexer

- **nextToken()**: Retorna el siguiente token
- **peekToken()**: Mira el siguiente token sin consumirlo
- **consumeToken()**: Consume el token actual

#### Manejo de Errores Léxicos

El lexer lanza `LexerError` para tokens inválidos:

```cpp
class LexerError : public RhoError {
public:
    LexerError(const std::string& message, SourceLocation loc);
};
```

## Árbol de Sintaxis Abstracta (AST)

### Jerarquía de Nodos

Todos los nodos AST heredan de `ASTNode`:

```cpp
class ASTNode {
public:
    virtual ~ASTNode() = default;
    virtual void accept(Visitor& visitor) = 0;
    virtual SourceLocation getLocation() const = 0;
};
```

### Nodos de Expresión

```cpp
class ExprNode : public ASTNode {
    // Clase base para expresiones
};

class IntLiteralNode : public ExprNode {
    int64_t value;
};

class FloatLiteralNode : public ExprNode {
    double value;
};

class VectorLiteralNode : public ExprNode {
    std::vector<std::unique_ptr<ExprNode>> elements;
};

class MatrixLiteralNode : public ExprNode {
    std::vector<std::vector<std::unique_ptr<ExprNode>>> rows;
};

class IdentifierNode : public ExprNode {
    std::string name;
};

class BinaryOpNode : public ExprNode {
    std::unique_ptr<ExprNode> left;
    TokenType op;
    std::unique_ptr<ExprNode> right;
};

class FunctionCallNode : public ExprNode {
    std::string function_name;
    std::vector<std::unique_ptr<ExprNode>> arguments;
};

class IndexAccessNode : public ExprNode {
    std::unique_ptr<ExprNode> target;
    std::unique_ptr<ExprNode> index;  // Para acceso simple
    std::unique_ptr<ExprNode> row_index;    // Para matrices
    std::unique_ptr<ExprNode> col_index;
};
```

### Nodos de Sentencia

```cpp
class StmtNode : public ASTNode {
    // Clase base para sentencias
};

class VarDeclNode : public StmtNode {
    RhoType type;
    std::string name;
    std::unique_ptr<ExprNode> initializer;
};

class AssignmentNode : public StmtNode {
    std::string name;
    std::unique_ptr<ExprNode> value;
    std::unique_ptr<IndexAccessNode> index;  // Para asignación indexada
};

class ReturnNode : public StmtNode {
    std::unique_ptr<ExprNode> value;  // nullptr para return void
};

class BlockNode : public StmtNode {
    std::vector<std::unique_ptr<StmtNode>> statements;
};

class IfStmtNode : public StmtNode {
    std::unique_ptr<ExprNode> condition;
    std::unique_ptr<BlockNode> then_branch;
    std::unique_ptr<BlockNode> else_branch;  // nullptr si no hay else
};

class ForLoopNode : public StmtNode {
    std::string iterator_name;
    std::unique_ptr<ExprNode> iterable;
    std::unique_ptr<BlockNode> body;
};

class WhileLoopNode : public StmtNode {
    std::unique_ptr<ExprNode> condition;
    std::unique_ptr<BlockNode> body;
};

class FunctionDeclNode : public StmtNode {
    std::string name;
    std::vector<std::pair<RhoType, std::string>> parameters;
    RhoType return_type;
    std::unique_ptr<BlockNode> body;
};
```

### Nodo Raíz

```cpp
class ProgramNode : public ASTNode {
    std::vector<std::unique_ptr<StmtNode>> statements;
};
```

## Parser (Análisis Sintáctico)

### Clase Parser

```cpp
class Parser {
public:
    explicit Parser(std::vector<Token> tokens);

    std::unique_ptr<ProgramNode> parseProgram();

private:
    std::vector<Token> tokens_;
    size_t current_ = 0;

    // Métodos de parsing principales
    std::unique_ptr<StmtNode> parseStatement();
    std::unique_ptr<ExprNode> parseExpression();

    // Parsing de expresiones (Precedence Climbing)
    std::unique_ptr<ExprNode> parsePrimary();
    std::unique_ptr<ExprNode> parseOr();
    std::unique_ptr<ExprNode> parseAnd();
    std::unique_ptr<ExprNode> parseEquality();
    std::unique_ptr<ExprNode> parseComparison();
    std::unique_ptr<ExprNode> parseTerm();
    std::unique_ptr<ExprNode> parseFactor();
    std::unique_ptr<ExprNode> parseUnary();
    std::unique_ptr<ExprNode> parsePostfix();

    // Parsing de sentencias
    std::unique_ptr<StmtNode> parseVarDecl();
    std::unique_ptr<StmtNode> parseAssignment();
    std::unique_ptr<StmtNode> parseReturn();
    std::unique_ptr<StmtNode> parseBlock();
    std::unique_ptr<StmtNode> parseFunctionDecl();
    std::unique_ptr<StmtNode> parseForLoop();
    std::unique_ptr<StmtNode> parseWhileLoop();
    std::unique_ptr<StmtNode> parseIfStmt();

    // Utilidades
    Token consume(TokenType expected, const std::string& error_msg);
    bool match(TokenType type);
    bool check(TokenType type) const;
    Token advance();
    Token peek() const;
    bool isAtEnd() const;
};
```

### Algoritmo de Parsing

El parser usa **Precedence Climbing** para expresiones, con la siguiente precedencia:

1. Primarios (literales, identificadores, paréntesis)
2. Postfijos (llamadas a función, índices)
3. Unarios (`-`, `not`)
4. Multiplicativos (`*`, `/`, `%`)
5. Aditivos (`+`, `-`)
6. Comparación (`<`, `<=`, `>`, `>=`)
7. Igualdad (`==`, `!=`)
8. Lógicos AND (`and`)
9. Lógicos OR (`or`)

## Tabla de Símbolos

### Clase SymbolTable

```cpp
class SymbolTable {
public:
    SymbolTable();

    void enterScope();
    void exitScope();

    void declare(const std::string& name, RhoType type,
                 const RhoValue& value = {});
    void assign(const std::string& name, const RhoValue& value);
    RhoValue lookup(const std::string& name) const;

    // Para funciones
    void declareFunction(const std::string& name,
                        const std::vector<RhoType>& param_types,
                        RhoType return_type,
                        std::unique_ptr<FunctionDeclNode> node);

    std::unique_ptr<FunctionDeclNode> lookupFunction(const std::string& name) const;

private:
    struct Symbol {
        RhoType type;
        RhoValue value;
        SourceLocation declaration_location;
    };

    std::vector<std::unordered_map<std::string, Symbol>> scopes_;
    std::unordered_map<std::string, std::unique_ptr<FunctionDeclNode>> functions_;
};
```

### Gestión de Ámbitos (Scopes)

- **Scopes anidados**: Stack de mapas hash
- **Shadowing**: Variables locales pueden ocultar variables externas
- **Re-declaración**: No permitida en el mismo scope
- **Variables no declaradas**: Error en tiempo de ejecución

### RAII para Scopes

```cpp
class ScopeGuard {
public:
    explicit ScopeGuard(SymbolTable& table) : table_(table) {
        table_.enterScope();
    }
    ~ScopeGuard() {
        table_.exitScope();
    }
private:
    SymbolTable& table_;
};
```

## Evaluator (Intérprete)

### Patrón Visitor

El evaluator implementa el patrón Visitor para recorrer el AST:

```cpp
class Evaluator {
public:
    explicit Evaluator(SymbolTable& symbol_table);

    RhoValue evaluate(const ProgramNode& program);

private:
    SymbolTable& symbol_table_;

    // Visit methods for expressions
    RhoValue visit(const IntLiteralNode& node);
    RhoValue visit(const FloatLiteralNode& node);
    RhoValue visit(const VectorLiteralNode& node);
    RhoValue visit(const MatrixLiteralNode& node);
    RhoValue visit(const IdentifierNode& node);
    RhoValue visit(const BinaryOpNode& node);
    RhoValue visit(const FunctionCallNode& node);
    RhoValue visit(const IndexAccessNode& node);

    // Visit methods for statements
    void visit(const VarDeclNode& node);
    void visit(const AssignmentNode& node);
    void visit(const ReturnNode& node);
    void visit(const BlockNode& node);
    void visit(const IfStmtNode& node);
    void visit(const ForLoopNode& node);
    void visit(const WhileLoopNode& node);
    void visit(const FunctionDeclNode& node);

    // Helper methods
    RhoValue evaluateBinaryOp(TokenType op, const RhoValue& left, const RhoValue& right);
    void checkTypeCompatibility(RhoType expected, RhoType actual, SourceLocation loc);
};
```

### Evaluación de Operaciones Binarias

```cpp
RhoValue Evaluator::evaluateBinaryOp(TokenType op, const RhoValue& left, const RhoValue& right) {
    // Dispatch basado en tipos usando std::visit
    return std::visit([&](auto&& l, auto&& r) -> RhoValue {
        using L = std::decay_t<decltype(l)>;
        using R = std::decay_t<decltype(r)>;

        // Implementación específica por tipo...
        if constexpr (std::is_same_v<L, int64_t> && std::is_same_v<R, int64_t>) {
            switch (op) {
                case TokenType::Plus: return l + r;
                case TokenType::Minus: return l - r;
                // ... otros operadores
            }
        }
        // ... otros casos de tipos
    }, left, right);
}
```

### Manejo de Control de Flujo

Para `break` y `continue`, se usan excepciones:

```cpp
class BreakException : public std::exception {};
class ContinueException : public std::exception {};
class ReturnValue : public std::exception {
public:
    RhoValue value;
    explicit ReturnValue(RhoValue v) : value(std::move(v)) {}
};
```

## Sistema de Errores

### Jerarquía de Errores

```cpp
class RhoError : public std::runtime_error {
public:
    RhoError(const std::string& message, SourceLocation location);
    virtual ~RhoError() = default;

    SourceLocation location() const { return location_; }

private:
    SourceLocation location_;
};

class LexerError : public RhoError {};
class ParseError : public RhoError {};
class RuntimeError : public RhoError {};
class TypeError : public RuntimeError {};
class ArgumentError : public RuntimeError {};
```

### Errores Comunes

#### Errores de Lexer
- Tokens inválidos
- Strings sin cerrar
- Números malformados

#### Errores de Parser
- Tokens inesperados
- Paréntesis/desbalanceados
- Sentencias incompletas

#### Errores de Runtime
- Variables no declaradas
- Tipos incompatibles
- Índices fuera de rango
- Matrices singulares (en inversión)

## Funciones Built-in

### Arquitectura de Built-ins

```cpp
using BuiltinFunc = std::function<RhoValue(const std::vector<RhoValue>&, SourceLocation)>;

class Builtins {
public:
    static Builtins& instance();

    bool isBuiltin(const std::string& name) const;
    RhoValue call(const std::string& name,
                  const std::vector<RhoValue>& args,
                  SourceLocation loc = {});

private:
    std::unordered_map<std::string, BuiltinFunc> functions_;
    Builtins();
    void registerAll();
};
```

### Implementación de Funciones

Cada función built-in valida sus argumentos y ejecuta la operación correspondiente:

```cpp
functions_["dot"] = [](const std::vector<RhoValue>& args, SourceLocation loc) -> RhoValue {
    if (args.size() != 2) {
        throw ArgumentError::wrongCount("dot", 2, args.size(), loc);
    }

    auto* v1 = std::get_if<Eigen::VectorXd>(&args[0]);
    auto* v2 = std::get_if<Eigen::VectorXd>(&args[1]);

    if (!v1 || !v2) {
        throw ArgumentError("dot", "both arguments must be vec", loc);
    }

    if (v1->size() != v2->size()) {
        throw RuntimeError::dimensionMismatch("dot",
            std::to_string(v1->size()), std::to_string(v2->size()), loc);
    }

    return v1->dot(*v2);
};
```

## Optimizaciones y Rendimiento

### Eigen Backend

- **Expresiones template**: Evaluación lazy para evitar copias temporales
- **SIMD**: Instrucciones vectoriales automáticas
- **Cache-aware**: Optimización de acceso a memoria
- **OpenMP**: Paralelización opcional para matrices grandes

### Broadcasting Automático

```cpp
// Escalares broadcast a vectores/matrices automáticamente
vec: v = [1, 2, 3];
vec: scaled = 2 * v;        // Broadcasting: escalar * vector
vec: shifted = v + 1;       // Broadcasting: vector + escalar

// Compatible con operaciones elemento-a-elemento
vec: result = (v * 2) + 1;  // [3, 5, 7]
```

### Gestión de Memoria

- **RAII**: Todos los recursos manejados automáticamente
- **Move semantics**: Transferencia eficiente de objetos grandes
- **Eigen memory pool**: Reutilización de buffers temporales

## Extensiones Futuras

### Compilador JIT

Posible extensión para generar código LLVM/Clang para rendimiento máximo.

### Backend CUDA/OpenCL

Aprovechar GPUs para operaciones matriciales masivas.

### Interoperabilidad

- **Python bindings**: Usar Rhodesia desde Python
- **NumPy integration**: Intercambio directo de arrays
- **Jupyter kernel**: Notebooks interactivos

Esta referencia técnica proporciona la base para entender y extender Rhodesia. La arquitectura modular facilita agregar nuevas características mientras mantiene el rendimiento y la corrección.
