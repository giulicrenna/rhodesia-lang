---
title: vec.map(fn) element-wise dispatch
date: 2026-06-24
status: approved (user selected option B)
---

# Element-wise mapping on `vec` / `mat`

## Context

`examples/11_new_features/plot/sin_line.rho:7` fails with
`Type error: Cannot apply '/' to int and vec` because `applyDiv` in
[`include/Evaluator.hpp:1549-1586`](../../include/Evaluator.hpp#L1549-L1586)
is missing the `Scalar/Vec` broadcast arm (only `Vec/Scalar` exists).

There is also no general syntax for applying a user-defined function
element-wise over a `vec`. Workarounds used in 10+ plot examples:

- C++ element-wise builtins (`math.sin`, `math.sqrt`, ...) — limited to ~18 fns.
- `plot(f, a, b, ...)` — bundles 200 linspace samples with a `line()` call.
- Hand-rolled `math.zeros(n) + while loop`.
- `libs/math/linear_algebra/vectors.rho` helpers (`vec_hadamard`, ...).

User asked: *propose a syntax for applying mapping operations to vec.*
Picked: **`v.map(fn)` method-call on vec and mat, always available
(no `include` required), and additionally close the `/` gap so
`1 / math.sin(x)` works directly.**

## Design

### Change 1 — `applyDiv`: add `Scalar/Vec` and `Vec/Vec` arms

In `include/Evaluator.hpp` `applyDiv` (line 1549), before the `throw`
fallback, add:

```cpp
// Scalar / Vector (broadcast)
else if constexpr ((std::is_same_v<L, double> || std::is_same_v<L, int64_t>) &&
                  std::is_same_v<R, Eigen::VectorXd>) {
    return Eigen::VectorXd(static_cast<double>(l) / r.array());
}
// Vector / Vector (element-wise)
else if constexpr (std::is_same_v<L, Eigen::VectorXd> &&
                  std::is_same_v<R, Eigen::VectorXd>) {
    if (l.size() != r.size()) {
        throw RuntimeError::dimensionMismatch("/", l.size(), r.size());
    }
    Eigen::VectorXd result(l.size());
    for (Eigen::Index i = 0; i < l.size(); ++i) {
        if (r(i) == 0.0) { throw RuntimeError::divisionByZero(loc); }
        result(i) = l(i) / r(i);
    }
    return result;
}
```

VM path (`include/VM.hpp:474-519`) is **deferred** to a follow-up —
the user is hitting the tree-walking interpreter path. The VM still
has the same gap; flag in a separate spec.

### Change 2 — `v.map(fn)` method dispatch

The member-call syntax (`expr.method(args)`) is already parsed via
`ChainedMemberAccessNode`. Currently `visit(ChainedMemberAccessNode)`
([Evaluator.hpp:288-315](../../include/Evaluator.hpp#L288-L315))
dispatches only on `RhoRecord`. Add a branch that intercepts `vec`
and `mat` receivers with method name `"map"`.

Add a new helper next to `applyVecMap` definitions:

```cpp
RhoValue applyVecMap(const RhoValue& receiver,
                     const std::shared_ptr<RhoFunction>& fn,
                     SourceLocation loc) {
    return std::visit([&fn, &loc, this](const auto& r) -> RhoValue {
        using T = std::decay_t<decltype(r)>;
        if constexpr (std::is_same_v<T, Eigen::VectorXd>) {
            Eigen::VectorXd out(r.size());
            for (Eigen::Index i = 0; i < r.size(); ++i) {
                RhoValue cell = callLambda(fn, {RhoValue(r(i))}, loc);
                out(i) = toDouble(cell);  // throws if not numeric scalar
            }
            return out;
        } else if constexpr (std::is_same_v<T, Eigen::MatrixXd>) {
            Eigen::MatrixXd out(r.rows(), r.cols());
            for (Eigen::Index i = 0; i < r.rows(); ++i) {
                for (Eigen::Index j = 0; j < r.cols(); ++j) {
                    RhoValue cell = callLambda(fn, {RhoValue(r(i, j))}, loc);
                    out(i, j) = toDouble(cell);
                }
            }
            return out;
        } else {
            throw TypeError("vec.map / mat.map: receiver must be vec or mat",
                            loc);
        }
    }, receiver);
}
```

Insert in `visit(ChainedMemberAccessNode)` **before** the record dispatch:

```cpp
if (node.field == "map") {
    RhoValue obj = result_;
    RhoType rt = getValueType(obj);
    if (rt == RhoType::Vec || rt == RhoType::Mat) {
        if (node.arguments.size() != 1) {
            throw RuntimeError(".map() expects exactly 1 argument (a function)",
                                node.location);
        }
        node.arguments[0]->accept(*this);
        if (!std::holds_alternative<std::shared_ptr<RhoFunction>>(result_)) {
            throw TypeError(".map() argument must be a function "
                            "(got " + typeToString(getValueType(result_)) + ")",
                            node.location);
        }
        auto fn = std::get<std::shared_ptr<RhoFunction>>(result_);
        result_ = applyVecMap(obj, fn, node.location);
        return;
    }
    // fall through to record dispatch for non-vec/mat receivers
}
```

`callLambda` ([Evaluator.hpp:1693](../../include/Evaluator.hpp#L1693-L1758))
already handles native vs AST-bodied lambdas and int↔float coercion.

### Change 3 — fix `examples/11_new_features/plot/sin_line.rho`

```rho
include plot
println("=== Sin line plot ===")
vec: x = numerical.linspace(0.1, 10.0, 50)         // start at 0.1, sin(0)=0
vec: y = 1.0 / math.sin(x)                         // works after Change 1
figure(800, 510)
line(x, y, "1/sin(x)", "blue")
title("Reciprocal sine")
xlabel("x"); ylabel("1/sin(x)"); legend(1)
int: bytes = savefig("line_sin.svg")
println("Wrote ", bytes, " bytes to line_sin.svg")
```

### Change 4 — new example showcasing `.map()`

`examples/11_new_features/plot/map_demo.rho`:

```rho
include plot
println("=== .map() demo ===")
vec: x = numerical.linspace(-3.14, 3.14, 200)

fun gauss_bump(float64: xx) -> float64 {
    return math.exp(-(xx * xx) / 2.0)
}

vec: y = x.map(gauss_bump)        // element-wise via user fn
figure(800, 510)
line(x, y, "gaussian", "purple")
title("User fn via .map()")
xlabel("x"); ylabel("exp(-x²/2)"); legend(1)
int: bytes = savefig("map_demo.svg")
println("Wrote ", bytes, " bytes")
```

### Change 5 — documentation

`docs/language/operators.md`: extend the `/` row and the vector-matrix
operator table to reflect that `s / v` and `v / v` are element-wise.

`docs/standard-library/`: short note under vec/mat that `.map(fn)` is
available without import.

## Verification

1. Build: invoke the existing C++ build (`cmake --build` or `ninja`).
2. `rhodesia examples/11_new_features/plot/sin_line.rho` → produces
   `line_sin.svg` without error.
3. `rhodesia examples/11_new_features/plot/map_demo.rho` → produces
   `map_demo.svg` without error.
4. Regression: `rhodesia examples/11_new_features/plot/basic_line.rho`,
   `function_plot.rho`, `test_origin.rho`, `area.rho`, `errorbar.rho`,
   `fill_between.rho`, `stacked_area.rho`, `histogram.rho`,
   `subplot_grid.rho` all still produce valid SVGs.
5. Spot check: `vec: z = [2.0, 4.0, 6.0]; vec: w = z.map(fn(x) => x + 1)`
   should yield `[3, 5, 7]`.

## Out of scope (YAGNI)

- `.filter(fn)`, `.reduce(fn, init)`, `.each(fn)`.
- Operator overloading.
- VM path parity for the `/` change.
- `.map()` returning vec-of-anything (output is constrained to numeric
  scalar via `toDouble`; throw on non-numeric return).

## Files

| File | Change |
|---|---|
| `include/Evaluator.hpp` | `applyDiv` +2 arms; `visit(ChainedMemberAccessNode)` +`.map` branch; new `applyVecMap` helper |
| `examples/11_new_features/plot/sin_line.rho` | Domain fix + `1.0` literal |
| `examples/11_new_features/plot/map_demo.rho` | NEW |
| `docs/language/operators.md` | `/` row + vector-matrix table updated |
