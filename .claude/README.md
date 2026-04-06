# Claude Workspace Configuration

Esta carpeta define la configuración de trabajo para Claude en este repositorio.

## Estructura

- `settings.local.json`: permisos locales para comandos.
- `directives/design.md`: directivas de diseño y estándares del proyecto.
- `agents/`: subagentes especializados para tareas frecuentes.

## Subagentes disponibles

- `rhodesia-architect.md`: arquitectura y decisiones de diseño.
- `rhodesia-cpp-implementer.md`: implementación en C++20 del core del lenguaje.
- `rhodesia-docs-steward.md`: documentación técnica y ejemplos.
- `rhodesia-quality-guardian.md`: validación, consistencia y revisión de cambios.

## Uso sugerido

1. Consultar primero `directives/design.md`.
2. Elegir el subagente según el tipo de tarea.
3. Cerrar con una revisión de `rhodesia-quality-guardian`.
