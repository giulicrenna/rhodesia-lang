# Guía de Rendimiento de Rhodesia

## Introducción

Rhodesia está diseñado para ofrecer alto rendimiento en computación numérica, aprovechando Eigen para operaciones matriciales optimizadas. Esta guía cubre técnicas para escribir código eficiente y evitar cuellos de botella comunes.

## Arquitectura de Rendimiento

### Backend Eigen

Rhodesia utiliza Eigen como backend numérico, que proporciona:

- **SIMD automático**: Instrucciones vectoriales para operaciones paralelas
- **Optimización de cache**: Acceso eficiente a memoria
- **Expresiones template**: Evaluación lazy para evitar copias temporales
- **OpenMP opcional**: Paralelización para matrices grandes

### Evaluación Lazy

Eigen usa evaluación lazy para expresiones complejas:

```cpp
// Esto NO crea temporales innecesarios
vec: result = (a + b) * (c + d) - e

// Se compila como una única operación optimizada
```

## Optimizaciones de Código

### 1. Evitar Copias Innecesarias

#### Problema: Múltiples temporales

```rhodesia
// Ineficiente: crea 3 objetos temporales
vec: temp1 = a + b
vec: temp2 = c + d
vec: result = temp1 * temp2
```

#### Solución: Expresiones en una línea

```rhodesia
// Eficiente: evaluación lazy, sin temporales
vec: result = (a + b) * (c + d)
```

### 2. Broadcasting Inteligente

El broadcasting automático es eficiente, pero puede causar sorpresas:

```rhodesia
vec: large = ones(10000)
vec: small = [1, 2, 3]

// Broadcasting: eficiente para escalares
vec: scaled = 2.0 * large

// Broadcasting: menos eficiente para arrays pequeños
vec: tiled = small * ones(10000)  // Repite small 10000/3 veces
```

#### Mejor: Operaciones vectorizadas directas

```rhodesia
vec: data = range(10000)
vec: result = data * data  // Elemento-a-elemento, muy eficiente
```

### 3. Pre-calcular Constantes

```rhodesia
// Ineficiente: recalcula en cada iteración
for i in range(1000) {
    float64: angle = i * 3.14159 / 180  // Conversión grados→radianes
    float64: sin_val = sin(angle)
    // usar sin_val...
}

// Eficiente: calcular constantes fuera del bucle
float64: pi_over_180 = 3.14159 / 180
for i in range(1000) {
    float64: angle = i * pi_over_180
    float64: sin_val = sin(angle)
    // usar sin_val...
}
```

### 4. Reutilizar Objetos

```rhodesia
// Ineficiente: crear/destruir objetos en bucle
for i in range(1000) {
    vec: temp = zeros(100)
    // usar temp...
}  // temp destruido aquí

// Eficiente: crear una vez, reutilizar
vec: temp = zeros(100)
for i in range(1000) {
    // reinicializar temp
    temp = zeros(100)  // O usar fill() si existiera
    // usar temp...
}
```

## Optimizaciones por Tipo de Datos

### Vectores

#### Construcción Eficiente

```rhodesia
// Bueno: construcción directa
vec: data = [1, 2, 3, 4, 5]

// Bueno: funciones de creación
vec: zeros_vec = zeros(1000)
vec: ones_vec = ones(1000)
vec: range_vec = range(1000)

// Evitar: construcción elemento por elemento en bucles
vec: slow = []
for i in range(1000) {
    slow = slow + [i]  // Muy ineficiente
}
```

#### Operaciones Vectorizadas

```rhodesia
vec: data = range(1000)

// Muy eficiente: operaciones elemento-a-elemento
vec: squared = data * data
vec: sqrt_data = sqrt(data)
vec: exp_data = exp(data)

// Eficiente: reducción
float64: total = sum(data)
float64: avg = mean(data)

// Menos eficiente: bucles explícitos
vec: manual_squared = zeros(1000)
for i in range(1000) {
    manual_squared[i] = data[i] * data[i]
}
```

### Matrices

#### Multiplicación Matricial

```rhodesia
mat: A = ones(100, 100)
mat: B = ones(100, 100)

// Eficiente: Eigen optimiza AxB
mat: C = A * B

// Menos eficiente: multiplicación elemento por elemento
mat: D = zeros(100, 100)
for i in range(100) {
    for j in range(100) {
        for k in range(100) {
            D[i, j] = D[i, j] + A[i, k] * B[k, j]
        }
    }
}
```

#### Acceso a Matrices

```rhodesia
mat: M = ones(1000, 1000)

// Eficiente: acceso por filas (cache-friendly)
float64: row_sum = 0
for i in range(1000) {
    for j in range(1000) {
        row_sum = row_sum + M[i, j]
    }
}

// Menos eficiente: acceso por columnas
float64: col_sum = 0
for j in range(1000) {
    for i in range(1000) {
        col_sum = col_sum + M[i, j]  // Salta en memoria
    }
}
```

## Optimizaciones de Algoritmos

### Búsqueda y Ordenamiento

```rhodesia
// Búsqueda lineal simple
fun buscar_lineal(vec: datos, float64: objetivo) -> int {
    for i in range(size(datos)) {
        if datos[i] == objetivo {
            return i
        }
    }
    return -1  // No encontrado
}

// Para datos grandes, considera algoritmos más eficientes
// (ordenar primero y usar búsqueda binaria)
```

### Cálculo Estadístico

```rhodesia
fun media_eficiente(vec: datos) -> float64 {
    // Una pasada: eficiente
    return sum(datos) / size(datos)
}

fun varianza_eficiente(vec: datos) -> float64 {
    // Dos pasadas: algoritmo estable numéricamente
    float64: m = mean(datos)
    vec: diff = datos - m
    return mean(diff * diff)
}

fun varianza_una_pasada(vec: datos) -> float64 {
    // Algoritmo de una pasada (menos estable numéricamente)
    float64: sum_sq = 0
    float64: sum_val = 0
    int: n = size(datos)

    for val in datos {
        sum_val = sum_val + val
        sum_sq = sum_sq + val * val
    }

    float64: media = sum_val / n
    return (sum_sq - n * media * media) / (n - 1)
}
```

### Algoritmos Numéricos

#### Método de Newton-Raphson Optimizado

```rhodesia
fun sqrt_newton(float64: numero, float64: tolerancia) -> float64 {
    if numero < 0 {
        return 0
    }

    float64: x = numero
    float64: prev_x = 0

    while abs(x - prev_x) > tolerancia {
        prev_x = x
        x = 0.5 * (x + numero / x)
    }

    return x
}

// Uso con vectorización
vec: valores = [4, 9, 16, 25, 36]
vec: raices = zeros(size(valores))
for i in range(size(valores)) {
    raices[i] = sqrt_newton(valores[i], 0.0001)
}
```

## Gestión de Memoria

### Alcance y Lifetime

```rhodesia
// Bueno: variables locales se liberan automáticamente
fun procesar_datos(vec: entrada) -> vec {
    vec: resultado = zeros(size(entrada))
    // ... procesamiento ...
    return resultado  // Copia eficiente con move semantics
}

// Evitar: variables globales grandes
vec: datos_globales = ones(1000000)  // Consume memoria todo el tiempo
```

### Copias vs Referencias

En Rhodesia, todas las asignaciones crean copias independientes:

```rhodesia
vec: original = [1, 2, 3]
vec: copia = original  // Copia completa

copia[0] = 99  // No afecta original
println(original)  // [1, 2, 3]
println(copia)     // [99, 2, 3]
```

Para rendimiento óptimo:
- **Usa copias** cuando necesites modificar datos
- **Minimiza copias** en bucles críticos
- **Reutiliza objetos** cuando sea posible

## Profiling y Medición

### Mediciones Básicas

```rhodesia
fun tiempo_ejecucion(fun: f) -> void {
    // En la práctica, necesitaríamos funciones de tiempo
    // int: start = clock()
    f()
    // int: end = clock()
    // println("Tiempo:", end - start, "unidades")
}

fun comparar_algoritmos() -> void {
    vec: datos = range(1000)

    // Algoritmo 1: bucle simple
    fun suma_bucle(vec: d) -> float64 {
        float64: total = 0
        for val in d {
            total = total + val
        }
        return total
    }

    // Algoritmo 2: función built-in
    fun suma_builtin(vec: d) -> float64 {
        return sum(d)
    }

    // Comparar tiempos
    tiempo_ejecucion(lambda suma_bucle(datos))
    tiempo_ejecucion(lambda suma_builtin(datos))
}
```

### Identificar Cuellos de Botella

```rhodesia
fun perfil_matmul() -> void {
    // Matrices de diferentes tamaños
    vec: tamanos = [10, 50, 100, 500]

    for n in tamanos {
        mat: A = ones(n, n)
        mat: B = ones(n, n)

        // Medir tiempo de AxB
        // En práctica: medir tiempo aquí
        mat: C = A * B
        println("Tamaño ", n, ": completado")
    }
}
```

## Optimizaciones Avanzadas

### Expresiones Template Eigen

Eigen genera código optimizado en compile-time:

```cpp
// Esto genera código SIMD automático
mat: result = A * B + C * D
```

### Paralelización OpenMP

Para matrices muy grandes, Eigen puede usar OpenMP:

```cpp
// Compilar con -fopenmp para paralelización automática
mat: large_A = ones(5000, 5000)
mat: large_B = ones(5000, 5000)
mat: result = large_A * large_B  // Paralelizado automáticamente
```

### Optimizaciones Numéricas

#### Evitar Underflow/Overflow

```rhodesia
fun log_sum_exp(vec: valores) -> float64 {
    // Técnica numérica para estabilidad
    float64: max_val = max(valores)  // Encontrar máximo
    vec: shifted = valores - max_val  // Centrar en cero
    vec: exp_shifted = exp(shifted)
    return max_val + log(sum(exp_shifted))
}
```

#### Precondicionamiento

```rhodesia
fun resolver_con_precondicionamiento(mat: A, vec: b) -> vec {
    // Precondicionador simple: diagonal
    vec: diag = zeros(rows(A))
    for i in range(rows(A)) {
        diag[i] = A[i, i]
    }

    // Resolver P^-1 * A * x = P^-1 * b
    mat: P_inv = diag.asDiagonal().inverse()
    vec: b_precond = P_inv * b
    mat: A_precond = P_inv * A

    return inv(A_precond) * b_precond
}
```

## Mejores Prácticas de Rendimiento

### 1. Perfil Primero

```rhodesia
// Identifica qué partes del código son lentas
// antes de optimizar
fun perfil_codigo() -> void {
    vec: datos = range(10000)

    // Parte 1: Creación de datos
    mat: matrices = []
    for i in range(100) {
        matrices = matrices + [ones(100, 100)]
    }

    // Parte 2: Procesamiento
    vec: resultados = zeros(100)
    for i in range(100) {
        resultados[i] = norm(matrices[i])
    }

    // Parte 3: Agregación
    float64: total = sum(resultados)
}
```

### 2. Optimización Guiada por Datos

```rhodesia
// Optimización basada en el tamaño de los datos
fun multiplicar_matrices(mat: A, mat: B) -> mat {
    int: n = rows(A)

    if n < 100 {
        // Para matrices pequeñas: algoritmo estándar
        return A * B
    } else {
        // Para matrices grandes: considera técnicas avanzadas
        // (bloqueo, algoritmos especializados, etc.)
        return A * B  // Por ahora, igual
    }
}
```

### 3. Memoria Eficiente

```rhodesia
// Procesamiento por lotes para grandes datasets
fun procesar_por_lotes(vec: datos_grandes, int: tam_lote) -> vec {
    vec: resultados = []
    int: n = size(datos_grandes)

    for i in range(0, n, tam_lote) {
        int: fin = min(i + tam_lote, n)
        vec: lote = datos_grandes[i:fin]  // Slicing cuando esté disponible

        // Procesar lote
        vec: resultado_lote = mean(lote)  // Ejemplo simple
        resultados = resultados + [resultado_lote]
    }

    return resultados
}
```

### 4. Algoritmos con Buena Complejidad

```rhodesia
// O(n log n) vs O(n²)
fun encontrar_duplicados(vec: datos) -> vec {
    // Método eficiente: ordenar y buscar
    // (Cuando sorting esté disponible)
    // datos.ordenar()
    // buscar duplicados en O(n)

    // Método ineficiente: comparación todos contra todos
    vec: duplicados = []
    for i in range(size(datos)) {
        for j in range(i + 1, size(datos)) {
            if datos[i] == datos[j] {
                duplicados = duplicados + [datos[i]]
                break
            }
        }
    }
    return duplicados
}
```

## Conclusión

El rendimiento en Rhodesia depende de:

1. **Aprovechar Eigen**: Usar operaciones vectorizadas y matriciales
2. **Minimizar copias**: Expresiones lazy y reutilización de objetos
3. **Algoritmos eficientes**: Elegir complejidad algorítmica apropiada
4. **Gestión de memoria**: Ámbitos apropiados y reutilización
5. **Optimizaciones numéricas**: Estabilidad y precisión

La clave es escribir código que permita a Eigen optimizar automáticamente. La mayoría de las optimizaciones vienen gratis con el uso apropiado de operaciones matriciales y vectorizadas.
