# Directivas de Diseño - Rhodesia

Estas directivas aplican a toda implementación propuesta por Claude y sus subagentes.

## 1) Principios de arquitectura

- Mantener separación clara entre **Lexer**, **Parser**, **AST**, **Evaluator** y utilidades.
- Evitar acoplamiento innecesario entre etapas del pipeline del lenguaje.
- Priorizar cambios incrementales y compatibles con la estructura existente.

## 2) Estilo de implementación (C++20)

- Favorecer código explícito, legible y mantenible por sobre micro-optimizaciones prematuras.
- Mantener nombres consistentes con el dominio de Rhodesia.
- Minimizar duplicación de lógica; extraer helpers cuando sea apropiado.
- Preservar mensajes de error claros y accionables.

## 3) Tipado y semántica del lenguaje

- Respetar el tipado fuerte del lenguaje (`int`, `float64`, `str`, `vec`, `mat`).
- Cualquier extensión semántica debe:
  1. estar documentada,
  2. incluir ejemplos,
  3. no romper comportamiento previo sin justificación explícita.

## 4) Documentación y ejemplos

- Todo cambio relevante debe reflejarse en documentación (`docs/` o `documentation/`).
- NO crear comentarios al estilo de CLAUDE, debe ser lo mas humano posible.
- Incluir ejemplos mínimos reproducibles cuando se agreguen funciones o sintaxis.
- Evitar divergencias entre README y documentación técnica.

## 5) Criterios de calidad

- Verificar impacto cruzado antes de cerrar (código, docs, ejemplos).
- Mantener compatibilidad con flujo de compilación actual (CMake/build).
- Comunicar claramente riesgos, supuestos y trabajo pendiente.