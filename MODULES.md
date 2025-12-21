# Sistema de Módulos de Rhodesia

El lenguaje Rhodesia ahora soporta un sistema de importación de código que permite crear programas modulares y reutilizables.

## Sintaxis

### Importación Selectiva

La forma recomendada de importar módulos es especificando explícitamente qué símbolos (funciones o variables) deseas importar:

```rhodesia
include nombre_modulo{simbolo1, simbolo2, simbolo3}
```

**Ejemplo:**

```rhodesia
// Importar funciones específicas del módulo math_utils
include math_utils{square, cube, double_it}

// Ahora puedes usar las funciones importadas
float64: x = 5.0
println("square(5.0) =", square(x))  // 25
println("cube(5.0) =", cube(x))      // 125
```

### Importación de Todo el Módulo

También puedes importar todos los símbolos de un módulo:

```rhodesia
include nombre_modulo{}
```

**Ejemplo:**

```rhodesia
// math_utils.rho contiene: square, cube, double_it
include math_utils{}

// Ahora todas las funciones están disponibles
float64: x = 5.0
println("square(5.0) =", square(x))
println("cube(5.0) =", cube(x))
println("double_it(5.0) =", double_it(x))
```

**Nota:** Aunque es conveniente, se recomienda usar importación selectiva para mantener el código más claro y evitar conflictos de nombres.

### Alias de Importación

Puedes renombrar símbolos durante la importación usando la palabra clave `as`:

```rhodesia
include nombre_modulo{simbolo1 as nuevo_nombre, simbolo2}
```

**Ejemplo:**

```rhodesia
// Importar con alias para evitar conflictos de nombres
include math_utils{square as sq, cube}

float64: x = 5.0
println("sq(5.0) =", sq(x))      // Usa el alias
println("cube(5.0) =", cube(x))  // Nombre original
```

Esto es útil cuando:
- Hay conflictos de nombres entre diferentes módulos
- Quieres usar un nombre más corto o descriptivo
- Necesitas mantener compatibilidad con código existente

## Resolución de Rutas

El sistema de módulos busca archivos `.rho` en las siguientes ubicaciones:

1. **Relativo al archivo actual**: Si ejecutas `main.rho`, buscará módulos en el mismo directorio
2. **Subdirectorios**: Puedes organizar módulos en subdirectorios usando rutas relativas

**Estructura de proyecto ejemplo:**

```
mi_proyecto/
├── main.rho
├── math_utils.rho
└── lib/
    └── helpers.rho
```

**Uso:**

```rhodesia
// Desde main.rho
include math_utils{square}     // Busca ./math_utils.rho
include lib/helpers{helper1}   // Busca ./lib/helpers.rho
```

## Caché de Módulos

Los módulos se cargan y parsean una sola vez. Si múltiples archivos importan el mismo módulo, se reutiliza la versión en caché, mejorando el rendimiento.

## Ejemplos

### Ejemplo 1: Módulo de Matemáticas

**simple_math.rho:**
```rhodesia
// Funciones matemáticas básicas
fun square(float64: x) -> float64 {
    return x * x
}

fun cube(float64: x) -> float64 {
    return x * x * x
}

fun double_it(float64: x) -> float64 {
    return x * 2.0
}
```

**main.rho:**
```rhodesia
include simple_math{square, cube}

float64: num = 4.0
println("square(4) =", square(num))    // 16
println("cube(4) =", cube(num))        // 64
```

### Ejemplo 2: Múltiples Módulos

**geometry.rho:**
```rhodesia
fun circle_area(float64: radius) -> float64 {
    float64: pi = 3.14159
    return pi * radius * radius
}
```

**physics.rho:**
```rhodesia
fun kinetic_energy(float64: mass, float64: velocity) -> float64 {
    return 0.5 * mass * velocity * velocity
}
```

**simulation.rho:**
```rhodesia
include geometry{circle_area}
include physics{kinetic_energy}

println("Área del círculo (r=5):", circle_area(5.0))
println("Energía cinética (m=2, v=10):", kinetic_energy(2.0, 10.0))
```

## Limitaciones Actuales

1. **Solo funciones y variables**: Actualmente solo se pueden exportar/importar funciones y variables. No hay soporte para tipos personalizados aún.

2. **Rutas relativas**: Los módulos se resuelven relativos al directorio del archivo que se está ejecutando.

3. **Extensión .rho requerida**: Los archivos de módulo deben tener la extensión `.rho`.

4. **Sin exportación explícita**: Todos los símbolos definidos en el módulo están disponibles para importación. No hay control sobre qué símbolos son públicos o privados.

## Buenas Prácticas

1. **Importa solo lo que necesitas**: Usa importación selectiva para mantener el código claro
   ```rhodesia
   // Bueno
   include utils{func1, func2}

   // Evitar
   include utils{}  // Importa todo
   ```

2. **Organiza en módulos por funcionalidad**: Agrupa funciones relacionadas
   ```
   proyecto/
   ├── main.rho
   ├── math/
   │   ├── algebra.rho
   │   └── geometry.rho
   └── stats/
       └── analysis.rho
   ```

3. **Nombres descriptivos**: Usa nombres claros para módulos y funciones
   ```rhodesia
   include math_operations{calculate_mean, calculate_variance}
   ```

4. **Documenta tus módulos**: Agrega comentarios explicando qué hace cada función
   ```rhodesia
   // stats.rho
   // Módulo con funciones estadísticas básicas

   // Calcula la media aritmética de un vector
   fun mean(vec: data) -> float64 {
       return sum(data) / size(data)
   }
   ```

## Ejemplos Adicionales

### Ejemplo 3: Uso de Alias para Evitar Conflictos

**modulo_a.rho:**
```rhodesia
fun process(float64: x) -> float64 {
    return x * 2.0
}
```

**modulo_b.rho:**
```rhodesia
fun process(float64: x) -> float64 {
    return x + 10.0
}
```

**main.rho:**
```rhodesia
// Usar alias para importar ambas funciones sin conflicto
include modulo_a{process as process_a}
include modulo_b{process as process_b}

float64: num = 5.0
println("Process A:", process_a(num))  // 10.0
println("Process B:", process_b(num))  // 15.0
```

### Ejemplo 4: Importación Completa de Módulo

**utils.rho:**
```rhodesia
fun add(float64: a, float64: b) -> float64 {
    return a + b
}

fun multiply(float64: a, float64: b) -> float64 {
    return a * b
}

float64: PI = 3.14159
```

**main.rho:**
```rhodesia
// Importar todo el contenido del módulo
include utils{}

println("add(2, 3) =", add(2.0, 3.0))
println("multiply(4, 5) =", multiply(4.0, 5.0))
println("PI =", PI)
```

## Próximas Mejoras

- Módulos con namespaces (`mod.function()`)
- Exportación explícita (declarar qué símbolos son públicos)
- Búsqueda de módulos en paths del sistema
- Importación de submódulos (`include dir/subdir/module`)

---

Para más información sobre Rhodesia, consulta el [README.md](README.md) principal.
