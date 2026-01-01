# Funciones de Orden Superior y Closures en Rhodesia

## Introducción

Rhodesia soporta funciones de orden superior y closures, permitiendo un estilo de programación funcional poderoso y expresivo.

## Características

### 1. Lambdas (Funciones Anónimas)

Las lambdas pueden ser definidas de dos formas:

#### Lambdas de Expresión
Usan el operador `->` (flecha) para retornar el resultado de una expresión:

```rhodesia
function: double = lambda(x) -> x * 2
function: add = lambda(a, b) -> a + b
```

Alias corto con `fn`:
```rhodesia
function: square = fn(x) -> x * x
```

**Nota**: La sintaxis correcta es `->` (flecha), no `=>`.

#### Lambdas de Bloque
Usan llaves `{}` para definir un cuerpo con múltiples sentencias:

```rhodesia
function: max = lambda(a, b) {
    if a > b {
        return a
    } else {
        return b
    }
}
```

### 2. Closures

Las lambdas pueden capturar variables del entorno donde fueron definidas:

```rhodesia
int: factor = 10
function: multiply = lambda(x) -> x * factor
print(multiply(5))  // 50
```

### 3. Funciones de Orden Superior

#### Funciones que Retornan Funciones

```rhodesia
fun makeAdder(int: n) -> function {
    return lambda(x) -> x + n
}

function: add10 = makeAdder(10)
print(add10(5))  // 15
```

#### Funciones que Reciben Funciones como Parámetros

```rhodesia
fun applyTwice(function: f, int: x) -> int {
    return f(f(x))
}

function: increment = lambda(n) -> n + 1
print(applyTwice(increment, 5))  // 7
```

### 4. Composición de Funciones

```rhodesia
fun compose(function: f, function: g, int: x) -> int {
    return f(g(x))
}

function: double = lambda(n) -> n * 2
function: addOne = lambda(n) -> n + 1

print(compose(double, addOne, 5))  // 12 (double(addOne(5)))
```

### 5. Currying

```rhodesia
fun curry_add(int: a) -> function {
    return lambda(b) -> a + b
}

function: add_10 = curry_add(10)
function: add_100 = curry_add(100)

print(add_10(7))   // 17
print(add_100(7))  // 107
```

## Sintaxis

### Declaración de Tipo Function

```rhodesia
function: variable_name = lambda_expression
```

### Parámetros de Función

En declaraciones de funciones:
```rhodesia
fun myFunction(function: f, int: x) -> int {
    return f(x)
}
```

### Tipo de Retorno Function

```rhodesia
fun makeFunction() -> function {
    return lambda(x) -> x * 2
}
```

## Ejemplos Prácticos

### Map-like Operation

```rhodesia
fun mapOperation(function: op, int: a, int: b) -> int {
    return op(a, b)
}

function: add = lambda(x, y) -> x + y
function: mult = lambda(x, y) -> x * y

print(mapOperation(add, 5, 3))   // 8
print(mapOperation(mult, 5, 3))  // 15
```

### Counter Factory

```rhodesia
fun makeCounter(int: start) -> function {
    return lambda(increment) -> start + increment
}

function: counter = makeCounter(100)
print(counter(5))   // 105
print(counter(50))  // 150
```

### Filter-like Logic

```rhodesia
function: isPositive = lambda(x) {
    if x > 0 {
        return true
    } else {
        return false
    }
}

fun checkValue(function: predicate, int: value) -> bool {
    return predicate(value)
}

print(checkValue(isPositive, 10))   // true
print(checkValue(isPositive, -5))   // false
```

## Limitaciones Actuales

1. **Closures de Funciones**: Para evitar referencias circulares, las funciones no capturan otras funciones en sus closures. Las funciones se resuelven dinámicamente desde la tabla de símbolos.

2. **Parámetros con Tipo**: Los parámetros de lambda pueden tener anotaciones de tipo opcionales, pero actualmente se infiere el tipo en tiempo de ejecución.

## Ejemplos de Código

Consulta los siguientes archivos de ejemplo:
- `test_simple_lambda.rho` - Lambdas básicas
- `test_block_lambda.rho` - Lambdas con bloques
- `test_closures.rho` - Ejemplos de closures
- `test_higher_order_complete.rho` - Demo completa de funcionalidades

## Casos de Uso

Las funciones de orden superior son útiles para:

1. **Abstracción**: Encapsular comportamientos comunes
2. **Configurabilidad**: Pasar comportamiento como parámetros
3. **Composición**: Construir funciones complejas desde funciones simples
4. **Callbacks**: Definir comportamiento para ser ejecutado más tarde
5. **Programación Funcional**: Implementar patrones funcionales como map, filter, reduce

## Mejores Prácticas

1. **Nombra las lambdas descriptivamente**: `function: isEven = lambda(x) -> (x % 2) == 0`
2. **Usa lambdas de expresión para lógica simple**: `lambda(x) -> x * 2`
3. **Usa lambdas de bloque para lógica compleja**: Con múltiples sentencias
4. **Documenta funciones de orden superior**: Especifica qué espera la función callback
5. **Evita closures muy profundos**: Pueden dificultar el debugging

## Conclusión

Las funciones de orden superior y closures en Rhodesia proporcionan herramientas poderosas para escribir código más expresivo, modular y reutilizable. Combinadas con el resto de características del lenguaje, permiten un estilo de programación flexible y elegante.
