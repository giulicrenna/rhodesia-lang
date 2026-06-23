#!/usr/bin/env bash

set -u

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
EXAMPLES_DIR="$ROOT_DIR/examples"
RUNNER="$ROOT_DIR/rhodesia"

MODE="all"
TIMEOUT_SECONDS=20
INCLUDE_INTERACTIVE=0
FAIL_FAST=0

print_help() {
  cat <<'USAGE'
Uso:
  bash scripts/test/run_examples.sh [all|fast] [opciones]

Modos:
  all   Corre todos los .rho dentro de examples/ (por defecto)
  fast  Excluye examples/10_applications/ para una corrida más rápida

Opciones:
  --runner <path>            Ruta al ejecutable de rhodesia (default: ./build/rhodesia)
  --timeout <segundos>       Timeout por archivo (default: 20)
  --include-interactive      Incluye ejemplos interactivos (por defecto se excluyen)
  --fail-fast                Corta en el primer error
  -h, --help                 Muestra esta ayuda

Ejemplos:
  bash scripts/test/run_examples.sh
  bash scripts/test/run_examples.sh fast
  bash scripts/test/run_examples.sh all --timeout 10
USAGE
}

if [[ $# -gt 0 ]]; then
  case "$1" in
    all|fast)
      MODE="$1"
      shift
      ;;
  esac
fi

while [[ $# -gt 0 ]]; do
  case "$1" in
    --runner)
      RUNNER="$2"
      shift 2
      ;;
    --timeout)
      TIMEOUT_SECONDS="$2"
      shift 2
      ;;
    --include-interactive)
      INCLUDE_INTERACTIVE=1
      shift
      ;;
    --fail-fast)
      FAIL_FAST=1
      shift
      ;;
    -h|--help)
      print_help
      exit 0
      ;;
    *)
      echo "[error] Opción desconocida: $1"
      print_help
      exit 2
      ;;
  esac
done

if [[ ! -x "$RUNNER" ]]; then
  echo "[error] No se encontró ejecutable: $RUNNER"
  echo "        Compilá primero: cmake -S . -B build -DCMAKE_BUILD_TYPE=Release && cmake --build build -j\$(nproc)"
  exit 2
fi

if [[ ! "$TIMEOUT_SECONDS" =~ ^[0-9]+$ ]] || [[ "$TIMEOUT_SECONDS" -le 0 ]]; then
  echo "[error] --timeout debe ser un entero positivo"
  exit 2
fi

mapfile -t ALL_FILES < <(find "$EXAMPLES_DIR" -type f -name "*.rho" | sort)

if [[ ${#ALL_FILES[@]} -eq 0 ]]; then
  echo "[error] No se encontraron archivos .rho en $EXAMPLES_DIR"
  exit 2
fi

FILES=()
for file in "${ALL_FILES[@]}"; do
  rel="${file#"$ROOT_DIR"/}"

  if [[ "$MODE" == "fast" && "$rel" == examples/10_applications/* ]]; then
    continue
  fi

  if [[ "$INCLUDE_INTERACTIVE" -eq 0 ]]; then
    if [[ "$rel" == *"/interactive.rho" ]] || \
       [[ "$rel" == *"/tcp_server.rho" ]]; then
      continue
    fi
  fi

  FILES+=("$file")
done

echo "============================================================"
echo "Rhodesia examples runner"
echo "Mode:       $MODE"
echo "Runner:     $RUNNER"
echo "Timeout:    ${TIMEOUT_SECONDS}s por archivo"
echo "Files:      ${#FILES[@]}"
echo "Interactive: $( [[ "$INCLUDE_INTERACTIVE" -eq 1 ]] && echo "incluidos" || echo "excluidos" )"
echo "============================================================"

if [[ ${#FILES[@]} -eq 0 ]]; then
  echo "[warn] No hay archivos para ejecutar con los filtros actuales"
  exit 0
fi

TOTAL=0
PASSED=0
FAILED=0
START_ALL=$(date +%s)

for file in "${FILES[@]}"; do
  TOTAL=$((TOTAL + 1))
  rel="${file#"$ROOT_DIR"/}"

  if timeout "${TIMEOUT_SECONDS}s" "$RUNNER" "$file" >/tmp/rhodesia_example.out 2>/tmp/rhodesia_example.err; then
    PASSED=$((PASSED + 1))
    printf "[%03d/%03d] [ok]   %s\n" "$TOTAL" "${#FILES[@]}" "$rel"
  else
    status=$?
    FAILED=$((FAILED + 1))

    if [[ "$status" -eq 124 ]]; then
      reason="timeout"
    else
      reason="exit $status"
    fi

    printf "[%03d/%03d] [fail] %s (%s)\n" "$TOTAL" "${#FILES[@]}" "$rel" "$reason"

    if [[ -s /tmp/rhodesia_example.err ]]; then
      echo "  --- stderr ---"
      sed -n '1,10p' /tmp/rhodesia_example.err | sed 's/^/  /'
    elif [[ -s /tmp/rhodesia_example.out ]]; then
      echo "  --- output ---"
      sed -n '1,10p' /tmp/rhodesia_example.out | sed 's/^/  /'
    fi

    if [[ "$FAIL_FAST" -eq 1 ]]; then
      echo "[info] fail-fast activado, deteniendo ejecución."
      break
    fi
  fi
done

END_ALL=$(date +%s)
ELAPSED=$((END_ALL - START_ALL))

echo ""
echo "==================== Summary ===================="
echo "Total:   $TOTAL"
echo "Passed:  $PASSED"
echo "Failed:  $FAILED"
echo "Elapsed: ${ELAPSED}s"
echo "================================================="

if [[ "$FAILED" -gt 0 ]]; then
  exit 1
fi

exit 0
