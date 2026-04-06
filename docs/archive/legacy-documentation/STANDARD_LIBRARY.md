# Biblioteca Estándar de Rhodesia

Rhodesia incluye módulos integrados (built-in) que proporcionan funcionalidad matemática, estadística y numérica avanzada sin necesidad de importación.

## Índice

- [Módulo Math](#módulo-math)
- [Módulo Stats](#módulo-stats)
- [Módulo Numerical](#módulo-numerical)
- [Módulo Vector](#módulo-vector)
- [Módulo Matrix](#módulo-matrix)
- [Módulo String](#módulo-string)
- [Módulo Mapping](#módulo-mapping)
- [Módulo IO](#módulo-io)

---

## Módulo Math

El módulo `math` proporciona funciones matemáticas fundamentales, operaciones trigonométricas y constantes matemáticas.

### Constantes Matemáticas

Las constantes se acceden directamente sin paréntesis:

```rhodesia
float64: pi_value = math.PI        // 3.141592653589793
float64: e_value = math.E          // 2.718281828459045
float64: phi_value = math.PHI      // 1.618033988749895 (Golden ratio)
float64: sqrt2 = math.SQRT_2       // 1.414213562373095
float64: sqrt3 = math.SQRT_3       // 1.732050807568877

// Usando constantes en cálculos
float64: circumference = 2.0 * math.PI * radius
float64: exp_result = math.pow(math.E, 2.0)  // e^2
```

### Funciones Trigonométricas

```rhodesia
// Funciones trigonométricas básicas (radianes)
math.sin(x)    // Seno
math.cos(x)    // Coseno
math.tan(x)    // Tangente
math.asin(x)   // Arcoseno
math.acos(x)   // Arcocoseno
math.atan(x)   // Arcotangente
math.atan2(y, x)  // Arcotangente con corrección de cuadrante

// Ejemplo
float64: angle = 0.785398  // π/4
println("sin(π/4) =", math.sin(angle))  // 0.707107
```

### Funciones Hiperbólicas

```rhodesia
math.sinh(x)   // Seno hiperbólico
math.cosh(x)   // Coseno hiperbólico
math.tanh(x)   // Tangente hiperbólica

// Ejemplo
println("sinh(1) =", math.sinh(1.0))  // 1.1752
```

### Conversión de Ángulos

```rhodesia
math.deg2rad(degrees)   // Convertir grados a radianes
math.rad2deg(radians)   // Convertir radianes a grados

// Ejemplo
float64: rad = math.deg2rad(180.0)  // π
float64: deg = math.rad2deg(3.14159)  // ≈180
```

### Funciones de Potencia y Raíz

```rhodesia
math.pow(base, exp)  // Potencia: base^exp
math.sqrt(x)         // Raíz cuadrada
math.cbrt(x)         // Raíz cúbica
math.exp(x)          // Exponencial: e^x
math.log(x)          // Logaritmo natural
math.abs(x)          // Valor absoluto

// Ejemplo
println("2^8 =", math.pow(2.0, 8.0))  // 256
```

### Funciones de Redondeo

```rhodesia
math.floor(x)   // Redondeo hacia abajo
math.ceil(x)    // Redondeo hacia arriba
math.round(x)   // Redondeo al entero más cercano
math.trunc(x)   // Truncar (eliminar decimales)

// Ejemplo
println("floor(3.7) =", math.floor(3.7))  // 3
println("ceil(3.2) =", math.ceil(3.2))    // 4
```

### Funciones Min/Max

```rhodesia
// Con vectores/matrices
math.min(vec)   // Mínimo elemento de vector
math.max(mat)   // Máximo elemento de matriz

// Con múltiples valores
math.min(a, b, c, ...)  // Mínimo de varios valores
math.max(a, b, c, ...)  // Máximo de varios valores

// Ejemplos
vec: data = [5, 2, 8, 1, 9, 3]
println("min =", math.min(data))  // 1
println("max =", math.max(data))  // 9
println("min(3, 7, 2) =", math.min(3.0, 7.0, 2.0))  // 2
```

### Funciones Utilitarias

```rhodesia
math.clamp(value, min, max)  // Restringir valor a rango
math.sign(x)                 // Signo: -1, 0, o 1
math.lerp(a, b, t)          // Interpolación lineal
math.factorial(n)           // Factorial (n!)

// Ejemplos
println("clamp(15, 0, 10) =", math.clamp(15.0, 0.0, 10.0))  // 10
println("sign(-5.5) =", math.sign(-5.5))  // -1
println("lerp(0, 100, 0.5) =", math.lerp(0.0, 100.0, 0.5))  // 50
println("5! =", math.factorial(5))  // 120
```

### Operaciones de Álgebra Lineal

```rhodesia
math.norm(vec)          // Norma euclidiana de vector
math.dot(vec1, vec2)    // Producto punto
math.transpose(mat)     // Transpuesta de matriz
math.inv(mat)           // Inversa de matriz
math.sum(vec)           // Suma de todos los elementos
math.mean(vec)          // Media aritmética

// Ejemplo
vec: v1 = [1, 2, 3]
vec: v2 = [4, 5, 6]
println("dot product =", math.dot(v1, v2))  // 32
```

### Funciones de Creación de Estructuras

```rhodesia
math.zeros(n)         // Vector de ceros de tamaño n
math.zeros(n, m)      // Matriz n×m de ceros
math.ones(n)          // Vector de unos de tamaño n
math.ones(n, m)       // Matriz n×m de unos
math.eye(n)           // Matriz identidad n×n

// Ejemplos
vec: v = math.zeros(5)     // [0, 0, 0, 0, 0]
mat: m = math.eye(3)       // Matriz identidad 3×3
```

### Información de Estructuras

```rhodesia
math.size(vec)    // Número total de elementos
math.rows(mat)    // Número de filas
math.cols(mat)    // Número de columnas

// Ejemplo
mat: m = [[1, 2, 3], [4, 5, 6]]
println("rows =", math.rows(m))  // 2
println("cols =", math.cols(m))  // 3
```

---

## Módulo Stats

El módulo `stats` proporciona funciones estadísticas para análisis de datos.

### Estadísticas Descriptivas

```rhodesia
stats.mean(vec)        // Media aritmética
stats.std(vec)         // Desviación estándar (N-1)
stats.var(vec)         // Varianza (N-1)
stats.median(vec)      // Mediana
stats.min(vec)         // Valor mínimo
stats.max(vec)         // Valor máximo
stats.percentile(vec, p)  // Percentil p (0-100)

// Ejemplo
vec: data = [1, 2, 3, 4, 5, 6, 7, 8, 9, 10]
println("mean =", stats.mean(data))      // 5.5
println("std =", stats.std(data))        // 3.02765
println("median =", stats.median(data))  // 5.5
println("75th percentile =", stats.percentile(data, 75.0))  // 7.75
```

### Correlación y Covarianza

```rhodesia
stats.corr(vec1, vec2)  // Correlación de Pearson
stats.cov(vec1, vec2)   // Covarianza

// Ejemplo
vec: x = [1, 2, 3, 4, 5]
vec: y = [2, 4, 6, 8, 10]
println("correlation =", stats.corr(x, y))  // 1.0 (correlación perfecta)
println("covariance =", stats.cov(x, y))    // 5.0
```

### Momentos Estadísticos

```rhodesia
stats.skewness(vec)  // Asimetría (skewness)
stats.kurtosis(vec)  // Curtosis (kurtosis)

// Ejemplo
vec: data = [1, 2, 2, 3, 3, 3, 4, 4, 5]
println("skewness =", stats.skewness(data))  // 0 (simétrico)
println("kurtosis =", stats.kurtosis(data))  // -1.22
```

### Normalización

```rhodesia
stats.zscore(vec)  // Normalización Z-score (media=0, std=1)

// Ejemplo
vec: data = [10, 20, 30, 40, 50]
vec: normalized = stats.zscore(data)
println("z-scores =", normalized)  // [-1.265, -0.632, 0, 0.632, 1.265]
```

---

## Módulo Numerical

El módulo `numerical` proporciona métodos numéricos para análisis, integración y resolución de ecuaciones.

### Resolución de Ecuaciones

```rhodesia
numerical.solve_quadratic(a, b, c)  // Resuelve ax² + bx + c = 0
// Retorna vector con raíces reales

// Ejemplo
vec: roots = numerical.solve_quadratic(1.0, -5.0, 6.0)
println("roots =", roots)  // [2, 3] (x² - 5x + 6 = 0)
```

### Generación de Secuencias

```rhodesia
numerical.linspace(start, end, n)  // n valores igualmente espaciados
numerical.logspace(start, end, n)  // n valores espaciados logarítmicamente

// Ejemplos
vec: lin = numerical.linspace(0.0, 10.0, 5)
println(lin)  // [0, 2.5, 5, 7.5, 10]

vec: log = numerical.logspace(0.0, 2.0, 3)
println(log)  // [1, 10, 100] (10^0, 10^1, 10^2)
```

### Operaciones Acumulativas

```rhodesia
numerical.cumsum(vec)   // Suma acumulativa
numerical.cumprod(vec)  // Producto acumulativo

// Ejemplo
vec: data = [1, 2, 3, 4, 5]
println("cumsum =", numerical.cumsum(data))  // [1, 3, 6, 10, 15]

vec: data2 = [1, 2, 3, 4]
println("cumprod =", numerical.cumprod(data2))  // [1, 2, 6, 24]
```

### Diferenciación Numérica

```rhodesia
numerical.diff(vec)           // Diferencias de primer orden
numerical.gradient(vec, h)    // Gradiente numérico con paso h

// Ejemplos
vec: data = [10, 15, 18, 25, 30]
println("diff =", numerical.diff(data))  // [5, 3, 7, 5]

vec: y = [0, 1, 4, 9, 16]  // y = x²
vec: dy = numerical.gradient(y, 1.0)
println("gradient =", dy)  // Aproximadamente [1, 2, 3, 4, 5] (derivada 2x)
```

### Integración Numérica

```rhodesia
numerical.trapz(y, x)   // Regla del trapecio
numerical.simps(y, x)   // Regla de Simpson

// Ejemplo
vec: x = [0, 1, 2, 3, 4]
vec: y = [0, 1, 4, 9, 16]  // y = x²
float64: integral = numerical.trapz(y, x)
println("∫x² dx from 0 to 4 ≈", integral)  // ≈22 (exacto: 21.33)
```

### Interpolación

```rhodesia
numerical.interp1d(x, y, x_new)  // Interpolación lineal

// Ejemplo
vec: x = [0, 1, 2, 3, 4]
vec: y = [0, 1, 4, 9, 16]
float64: y_interp = numerical.interp1d(x, y, 2.5)
println("f(2.5) ≈", y_interp)  // Interpolación entre f(2) y f(3)
```

### Polinomios

```rhodesia
numerical.polyval(coeffs, x)      // Evaluar polinomio en x
numerical.polyfit(x, y, degree)   // Ajuste polinomial

// Ejemplo - Evaluar polinomio
vec: coeffs = [1, 2, 3]  // 1 + 2x + 3x²
float64: result = numerical.polyval(coeffs, 2.0)
println("p(2) =", result)  // 1 + 2*2 + 3*4 = 17

// Ejemplo - Ajuste polinomial
vec: x_data = [0, 1, 2, 3]
vec: y_data = [1, 2, 5, 10]
vec: fitted = numerical.polyfit(x_data, y_data, 2)
println("coefficients =", fitted)  // Aproximadamente [1, 0, 1] para y ≈ x²
```

---

## Módulo Vector

El módulo `vector` proporciona operaciones avanzadas para vectores.

```rhodesia
vector.append(vec, value)      // Agregar elemento al final
vector.remove(vec, index)      // Eliminar elemento en posición
vector.insert(vec, index, val) // Insertar elemento en posición
vector.reverse(vec)            // Invertir orden
vector.size(vec)               // Tamaño del vector

// Ejemplos
vec: v = [1, 2, 3, 4]
vec: v2 = vector.append(v, 5)      // [1, 2, 3, 4, 5]
vec: v3 = vector.remove(v, 1)      // [1, 3, 4]
vec: v4 = vector.reverse(v)        // [4, 3, 2, 1]
vec: v5 = vector.insert(v, 2, 99)  // [1, 2, 99, 3, 4]
```

---

## Módulo Matrix

El módulo `matrix` proporciona operaciones avanzadas para matrices.

```rhodesia
matrix.append_row(mat, row_vec)  // Agregar fila
matrix.append_col(mat, col_vec)  // Agregar columna
matrix.remove_row(mat, index)    // Eliminar fila
matrix.remove_col(mat, index)    // Eliminar columna
matrix.rows(mat)                 // Número de filas
matrix.cols(mat)                 // Número de columnas
matrix.size(mat)                 // Número total de elementos

// Ejemplo
mat: m = [[1, 2], [3, 4]]
vec: new_row = [5, 6]
mat: m2 = matrix.append_row(m, new_row)  // [[1,2], [3,4], [5,6]]
```

---

## Módulo String

El módulo `string` proporciona funciones para manipulación de cadenas de texto.

### Declaración de Strings

```rhodesia
str: nombre = "Rhodesia"
str: mensaje = "Hello, World!"
str: vacio = ""
```

### Información y Propiedades

```rhodesia
string.length(str)    // Longitud del string
string.isempty(str)   // Verificar si está vacío (1 = true, 0 = false)

// Ejemplo
str: texto = "Hola"
int: longitud = string.length(texto)  // 4
int: vacio = string.isempty(texto)    // 0 (false)
```

### Transformación de Mayúsculas/Minúsculas

```rhodesia
string.lower(str)   // Convertir a minúsculas
string.upper(str)   // Convertir a MAYÚSCULAS

// Ejemplo
str: original = "RhOdEsIa"
str: minusculas = string.lower(original)  // "rhodesia"
str: mayusculas = string.upper(original)  // "RHODESIA"
```

### Búsqueda y Verificación

```rhodesia
string.find(str, substr)          // Buscar substring (retorna índice o -1)
string.contains(str, substr)      // Verificar si contiene substring (1/0)
string.startswith(str, prefix)    // Verificar prefijo (1/0)
string.endswith(str, suffix)      // Verificar sufijo (1/0)
string.count(str, substr)         // Contar ocurrencias de substring

// Ejemplos
str: texto = "Rhodesia Language"

int: pos = string.find(texto, "Lang")           // 9
int: tiene = string.contains(texto, "esia")     // 1 (true)
int: inicia = string.startswith(texto, "Rho")   // 1 (true)
int: termina = string.endswith(texto, "age")    // 1 (true)

str: repetido = "banana"
int: ocurrencias = string.count(repetido, "na")  // 2
```

### Extracción y Manipulación

```rhodesia
string.substr(str, start, length)   // Extraer substring
string.at(str, index)               // Obtener carácter en posición
string.trim(str)                    // Eliminar espacios inicio/fin
string.reverse(str)                 // Invertir string

// Ejemplos
str: texto = "Hello World"
str: sub = string.substr(texto, 0, 5)    // "Hello"
str: char = string.at(texto, 6)          // "W"

str: espacios = "  texto  "
str: limpio = string.trim(espacios)      // "texto"

str: original = "abcd"
str: invertido = string.reverse(original) // "dcba"
```

### Reemplazo y Modificación

```rhodesia
string.replace(str, old, new)    // Reemplazar todas las ocurrencias
string.repeat(str, count)        // Repetir string n veces
string.concat(str1, str2, ...)  // Concatenar múltiples strings

// Ejemplos
str: texto = "Hola mundo, mundo"
str: nuevo = string.replace(texto, "mundo", "Rhodesia")
// "Hola Rhodesia, Rhodesia"

str: patron = "ab"
str: repetido = string.repeat(patron, 3)  // "ababab"

str: resultado = string.concat("Rho", "de", "sia")  // "Rhodesia"
```

### División y Unión

```rhodesia
string.split(str, delimiter)    // Dividir string (retorna count por ahora)
string.join(parts, separator)   // Unir strings (placeholder)

// Nota: split y join tienen implementación limitada
// hasta que se agregue soporte completo para arrays de strings
```

### Ejemplos Completos

#### Ejemplo 1: Procesamiento de Texto

```rhodesia
str: texto = "  RHODESIA Programming Language  "

// Limpiar y normalizar
str: limpio = string.trim(texto)
str: lower = string.lower(limpio)
println("Normalizado:", lower)  // "rhodesia programming language"

// Verificar contenido
if string.contains(lower, "programming") {
    println("Es un lenguaje de programación")
}
```

#### Ejemplo 2: Manipulación de Nombres

```rhodesia
str: nombre_completo = "john doe"

// Capitalizar (de forma manual)
str: nombre_upper = string.upper(nombre_completo)
str: primer_char = string.substr(nombre_upper, 0, 1)
str: resto = string.lower(string.substr(nombre_upper, 1, 100))

println("Nombre:", primer_char)  // "J"
```

#### Ejemplo 3: Análisis de Strings

```rhodesia
str: codigo = "var x = 10;"

int: longitud = string.length(codigo)
println("Longitud:", longitud)  // 11

int: tiene_igual = string.contains(codigo, "=")
println("Tiene asignación:", tiene_igual)  // 1

int: posicion = string.find(codigo, "x")
println("Posición de x:", posicion)  // 4
```

#### Ejemplo 4: Construcción de Strings

```rhodesia
str: parte1 = "Rhodesia"
str: parte2 = " es "
str: parte3 = "genial"

str: completo = string.concat(parte1, parte2, parte3)
println(completo)  // "Rhodesia es genial"

str: repetido = string.repeat("Ha", 3)
println(repetido)  // "HaHaHa"
```

#### Ejemplo 5: Validación de Formato

```rhodesia
str: archivo = "programa.rho"

int: es_rhodesia = string.endswith(archivo, ".rho")
if es_rhodesia {
    println("Archivo Rhodesia válido")
}

int: comienza_correcto = string.startswith(archivo, "prog")
if comienza_correcto {
    println("Comienza con 'prog'")
}
```

---

## Módulo Mapping

El módulo `mapping` proporciona funciones para trabajar con mapas (diccionarios) de pares clave-valor.

### Declaración de Maps

```rhodesia
map: usuario = mapping.create()
map: datos = mapping.create()
```

### Operaciones Básicas

```rhodesia
mapping.create()                // Crear un map vacío
mapping.set(map, key, value)    // Establecer un par clave-valor
mapping.get(map, key)           // Obtener el valor de una clave
mapping.has(map, key)           // Verificar si existe una clave (retorna 1/0)
mapping.delete(map, key)        // Eliminar una clave (retorna 1/0)

// Ejemplo
map: persona = mapping.create()
mapping.set(persona, "nombre", "Ana")
mapping.set(persona, "edad", 25)
mapping.set(persona, "ciudad", "Madrid")

str: nombre = mapping.get(persona, "nombre")  // "Ana"
int: existe = mapping.has(persona, "edad")    // 1 (true)
int: eliminado = mapping.delete(persona, "ciudad")  // 1 (true)
```

### Información del Map

```rhodesia
mapping.size(map)      // Número de pares clave-valor
mapping.isempty(map)   // Verificar si está vacío (1/0)

// Ejemplo
map: config = mapping.create()
mapping.set(config, "debug", 1)
mapping.set(config, "verbose", 0)

int: tamanio = mapping.size(config)     // 2
int: vacio = mapping.isempty(config)    // 0 (false)
```

### Manipulación de Maps

```rhodesia
mapping.clear(map)           // Eliminar todos los elementos
mapping.copy(map)            // Crear una copia del map
mapping.update(map1, map2)   // Fusionar map2 en map1

// Ejemplo - Clear
map: temp = mapping.create()
mapping.set(temp, "a", 1)
mapping.set(temp, "b", 2)
mapping.clear(temp)
int: vacio = mapping.isempty(temp)  // 1 (true)

// Ejemplo - Copy
map: original = mapping.create()
mapping.set(original, "x", 10)
map: copia = mapping.copy(original)

// Ejemplo - Update
map: base = mapping.create()
mapping.set(base, "nombre", "Juan")

map: extra = mapping.create()
mapping.set(extra, "edad", 30)
mapping.set(extra, "ciudad", "Lima")

mapping.update(base, extra)
// base ahora tiene: nombre, edad, ciudad
```

### Consulta de Claves y Valores

```rhodesia
mapping.keys(map)     // Retorna el número de claves (actualmente)
mapping.values(map)   // Retorna el número de valores (actualmente)

// Nota: Actualmente estas funciones retornan el conteo
// hasta que se implemente soporte completo para arrays de strings
// y arrays de tipos mixtos

// Ejemplo
map: datos = mapping.create()
mapping.set(datos, "a", 1)
mapping.set(datos, "b", 2)
mapping.set(datos, "c", 3)

int: num_claves = mapping.keys(datos)    // 3
int: num_valores = mapping.values(datos) // 3
```

### Ejemplos Completos

#### Ejemplo 1: Almacenar Configuración

```rhodesia
map: config = mapping.create()
mapping.set(config, "host", "localhost")
mapping.set(config, "port", 8080)
mapping.set(config, "timeout", 30.0)
mapping.set(config, "debug", 1)

println("Configuración:", config)

// Verificar y obtener valores
if mapping.has(config, "debug") {
    int: debug = mapping.get(config, "debug")
    if debug {
        println("Modo debug activado")
    }
}

int: puerto = mapping.get(config, "port")
println("Servidor en puerto:", puerto)
```

#### Ejemplo 2: Datos de Usuario

```rhodesia
map: usuario = mapping.create()
mapping.set(usuario, "id", 1001)
mapping.set(usuario, "username", "jdoe")
mapping.set(usuario, "email", "jdoe@example.com")
mapping.set(usuario, "activo", 1)

println("Usuario:", usuario)
println("Tamaño:", mapping.size(usuario))

// Actualizar un campo
mapping.set(usuario, "activo", 0)

// Verificar existencia antes de acceder
if mapping.has(usuario, "email") {
    str: email = mapping.get(usuario, "email")
    println("Email:", email)
}
```

#### Ejemplo 3: Contador de Palabras

```rhodesia
map: contador = mapping.create()

// Simular conteo de palabras
str: palabra1 = "hola"
str: palabra2 = "mundo"
str: palabra3 = "hola"

// Incrementar contador para "hola"
int: count_hola = 0
if mapping.has(contador, palabra1) {
    count_hola = mapping.get(contador, palabra1)
}
count_hola = count_hola + 1
mapping.set(contador, palabra1, count_hola)

// Incrementar contador para "mundo"
mapping.set(contador, palabra2, 1)

println("Contadores:", contador)
```

#### Ejemplo 4: Punto 2D con Map

```rhodesia
map: punto = mapping.create()
mapping.set(punto, "x", 3.0)
mapping.set(punto, "y", 4.0)

float64: x = mapping.get(punto, "x")
float64: y = mapping.get(punto, "y")

float64: distancia = math.sqrt(x * x + y * y)
println("Punto:", punto)
println("Distancia al origen:", distancia)  // 5.0
```

#### Ejemplo 5: Fusionar Maps

```rhodesia
map: defaults = mapping.create()
mapping.set(defaults, "color", "azul")
mapping.set(defaults, "tamanio", 10)
mapping.set(defaults, "visible", 1)

map: custom = mapping.create()
mapping.set(custom, "color", "rojo")
mapping.set(custom, "opacidad", 0.8)

// Fusionar custom en defaults
map: final = mapping.copy(defaults)
mapping.update(final, custom)

println("Configuración final:", final)
// final tiene: color="rojo", tamanio=10, visible=1, opacidad=0.8
```

### Tipos de Datos Soportados

Los maps pueden almacenar cualquier tipo de valor de Rhodesia:

```rhodesia
map: mixto = mapping.create()

// Tipos escalares
mapping.set(mixto, "entero", 42)
mapping.set(mixto, "flotante", 3.14159)
mapping.set(mixto, "texto", "Rhodesia")

// Vectores
vec: v = [1, 2, 3, 4, 5]
mapping.set(mixto, "vector", v)

// Matrices
mat: m = [[1, 2], [3, 4]]
mapping.set(mixto, "matriz", m)

// Incluso otros maps
map: anidado = mapping.create()
mapping.set(anidado, "interno", 100)
mapping.set(mixto, "submap", anidado)

println("Map mixto:", mixto)
```

---

## Módulo IO

El módulo `io` proporciona funciones para entrada/salida de datos, tanto por terminal como con archivos. Utiliza un sistema de file handles similar a C/Python para operaciones de archivos.

### Entrada por Terminal

```rhodesia
io.input()          // Leer línea desde terminal sin prompt
io.input(prompt)    // Leer línea con mensaje de prompt

// Ejemplos
str: nombre = io.input("Ingrese su nombre: ")
println("Hola,", nombre)

str: edad_str = io.input("Ingrese su edad: ")
// Convertir a número (requiere conversión manual por ahora)
```

### Operaciones con Archivos (File Handles)

#### Abrir y Cerrar Archivos

```rhodesia
io.open(filename, mode)  // Abrir archivo, retorna handle (int)
io.close(handle)         // Cerrar archivo (retorna 1 si éxito, 0 si falla)

// Modos de apertura:
// "r"   - Lectura (read)
// "w"   - Escritura, sobrescribe (write)
// "a"   - Añadir al final (append)
// "r+"  - Lectura y escritura
// "w+"  - Lectura y escritura, sobrescribe
// "a+"  - Lectura y añadir al final

// Ejemplo - Abrir y cerrar manualmente
int: archivo = io.open("datos.txt", "w")
// ... operaciones con el archivo ...
io.close(archivo)

// Ejemplo - Using block (cierre automático) - RECOMENDADO
using io.open("datos.txt", "w") as archivo {
    io.write(archivo, "contenido")
    // El archivo se cierra automáticamente al salir del bloque
}
```

#### Bloques Using (Manejo Automático de Recursos)

Rhodesia soporta bloques `using` para garantizar que los archivos se cierren automáticamente, incluso si ocurre un error:

```rhodesia
// Sintaxis: using expr as var { body }
using io.open("archivo.txt", "r") as file {
    str: contenido = io.read(file)
    println(contenido)
}
// El archivo se cierra automáticamente aquí

// Ventajas del using:
// 1. No necesitas recordar llamar io.close()
// 2. El archivo se cierra incluso si hay errores
// 3. Código más limpio y seguro

// Using anidados para trabajar con múltiples archivos
using io.open("entrada.txt", "r") as input {
    using io.open("salida.txt", "w") as output {
        str: datos = io.read(input)
        io.write(output, datos)
    }
}
// Ambos archivos se cierran automáticamente
```

#### Lectura de Archivos

```rhodesia
io.read(handle)         // Leer todo el contenido del archivo
io.read(handle, size)   // Leer size bytes
io.readline(handle)     // Leer una línea

// Ejemplo - Leer archivo completo
int: file = io.open("datos.txt", "r")
str: contenido = io.read(file)
println(contenido)
io.close(file)

// Ejemplo - Leer línea por línea
int: file = io.open("log.txt", "r")
str: linea1 = io.readline(file)
str: linea2 = io.readline(file)
println("Primera línea:", linea1)
println("Segunda línea:", linea2)
io.close(file)

// Ejemplo - Leer cantidad específica de bytes
int: file = io.open("datos.txt", "r")
str: primeros_100 = io.read(file, 100)
println("Primeros 100 bytes:", primeros_100)
io.close(file)
```

#### Escritura de Archivos

```rhodesia
io.write(handle, content)  // Escribir string al archivo (retorna bytes escritos)
io.flush(handle)           // Forzar escritura de buffer al disco

// Ejemplo - Escritura simple
int: file = io.open("saludo.txt", "w")
io.write(file, "Hola desde Rhodesia!\n")
io.close(file)

// Ejemplo - Múltiples escrituras
int: file = io.open("log.txt", "w")
io.write(file, "Línea 1\n")
io.write(file, "Línea 2\n")
io.write(file, "Línea 3\n")
io.close(file)

// Ejemplo - Append (añadir)
int: file = io.open("log.txt", "a")
io.write(file, "Línea adicional\n")
io.close(file)
```

#### Navegación en Archivos

```rhodesia
io.seek(handle, offset, whence)  // Mover posición de lectura/escritura
io.tell(handle)                  // Obtener posición actual

// whence valores:
// 0 - Desde el inicio del archivo (por defecto)
// 1 - Desde la posición actual
// 2 - Desde el final del archivo

// Ejemplo - Navegación básica
int: file = io.open("datos.txt", "r")

// Leer los primeros 10 bytes
str: inicio = io.read(file, 10)
println("Inicio:", inicio)

// Ver posición actual
int: pos = io.tell(file)
println("Posición actual:", pos)

// Volver al inicio
io.seek(file, 0, 0)

// Ir al final
io.seek(file, 0, 2)

// Retroceder 20 bytes desde el final
io.seek(file, -20, 2)

io.close(file)

// Ejemplo - Saltar líneas
int: file = io.open("datos.txt", "r")
io.readline(file)  // Saltar primera línea
str: segunda = io.readline(file)  // Leer segunda
println("Segunda línea:", segunda)
io.close(file)
```

### Funciones Utilitarias

```rhodesia
io.exists(filename)  // Verificar si archivo existe (1/0)
io.remove(filename)  // Eliminar archivo (1 si éxito, 0 si falla)

// Ejemplo - Verificar existencia
int: existe = io.exists("config.rho")
if existe {
    println("El archivo existe")
} else {
    println("El archivo no existe")
}

// Ejemplo - Eliminar archivo
if io.exists("temporal.txt") {
    int: eliminado = io.remove("temporal.txt")
    if eliminado {
        println("Archivo eliminado")
    } else {
        println("Error al eliminar")
    }
}
```

### Ejemplos Completos

#### Ejemplo 1: Sistema de Registro (Log) con Using

```rhodesia
str: log_file = "app.log"
str: timestamp = "2024-01-15 10:30:00"
str: mensaje = "[INFO] Aplicación iniciada\n"
str: linea_log = string.concat(timestamp, " ", mensaje)

// Añadir al log usando 'using' block
using io.open(log_file, "a") as file {
    io.write(file, linea_log)
    println("Log registrado")
}

// Leer logs
if io.exists(log_file) {
    using io.open(log_file, "r") as file_read {
        str: logs = io.read(file_read)
        println("=== LOGS ===")
        println(logs)
    }
}
```

#### Ejemplo 2: Guardar Resultados de Cálculos

```rhodesia
// Realizar cálculos
vec: datos = [1.5, 2.3, 4.1, 3.7, 5.2]
float64: promedio = stats.mean(datos)
float64: desviacion = stats.std(datos)

// Crear reporte
int: file = io.open("reporte.txt", "w")
io.write(file, "=== REPORTE ESTADÍSTICO ===\n")
io.write(file, "Datos analizados: 5 valores\n")
io.write(file, "Promedio calculado\n")
io.write(file, "Desviación estándar calculada\n")
io.close(file)

println("Reporte guardado en reporte.txt")
```

#### Ejemplo 3: Lectura Interactiva con Validación

```rhodesia
println("=== Sistema de Configuración ===")

// Pedir datos al usuario
str: host = io.input("Servidor host: ")
str: port = io.input("Puerto: ")
str: debug = io.input("Modo debug (si/no): ")

// Guardar configuración
int: file = io.open("config.txt", "w")
io.write(file, "host=")
io.write(file, host)
io.write(file, "\nport=")
io.write(file, port)
io.write(file, "\ndebug=")
io.write(file, debug)
io.write(file, "\n")
io.close(file)

println("Configuración guardada exitosamente")

// Verificar
if io.exists("config.txt") {
    println("\nContenido guardado:")
    int: file_read = io.open("config.txt", "r")
    str: contenido = io.read(file_read)
    io.close(file_read)
    println(contenido)
}
```

#### Ejemplo 4: Copia de Seguridad con Using Anidado

```rhodesia
str: archivo_original = "datos.txt"
str: archivo_backup = "datos.backup.txt"

if io.exists(archivo_original) {
    // Using anidado para leer y escribir simultáneamente
    using io.open(archivo_original, "r") as file_in {
        using io.open(archivo_backup, "w") as file_out {
            str: contenido = io.read(file_in)
            io.write(file_out, contenido)
            println("Backup creado exitosamente")
        }
    }
    // Ambos archivos se cierran automáticamente
} else {
    println("Error: archivo original no encontrado")
}
```

#### Ejemplo 5: Procesamiento Línea por Línea

```rhodesia
if io.exists("entrada.txt") {
    int: file_in = io.open("entrada.txt", "r")
    int: file_out = io.open("salida.txt", "w")

    // Procesar línea por línea
    str: linea = io.readline(file_in)
    while string.length(linea) > 0 {
        str: procesada = string.upper(linea)
        io.write(file_out, procesada)
        io.write(file_out, "\n")

        linea = io.readline(file_in)
    }

    io.close(file_in)
    io.close(file_out)

    println("Archivo procesado exitosamente")
} else {
    println("Archivo de entrada no encontrado")
}
```

#### Ejemplo 6: Navegación y Modificación de Archivo

```rhodesia
// Crear archivo de datos
int: file = io.open("numeros.txt", "w")
io.write(file, "10 20 30 40 50\n")
io.close(file)

// Leer y modificar
int: file_rw = io.open("numeros.txt", "r+")

// Leer primeros 5 caracteres
str: inicio = io.read(file_rw, 5)
println("Inicio:", inicio)

// Ir al final y añadir
io.seek(file_rw, 0, 2)
io.write(file_rw, "60 70 80\n")

// Volver al inicio y leer todo
io.seek(file_rw, 0, 0)
str: todo = io.read(file_rw)
println("Contenido completo:")
println(todo)

io.close(file_rw)
```

#### Ejemplo 7: Gestor de Notas Mejorado

```rhodesia
println("=== GESTOR DE NOTAS ===")
println("1. Crear nota")
println("2. Leer nota")
println("3. Añadir a nota existente")

str: opcion = io.input("Seleccione opción: ")

if string.contains(opcion, "1") {
    str: titulo = io.input("Título de la nota: ")
    str: contenido = io.input("Contenido: ")

    str: archivo = string.concat(titulo, ".txt")
    int: file = io.open(archivo, "w")
    io.write(file, contenido)
    io.write(file, "\n")
    io.close(file)

    println("Nota creada:", archivo)
}

if string.contains(opcion, "2") {
    str: archivo = io.input("Nombre del archivo: ")

    if io.exists(archivo) {
        int: file = io.open(archivo, "r")
        str: contenido = io.read(file)
        io.close(file)

        println("\n=== CONTENIDO ===")
        println(contenido)
    } else {
        println("Archivo no encontrado")
    }
}

if string.contains(opcion, "3") {
    str: archivo = io.input("Nombre del archivo: ")

    if io.exists(archivo) {
        str: nuevo = io.input("Texto a añadir: ")

        int: file = io.open(archivo, "a")
        io.write(file, nuevo)
        io.write(file, "\n")
        io.close(file)

        println("Contenido añadido exitosamente")
    } else {
        println("Archivo no encontrado")
    }
}
```

### Notas Importantes

- **File Handles**: Los archivos se manejan mediante handles (números enteros) que se obtienen con `io.open()`
- **Bloques Using** (RECOMENDADO): Usa `using io.open(...) as var { }` para cerrar archivos automáticamente
  - El archivo se cierra al salir del bloque, incluso si hay errores
  - Es la forma más segura de manejar archivos
  - Soporta anidamiento para trabajar con múltiples archivos
- **Cerrar Manualmente**: Si no usas `using`, debes llamar `io.close()` explícitamente
- **Modos de Apertura**:
  - `"r"` para lectura
  - `"w"` para escritura (sobrescribe)
  - `"a"` para añadir al final
  - `"r+"`, `"w+"`, `"a+"` para lectura/escritura combinadas
- **Navegación**: Usa `io.seek()` y `io.tell()` para moverte por el archivo
- **Input Terminal**: `io.input()` lee hasta encontrar un salto de línea (Enter)
- **Funciones Utilitarias**: `io.exists()` y `io.remove()` no requieren handles, trabajan directamente con nombres de archivo

---

## Funciones Globales

Además de los módulos, existen funciones globales disponibles sin prefijo:

```rhodesia
// Entrada/Salida
print(...)     // Imprimir sin salto de línea
println(...)   // Imprimir con salto de línea

// Generación de Rangos
range(n)          // Rango de 0 a n-1
range(start, end) // Rango de start a end-1

// Tiempo
get_tick()  // Timestamp de alta precisión (monotónico)

// Ejemplos
println("Hello, World!")

for i in range(5) {
    println(i)  // 0, 1, 2, 3, 4
}

float64: start = get_tick()
// ... código a medir ...
float64: elapsed = get_tick() - start
println("Tiempo transcurrido:", elapsed, "segundos")
```

---

## Ejemplos Completos

### Ejemplo 1: Análisis Estadístico

```rhodesia
// Análisis estadístico de datos
vec: data = [12, 15, 18, 20, 22, 25, 28, 30, 32, 35]

println("=== Análisis Estadístico ===")
println("Media:", stats.mean(data))
println("Mediana:", stats.median(data))
println("Desv. Est.:", stats.std(data))
println("Varianza:", stats.var(data))
println("Mínimo:", stats.min(data))
println("Máximo:", stats.max(data))

// Normalización
vec: norm_data = stats.zscore(data)
println("Datos normalizados:", norm_data)
```

### Ejemplo 2: Ajuste de Curva

```rhodesia
// Ajuste polinomial de datos experimentales
vec: x_exp = [0, 1, 2, 3, 4, 5]
vec: y_exp = [1.2, 2.8, 7.5, 15.1, 25.9, 40.2]

// Ajustar polinomio de grado 2
vec: coeffs = numerical.polyfit(x_exp, y_exp, 2)
println("Coeficientes:", coeffs)

// Evaluar en nuevos puntos
vec: x_new = numerical.linspace(0.0, 5.0, 11)
for i in range(11) {
    float64: y_pred = numerical.polyval(coeffs, x_new[i])
    println("f(", x_new[i], ") =", y_pred)
}
```

### Ejemplo 3: Integración Numérica

```rhodesia
// Calcular integral de sen(x) de 0 a π
int: n = 100
vec: x = numerical.linspace(0.0, 3.14159, n)
vec: y = math.zeros(n)

// Calcular sin(x) para cada punto
for i in range(n) {
    y[i] = math.sin(x[i])
}

// Integrar usando regla del trapecio
float64: integral = numerical.trapz(y, x)
println("∫sin(x)dx de 0 a π ≈", integral)  // Debería ser ≈2.0
```

---

## Notas

- Todas las funciones que operan sobre vectores/matrices también validan dimensiones y lanzarán errores apropiados si hay inconsistencias
- Las funciones trigonométricas trabajan en radianes por defecto
- Los índices comienzan en 0
- Las operaciones de álgebra lineal usan la biblioteca Eigen internamente para máximo rendimiento

---

Para más información sobre el sistema de módulos y cómo crear tus propios módulos, consulta [MODULES.md](MODULES.md).
