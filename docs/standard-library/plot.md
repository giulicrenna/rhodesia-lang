---
title: plot
layout: default
nav_order: 13
parent: Standard Library
---

# `plot` library

Matplotlib-like 2D plotting for Rhodesia. Pure `.rho` library that produces SVG
files — no native dependencies, no rebuild, ships under `libs/plot/`.

Import with `include plot` (explicit, not auto-imported).

```rho
include plot

vec: x = numerical.linspace(0.0, 10.0, 50)
vec: y = math.sqrt(x)

figure(800, 510)
line(x, y, "sqrt(x)", "blue")
title("Square root")
xlabel("x")
ylabel("sqrt(x)")
legend(1)
int: bytes = savefig("out.svg")
```

Output is a self-contained SVG that opens in any browser. Pixel coordinates are
floats; axis labels are formatted to one decimal (`"3.1"`, `"-0.5"`).

## Figure setup

| Function | Description |
|---|---|
| `figure(w, h)` | Reset all state, start a new figure of size `w × h`. |
| `subplot(rows, cols, idx)` | Select the active panel in a `rows × cols` grid (1-based, row-major). Resets to 1×1 on next `figure()`. |
| `current_subplot()` | Return the active panel index (1-based). |
| `title(s)` | Set the current panel's title. |
| `xlabel(s)` / `ylabel(s)` | Axis labels for the current panel. |
| `xlim(lo, hi)` / `ylim(lo, hi)` | Force axis range (else auto with 5% padding). |
| `legend(on)` | `1` = show legend, `0` = hide. |

## Data series

| Function | Description |
|---|---|
| `line(x, y, label, color)` | Polyline through `(x[i], y[i])`. `color=""` → cycle palette. |
| `scatter(x, y, label, color)` | One circle per point. |
| `bar(labels, values, label, color)` | Bars indexed 0..n. `labels` is an `arr` of strings. |
| `hist(values, bins, label, color)` | Compute equal-width bins, render as bars. Baselines at y=0. |
| `area(x, y, label, color)` | Filled polygon from y down to y=0. |
| `fill_between(x, y1, y2, label, color)` | Filled polygon between two curves. |
| `stacked_area(x, layers, labels, colors)` | Multiple y series stacked vertically. `layers` is an `arr` of `vec`. |
| `errorbar(x, y, err, label, color)` | Line + vertical error bars with caps. |
| `heatmap(rows, row_labels, col_labels, label)` | 2D grid of colored rects. `rows` is an `arr` of `vec`. |
| `radar(axes, values, label, color)` | Spider chart over N categorical axes. When a panel contains only radar series, the rectangular axes and ticks are suppressed and the chart is centered with concentric ring guides. |
| `boxplot(groups, labels, orient, notch, label, color)` | Side-by-side box plots. `groups` is an `arr` of `vec`, `labels` is an `arr` of strings. `orient`: 0 = vertical, 1 = horizontal. `notch`: 0 = plain box, 1 = notched median with 95% CI. Each box draws whiskers (1.5×IQR), Q1-Q3 box, median line, outlier circles. |
| `plot(f, a, b, label, color)` | Sample `f` on `linspace(a, b, 200)`, add as line. `f` must be a local `.rho` function (built-in `math.sin` etc. can't be passed directly — wrap them). |

`hist`, `bar`, and `area` always baseline at y=0, so the panel resolver forces
`ymin ≤ 0` when any of these series are present.

### Color palette

All series cycle through ten colors when `color` is `""`. Pass any CSS color
string (`"steelblue"`, `"#ff00aa"`) to override.

## Output

| Function | Description |
|---|---|
| `savefig(path)` | Render the current figure to `path` (SVG). Returns bytes written. |
| `show()` | Print the SVG to stdout. |
| `to_svg()` | Render to a string (useful for embedding / inspection). |

## State model

All figure state lives in module-level variables. Each figure is a grid of one
or more panels (`_PANELS` array); each panel holds its own series, title,
labels, axis limits, and legend flag. The figure-level setters (line width,
alpha, background, theme) are shared across all panels.

State vars are declared in `index.rho` (not a sub-file) because Rhodesia's
`include` shares function definitions across sibling includes but **not**
variable declarations — each include runs its top-level statements in a
private scope. Sub-files are functions-only.

## Themes

Five built-in themes, switched with `set_theme(name)`:

| Constant | Description |
|---|---|
| `THEME_DEFAULT` | Matplotlib-style default (blue / orange / green / red / purple / …) |
| `THEME_DARK` | Dark background (`#1e1e1e`), light strokes (`#4c9aff`, `#ff6b6b`, …) |
| `THEME_PASTEL` | Soft fills on `#fdfcf7` background |
| `THEME_MONO` | Greyscale only |
| `THEME_VIBRANT` | High-saturation accent colors |

```rho
figure(800, 510)
set_theme(THEME_DARK)
set_linewidth(3.0)
line(x, y, "series", "")
savefig("dark.svg")
```

## Customization

These setters adjust the current figure (sticky until next `figure()`):

| Function | Default | Effect |
|---|---|---|
| `set_linewidth(w)` | `1.5` | Stroke width for `line` series and legend swatches |
| `set_marker_size(r)` | `3.0` | Circle radius for `scatter` and legend swatches |
| `set_alpha(a)` | `0.5` | Series opacity (0..1) |
| `set_background_color(c)` | `"white"` (or theme default) | SVG background fill |
| `set_figure_size(w, h)` | `800×510` | Canvas dimensions |

Call before `savefig()`. Call `figure()` to reset to defaults + default theme.

## Module layout

```
libs/plot/
├── index.rho                — entry: state vars + public API
├── format/
│   └── format.rho           — number formatting (_num_to_str, _int_to_str, _intify)
├── themes/
│   └── themes.rho           — predefined themes + set_theme() + _next_color()
├── series/
│   ├── basic.rho            — line/scatter/bar/hist/plot/area/fill_between/stacked_area/errorbar
│   ├── heatmap.rho          — heatmap
│   ├── radar.rho            — radar
│   └── boxplot.rho          — boxplot (median/Q1/Q3/whiskers/outliers)
├── svg/
│   ├── svg.rho              — to_svg, axis ticks, panel resolver
│   ├── render_basic.rho     — _render_line/scatter/bar/hist/area/...
│   ├── render_grid.rho      — _render_heatmap + color interpolation
│   ├── render_polar.rho     — _render_radar
│   ├── render_boxplot.rho   — _render_boxplot (vertical/horizontal, notched)
│   └── legend.rho           — _legend_block
└── layout/
    └── subplot.rho          — subplot(), current_subplot()
```

State vars are declared in `index.rho` (not a sub-file) because Rhodesia's
`include` shares function definitions across sibling includes but **not**
variable declarations — each include runs its top-level statements in a
private scope. Sub-files are functions-only.

## Examples

See [examples/11_new_features/plot/](../../examples/11_new_features/plot/):

- `basic_line.rho` — sqrt curve from data vectors.
- `function_plot.rho` — sine wave from `plot(sin_fn, 0, 2*PI)`.
- `histogram.rho` — bins 1000 uniform samples into 25 bars.
- `multi_series.rho` — sin and cos on the same axes with a legend.
- `themes_demo.rho` — same data rendered five times with different themes.
- `area.rho` — Gaussian area chart.
- `fill_between.rho` — confidence band between two curves.
- `stacked_area.rho` — three stacked series.
- `errorbar.rho` — line with vertical error bars.
- `heatmap.rho` — 8×8 colored grid.
- `radar.rho` — three overlapping spider polygons over 6 axes.
- `boxplot.rho` — three-group vertical boxplot with notched median.
- `boxplot_horizontal.rho` — four-group horizontal boxplot for long category labels.
- `subplot_grid.rho` — 2×2 grid with line / scatter / bar / hist in each.

## Caveats / ponytail simplifications

- 1-decimal axis labels. Sub-pixel tick alignment is good enough for browsers.
- No line styles, no log axes, no twin axes.
- No PNG/PDF output — SVG only.
- No colorbar for heatmap (single gradient is enough for v1).
- No auto-open. Saves to file; user opens.
- No per-panel themes (all panels share the current theme).
- Heatmap data shape is `arr` of `vec` (rows). Matrix element assignment isn't
  supported by Rhodesia, so 2D data is built row-by-row.