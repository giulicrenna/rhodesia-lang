# Rhodesia Language Implementation Roadmap

## 🎯 Visión General

Rhodesia es un lenguaje imperativo de tipado fuerte diseñado para Data Science, con backend numérico basado en Eigen.

---

## Phase 1: Core Infrastructure COMPLETADA

### 1.1 Lexer (Análisis Léxico) 
- [x] Definir enum `TokenType` con todos los tokens del lenguaje
- [x] Implementar clase `Token` con tipo, valor, línea y columna
- [x] Implementar clase `Lexer` con métodos:
  - [x] `tokenize()` - Función principal
  - [x] `skipWhitespaceAndComments()` - Ignorar espacios y comentarios
  - [x] `readNumber()` - Parsear literales numéricos (int y float64)
  - [x] `readIdentifier()` - Parsear identificadores y keywords
  - [x] `readOperatorOrDelimiter()` - Parsear operadores (+, -, *, /, ->, etc.)
  - [x] `readString()` - Parsear literales string
- [x] Manejar keywords: `fun`, `return`, `for`, `if`, `else`, `in`, `while`, `break`, `continue`
- [x] Manejar tipos: `int`, `float64`, `vec`, `mat`, `void`
- [x] Implementar manejo de errores léxicos con posición
- [ ] Tests unitarios del lexer

### 1.2 AST (Árbol de Sintaxis Abstracta) 
- [x] Definir `RhoValue` como `std::variant<int64_t, double, std::string, Eigen::VectorXd, Eigen::MatrixXd>`
- [x] Definir enum `RhoType` para tipado estático
- [x] Implementar nodos base:
  - [x] `ASTNode` - Clase base abstracta con método `accept(Visitor&)`
  - [x] `ExprNode` - Nodo base para expresiones
  - [x] `StmtNode` - Nodo base para statements
- [x] Implementar nodos de expresión:
  - [x] `IntLiteralNode` - Literales enteros
  - [x] `FloatLiteralNode` - Literales float64
  - [x] `StringLiteralNode` - Literales string
  - [x] `BoolLiteralNode` - Literales booleanos
  - [x] `VectorLiteralNode` - Literales de vector `[1.0, 2.0, 3.0]`
  - [x] `MatrixLiteralNode` - Literales de matriz `[[1,2],[3,4]]`
  - [x] `IdentifierNode` - Referencias a variables
  - [x] `BinaryOpNode` - Operaciones binarias (+, -, *, /, %, ==, !=, <, >, <=, >=, and, or)
  - [x] `UnaryOpNode` - Operaciones unarias (-, not)
  - [x] `FunctionCallNode` - Llamadas a funciones
  - [x] `IndexAccessNode` - Acceso por índice `v[i]` o `m[i,j]`
- [x] Implementar nodos de statement:
  - [x] `VarDeclNode` - Declaración de variables `tipo: nombre = valor`
  - [x] `AssignmentNode` - Asignación `nombre = valor` e índice `v[i] = valor`
  - [x] `ReturnNode` - Sentencia return
  - [x] `ExprStmtNode` - Expresión como statement
  - [x] `BlockNode` - Bloque de código `{ ... }`
  - [x] `FunctionDeclNode` - Declaración de función
  - [x] `ForLoopNode` - Bucle for
  - [x] `WhileLoopNode` - Bucle while
  - [x] `IfStmtNode` - Condicional if/else
  - [x] `BreakNode` - Sentencia break
  - [x] `ContinueNode` - Sentencia continue
- [x] Implementar nodo raíz `ProgramNode`
- [ ] Tests de construcción del AST

### 1.3 Parser (Análisis Sintáctico) 
- [x] Implementar clase `Parser` con buffer de tokens
- [x] Implementar parsing de expresiones (Pratt Parser / Precedence Climbing):
  - [x] `parseExpression()` - Punto de entrada
  - [x] `parsePrimary()` - Literales, identificadores, paréntesis
  - [x] `parseOr()`, `parseAnd()`, `parseEquality()`, `parseComparison()` - Precedencia
  - [x] `parseTerm()`, `parseFactor()` - Aritmética
  - [x] `parseUnary()` - Operadores unarios
  - [x] `parsePostfix()` - Llamadas e índices
  - [x] `parseFunctionCall()` - Llamadas a funciones
  - [x] `parseArrayLiteral()` - Vectores y matrices
- [x] Implementar parsing de statements:
  - [x] `parseStatement()` - Dispatcher principal
  - [x] `parseVarDecl()` - Declaraciones de variables
  - [x] `parseAssignment()` - Asignaciones
  - [x] `parseReturn()` - Return statements
  - [x] `parseBlock()` - Bloques `{ ... }`
  - [x] `parseFunctionDecl()` - Funciones `fun name(...) -> type { }`
  - [x] `parseForLoop()` - Bucles `for i in expr { }`
  - [x] `parseWhileLoop()` - Bucles `while cond { }`
  - [x] `parseIfStmt()` - Condicionales
- [x] Implementar `parseProgram()` como entry point
- [x] Manejo de errores sintácticos
- [ ] Tests del parser

---

## Phase 2: Symbol Table & Type System COMPLETADA

### 2.1 Symbol Table 
- [x] Definir estructura `Symbol` con nombre, tipo, valor y ubicación
- [x] Implementar clase `SymbolTable`:
  - [x] Soporte para scopes anidados (stack de hash maps)
  - [x] `enterScope()` / `exitScope()`
  - [x] `declare(name, type, value)` - Declarar nueva variable
  - [x] `assign(name, value)` - Asignar valor
  - [x] `lookup(name)` - Buscar variable
- [x] `ScopeGuard` RAII para manejo automático de scopes
- [x] Registro de funciones definidas por usuario (`FunctionSignature`)
- [x] Detección de redeclaración en mismo scope
- [x] Detección de uso de variable no declarada
- [ ] Tests de symbol table

### 2.2 Type Checker (Runtime Check) Parcial
- [x] Validación de tipos en tiempo de ejecución
- [x] Validar compatibilidad de operaciones:
  - [x] `mat * mat` → `mat` (multiplicación matricial)
  - [x] `mat * vec` → `vec`
  - [x] `vec + vec` → `vec`
  - [x] `scalar * mat` → `mat` (broadcasting)
- [x] Validar argumentos de funciones built-in
- [ ] Implementar `TypeChecker` visitor para validación estática (opcional)
- [ ] Reportar errores de tipo con sugerencias

---

## Phase 3: Evaluator & Built-ins COMPLETADA

### 3.1 Evaluator (Interpreter)
- [x] Implementar clase `Evaluator` como Visitor:
  - [x] `visit(IntLiteralNode&)` → `int64_t`
  - [x] `visit(FloatLiteralNode&)` → `double`
  - [x] `visit(StringLiteralNode&)` → `std::string`
  - [x] `visit(BoolLiteralNode&)` → `int64_t` (0 o 1)
  - [x] `visit(VectorLiteralNode&)` → `Eigen::VectorXd`
  - [x] `visit(MatrixLiteralNode&)` → `Eigen::MatrixXd`
  - [x] `visit(IdentifierNode&)` → lookup en symbol table
  - [x] `visit(BinaryOpNode&)` → evaluar con dispatch por tipo
  - [x] `visit(UnaryOpNode&)` → negación, not
  - [x] `visit(FunctionCallNode&)` → dispatch a built-ins o user-defined
  - [x] `visit(IndexAccessNode&)` → acceso a elementos
- [x] Implementar evaluación de statements:
  - [x] `visit(VarDeclNode&)` → declarar en symbol table
  - [x] `visit(AssignmentNode&)` → asignar en symbol table
  - [x] `visit(ReturnNode&)` → throw `ReturnValue` exception
  - [x] `visit(BlockNode&)` → evaluar secuencia con scope
  - [x] `visit(FunctionDeclNode&)` → registrar función
  - [x] `visit(ForLoopNode&)` → iteración con scope por iteración
  - [x] `visit(WhileLoopNode&)` → iteración con scope por iteración
  - [x] `visit(IfStmtNode&)` → evaluación condicional
  - [x] `visit(BreakNode&)` → throw `BreakException`
  - [x] `visit(ContinueNode&)` → throw `ContinueException`
- [x] Manejo de errores en runtime con mensajes claros
- [ ] Tests del evaluador

### 3.2 Built-in Functions
- [x] Implementar registro de built-ins con `BuiltinRegistry`:
  - [x] `norm(vec)` → `double` (norma L2)
  - [x] `norm(mat)` → `double` (norma Frobenius)
  - [x] `dot(vec, vec)` → `double` (producto punto)
  - [x] `transpose(mat)` → `mat`
  - [x] `transpose(vec)` → `mat` (row vector)
  - [x] `inv(mat)` → `mat` (inversa)
  - [x] `sum(vec)` → `double`
  - [x] `sum(mat)` → `double`
  - [x] `mean(vec)` → `double`
  - [x] `mean(mat)` → `double`
- [x] Implementar funciones de creación:
  - [x] `zeros(n)` → `vec` de ceros
  - [x] `zeros(n, m)` → `mat` de ceros
  - [x] `ones(n)` → `vec` de unos
  - [x] `ones(n, m)` → `mat` de unos
  - [x] `eye(n)` → `mat` identidad
  - [x] `range(n)` → `vec` de 0 a n-1
  - [x] `range(start, end)` → `vec`
- [x] Implementar funciones matemáticas (element-wise para vectores):
  - [x] `sqrt(x)` - Raíz cuadrada
  - [x] `exp(x)` - Exponencial
  - [x] `log(x)` - Logaritmo natural
  - [x] `abs(x)` - Valor absoluto
  - [x] `sin(x)`, `cos(x)`, `tan(x)` - Trigonometría (solo scalares)
- [x] Funciones de información:
  - [x] `rows(mat)` → `int`
  - [x] `cols(mat)` → `int`
  - [x] `size(vec)` → `int`
  - [x] `size(mat)` → `int` (total de elementos)
- [x] I/O Functions:
  - [x] `print(...)` - Imprimir sin newline (múltiples args)
  - [x] `println(...)` - Imprimir con newline (múltiples args)
- [x] Validación de argumentos con `ArgumentError`
- [ ] Tests de built-ins

---

## Phase 4: Advanced Features COMPLETADA

### 4.1 Control Flow Avanzado 
- [x] Soporte para `break` y `continue` en loops
- [x] Soporte para `while` loops
- [x] Operadores de comparación: `<`, `>`, `<=`, `>=`, `==`, `!=`
- [x] Operadores lógicos: `and`, `or`, `not`
- [ ] Expresión ternaria: `cond ? expr1 : expr2`

### 4.2 Funciones Definidas por Usuario 
- [x] Almacenar funciones en registry
- [x] Crear nuevo scope para llamada a función
- [x] Pasar argumentos por valor
- [x] Manejar recursión
- [x] Verificar tipos de argumentos vs parámetros (runtime)
- [x] Verificar tipo de retorno (runtime)
- [x] Funciones void sin return

### 4.3 Operaciones Matriciales Avanzadas Parcial
- [x] Slicing: `v<start:end>`, `m<row_start:row_end, col_start:col_end>`
- [x] Broadcasting para operaciones escalar-matriz y escalar-vector
- [ ] Concatenación: `hstack(mat, mat)`, `vstack(mat, mat)`
- [ ] Reshape: `reshape(mat, rows, cols)`
- [ ] Operaciones element-wise explícitas: `.*`, `./`

### 4.4 I/O y Utilidades 
- [x] `print(values...)` - Imprimir valores separados por espacio
- [x] `println(values...)` - Imprimir con newline
- [ ] `load_csv(filename)` → `mat`
- [ ] `save_csv(mat, filename)`
- [x] Comentarios: `// single line` y `/* multi line */`
- [x] String literals con escape sequences (`\n`, `\t`, `\\`, `\"`)

---

## Phase 5: Optimization & Polish EN PROGRESO

### 5.1 Optimización Pendiente
- [ ] Lazy evaluation para operaciones encadenadas (Eigen Expression Templates)
- [ ] Evitar copias innecesarias (move semantics)
- [ ] Pool de memoria para matrices temporales
- [ ] Cache de funciones built-in lookup

### 5.2 Error Handling 
- [x] Mensajes de error descriptivos con línea y columna
- [x] Jerarquía de excepciones (`RhoError`, `LexerError`, `ParseError`, `RuntimeError`, etc.)
- [ ] Stack trace para errores en funciones anidadas
- [ ] Sugerencias de corrección ("Did you mean...?")
- [ ] Warnings para operaciones potencialmente problemáticas

### 5.3 REPL 
- [x] Implementar Read-Eval-Print Loop interactivo
- [x] Soporte multi-línea automático (detecta braces desbalanceados)
- [x] Comando `help` para ayuda
- [x] Comando `vars` para listar variables
- [x] Comando `exit`/`quit` para salir
- [x] Pretty-print de matrices y vectores
- [ ] Historial de comandos (readline)
- [ ] Auto-completado de identificadores

### 5.4 Documentación y Testing Parcial
- [x] README completo con ejemplos
- [x] Documentación de API de built-ins
- [x] Ejemplos de casos de uso (regresión, matrices, vectores, control flow, data science)
- [ ] Suite de tests unitarios
- [ ] Suite de tests de integración
- [ ] Benchmarks de performance vs NumPy/MATLAB

---

## Phase 6: Extensions (Futuro) PENDIENTE

### 6.1 Compilador a C++
- [ ] Backend que genere código C++ optimizado
- [ ] Linking con Eigen para código compilado
- [ ] Compilación JIT opcional

### 6.2 Interoperabilidad
- [ ] Bindings Python para usar Rhodesia desde Python
- [ ] Importar/exportar NumPy arrays
- [ ] Integración con Jupyter notebooks

### 6.3 Paralelización
- [ ] Usar Eigen con OpenMP
- [ ] Operaciones paralelas explícitas
- [ ] GPU support (CUDA/OpenCL)

### 6.4 Características Adicionales
- [ ] Números complejos (`complex` type)
- [ ] Sparse matrices (`sparse_mat`)
- [ ] Random number generation (`rand`, `randn`, `seed`)
- [ ] Statistical functions (`std`, `var`, `cov`, `corr`)
- [ ] Linear algebra (`eig`, `svd`, `qr`, `lu`, `chol`)
- [ ] Sorting and searching (`sort`, `argsort`, `find`)

---

## 📊 Progreso Actual

| Phase | Componente | Estado | Notas |
|-------|-----------|--------|-------|
| 1.1 | Lexer | Completo | Tokens, keywords, operators, strings |
| 1.2 | AST | Completo | Todos los nodos implementados |
| 1.3 | Parser | Completo | Recursive descent con precedence climbing |
| 2.1 | Symbol Table | Completo | Scopes anidados, funciones |
| 2.2 | Type Checker | Runtime | Validación en tiempo de ejecución |
| 3.1 | Evaluator | Completo | Visitor pattern, Eigen integration |
| 3.2 | Built-ins | Completo | 25+ funciones implementadas |
| 4.1 | Control Flow | Completo | if/else, for, while, break, continue |
| 4.2 | User Functions | Completo | Recursión, scopes, tipos |
| 4.3 | Matrix Ops | Parcial | Falta slicing, concat |
| 4.4 | I/O | Parcial | Falta CSV I/O |
| 5.x | Polish | Parcial | REPL funcional, docs completas |
| 6.x | Extensions | Pendiente | Futuro |

### Leyenda
- Completado
- En progreso / Parcial
- Pendiente

---

## 🔧 Comandos de Build

```bash
# Compilar
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make -j$(nproc)

# Ejecutar archivo .rho
./rhodesia examples/hello.rho

# Ejecutar código inline
./rhodesia -e "println(norm([3, 4]))"

# REPL interactivo
./rhodesia
```

---

## 📁 Estructura del Proyecto

```
rhodesia/
├── CMakeLists.txt        # Build configuration
├── README.md             # Documentation
├── ROADMAP.md            # This file
├── include/
│   ├── RhoValue.hpp      # Core type system (variant-based)
│   ├── Token.hpp         # Token definitions and keywords
│   ├── Lexer.hpp         # Lexical analyzer
│   ├── Error.hpp         # Error hierarchy and handling
│   ├── AST.hpp           # Abstract Syntax Tree nodes
│   ├── Parser.hpp        # Recursive descent parser
│   ├── SymbolTable.hpp   # Variable/function storage with scopes
│   ├── Builtins.hpp      # Built-in function registry
│   └── Evaluator.hpp     # AST interpreter (Visitor pattern)
├── src/
│   └── main.cpp          # Entry point (REPL + file execution)
├── examples/
│   ├── hello.rho         # Hello world and basics
│   ├── vector_demo.rho   # Vector operations
│   ├── matrix_demo.rho   # Matrix operations
│   ├── control_flow.rho  # Control structures demo
│   ├── regression.rho    # Linear regression example
│   └── data_science.rho  # Full ML pipeline example
└── build/                # Build output directory
```

---

## Próximos Pasos Inmediatos

1. **Testing**: Crear suite de tests unitarios con GoogleTest o Catch2
2. **CSV I/O**: Implementar `load_csv()` y `save_csv()`
3. **Random**: Agregar `rand()`, `randn()`, `seed()`
4. ~~**Statistics**: Agregar `std()`, `var()`, `cov()`~~ ✅ **COMPLETADO**
5. **Concatenación**: Implementar `hstack()` y `vstack()`

---

## Métricas del Proyecto

- **Líneas de código**: ~3000 LOC (header-only C++20)
- **Built-in functions**: 25+
- **Example programs**: 6
- **Supported platforms**: Linux, macOS (con Eigen3)
