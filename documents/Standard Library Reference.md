# Referencia de la Biblioteca Estándar de Rhodesia

## Introducción

La biblioteca estándar de Rhodesia proporciona funciones integradas para operaciones matemáticas, álgebra lineal, creación de datos y entrada/salida. Todas las funciones están optimizadas usando la biblioteca Eigen para máximo rendimiento.

## Funciones de Álgebra Lineal

### norm(vec|mat) → float64

Calcula la norma de un vector o matriz.

**Parámetros:**
- `vec|mat`: Vector o matriz de entrada

**Retorna:**
- Norma L2 (Euclidiana) para vectores
- Norma Frobenius para matrices

**Ejemplos:**
```rhodesia
vec: v = [3, 4]
float64: n = norm(v)  // 5.0

mat: A = [[1, 2], [3, 4]]
float64: frobenius = norm(A)  // ≈ 5.477
```

### dot(vec, vec) → float64

Calcula el producto punto (producto escalar) entre dos vectores.

**Parámetros:**
- `vec`: Primer vector
- `vec`: Segundo vector

**Retorna:**
- Producto punto como flotante

**Ejemplos:**
```rhodesia
vec: u = [1, 2, 3]
vec: v = [4, 5, 6]
float64: dp = dot(u, v)  // 32.0
```

### transpose(vec|mat) → mat

Calcula la transpuesta de un vector o matriz.

**Parámetros:**
- `vec|mat`: Vector o matriz de entrada

**Retorna:**
- Matriz transpuesta

**Ejemplos:**
```rhodesia
vec: v = [1, 2, 3]
mat: vt = transpose(v)  // [[1, 2, 3]] (vector fila)

mat: A = [[1, 2], [3, 4]]
mat: At = transpose(A)  // [[1, 3], [2, 4]]
```

### inv(mat) → mat

Calcula la inversa de una matriz cuadrada.

**Parámetros:**
- `mat`: Matriz cuadrada

**Retorna:**
- Matriz inversa

**Errores:**
- `RuntimeError`: Si la matriz no es cuadrada
- `RuntimeError`: Si la matriz es singular (no invertible)

**Ejemplos:**
```rhodesia
mat: A = [[1, 2], [3, 4]]
mat: Ainv = inv(A)
mat: identity = A * Ainv  // ≈ [[1, 0], [0, 1]]
```

### sum(vec|mat|int|float64) → float64

Calcula la suma de todos los elementos.

**Parámetros:**
- `vec|mat|int|float64`: Datos de entrada

**Retorna:**
- Suma como flotante

**Ejemplos:**
```rhodesia
vec: v = [1, 2, 3, 4]
float64: total = sum(v)  // 10.0

mat: M = [[1, 2], [3, 4]]
float64: total = sum(M)  // 10.0
```

### mean(vec|mat) → float64

Calcula la media aritmética de los elementos.

**Parámetros:**
- `vec|mat`: Datos de entrada

**Retorna:**
- Media como flotante

**Ejemplos:**
```rhodesia
vec: data = [1, 2, 3, 4, 5]
float64: avg = mean(data)  // 3.0
```

## Funciones de Creación

### zeros(n) → vec | zeros(n, m) → mat

Crea un vector o matriz de ceros.

**Parámetros:**
- `n`: Tamaño del vector o número de filas de la matriz
- `m` (opcional): Número de columnas de la matriz

**Retorna:**
- Vector de ceros o matriz de ceros

**Ejemplos:**
```rhodesia
vec: v = zeros(3)        // [0, 0, 0]
mat: M = zeros(2, 3)     // [[0, 0, 0], [0, 0, 0]]
```

### ones(n) → vec | ones(n, m) → mat

Crea un vector o matriz de unos.

**Parámetros:**
- `n`: Tamaño del vector o número de filas de la matriz
- `m` (opcional): Número de columnas de la matriz

**Retorna:**
- Vector de unos o matriz de unos

**Ejemplos:**
```rhodesia
vec: v = ones(3)         // [1, 1, 1]
mat: M = ones(2, 3)      // [[1, 1, 1], [1, 1, 1]]
```

### eye(n) → mat

Crea una matriz identidad.

**Parámetros:**
- `n`: Tamaño de la matriz cuadrada

**Retorna:**
- Matriz identidad n×n

**Ejemplos:**
```rhodesia
mat: I = eye(3)  // [[1, 0, 0], [0, 1, 0], [0, 0, 1]]
```

### range(n) → vec | range(start, end) → vec

Crea un vector con una secuencia de números.

**Parámetros:**
- `n`: Número final (exclusivo), inicio en 0
- `start`: Número inicial (inclusivo)
- `end`: Número final (exclusivo)

**Retorna:**
- Vector con la secuencia

**Ejemplos:**
```rhodesia
vec: seq1 = range(5)      // [0, 1, 2, 3, 4]
vec: seq2 = range(2, 8)   // [2, 3, 4, 5, 6, 7]
```

## Funciones Matemáticas

### Funciones Element-wise

Las siguientes funciones aplican operaciones elemento por elemento a vectores y matrices:

#### sqrt(x) → float64|vec|mat

Raíz cuadrada.

```rhodesia
float64: s = sqrt(16)     // 4.0
vec: vs = sqrt([4, 9, 16]) // [2, 3, 4]
```

#### exp(x) → float64|vec|mat

Exponencial natural (e^x).

```rhodesia
float64: e = exp(1)       // ≈ 2.718
vec: ve = exp([0, 1, 2])  // [1, 2.718, 7.389]
```

#### log(x) → float64|vec|mat

Logaritmo natural.

```rhodesia
float64: ln = log(2.718)  // ≈ 1.0
vec: vl = log([1, 2.718]) // [0, 1]
```

#### abs(x) → int|float64|vec|mat

Valor absoluto.

```rhodesia
int: ai = abs(-5)         // 5
float64: af = abs(-3.14)  // 3.14
vec: av = abs([-1, 2, -3]) // [1, 2, 3]
```

### Funciones Trigonométricas

Solo para escalares (int/float64):

#### sin(x) → float64

Seno.

```rhodesia
float64: s = sin(3.14159/2)  // ≈ 1.0
```

#### cos(x) → float64

Coseno.

```rhodesia
float64: c = cos(0)  // 1.0
```

#### tan(x) → float64

Tangente.

```rhodesia
float64: t = tan(3.14159/4)  // ≈ 1.0
```

## Funciones de Información

### rows(mat) → int

Número de filas de una matriz.

**Parámetros:**
- `mat`: Matriz de entrada

**Retorna:**
- Número de filas como entero

**Ejemplos:**
```rhodesia
mat: M = [[1, 2, 3], [4, 5, 6]]
int: r = rows(M)  // 2
```

### cols(mat) → int

Número de columnas de una matriz.

**Parámetros:**
- `mat`: Matriz de entrada

**Retorna:**
- Número de columnas como entero

**Ejemplos:**
```rhodesia
mat: M = [[1, 2, 3], [4, 5, 6]]
int: c = cols(M)  // 3
```

### size(vec|mat) → int

Número total de elementos.

**Parámetros:**
- `vec|mat`: Vector o matriz de entrada

**Retorna:**
- Número total de elementos

**Ejemplos:**
```rhodesia
vec: v = [1, 2, 3]
int: sv = size(v)  // 3

mat: M = [[1, 2], [3, 4]]
int: sm = size(M)  // 4
```

## Funciones de Entrada/Salida

### print(values...) → void

Imprime valores separados por espacios (sin salto de línea).

**Parámetros:**
- `values...`: Valores a imprimir (cualquier tipo, múltiples)

**Retorna:**
- Nada (void)

**Ejemplos:**
```rhodesia
print("Hola")
print("x =", 42)
print("vector:", [1, 2, 3])
```

### println(values...) → void

Imprime valores separados por espacios con salto de línea.

**Parámetros:**
- `values...`: Valores a imprimir (cualquier tipo, múltiples)

**Retorna:**
- Nada (void)

**Ejemplos:**
```rhodesia
println("Hola Mundo")
println("x =", 42, "y =", 3.14)
println("Resultado:", [1, 2, 3] + [4, 5, 6])
```

## Funciones de Ciencia de Datos

### Funciones Estadísticas

Aunque Rhodesia no tiene funciones estadísticas integradas avanzadas en la versión actual, se pueden implementar fácilmente:

```rhodesia
// Varianza
fun variance(vec: data) -> float64 {
    float64: m = mean(data)
    vec: diff = data - m
    return mean(diff * diff)
}

// Desviación estándar
fun std_dev(vec: data) -> float64 {
    return sqrt(variance(data))
}

// Covarianza
fun covariance(vec: x, vec: y) -> float64 {
    float64: mx = mean(x)
    float64: my = mean(y)
    vec: dx = x - mx
    vec: dy = y - my
    return mean(dx * dy)
}
```

### Regresión Lineal

```rhodesia
// Regresión lineal simple (OLS)
fun linear_regression(vec: x, vec: y) -> vec {
    int: n = size(x)

    // Agregar columna de unos para intercepto
    mat: X = zeros(n, 2)
    for i in range(n) {
        X[i, 0] = 1
        X[i, 1] = x[i]
    }

    // Resolver: β = (X^T X)^(-1) X^T y
    mat: Xt = transpose(X)
    mat: XtX = Xt * X
    mat: XtX_inv = inv(XtX)
    vec: Xty = Xt * y

    return XtX_inv * Xty
}

// Uso
vec: x = [1, 2, 3, 4, 5]
vec: y = [2, 4, 6, 8, 10]
vec: coef = linear_regression(x, y)  // [0, 2] (intercepto=0, pendiente=2)
```

### Métricas de Evaluación

```rhodesia
// Error cuadrático medio
fun mse(vec: y_true, vec: y_pred) -> float64 {
    vec: diff = y_true - y_pred
    return mean(diff * diff)
}

// Raíz del error cuadrático medio
fun rmse(vec: y_true, vec: y_pred) -> float64 {
    return sqrt(mse(y_true, y_pred))
}

// Error absoluto medio
fun mae(vec: y_true, vec: y_pred) -> float64 {
    vec: diff = y_true - y_pred
    return mean(abs(diff))
}

// Coeficiente de determinación (R²)
fun r_squared(vec: y_true, vec: y_pred) -> float64 {
    float64: y_mean = mean(y_true)
    vec: residuals = y_true - y_pred
    vec: total = y_true - y_mean

    float64: ss_res = dot(residuals, residuals)
    float64: ss_tot = dot(total, total)

    return 1.0 - ss_res / ss_tot
}
```

## Funciones de Utilidad Comunes

### Normalización de Vectores

```rhodesia
fun normalize(vec: v) -> vec {
    float64: n = norm(v)
    if n == 0 {
        return v  // Evitar división por cero
    }
    return v / n
}
```

### Producto Cruz (3D)

```rhodesia
fun cross_product(vec: u, vec: v) -> vec {
    // Solo para vectores 3D
    if size(u) != 3 or size(v) != 3 {
        // Error handling would go here
    }

    vec: result = zeros(3)
    result[0] = u[1] * v[2] - u[2] * v[1]
    result[1] = u[2] * v[0] - u[0] * v[2]
    result[2] = u[0] * v[1] - u[1] * v[0]

    return result
}
```

### Matriz de Rotación 2D

```rhodesia
fun rotation_matrix_2d(float64: angle) -> mat {
    float64: c = cos(angle)
    float64: s = sin(angle)

    return [[c, -s], [s, c]]
}
```

## Consideraciones de Rendimiento

### Eigen Backend

Todas las operaciones numéricas están respaldadas por Eigen, que proporciona:

- **Optimización SIMD**: Instrucciones vectoriales para operaciones paralelas
- **Cache eficiente**: Acceso optimizado a memoria
- **Lazy evaluation**: Expresiones template que evitan copias temporales
- **OpenMP**: Paralelización automática para matrices grandes

### Broadcasting Automático

Rhodesia automáticamente "broadcast" operaciones entre escalares y arrays:

```rhodesia
vec: v = [1, 2, 3]
vec: scaled = 2 * v     // Broadcasting: escalar * vector
vec: added = v + 1      // Broadcasting: vector + escalar
```

### Copias vs Referencias

- Las asignaciones crean copias independientes
- Las operaciones aritméticas crean nuevos objetos
- Para rendimiento óptimo, minimizar operaciones intermedias

```rhodesia
// Menos eficiente (múltiples temporales)
vec: result = (a + b) * (c + d)

// Más eficiente (reutilizar variables)
vec: temp1 = a + b
vec: temp2 = c + d
vec: result = temp1 * temp2
```

Esta biblioteca estándar proporciona las bases para computación numérica eficiente en Rhodesia, con extensiones naturales para análisis de datos, aprendizaje automático y computación científica.
