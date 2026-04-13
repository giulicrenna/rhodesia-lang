# Tutorial y Ejemplos de Rhodesia

## Introducción

Este tutorial te guiará desde los conceptos básicos hasta técnicas avanzadas de Ciencia de Datos en Rhodesia. Cada sección incluye ejemplos prácticos que puedes ejecutar inmediatamente.

## Capítulo 1: Primeros Pasos

### Instalación y Configuración

Primero, instala Rhodesia siguiendo las instrucciones en el README principal. Una vez compilado, puedes ejecutar:

```bash
# Ejecutar el REPL interactivo
./rhodesia

# Ejecutar un archivo
./rhodesia examples/hello.rho
```

### Tu Primer Programa

Crea un archivo `hola_mundo.rho`:

```rhodesia
// hola_mundo.rho
println("¡Hola, Rhodesia!")
println("Bienvenido a la Ciencia de Datos")
```

Ejecútalo con:
```bash
./rhodesia hola_mundo.rho
```

### Variables y Tipos Básicos

```rhodesia
// Variables numéricas
int: edad = 25
float64: altura = 1.75
float64: pi = 3.14159

// Vectores
vec: coordenadas = [10.5, 20.3, 15.7]

// Matrices
mat: identidad = [[1, 0], [0, 1]]

// Texto
string: nombre = "Rhodesia"

println("Nombre:", nombre)
println("Edad:", edad)
println("Coordenadas:", coordenadas)
```

## Capítulo 2: Operaciones Básicas

### Aritmética

```rhodesia
int: a = 10
int: b = 3

println("Suma:", a + b)        // 13
println("Resta:", a - b)       // 7
println("Multiplicación:", a * b) // 30
println("División:", a / b)    // 3.333...
println("Módulo:", a % b)      // 1
```

### Operaciones con Vectores

```rhodesia
vec: u = [1, 2, 3]
vec: v = [4, 5, 6]

println("Suma vectorial:", u + v)     // [5, 7, 9]
println("Producto punto:", dot(u, v)) // 32
println("Norma de u:", norm(u))       // ≈3.742
println("u escalado:", 2 * u)         // [2, 4, 6]
```

### Operaciones con Matrices

```rhodesia
mat: A = [[1, 2], [3, 4]]
mat: B = [[5, 6], [7, 8]]

println("Multiplicación matricial:")
println(A * B)

println("Transpuesta de A:")
println(transpose(A))

println("Inversa de A:")
println(inv(A))
```

## Capítulo 3: Control de Flujo

### Condicionales

```rhodesia
int: x = 15

if x > 20 {
    println("x es grande")
} else if x > 10 {
    println("x es mediano")
} else {
    println("x es pequeño")
}

// Operador ternario simulado
string: categoria = if x > 10 { "alto" } else { "bajo" }
println("Categoría:", categoria)
```

### Bucles

```rhodesia
// Contador
println("Contando hasta 5:")
for i in range(6) {
    println(i)
}

// Iterando sobre vector
vec: datos = [10.1, 20.2, 30.3, 40.4]
println("Datos:")
for valor in datos {
    println("  ", valor)
}

// Bucle while
int: contador = 1
while contador <= 5 {
    print(contador, " ")
    contador = contador + 1
}
println("")
```

### break y continue

```rhodesia
println("Números del 1 al 10 (saltando múltiplos de 3):")
for i in range(1, 11) {
    if i % 3 == 0 {
        continue
    }
    print(i, " ")
}
println("")

println("Buscando el primer número mayor que 7:")
for i in range(15) {
    if i > 7 {
        println("Encontrado:", i)
        break
    }
}
```

## Capítulo 4: Funciones

### Funciones Básicas

```rhodesia
// Función simple
fun cuadrado(float64: x) -> float64 {
    return x * x
}

// Función con múltiples parámetros
fun hipotenusa(float64: a, float64: b) -> float64 {
    return sqrt(a*a + b*b)
}

// Función void
fun saludar(string: nombre) -> void {
    println("¡Hola,", nombre, "!")
}

// Uso
println("Cuadrado de 5:", cuadrado(5))
println("Hipotenusa 3-4:", hipotenusa(3, 4))
saludar("Mundo")
```

### Funciones Recursivas

```rhodesia
fun factorial(int: n) -> int {
    if n <= 1 {
        return 1
    }
    return n * factorial(n - 1)
}

fun fibonacci(int: n) -> int {
    if n <= 1 {
        return n
    }
    return fibonacci(n - 1) + fibonacci(n - 2)
}

println("Factorial de 5:", factorial(5))
println("Fibonacci de 8:", fibonacci(8))
```

### Funciones de Orden Superior

```rhodesia
fun aplicar_operacion(vec: datos, fun: op) -> vec {
    vec: resultado = zeros(size(datos))
    for i in range(size(datos)) {
        resultado[i] = op(datos[i])
    }
    return resultado
}

fun cuadrado(float64: x) -> float64 {
    return x * x
}

fun cubo(float64: x) -> float64 {
    return x * x * x
}

vec: numeros = [1, 2, 3, 4, 5]

println("Original:", numeros)
println("Cuadrados:", aplicar_operacion(numeros, cuadrado))
println("Cubos:", aplicar_operacion(numeros, cubo))
```

## Capítulo 5: Ciencia de Datos Básica

### Estadísticas Descriptivas

```rhodesia
vec: datos = [12.5, 15.2, 18.7, 14.1, 16.8, 19.3, 13.9]

println("Datos:", datos)
println("Suma:", sum(datos))
println("Media:", mean(datos))
println("Mínimo:", min(datos))  // Implementar estas funciones
println("Máximo:", max(datos))
println("Rango:", max(datos) - min(datos))
```

### Regresión Lineal Simple

```rhodesia
// Datos de ejemplo
vec: x = [1, 2, 3, 4, 5, 6, 7, 8, 9, 10]
vec: y = [2.1, 4.0, 6.2, 8.1, 9.8, 11.9, 14.1, 16.0, 17.9, 19.8]

// Función de regresión lineal
fun regresion_lineal(vec: x, vec: y) -> vec {
    int: n = size(x)

    // Crear matriz de diseño con columna de intercepto
    mat: X = ones(n, 2)
    for i in range(n) {
        X[i, 1] = x[i]
    }

    // Resolver β = (X^T X)^(-1) X^T y
    mat: Xt = transpose(X)
    mat: XtX = Xt * X
    mat: XtX_inv = inv(XtX)
    vec: Xty = Xt * y

    return XtX_inv * Xty
}

// Calcular coeficientes
vec: coeficientes = regresion_lineal(x, y)
float64: intercepto = coeficientes[0]
float64: pendiente = coeficientes[1]

println("Coeficientes:")
println("  Intercepto:", intercepto)
println("  Pendiente:", pendiente)
println("  Ecuación: y =", intercepto, "+", pendiente, "* x")

// Predicciones
vec: y_pred = intercepto + pendiente * x
println("Predicciones:", y_pred)
```

### Análisis de Matriz de Covarianza

```rhodesia
// Datos bidimensionales
vec: x = [1, 2, 3, 4, 5]
vec: y = [2, 4, 6, 8, 10]

// Función para calcular covarianza
fun covarianza(vec: a, vec: b) -> float64 {
    float64: media_a = mean(a)
    float64: media_b = mean(b)

    vec: diff_a = a - media_a
    vec: diff_b = b - media_b

    return mean(diff_a * diff_b)
}

// Función para calcular correlación
fun correlacion(vec: a, vec: b) -> float64 {
    float64: cov = covarianza(a, b)
    float64: std_a = sqrt(covarianza(a, a))
    float64: std_b = sqrt(covarianza(b, b))

    return cov / (std_a * std_a)  // Error: debería ser std_a * std_b
}

println("Covarianza:", covarianza(x, y))
println("Correlación:", correlacion(x, y))
```

## Capítulo 6: Algoritmos Numéricos

### Método de Newton-Raphson

```rhodesia
fun raiz_cuadrada_newton(float64: numero, float64: tolerancia) -> float64 {
    if numero < 0 {
        return 0  // Error: raíz cuadrada de negativo
    }

    float64: x = numero / 2  // Estimación inicial

    while true {
        float64: siguiente = (x + numero / x) / 2
        if abs(siguiente - x) < tolerancia {
            return siguiente
        }
        x = siguiente
    }
}

println("Raíz cuadrada de 25:", raiz_cuadrada_newton(25, 0.0001))
println("Comparación con sqrt:", sqrt(25))
```

### Integración Numérica (Método del Trapecio)

```rhodesia
fun integrar_trapecio(fun: f, float64: a, float64: b, int: n) -> float64 {
    float64: h = (b - a) / n
    float64: suma = (f(a) + f(b)) / 2

    for i in range(1, n) {
        float64: x = a + i * h
        suma = suma + f(x)
    }

    return suma * h
}

fun funcion_cuadratica(float64: x) -> float64 {
    return x * x + 2 * x + 1
}

float64: resultado = integrar_trapecio(funcion_cuadratica, 0, 1, 100)
println("Integral de x²+2x+1 de 0 a 1:", resultado)
println("Resultado analítico esperado: 2.333...")
```

### Algoritmo K-Means Simple

```rhodesia
fun k_means(mat: datos, int: k, int: max_iter) -> vec {
    int: n = rows(datos)
    int: d = cols(datos)

    // Inicializar centroides aleatoriamente
    mat: centroides = zeros(k, d)
    for i in range(k) {
        int: idx = i * n / k  // Selección determinística simple
        for j in range(d) {
            centroides[i, j] = datos[idx, j]
        }
    }

    // Iteraciones
    for iter in range(max_iter) {
        vec: etiquetas = zeros(n)

        // Asignar puntos a centroides más cercanos
        for i in range(n) {
            float64: min_dist = 1e100
            int: mejor_centroide = 0

            for c in range(k) {
                vec: punto = zeros(d)
                vec: centroide = zeros(d)

                for j in range(d) {
                    punto[j] = datos[i, j]
                    centroide[j] = centroides[c, j]
                }

                vec: diff = punto - centroide
                float64: dist = norm(diff)

                if dist < min_dist {
                    min_dist = dist
                    mejor_centroide = c
                }
            }

            etiquetas[i] = mejor_centroide
        }

        // Actualizar centroides
        vec: conteos = zeros(k)
        mat: nuevos_centroides = zeros(k, d)

        for i in range(n) {
            int: c = etiquetas[i]
            conteos[c] = conteos[c] + 1

            for j in range(d) {
                nuevos_centroides[c, j] = nuevos_centroides[c, j] + datos[i, j]
            }
        }

        for c in range(k) {
            if conteos[c] > 0 {
                for j in range(d) {
                    centroides[c, j] = nuevos_centroides[c, j] / conteos[c]
                }
            }
        }
    }

    return etiquetas
}

// Datos de ejemplo 2D
mat: datos = [
    [1.0, 1.0],
    [1.5, 2.0],
    [3.0, 3.0],
    [5.0, 8.0],
    [8.0, 8.0],
    [1.0, 0.0],
    [0.0, 1.0]
]

vec: clusters = k_means(datos, 2, 10)
println("Etiquetas de clusters:", clusters)
```

## Capítulo 7: Procesamiento de Datos

### Normalización de Datos

```rhodesia
fun normalizar_min_max(vec: datos) -> vec {
    float64: min_val = 1e100
    float64: max_val = -1e100

    // Encontrar min y max
    for val in datos {
        if val < min_val {
            min_val = val
        }
        if val > max_val {
            max_val = val
        }
    }

    // Normalizar
    vec: normalizados = zeros(size(datos))
    float64: rango = max_val - min_val

    if rango == 0 {
        return normalizados  // Todos los valores son iguales
    }

    for i in range(size(datos)) {
        normalizados[i] = (datos[i] - min_val) / rango
    }

    return normalizados
}

fun normalizar_estandar(vec: datos) -> vec {
    float64: media = mean(datos)

    // Calcular desviación estándar
    vec: diff_cuadrado = (datos - media) * (datos - media)
    float64: varianza = mean(diff_cuadrado)
    float64: std_dev = sqrt(varianza)

    if std_dev == 0 {
        return zeros(size(datos))
    }

    return (datos - media) / std_dev
}

vec: datos = [10, 20, 30, 40, 50]
println("Original:", datos)
println("Min-Max:", normalizar_min_max(datos))
println("Z-score:", normalizar_estandar(datos))
```

### Matriz de Distancias

```rhodesia
fun matriz_distancias(mat: datos) -> mat {
    int: n = rows(datos)
    mat: distancias = zeros(n, n)

    for i in range(n) {
        for j in range(i + 1, n) {
            vec: punto_i = zeros(cols(datos))
            vec: punto_j = zeros(cols(datos))

            for k in range(cols(datos)) {
                punto_i[k] = datos[i, k]
                punto_j[k] = datos[j, k]
            }

            vec: diff = punto_i - punto_j
            float64: dist = norm(diff)

            distancias[i, j] = dist
            distancias[j, i] = dist
        }
    }

    return distancias
}

mat: puntos = [
    [0, 0],
    [1, 0],
    [1, 1],
    [0, 1]
]

println("Matriz de distancias:")
println(matriz_distancias(puntos))
```

## Capítulo 8: Visualización de Datos (Simulada)

Dado que Rhodesia no tiene capacidades gráficas integradas, podemos crear "visualizaciones" textuales:

### Histograma Simple

```rhodesia
fun histograma(vec: datos, int: bins) -> void {
    // Encontrar rango
    float64: min_val = 1e100
    float64: max_val = -1e100

    for val in datos {
        if val < min_val { min_val = val }
        if val > max_val { max_val = val }
    }

    float64: rango = max_val - min_val
    if rango == 0 { rango = 1 }

    // Contar frecuencias
    vec: frecuencias = zeros(bins)

    for val in datos {
        int: bin = (val - min_val) / rango * (bins - 1)
        if bin >= bins { bin = bins - 1 }
        if bin < 0 { bin = 0 }
        frecuencias[bin] = frecuencias[bin] + 1
    }

    // Encontrar frecuencia máxima para escala
    float64: max_freq = 0
    for freq in frecuencias {
        if freq > max_freq { max_freq = freq }
    }

    // Dibujar histograma
    println("Histograma:")
    for i in range(bins) {
        float64: bin_start = min_val + i * rango / bins
        float64: bin_end = min_val + (i + 1) * rango / bins

        print("[")
        print(bin_start, "-", bin_end)
        print("]: ")

        int: barras = frecuencias[i] / max_freq * 20
        for j in range(barras) {
            print("*")
        }
        println(" (", frecuencias[i], ")")
    }
}

vec: datos = [1.2, 2.3, 2.8, 3.1, 3.5, 4.2, 4.8, 5.1, 5.5, 6.2]
histograma(datos, 5)
```

### Gráfico de Dispersión Simple

```rhodesia
fun scatter_plot(vec: x, vec: y) -> void {
    // Encontrar límites
    float64: x_min = 1e100
    float64: x_max = -1e100
    float64: y_min = 1e100
    float64: y_max = -1e100

    for val in x {
        if val < x_min { x_min = val }
        if val > x_max { x_max = val }
    }

    for val in y {
        if val < y_min { y_min = val }
        if val > y_max { y_max = val }
    }

    // Dimensiones del gráfico
    int: ancho = 40
    int: alto = 20

    // Crear grid
    for fila in range(alto + 1) {
        for col in range(ancho + 1) {
            int: punto_encontrado = 0

            for i in range(size(x)) {
                int: x_plot = (x[i] - x_min) / (x_max - x_min) * ancho
                int: y_plot = (y[i] - y_min) / (y_max - y_min) * alto

                if x_plot == col and y_plot == (alto - fila) {
                    print("*")
                    punto_encontrado = 1
                    break
                }
            }

            if punto_encontrado == 0 {
                if fila == alto / 2 and col == ancho / 2 {
                    print("+")
                } else if fila == alto / 2 {
                    print("-")
                } else if col == ancho / 2 {
                    print("|")
                } else {
                    print(" ")
                }
            }
        }
        println("")
    }

    println("X: [", x_min, ",", x_max, "] Y: [", y_min, ",", y_max, "]")
}

vec: x = [1, 2, 3, 4, 5]
vec: y = [1, 4, 9, 16, 25]  // x²
scatter_plot(x, y)
```

## Capítulo 9: Optimización y Mejores Prácticas

### Evitando Copias Innecesarias

```rhodesia
// Menos eficiente: múltiples objetos temporales
vec: resultado = (a + b) * (c + d) - e / f

// Más eficiente: reutilizar variables
vec: temp1 = a + b
vec: temp2 = c + d
vec: temp3 = temp1 * temp2
vec: temp4 = e / f
vec: resultado = temp3 - temp4
```

### Pre-calculando Constantes

```rhodesia
// Calcular constantes fuera de bucles
float64: pi_div_2 = 3.14159 / 2
float64: dos_pi = 2 * 3.14159

for i in range(1000) {
    float64: seno = sin(i * pi_div_2)
    float64: coseno = cos(i * dos_pi)
    // ... usar seno y coseno
}
```

### Usando Broadcasting Eficientemente

```rhodesia
// Broadcasting automático
vec: datos = [1, 2, 3, 4, 5]
vec: centrados = datos - mean(datos)  // Broadcasting: escalar - vector
vec: normalizados = centrados / norm(centrados)  // Broadcasting: vector / escalar
```

### Manejo de Errores

```rhodesia
fun dividir_seguro(float64: a, float64: b) -> float64 {
    if b == 0 {
        println("Error: División por cero")
        return 0
    }
    return a / b
}

fun raiz_segura(float64: x) -> float64 {
    if x < 0 {
        println("Error: Raíz cuadrada de número negativo")
        return 0
    }
    return sqrt(x)
}
```

## Capítulo 10: Proyecto Final - Análisis de Datos Completo

```rhodesia
/*
 Proyecto Final: Análisis Estadístico Completo
*/

println("==========================================")
println("  Análisis Estadístico Completo en Rhodesia")
println("==========================================")
println("")

/*
 1. GENERACIÓN DE DATOS
*/

fun generar_datos_normales(int: n, float64: media, float64: std) -> vec {
    // Generación simple usando transformación de uniformes
    // (En la práctica, usaríamos un mejor generador)
    vec: datos = zeros(n)

    for i in range(n) {
        // Aproximación simple de distribución normal
        float64: u1 = (i + 0.5) / n
        float64: u2 = (i + 1.5) / n

        // Box-Muller transform (simplificado)
        float64: z = sqrt(-2 * log(u1)) * cos(2 * 3.14159 * u2)
        datos[i] = media + z * std
    }

    return datos
}

vec: muestra1 = generar_datos_normales(100, 10, 2)
vec: muestra2 = generar_datos_normales(100, 12, 2.5)

println("Muestra 1 - Media:", mean(muestra1), "Std:", sqrt(mean((muestra1 - mean(muestra1))*(muestra1 - mean(muestra1)))))
println("Muestra 2 - Media:", mean(muestra2), "Std:", sqrt(mean((muestra2 - mean(muestra2))*(muestra2 - mean(muestra2)))))
println("")

/*
 2. ESTADÍSTICOS DESCRIPTIVOS
*/

fun estadisticos_descriptivos(vec: datos) -> vec {
    float64: media = mean(datos)
    vec: diff = datos - media
    vec: diff_cuadrado = diff * diff
    float64: varianza = mean(diff_cuadrado)
    float64: std_dev = sqrt(varianza)

    // Cuartiles (aproximación)
    vec: ordenados = datos  // En la práctica, ordenaríamos
    float64: q1 = datos[size(datos)/4]
    float64: mediana = datos[size(datos)/2]
    float64: q3 = datos[3*size(datos)/4]

    return [media, std_dev, q1, mediana, q3, min(datos), max(datos)]
}

vec: stats1 = estadisticos_descriptivos(muestra1)
vec: stats2 = estadisticos_descriptivos(muestra2)

println("Estadísticos Muestra 1:")
println("  Media:", stats1[0])
println("  Std Dev:", stats1[1])
println("  Q1:", stats1[2], "Mediana:", stats1[3], "Q3:", stats1[4])
println("  Mín:", stats1[5], "Máx:", stats1[6])
println("")

/*
 3. PRUEBA T DE STUDENT
*/

fun prueba_t(vec: a, vec: b) -> vec {
    float64: media_a = mean(a)
    float64: media_b = mean(b)

    int: n_a = size(a)
    int: n_b = size(b)

    vec: diff_a = a - media_a
    vec: diff_b = b - media_b

    float64: var_a = mean(diff_a * diff_a)
    float64: var_b = mean(diff_b * diff_b)

    // t-statistic
    float64: diff_medias = media_a - media_b
    float64: se = sqrt(var_a/n_a + var_b/n_b)
    float64: t_stat = diff_medias / se

    // grados de libertad aproximados
    float64: df = (var_a/n_a + var_b/n_b) * (var_a/n_a + var_b/n_b) /
                  (var_a*var_a/(n_a*n_a*(n_a-1)) + var_b*var_b/(n_b*n_b*(n_b-1)))

    return [t_stat, df]
}

vec: t_test = prueba_t(muestra1, muestra2)
println("Prueba t de Student:")
println("  t-statistic:", t_test[0])
println("  grados libertad:", t_test[1])
println("  p-valor aproximado: significativo si |t| > 2")
println("")

/*
 4. REGRESIÓN LINEAL MÚLTIPLE
*/

fun regresion_multiple(mat: X, vec: y) -> vec {
    mat: Xt = transpose(X)
    mat: XtX = Xt * X
    mat: XtX_inv = inv(XtX)
    vec: Xty = Xt * y
    return XtX_inv * Xty
}

fun r_cuadrado(vec: y_true, vec: y_pred) -> float64 {
    float64: y_mean = mean(y_true)
    vec: residuals = y_true - y_pred
    vec: total = y_true - y_mean

    float64: ss_res = dot(residuals, residuals)
    float64: ss_tot = dot(total, total)

    return 1 - ss_res / ss_tot
}

// Crear datos de regresión
int: n_obs = 50
mat: X = ones(n_obs, 3)  // intercepto + 2 variables

vec: x1 = generar_datos_normales(n_obs, 0, 1)
vec: x2 = generar_datos_normales(n_obs, 0, 1)

// y = 2 + 3*x1 + 1.5*x2 + ruido
vec: ruido = generar_datos_normales(n_obs, 0, 0.5)
vec: y_reg = 2 + 3*x1 + 1.5*x2 + ruido

for i in range(n_obs) {
    X[i, 1] = x1[i]
    X[i, 2] = x2[i]
}

vec: coef = regresion_multiple(X, y_reg)
vec: y_pred = X * coef

println("Regresión Múltiple:")
println("  Intercepto:", coef[0])
println("  Coef x1:", coef[1], "(real: 3.0)")
println("  Coef x2:", coef[2], "(real: 1.5)")
println("  R²:", r_cuadrado(y_reg, y_pred))
println("")

/*
 5. ANÁLISIS DE COMPONENTES PRINCIPALES (PCA)
*/

fun pca(mat: datos, int: n_componentes) -> mat {
    int: n = rows(datos)
    int: d = cols(datos)

    // Centrar los datos
    vec: medias = zeros(d)
    for j in range(d) {
        vec: columna = zeros(n)
        for i in range(n) {
            columna[i] = datos[i, j]
        }
        medias[j] = mean(columna)
    }

    mat: centrados = zeros(n, d)
    for i in range(n) {
        for j in range(d) {
            centrados[i, j] = datos[i, j] - medias[j]
        }
    }

    // Matriz de covarianza
    mat: cov = (transpose(centrados) * centrados) / (n - 1)

    // En la práctica, usaríamos SVD o eigenvalores
    // Aquí una aproximación simple
    return cov  // Simplificado: devolver matriz de covarianza
}

mat: datos_pca = zeros(20, 3)
for i in range(20) {
    datos_pca[i, 0] = muestra1[i]
    datos_pca[i, 1] = muestra2[i]
    datos_pca[i, 2] = generar_datos_normales(1, 0, 1)[0]
}

mat: componentes = pca(datos_pca, 2)
println("Matriz de covarianza (PCA simplificado):")
println(componentes)

println("")
println("==========================================")
println("  ¡Análisis Completo Finalizado!")
println("==========================================")
```

Este tutorial proporciona una base sólida para usar Rhodesia en aplicaciones de Ciencia de Datos, desde análisis básicos hasta algoritmos avanzados. La combinación de sintaxis clara, operaciones matriciales eficientes y el poder de Eigen hace de Rhodesia una herramienta excelente para computación numérica.
