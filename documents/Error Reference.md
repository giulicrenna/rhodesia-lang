# Referencia de Errores de Rhodesia

## Introducción

Rhodesia proporciona un sistema de errores comprehensivo con mensajes descriptivos y ubicación precisa del código fuente. Esta referencia documenta todos los tipos de error, sus causas comunes y estrategias de resolución.

## Jerarquía de Errores

### Clase Base RhoError

Todos los errores heredan de `RhoError`:

```cpp
class RhoError : public std::runtime_error {
public:
    RhoError(const std::string& message, SourceLocation location);

    SourceLocation location() const;
    std::string what() const override;  // Incluye ubicación
};
```

### Tipos de Error

```cpp
class LexerError : public RhoError {};        // Errores léxicos
class ParseError : public RhoError {};        // Errores sintácticos
class TypeError : public RhoError {};         // Errores de tipo
class MemoryError : public RhoError {};       // Errores de memoria
class RuntimeError : public RhoError {};      // Errores de ejecución
class ArgumentError : public RuntimeError {}; // Errores de argumentos
```

## Errores Léxicos (LexerError)

### Tokens Inválidos

**Mensaje**: `"Unexpected character: '{char}'"`

**Causa**: Carácter no reconocido en el código fuente.

**Ejemplos**:
```rhodesia
int: x = 5 @ 3;  // @ no es un token válido
```

**Solución**: Reemplazar con operador válido (`+`, `-`, `*`, `/`, etc.).

### Strings sin Cerrar

**Mensaje**: `"Unterminated string literal"`

**Causa**: String literal que no termina con comillas.

**Ejemplos**:
```rhodesia
string: msg = "Hola mundo  // Falta comilla de cierre
```

**Solución**: Agregar comilla de cierre.

### Números Malformados

**Mensaje**: `"Invalid number format: '{token}'"`

**Causa**: Número con formato inválido.

**Ejemplos**:
```rhodesia
float64: x = 3.14.59;  // Dos puntos decimales
int: y = 123abc;       // Caracteres no numéricos
```

**Solución**: Corregir el formato numérico.

### Comentarios sin Cerrar

**Mensaje**: `"Unterminated block comment"`

**Causa**: Comentario multilínea sin cierre `*/`.

**Ejemplos**:
```rhodesia
/* Este comentario no termina
println("Hola");  // Error aquí
```

**Solución**: Cerrar el comentario con `*/`.

## Errores Sintácticos (ParseError)

### Tokens Inesperados

**Mensaje**: `"Expected {expected}, but got {actual}"`

**Causa**: Token diferente al esperado en esa posición.

**Ejemplos**:
```rhodesia
int: x = 5 print(x)  // Falta punto y coma o nueva línea
```

**Solución**: Agregar token faltante o corregir sintaxis.

### Paréntesis Desbalanceados

**Mensaje**: `"Unmatched parenthesis/bracket/brace"`

**Causa**: Paréntesis, corchetes o llaves sin pareja.

**Ejemplos**:
```rhodesia
vec: v = [1, 2, 3;    // Falta corchete de cierre
if x > 0 { println(x) // Falta llave de cierre
```

**Solución**: Agregar el delimitador faltante.

### Declaraciones Incompletas

**Mensaje**: `"Incomplete variable declaration"` o `"Incomplete function declaration"`

**Causa**: Declaraciones sin todos los elementos requeridos.

**Ejemplos**:
```rhodesia
int: x  // Falta inicialización
fun suma(int: a, int: b)  // Falta cuerpo de función
```

**Solución**: Completar la declaración.

### Operadores sin Operandos

**Mensaje**: `"Operator '{op}' missing operand"`

**Causa**: Operadores binarios sin operandos izquierdo o derecho.

**Ejemplos**:
```rhodesia
int: x = 5 + ;     // Falta operando derecho
int: y = * 3;      // Falta operando izquierdo
```

**Solución**: Proporcionar operandos faltantes.

## Errores de Runtime (RuntimeError)

### Variables no Declaradas

**Mensaje**: `"Undefined variable '{name}'. Did you mean to declare it first?"`

**Causa**: Uso de variable no declarada.

**Ejemplos**:
```rhodesia
println(x)  // x no declarada
```

**Solución**: Declarar la variable antes de usarla.

### Funciones no Definidas

**Mensaje**: `"Undefined function '{name}'. Check function name spelling or define it."`

**Causa**: Llamada a función no existente.

**Ejemplos**:
```rhodesia
resultado = calcular_media(datos)  // calcular_media no existe
```

**Solución**: Definir la función o usar función built-in correcta.

### Índices Fuera de Rango

**Mensaje**: `"Index {index} out of bounds for size {size}. Valid indices are 0 to {size-1}."`

**Causa**: Acceso a índice inválido en vector o matriz.

**Ejemplos**:
```rhodesia
vec: v = [1, 2, 3]
float64: x = v[5]  // Índice 5 en vector de tamaño 3
```

**Solución**: Verificar límites antes de acceder.

### Matrices Singulares

**Mensaje**: `"Cannot invert singular matrix (determinant is zero). Check matrix condition or use a different matrix."`

**Causa**: Intento de invertir matriz singular (determinante = 0).

**Ejemplos**:
```rhodesia
mat: singular = [[1, 2], [2, 4]]  // Filas linealmente dependientes
mat: inv = inv(singular)           // Error
```

**Solución**: Verificar si la matriz es invertible o usar aproximación.

### Dimensiones Incompatibles

**Mensaje**: `"Dimension mismatch in {operation}: {dim1} vs {dim2}. Ensure operands have compatible dimensions."`

**Causa**: Operaciones entre arrays de dimensiones incompatibles.

**Ejemplos**:
```rhodesia
vec: u = [1, 2]
vec: v = [1, 2, 3]
float64: dp = dot(u, v)  // Tamaños diferentes
```

**Solución**: Asegurar dimensiones compatibles.

### Break/Continue Fuera de Loops

**Mensaje**: `"Break statement outside of loop. Break can only be used inside for or while loops."`

**Mensaje**: `"Continue statement outside of loop. Continue can only be used inside for or while loops."`

**Causa**: Uso de break/continue fuera de estructuras de loop.

**Ejemplos**:
```rhodesia
if x > 0 {
    break  // Error: break fuera de loop
}
```

**Solución**: Mover break/continue dentro de un for o while loop.

### Loops con Tipos Inválidos

**Mensaje**: `"For loop requires a vector to iterate over, got {type}. Use a vector or range() function."`

**Causa**: Intento de iterar sobre un tipo no iterable en un for loop.

**Ejemplos**:
```rhodesia
for i in 42 {  // Error: no se puede iterar sobre int
    println(i)
}
```

**Solución**: Usar un vector o la función range().

### Matrices no Cuadradas

**Mensaje**: `"Matrix must be square for this operation. Got {rows}x{cols}. Ensure rows equal columns."`

**Causa**: Operaciones que requieren matrices cuadradas.

**Ejemplos**:
```rhodesia
mat: rectangular = [[1, 2, 3], [4, 5, 6]]  // 2x3
mat: inv = inv(rectangular)                  // Error: no cuadrada
```

**Solución**: Asegurar que la matriz sea cuadrada (filas = columnas).

### Índices de Matriz Inválidos

**Mensaje**: `"Matrix index [{row},{col}] out of bounds for {maxRow}x{maxCol} matrix. Valid indices: [0..{maxRow-1}, 0..{maxCol-1}]"`

**Causa**: Acceso a elemento de matriz fuera de límites.

**Ejemplos**:
```rhodesia
mat: m = [[1, 2], [3, 4]]  // 2x2
float64: x = m[2, 1]       // Error: fila 2 no existe (0-1 válido)
```

**Solución**: Usar índices válidos dentro de los límites de la matriz.

### Recursión Infinita

**Mensaje**: `"Maximum recursion depth exceeded"`

**Causa**: Llamadas recursivas sin condición de parada.

**Ejemplos**:
```rhodesia
fun infinito() -> void {
    infinito()  // Sin condición de parada
}
```

**Solución**: Agregar condición de parada a la recursión.

## Errores de Tipo (TypeError)

### Tipos Incompatibles en Operaciones

**Mensaje**: `"Cannot apply '{op}' to {type1} and {type2}"`

**Causa**: Operación entre tipos incompatibles.

**Ejemplos**:
```rhodesia
string: s = "hola"
int: x = s + 5  // No se puede sumar string e int
```

**Solución**: Convertir tipos explícitamente o usar operación válida.

### Tipos Incorrectos en Funciones

**Mensaje**: `"Function '{name}' expected {expected} but got {actual}"`

**Causa**: Argumentos de tipo incorrecto en llamada a función.

**Ejemplos**:
```rhodesia
vec: v = [1, 2, 3]
mat: m = inv(v)  // inv espera matriz, no vector
```

**Solución**: Proporcionar argumentos del tipo correcto.

### Asignaciones de Tipo Incorrecto

**Mensaje**: `"Cannot assign {actual} to variable of type {expected}"`

**Causa**: Asignación de valor de tipo incompatible.

**Ejemplos**:
```rhodesia
vec: v = [1, 2, 3]
v = "string"  // No se puede asignar string a vec
```

**Solución**: Asegurar compatibilidad de tipos.

## Errores de Memoria (MemoryError)

### Asignación de Memoria Fallida

**Mensaje**: `"Failed to allocate memory for {operation} of size {size}. Try using smaller dimensions or check available memory."`

**Causa**: Intento de crear estructuras de datos que requieren más memoria que la disponible.

**Ejemplos**:
```rhodesia
vec: huge = range(0, 1000000000000)  // Vector con un billón de elementos
```

**Solución**: Reducir el tamaño de los datos o verificar la memoria disponible.

### Tamaño Demasiado Grande

**Mensaje**: `"{operation} size {size} is too large. Maximum recommended size is {max} elements."`

**Causa**: Creación de arrays que exceden límites recomendados.

**Ejemplos**:
```rhodesia
vec: big = range(0, 200000000)  // 200 millones de elementos
```

**Solución**: Considerar si realmente necesitas tantos elementos o dividir en chunks más pequeños.

## Errores de Argumentos (ArgumentError)

### Número Incorrecto de Argumentos

**Mensaje**: `"Function '{name}' expected {expected} arguments, got {actual}"`

**Causa**: Número de argumentos diferente al esperado.

**Ejemplos**:
```rhodesia
vec: v = [1, 2, 3]
float64: n = norm(v, 2)  // norm solo toma 1 argumento
```

**Solución**: Proporcionar el número correcto de argumentos.

### Tipos de Argumento Incorrectos

**Mensaje**: `"{context}: argument must be {expected}"`

**Causa**: Argumento de tipo incorrecto.

**Ejemplos**:
```rhodesia
mat: m = [[1, 2], [3, 4]]
float64: r = rows("not a matrix")  // rows espera matriz
```

**Solución**: Convertir o proporcionar argumento del tipo correcto.

## Manejo de Errores

### Estrategias de Manejo

Dado que Rhodesia no tiene bloques try-catch integrados, el manejo de errores se basa en:

1. **Validación previa**: Verificar condiciones antes de operaciones riesgosas
2. **Funciones seguras**: Crear funciones que manejen errores gracefully
3. **Logging**: Usar print/println para debugging

### Ejemplos de Funciones Seguras

```rhodesia
fun dividir_seguro(float64: a, float64: b) -> float64 {
    if b == 0 {
        println("Error: División por cero, retornando 0")
        return 0
    }
    return a / b
}

fun acceso_seguro(vec: v, int: index) -> float64 {
    if index < 0 or index >= size(v) {
        println("Error: Índice fuera de rango, retornando 0")
        return 0
    }
    return v[index]
}

fun invertir_seguro(mat: m) -> mat {
    // Verificar si es cuadrada
    if rows(m) != cols(m) {
        println("Error: Matriz no cuadrada, retornando matriz original")
        return m
    }

    // Intentar inversión (en práctica, verificar determinante)
    return inv(m)
}
```

### Debugging Sistemático

```rhodesia
fun debug_vector(vec: v, string: nombre) -> void {
    println("Debug ", nombre, ":")
    println("  Tamaño: ", size(v))
    println("  Tipo: vec")
    println("  Valores: ", v)

    // Estadísticas básicas
    if size(v) > 0 {
        println("  Media: ", mean(v))
        println("  Norma: ", norm(v))
    }
}

fun debug_matrix(mat: m, string: nombre) -> void {
    println("Debug ", nombre, ":")
    println("  Dimensiones: ", rows(m), "x", cols(m))
    println("  Tipo: mat")
    println("  Determinante aproximado: ", m.determinant())

    if rows(m) <= 5 and cols(m) <= 5 {
        println("  Valores:")
        println(m)
    }
}
```

## Mensajes de Error Comunes y Soluciones

### Problemas Frecuentes

| Error | Causa Común | Solución |
|-------|-------------|----------|
| `Unexpected character` | Carácter especial o typo | Revisar sintaxis |
| `Undefined variable` | Variable no declarada | Declarar variable primero |
| `Index out of bounds` | Acceso sin verificar límites | Agregar validación |
| `Dimension mismatch` | Arrays de tamaños incompatibles | Verificar dimensiones |
| `Matrix is singular` | Matriz no invertible | Verificar condición numérica |
| `Wrong argument count` | Número incorrecto de parámetros | Revisar documentación de función |

### Checklist de Debugging

Cuando encuentres un error:

1. **Leer el mensaje completo**: Incluye ubicación (línea/columna)
2. **Verificar sintaxis**: ¿Hay typos o errores de sintaxis?
3. **Revisar tipos**: ¿Son compatibles los tipos en operaciones?
4. **Verificar límites**: ¿Índices dentro de rango válido?
5. **Simplificar**: ¿El error persiste en código mínimo?
6. **Documentación**: ¿Uso correcto de funciones?

### Ejemplos de Corrección de Errores

```rhodesia
// Error original: Variable no declarada
// println(x)  // Error: Undefined variable 'x'

// Corrección: Declarar variable
int: x = 42
println(x)

// Error original: Índice fuera de rango
// vec: v = [1, 2, 3]
// println(v[5])  // Error: Index 5 out of bounds

// Corrección: Verificar límites
vec: v = [1, 2, 3]
int: index = 5
if index >= 0 and index < size(v) {
    println(v[index])
} else {
    println("Índice inválido")
}

// Error original: Tipos incompatibles
// vec: v = [1, 2, 3]
// mat: m = v * 2  // Error: Cannot multiply vec and int

// Corrección: Operación válida
vec: v = [1, 2, 3]
vec: scaled = 2 * v  // Broadcasting automático
println(scaled)
```

## Mejores Prácticas para Evitar Errores

### 1. Inicialización Explícita

```rhodesia
// Bueno
int: contador = 0
vec: datos = []

// Evitar
int: contador  // Sin inicialización
```

### 2. Validación de Entrada

```rhodesia
fun procesar_datos(vec: datos) -> void {
    if size(datos) == 0 {
        println("Error: Vector vacío")
        return
    }

    // Procesar datos...
}
```

### 3. Manejo Seguro de Matrices

```rhodesia
fun resolver_sistema(mat: A, vec: b) -> vec {
    if rows(A) != cols(A) {
        println("Error: Matriz A no cuadrada")
        return zeros(size(b))
    }

    if rows(A) != size(b) {
        println("Error: Dimensiones incompatibles")
        return zeros(size(b))
    }

    return inv(A) * b
}
```

### 4. Nombres Descriptivos

```rhodesia
// Bueno
vec: coordenadas_cartesianas = [x, y, z]
mat: matriz_rotacion = eye(3)

// Evitar
vec: v = [x, y, z]
mat: m = eye(3)
```

### 5. Comentarios Explicativos

```rhodesia
// Calcular la distancia euclidiana entre dos puntos
fun distancia_euclidiana(vec: p1, vec: p2) -> float64 {
    vec: diferencia = p1 - p2          // Vector diferencia
    float64: distancia_cuadrada = dot(diferencia, diferencia)
    return sqrt(distancia_cuadrada)    // Raíz cuadrada de la suma de cuadrados
}
```

El sistema de errores de Rhodesia está diseñado para ser informativo y ayudar en el debugging. Los mensajes incluyen ubicación precisa del código fuente y sugerencias implícitas sobre cómo resolver el problema. Con práctica, los errores se convierten en guías útiles para escribir código correcto.
