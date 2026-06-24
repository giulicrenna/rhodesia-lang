---
title: flash
layout: default
nav_order: 14
parent: Standard Library
---

# `flash` library

DataFrames estilo pandas para Rhodesia. Librería pura en `.rho` (sin cambios C++)
ubicada en `libs/flash/`. Permite manipular datos tabulares con una API funcional
e inmutable.

Import con `include flash`.

```rho
include flash

df: d = read_csv("datos.csv")
show_df(d)
df: mayores = filter_gt_f(d, "edad", 18.0)
show_df(mayores)
```

## Forma del DataFrame

Un DataFrame es un `record` con cinco campos internos:

```rho
record: DataFrame {
    map: _cols   // map<string, vec|arr<str>>  — columnas
    arr: _names  // arr<str>                    — orden de columnas
    int: _nrows  // int                         — cantidad de filas
    int: _ncols  // int                         — cantidad de columnas
    map: _dtypes // map<string, str>            — dtype por columna
}
```

El usuario **no manipula** estos campos directamente; todas las operaciones se
hacen vía funciones que devuelven un DataFrame nuevo (inmutable desde el punto
de vista del API). Internamente, los `vec` y `arr<str>` se copian defensivamente
para evitar aliasing entre DataFrames.

Dtypes soportados: `"int"`, `"float"`, `"str"`, `"bool"`, `"datetime"`.

## API por categoría

### Creación e IO

| Función | Descripción |
|---|---|
| `read_csv(path)` | Lee un CSV desde disco |
| `read_csv_str(content)` | Lee un CSV desde un string |
| `read_json(path)` | Lee un JSON simple (lista de objetos) |
| `write_csv(df, path)` | Escribe el DataFrame a un CSV |
| `to_csv(df)` | Devuelve el contenido CSV como string |
| `frame()` | Crea un DataFrame vacío |
| `frame_from_cols(map, names, dtypes)` | Construye un df desde un map de columnas |

### Inspección

| Función | Descripción |
|---|---|
| `nrow(df)` / `ncol(df)` | Cantidad de filas / columnas |
| `col_names(df)` | `arr<str>` con los nombres |
| `col_dtypes(df)` | `map<string, str>` con los dtypes |
| `has_col(df, name)` | `1` si la columna existe |
| `col_dtype(df, name)` | dtype de una columna |
| `show_df(df)` | Imprime una tabla ASCII a stdout |
| `to_string(df)` | Devuelve la tabla como string |
| `repr_df(df)` | Resumen compacto (`DataFrame<N cols × M rows>`) |

### Selección

| Función | Descripción |
|---|---|
| `head(df, n)` | Primeras `n` filas |
| `tail(df, n)` | Últimas `n` filas |
| `select_cols(df, names)` | df con sólo las columnas indicadas |
| `drop_cols(df, names)` | df sin esas columnas |
| `reorder(df, names)` | Reordena columnas |
| `at(df, col, row)` | Celda escalar como string |
| `iat(df, i, j)` | Celda escalar por índices i,j |

### Filtros (comparación)

| Función | Descripción |
|---|---|
| `filter_eq(df, col, val)` / `_f` | Igual a `val` |
| `filter_ne(df, col, val)` / `_f` | Distinto de `val` |
| `filter_gt(df, col, val)` / `_f` | Mayor que |
| `filter_ge(df, col, val)` / `_f` | Mayor o igual |
| `filter_lt(df, col, val)` / `_f` | Menor que |
| `filter_le(df, col, val)` / `_f` | Menor o igual |
| `filter_isin(df, col, arr)` | Filtra por lista de valores |
| `filter_notin(df, col, arr)` | Excluye por lista |
| `filter_and(df1, df2)` | Intersección de filas |
| `filter_or(df1, df2)` | Unión de filas |
| `query(df, expr)` | Mini-parser: `"col op val AND col op val"` |

Las versiones `_f` reciben un `float64` directamente y evitan parsing del valor.

### Sort

| Función | Descripción |
|---|---|
| `sort_by(df, col, order)` | `order ∈ {"asc", "desc"}` |

### Agregaciones

| Función | Descripción |
|---|---|
| `col_sum(df, col)` | Suma de una col numérica |
| `col_mean(df, col)` | Media |
| `col_std(df, col)` / `col_var(df, col)` | Desvío estándar / varianza |
| `col_min(df, col)` / `col_max(df, col)` | Mínimo / máximo |
| `col_median(df, col)` | Mediana |
| `col_count(df)` / `col_n_missing(df, col)` | Conteo de filas / missing |
| `describe(df)` | Resumen estadístico por col numérica |
| `group_by(df, col)` | Agrupa; devuelve un GroupBy |
| `group_agg(gb, col, op)` | Agrega; `op ∈ {sum,mean,median,min,max,count,std}` |
| `value_counts(df, col)` | Frecuencia de cada valor |
| `unique(df, col)` | Valores únicos como `arr<str>` |
| `drop_duplicates(df)` / `duplicated(df)` | Elimina/reporting duplicados |

### Missing data

| Función | Descripción |
|---|---|
| `isna(df)` | df de bools (1 = missing) |
| `isna_col(df, col)` | `vec` de 0/1 para una col |
| `dropna(df)` | Elimina filas con cualquier missing |
| `dropna_col(df, col)` | Elimina filas donde esa col es missing |
| `fillna(df, str)` | Rellena todos los str missing con un valor |
| `fillna_col(df, col, str)` | Rellena una col |
| `interpolate_linear(df, col)` | Interpola linealmente |

### Stats

| Función | Descripción |
|---|---|
| `corr(df, c1, c2)` | Correlación de Pearson |
| `cov(df, c1, c2)` | Covarianza |
| `corr_matrix(df, cols)` | Matriz de correlaciones como df |
| `rank_col(df, col)` | Ranking (con ties averaging) |
| `quantile(df, col, p)` | Cuantil `p ∈ [0,1]` |
| `quartiles(df, col)` | `vec` con [q25, q50, q75] |
| `iqr(df, col)` | Rango intercuartil |
| `diff_col(df, col, periods)` | Diferencia discreta |
| `diff_col_v(df, col, periods)` | Como `vec` |
| `pct_change_col(df, col)` | Cambio porcentual |

### Window functions

| Función | Descripción |
|---|---|
| `rolling_mean(df, col, w)` | Media móvil de ventana `w` |
| `rolling_std(df, col, w)` | Desvío móvil |
| `rolling_sum(df, col, w)` | Suma móvil |
| `expanding_mean(df, col)` | Media acumulada |
| `expanding_sum(df, col)` | Suma acumulada |
| `expanding_std(df, col)` | Desvío acumulado |

### Joins

| Función | Descripción |
|---|---|
| `merge(d1, d2, on, how)` | `how ∈ {"inner","left","right","outer"}` |
| `join(d1, d2, on, how)` | Alias de `merge` |
| `concat(arr<df>, axis)` | `axis ∈ {"vertical","horizontal"}` |
| `add_col(df, name, col_data, dtype)` | Agrega/sobreescribe una col |

### Reshape

| Función | Descripción |
|---|---|
| `pivot(df, index, columns, values)` | Wide: long → wide |
| `melt(df, id_vars, value_vars, var_name, val_name)` | Long: wide → long |
| `transpose_df(df)` | Intercambia filas y columnas (resultado: str cols) |
| `stack(df)` | wide → long (todas las cols como values) |
| `unstack(df)` | long → wide |

### Datetime

| Función | Descripción |
|---|---|
| `to_datetime(df, col)` | Parsea una col str `"YYYY-MM-DD"` a DateTime interno |

### Plot (delegación a `libs/plot`)

Para usar estas funciones, `include plot` debe estar presente (antes o después
de `include flash`).

| Función | Descripción |
|---|---|
| `plot_line(df, x, y)` | Gráfico de línea |
| `plot_scatter(df, x, y)` | Scatter |
| `plot_hist(df, col, bins)` | Histograma |
| `plot_bar(df, labels_col, values_col)` | Barras |

## Ejemplo quick-start

```rho
include flash

// 1. Cargar CSV
df: d = read_csv("ventas.csv")
show_df(d)

// 2. Filtrar + seleccionar
arr: cols = array.create(0)
array.push(cols, "producto")
array.push(cols, "monto")
df: view = select_cols(filter_gt_f(d, "monto", 1000.0), cols)

// 3. Agregar por grupo
df: gb = group_agg(group_by(d, "producto"), "monto", "mean")

// 4. Stats
float64: r = corr(d, "monto", "unidades")
print("correlación: ", _float_to_str(r))

// 5. Escribir CSV
write_csv(view, "view.csv")
```

## Ejemplos completos

Hay 8 ejemplos ejecutables en `examples/11_new_features/flash/`:

- `01_basic_csv.rho` — read/write + head/tail + select/drop
- `02_filter_sort.rho` — filtros encadenados + sort + query
- `03_group_by.rho` — group_by + group_agg + describe + value_counts
- `04_merge.rho` — inner/left/outer merge + concat vertical
- `05_reshape.rho` — pivot + melt + transpose + stack
- `06_missing.rho` — isna + dropna + fillna + interpolate_linear
- `07_window.rho` — rolling + expanding + rank + quantile + corr
- `08_plot_integration.rho` — flash + libs/plot → SVG

Para ejecutarlos:

```bash
./installer/windows/staging/src/build/rhodesia.exe examples/11_new_features/flash/01_basic_csv.rho
```

## Caveats y simplificaciones

- **Sin cambios C++** — todo se implementa en `.rho`. Si una operación resulta
  prohibitivamente lenta, está marcada con `ponytail:` indicando el techo y la
  ruta de upgrade.
- **`query()` limitado** — sólo soporta `col op val` y combinaciones con
  `AND`/`OR`. No permite expresiones aritméticas ni llamadas a función.
- **Sin Excel/Parquet/HTML/SQL** — sólo CSV y JSON simple (lista de objetos).
- **Sin MultiIndex** — los índices son siempre integer-position.
- **Sin Categorical/Period/Interval** — dtypes son `int`, `float`, `str`,
  `datetime`, `bool`.
- **`merge`/`group_by` con sorts O(n²)** en el peor caso — anotado, no optimizado.
- **Cada operación copia `vec`s y `arr`s** defensivamente — sin copy-on-write.
  Esto asegura inmutabilidad semántica pero cuesta memoria.
- **Sin tests automáticos** — los 8 ejemplos son a la vez demo y verificación
  manual.
- **Convención de missing en numéricas**: las celdas vacías en CSV se almacenan
  como string vacío en una col `arr<str>`. No se promueven automáticamente a
  NaN para no perder la información de "estaba vacío".
- **Nombre `show` colisiona con `libs/plot`** — flash exporta `show_df(df)`
  en su lugar. Si sólo usás flash, podés llamar `show_df(df)`; si combinás con
  plot, también es `show_df`.
- **`_intify` interno** — flash renombra su helper a `_flash_intify` para no
  chocar con el de `libs/plot`.

## Tests manuales

Ejecutar todos los ejemplos en orden:

```bash
for f in examples/11_new_features/flash/*.rho; do
  echo "=== $f ==="
  ./installer/windows/staging/src/build/rhodesia.exe "$f" 2>&1 | tail -3
done
```

Todos deben terminar con `Done.` y no producir `Internal error:` ni
`Runtime error:`.