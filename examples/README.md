# Ejemplos de Rhodesia

Índice completo de ejemplos organizados por categoría.

## 📁 Estructura de Ejemplos

### 01_basic - Ejemplos Básicos
Introducción al lenguaje Rhodesia para principiantes.

| Archivo | Descripción | Características |
|---------|-------------|-----------------|
| [hello.rho](01_basic/hello.rho) | Hello World completo | Variables, vectores, matrices, funciones |
| [control_flow.rho](01_basic/control_flow.rho) | Estructuras de control | if/else, for, while, break, continue |
| [print_examples.rho](01_basic/print_examples.rho) | Ejemplos de print/println | Múltiples argumentos, debugging, formato |

**Empieza aquí si eres nuevo en Rhodesia!**

💡 **Nota importante sobre print/println**: Ambas funciones aceptan **cantidad indefinida de argumentos** de cualquier tipo. Los argumentos se separan automáticamente con espacios.

---

### 02_data_types - Tipos de Datos
Ejemplos de los diferentes tipos de datos en Rhodesia.

| Archivo | Descripción | Tipos Demostrados |
|---------|-------------|-------------------|
| [integer_types.rho](02_data_types/integer_types.rho) | Tipos enteros extendidos | int8, int16, int32, uint8, uint16, uint32, uint64, byte |
| [strings_simple.rho](02_data_types/strings_simple.rho) | Strings básicos | Operaciones básicas con cadenas |
| [strings_test.rho](02_data_types/strings_test.rho) | Testing de strings | Pruebas del módulo string |
| [strings_demo.rho](02_data_types/strings_demo.rho) | Demo completa de strings | Todas las funciones del módulo string |
| [vectors.rho](02_data_types/vectors.rho) | Operaciones con vectores | Creación, operaciones, funciones |
| [matrices.rho](02_data_types/matrices.rho) | Operaciones con matrices | Creación, multiplicación, transposición |

**Tipos adicionales**: complex, set, tuple, record, null (ver integer_types.rho)

---

### 03_operators - Operadores Especiales
Operadores avanzados del lenguaje.

| Archivo | Descripción | Operador |
|---------|-------------|----------|
| [ternary.rho](03_operators/ternary.rho) | Operador ternario | `condition ? true_val : false_val` |
| [indexing.rho](03_operators/indexing.rho) | Indexación avanzada | Acceso a elementos de vectores y matrices |

---

### 04_slicing - Slicing
Sistema de slicing para vectores y matrices.

| Archivo | Descripción | Características |
|---------|-------------|-----------------|
| [basic_slicing.rho](04_slicing/basic_slicing.rho) | Slicing básico | `vec<start:end>`, `mat<rows, cols>` |
| [advanced_slicing.rho](04_slicing/advanced_slicing.rho) | Slicing avanzado | Índices negativos, variables, expresiones |

**Sintaxis**: `<start:end>` para rangos, `<:>` para todo, `<-n:>` para últimos n elementos

---

### 05_functions - Funciones y Lambdas
Programación funcional con lambdas, closures y funciones de orden superior.

#### Lambdas
| Archivo | Descripción | Nivel |
|---------|-------------|-------|
| [lambdas_basic.rho](05_functions/lambdas_basic.rho) | Lambdas simples | Principiante |
| [lambdas_block.rho](05_functions/lambdas_block.rho) | Lambdas con bloques | Intermedio |
| [lambdas_complete.rho](05_functions/lambdas_complete.rho) | Demo completa de lambdas | Avanzado |

#### Closures
| Archivo | Descripción | Nivel |
|---------|-------------|-------|
| [closures_minimal.rho](05_functions/closures_minimal.rho) | Closure mínimo | Principiante |
| [closures_simple.rho](05_functions/closures_simple.rho) | Closures básicos | Principiante |
| [closures_two.rho](05_functions/closures_two.rho) | Dos closures | Intermedio |
| [closures_advanced.rho](05_functions/closures_advanced.rho) | Closures avanzados | Avanzado |

#### Funciones de Orden Superior
| Archivo | Descripción | Contenido |
|---------|-------------|-----------|
| [higher_order.rho](05_functions/higher_order.rho) | Funciones de orden superior | map, filter, compose |
| [return_function.rho](05_functions/return_function.rho) | Retornar funciones | Factories de funciones |

**Sintaxis lambda**: `lambda(params) -> expr` o `lambda(params) { block }`
**Alias corto**: `fn(params) -> expr`

📖 **Ver**: [LAMBDAS_README.md](LAMBDAS_README.md) para documentación completa

---

### 06_modules - Sistema de Módulos
Organización de código en módulos reutilizables.

#### Simple
Ejemplos básicos del sistema de módulos.
- [main.rho](06_modules/simple/main.rho) - Programa principal
- [math_utils.rho](06_modules/simple/math_utils.rho) - Utilidades matemáticas
- [stats.rho](06_modules/simple/stats.rho) - Funciones estadísticas

#### Alias
Uso de alias e importaciones selectivas.
- [module_a.rho](06_modules/alias/module_a.rho), [module_b.rho](06_modules/alias/module_b.rho) - Módulos de ejemplo
- [test_alias.rho](06_modules/alias/test_alias.rho) - Alias de módulos
- [test_import_all.rho](06_modules/alias/test_import_all.rho) - Importar todo
- [test_conflict_resolution.rho](06_modules/alias/test_conflict_resolution.rho) - Resolución de conflictos

📖 **Ver**: [06_modules/README.md](06_modules/README.md) para documentación completa

---

### 07_io - Input/Output
Operaciones de entrada y salida.

| Archivo | Descripción | Características |
|---------|-------------|-----------------|
| [basic_io.rho](07_io/basic_io.rho) | I/O básico | read, write, readLine |
| [file_handles.rho](07_io/file_handles.rho) | Manejo de archivos | open, close, handles |
| [interactive.rho](07_io/interactive.rho) | Entrada interactiva | Lectura de usuario |
| [complete_demo.rho](07_io/complete_demo.rho) | Demo completa I/O | Todas las operaciones |
| [using_blocks.rho](07_io/using_blocks.rho) | Bloques using | Gestión automática de recursos |
| [using_blocks_test.rho](07_io/using_blocks_test.rho) | Tests de using | Pruebas de bloques using |

---

### 08_advanced - Características Avanzadas
Features avanzadas del lenguaje.

| Archivo | Descripción | Tema |
|---------|-------------|------|
| [exceptions.rho](08_advanced/exceptions.rho) | Manejo de excepciones | try/catch/throw |
| [error_messages.rho](08_advanced/error_messages.rho) | Mensajes de error | Depuración |
| [constants.rho](08_advanced/constants.rho) | Constantes | Valores inmutables |
| [builtin_modules.rho](08_advanced/builtin_modules.rho) | Módulos built-in | math, stats, string |
| [map.rho](08_advanced/map.rho) | Tipo Map (diccionario) | Pares clave-valor |

---

### 09_stdlib - Biblioteca Estándar
Demostraciones de la biblioteca estándar de Rhodesia.

#### Módulo Math
| Archivo | Descripción |
|---------|-------------|
| [math_demo.rho](09_stdlib/math_demo.rho) | Funciones matemáticas |

#### Módulo Statistics
| Archivo | Descripción |
|---------|-------------|
| [statistics_basic.rho](09_stdlib/statistics_basic.rho) | Estadísticas básicas |
| [statistics_demo.rho](09_stdlib/statistics_demo.rho) | Demo completa de estadísticas |

#### Operaciones con Vectores/Matrices
| Archivo | Descripción |
|---------|-------------|
| [vector_methods.rho](09_stdlib/vector_methods.rho) | Métodos de vectores |
| [matrix_methods.rho](09_stdlib/matrix_methods.rho) | Métodos de matrices |

---

### 10_applications - Aplicaciones Completas
Proyectos y aplicaciones completas que demuestran Rhodesia en acción.

#### Machine Learning
- [regression.rho](10_applications/machine_learning/regression.rho) - Regresión lineal (OLS)
- [kmeans.rho](10_applications/machine_learning/kmeans.rho) - Clustering K-means
- [data_science.rho](10_applications/machine_learning/data_science.rho) - Data science general

#### Math Library
Librería matemática completa con módulos organizados:
- **core/**: Funciones trigonométricas
- **linear_algebra/**: Vectores, matrices, descomposiciones
- **statistics/**: Estadística descriptiva, probabilidad, regresión
- **numerical/**: Búsqueda de raíces, optimización, integración

#### Performance
- [performance.rho](10_applications/performance/performance.rho) - Benchmarks y optimización

---

## 🚀 Cómo Ejecutar los Ejemplos

### Ejecutar un archivo individual
```bash
./build/rhodesia examples/01_basic/hello.rho
```

### REPL interactivo
```bash
./build/rhodesia
```

### Ejecutar código inline
```bash
./build/rhodesia -e "println(norm([3, 4]))"
```

---

## 📚 Recursos Adicionales

- **[NEW_TYPES_README.md](../NEW_TYPES_README.md)** - Documentación de nuevos tipos de datos
- **[LAMBDAS_README.md](LAMBDAS_README.md)** - Guía completa de lambdas y closures
- **[README.md principal](../README.md)** - Documentación general del lenguaje

---

## 🎯 Rutas de Aprendizaje

### Para Principiantes
1. [hello.rho](01_basic/hello.rho) - Comienza aquí
2. [control_flow.rho](01_basic/control_flow.rho) - Aprende estructuras de control
3. [vectors.rho](02_data_types/vectors.rho) - Vectores básicos
4. [matrices.rho](02_data_types/matrices.rho) - Matrices básicas

### Para Programadores Experimentados
1. [lambdas_complete.rho](05_functions/lambdas_complete.rho) - Programación funcional
2. [higher_order.rho](05_functions/higher_order.rho) - Funciones de orden superior
3. [06_modules/](06_modules/) - Sistema de módulos
4. [10_applications/machine_learning/](10_applications/machine_learning/) - Aplicaciones reales

### Para Data Science
1. [regression.rho](10_applications/machine_learning/regression.rho) - Regresión lineal
2. [kmeans.rho](10_applications/machine_learning/kmeans.rho) - Machine learning
3. [statistics_demo.rho](09_stdlib/statistics_demo.rho) - Estadísticas
4. [math_library/](10_applications/math_library/) - Biblioteca matemática completa

---

## ✨ Contribuir

Para agregar nuevos ejemplos, sigue la estructura de carpetas existente y asegúrate de:
- Usar la sintaxis correcta: `type: name = value`
- Incluir comentarios explicativos
- Seguir las convenciones de nombres
- Probar que el ejemplo funcione correctamente

---

**¡Feliz programación con Rhodesia! 🎉**
