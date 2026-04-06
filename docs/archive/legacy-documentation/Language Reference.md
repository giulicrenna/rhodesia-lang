# Referencia del Lenguaje Rhodesia

## Introducción

Rhodesia es un lenguaje de programación imperativo de tipado fuerte diseñado específicamente para Ciencia de Datos y computación numérica. Combina una sintaxis limpia y legible con operaciones numéricas de alto rendimiento basadas en la biblioteca Eigen.

## Sintaxis General

### Comentarios

Rhodesia soporta comentarios de una línea y multilínea:

```rhodesia
// Este es un comentario de una línea

/*
   Este es un comentario
   multilínea
*/
```

### Separación de Sentencias

Las sentencias se separan automáticamente por nuevas líneas. No se requiere punto y coma (`;`), aunque puede usarse opcionalmente para claridad.

## Sistema de Tipos

Rhodesia tiene un sistema de tipos estático con cinco tipos primitivos:

| Tipo | Descripción | Mapeo C++ | Ejemplo |
|------|-------------|-----------|---------|
| `int` | Entero de 64 bits | `int64_t` | `42`, `-15` |
| `float64` | Flotante de doble precisión | `double` | `3.14159`, `-2.5` |
| `vec` | Vector columna dinámico | `Eigen::VectorXd` | `[1.0, 2.0, 3.0]` |
| `mat` | Matriz dinámica | `Eigen::MatrixXd` | `[[1, 2], [3, 4]]` |
| `string` | Cadena de texto | `std::string` | `"Hola Mundo"` |

### Conversión de Tipos

Rhodesia realiza conversión automática en operaciones aritméticas entre tipos compatibles:

- `int` ↔ `float64`: Conversión automática
- Operaciones elemento-a-elemento entre escalares y vectores/matrices (broadcasting)
- Multiplicación matricial: `mat * mat`, `mat * vec`

## Literales

### Literales Numéricos

```rhodesia
// Enteros
int: a = 42
int: b = -15

// Flotantes
float64: pi = 3.14159
float64: e = 2.718

// Notación científica
float64: avogadro = 6.022e23
```

### Literales de Vector

```rhodesia
// Vectores columna
vec: v1 = [1, 2, 3]
vec: v2 = [1.0, 2.5, -3.14]
vec: empty = []  // Vector vacío
```

### Literales de Matriz

```rhodesia
// Matrices (filas separadas por comas)
mat: m1 = [[1, 2], [3, 4]]
mat: m2 = [[1.0, 2.0, 3.0],
           [4.0, 5.0, 6.0]]

// Matriz identidad 3x3
mat: identity = [[1, 0, 0],
                 [0, 1, 0],
                 [0, 0, 1]]
```

### Literales Booleanos

```rhodesia
int: t = true   // 1
int: f = false  // 0
```

### Literales de String

```rhodesia
string: greeting = "Hola Mundo"
string: multiline = "Línea 1\nLínea 2"
string: escaped = "Comillas: \", Barra invertida: \\"
```

## Declaración de Variables

### Sintaxis Básica

```rhodesia
tipo: nombre = valor
```

### Ejemplos

```rhodesia
int: edad = 25
float64: pi = 3.14159
vec: coordenadas = [10.5, 20.3]
mat: transformacion = [[1, 0], [0, 1]]
string: nombre = "Rhodesia"
```

### Reglas de Nombres

- Los nombres de variables deben comenzar con una letra o guión bajo (`_`)
- Pueden contener letras, dígitos y guiones bajos
- No pueden ser palabras reservadas del lenguaje
- Case-sensitive (mayúsculas y minúsculas se distinguen)

### Variables sin Inicialización

Rhodesia requiere inicialización explícita de todas las variables. No se permiten variables sin valor inicial.

## Operadores

### Precedencia de Operadores

| Precedencia | Operador | Descripción | Asociatividad |
|-------------|----------|-------------|---------------|
| 1 (más alta) | `()` | Llamada a función | Izquierda |
| 2 | `[]` | Acceso por índice | Izquierda |
| 3 | `-` (unario), `not` | Negación, not lógico | Derecha |
| 4 | `*`, `/`, `%` | Multiplicación, división, módulo | Izquierda |
| 5 | `+`, `-` | Suma, resta | Izquierda |
| 6 | `<`, `<=`, `>`, `>=` | Comparación | Izquierda |
| 7 | `==`, `!=` | Igualdad | Izquierda |
| 8 | `and` | Y lógico | Izquierda |
| 9 (más baja) | `or` | O lógico | Izquierda |

### Operadores Aritméticos

| Operador | Descripción | Tipos Aplicables |
|----------|-------------|------------------|
| `+` | Suma | Todos los tipos numéricos |
| `-` | Resta | Todos los tipos numéricos |
| `*` | Multiplicación | Todos los tipos numéricos |
| `/` | División | Todos los tipos numéricos |
| `%` | Módulo | Solo enteros |

#### Ejemplos

```rhodesia
int: a = 10 + 5        // 15
int: b = 20 - 3        // 17
float64: c = 15.0 / 4  // 3.75
int: d = 17 % 5        // 2

// Operaciones vectoriales
vec: u = [1, 2, 3]
vec: v = [4, 5, 6]
vec: sum = u + v        // [5, 7, 9]
vec: scaled = 2 * u     // [2, 4, 6]

// Multiplicación matricial
mat: A = [[1, 2], [3, 4]]
mat: B = [[5, 6], [7, 8]]
mat: C = A * B          // Multiplicación matricial
```

### Operadores de Comparación

| Operador | Descripción |
|----------|-------------|
| `==` | Igual a |
| `!=` | Diferente de |
| `<` | Menor que |
| `<=` | Menor o igual que |
| `>` | Mayor que |
| `>=` | Mayor o igual que |

#### Ejemplos

```rhodesia
int: x = 5
int: y = 10

int: eq = (x == 5)    // 1 (verdadero)
int: ne = (x != y)    // 1 (verdadero)
int: lt = (x < y)     // 1 (verdadero)
int: gt = (x > y)     // 0 (falso)
```

### Operadores Lógicos

| Operador | Descripción |
|----------|-------------|
| `and` | Y lógico (corto circuito) |
| `or` | O lógico (corto circuito) |
| `not` | Negación lógica |

#### Ejemplos

```rhodesia
int: a = 1  // verdadero
int: b = 0  // falso

int: result1 = a and b  // 0 (falso)
int: result2 = a or b   // 1 (verdadero)
int: result3 = not a    // 0 (falso)
int: result4 = not b    // 1 (verdadero)

// Uso en condiciones
if x > 0 and x < 100 {
    println("x está entre 1 y 99")
}
```

## Estructuras de Control

### Condicional if-else

```rhodesia
if condición {
    // código si verdadero
} else if otra_condición {
    // código si la otra condición es verdadera
} else {
    // código si ninguna condición es verdadera
}
```

#### Ejemplos

```rhodesia
int: x = 15

if x > 20 {
    println("x es mayor que 20")
} else if x > 10 {
    println("x está entre 11 y 20")
} else {
    println("x es menor o igual a 10")
}

// Condicional simple
if x % 2 == 0 {
    println("x es par")
}
```

### Bucle for

#### Iteración sobre Rangos

```rhodesia
// Iterar de 0 a n-1
for i in range(5) {
    println(i)  // 0, 1, 2, 3, 4
}

// Iterar de start a end-1
for i in range(2, 8) {
    println(i)  // 2, 3, 4, 5, 6, 7
}
```

#### Iteración sobre Vectores

```rhodesia
vec: data = [10.5, 20.3, 15.7, 8.9]

for value in data {
    println(value)
}
```

#### Iteración con Índice

```rhodesia
vec: values = [100, 200, 300]

for i in range(size(values)) {
    float64: val = values[i]
    println("Índice", i, ": valor", val)
}
```

### Bucle while

```rhodesia
while condición {
    // código que se ejecuta mientras la condición sea verdadera
    // debe modificar la condición para evitar bucles infinitos
}
```

#### Ejemplos

```rhodesia
// Contador descendente
int: count = 5
while count > 0 {
    println(count)
    count = count - 1
}
println("¡Despegue!")

// Acumulador
int: sum = 0
int: i = 1
while i <= 10 {
    sum = sum + i
    i = i + 1
}
println("Suma de 1 a 10:", sum)
```

### break y continue

```rhodesia
// break: salir del bucle inmediatamente
for i in range(100) {
    if i == 5 {
        break  // Sale del bucle cuando i == 5
    }
    println(i)  // Imprime 0, 1, 2, 3, 4
}

// continue: saltar a la siguiente iteración
for i in range(10) {
    if i % 2 == 0 {
        continue  // Salta números pares
    }
    println(i)  // Imprime 1, 3, 5, 7, 9
}
```

## Funciones

### Definición de Funciones

```rhodesia
fun nombre_función(tipo_param1: nombre_param1, tipo_param2: nombre_param2) -> tipo_retorno {
    // cuerpo de la función
    // puede contener múltiples sentencias
    return valor_retorno
}
```

#### Ejemplos

```rhodesia
// Función simple
fun cuadrado(float64: x) -> float64 {
    return x * x
}

// Función con múltiples parámetros
fun distancia(vec: p1, vec: p2) -> float64 {
    vec: diff = p1 - p2
    return norm(diff)
}

// Función void (sin retorno)
fun saludar(string: nombre) -> void {
    println("Hola,", nombre, "!")
}

// Función recursiva
fun factorial(int: n) -> int {
    if n <= 1 {
        return 1
    }
    return n * factorial(n - 1)
}
```

### Llamadas a Funciones

```rhodesia
// Llamada simple
float64: result = cuadrado(5.0)

// Llamada con múltiples argumentos
vec: punto1 = [0, 0]
vec: punto2 = [3, 4]
float64: dist = distancia(punto1, punto2)

// Llamada a función void
saludar("Mundo")
```

### Alcance y Variables Locales

Cada llamada a función crea un nuevo alcance (scope) con sus propias variables locales:

```rhodesia
fun ejemplo_scopes() -> void {
    int: x = 10
    {
        int: x = 20  // Variable diferente
        println("x interno:", x)  // 20
    }
    println("x externo:", x)  // 10
}
```

### Recursión

Rhodesia soporta recursión con límite práctico determinado por la pila del sistema:

```rhodesia
fun fibonacci(int: n) -> int {
    if n <= 1 {
        return n
    }
    return fibonacci(n - 1) + fibonacci(n - 2)
}

// Uso
for i in range(10) {
    println("fib(", i, ") =", fibonacci(i))
}
```

## Acceso por Índice

### Vectores

```rhodesia
vec: v = [10, 20, 30, 40, 50]

// Acceso a elementos individuales
float64: first = v[0]   // 10
float64: third = v[2]   // 30

// Modificación de elementos
v[1] = 25  // v = [10, 25, 30, 40, 50]

// Acceso fuera de límites causa error en runtime
// float64: error = v[10]  // RuntimeError!
```

### Matrices

```rhodesia
mat: m = [[1, 2, 3],
          [4, 5, 6],
          [7, 8, 9]]

// Acceso a elementos individuales
float64: element = m[1, 2]  // 6 (fila 1, columna 2)

// Modificación de elementos
m[0, 0] = 99  // m[0,0] = 99

// Acceso a filas completas (devuelve vector)
vec: row1 = m[1, :]  // [4, 5, 6]

// Acceso a columnas completas
vec: col2 = m[:, 2]  // [3, 6, 9]
```

## Asignación

### Asignación Simple

```rhodesia
tipo: variable = valor
variable = nuevo_valor  // Para variables ya declaradas
```

### Asignación por Índice

```rhodesia
vec: v = [1, 2, 3, 4, 5]
v[2] = 99  // v = [1, 2, 99, 4, 5]

mat: m = [[1, 2], [3, 4]]
m[0, 1] = 99  // m = [[1, 99], [3, 4]]
```

## Gramática Formal

### Programa

```
program ::= statement*
```

### Sentencias

```
statement ::= var_decl
           | assignment
           | return_stmt
           | if_stmt
           | for_stmt
           | while_stmt
           | break_stmt
           | continue_stmt
           | block
           | expr_stmt
           | func_decl
```

### Declaraciones

```
var_decl ::= type ':' identifier '=' expression

type ::= 'int' | 'float64' | 'vec' | 'mat' | 'string'

func_decl ::= 'fun' identifier '(' param_list? ')' '->' type block

param_list ::= param (',' param)*

param ::= type ':' identifier
```

### Expresiones

```
expression ::= or_expr

or_expr ::= and_expr ('or' and_expr)*

and_expr ::= comp_expr ('and' comp_expr)*

comp_expr ::= add_expr (comp_op add_expr)*

comp_op ::= '==' | '!=' | '<' | '<=' | '>' | '>='

add_expr ::= mul_expr (('+' | '-') mul_expr)*

mul_expr ::= unary_expr (('*' | '/' | '%') unary_expr)*

unary_expr ::= ('-' | 'not') unary_expr
             | postfix_expr

postfix_expr ::= primary_expr (postfix_op)*

postfix_op ::= '(' arg_list? ')'
             | '[' index ']'

index ::= expression
        | expression ',' expression
        | expression ':' expression
        | expression ',' expression ':' expression

arg_list ::= expression (',' expression)*

primary_expr ::= literal
               | identifier
               | '(' expression ')'

literal ::= int_literal
          | float_literal
          | string_literal
          | vec_literal
          | mat_literal
          | bool_literal

vec_literal ::= '[' expr_list? ']'

mat_literal ::= '[' vec_literal (',' vec_literal)* ']'

expr_list ::= expression (',' expression)*
```

### Estructuras de Control

```
if_stmt ::= 'if' expression block ('else' ('if' expression)? block)*

for_stmt ::= 'for' identifier 'in' expression block

while_stmt ::= 'while' expression block

block ::= '{' statement* '}'

return_stmt ::= 'return' expression?

break_stmt ::= 'break'

continue_stmt ::= 'continue'
```

Esta gramática define completamente la sintaxis del lenguaje Rhodesia. Todas las construcciones del lenguaje derivan de estas reglas de producción.
