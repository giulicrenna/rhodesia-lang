// builtinDescriptors.js
// Complete documentation for Rhodesia built-in functions and standard libraries

// ============================================================
// Global built-in functions (callable without any module prefix)
// Includes: print/IO, range, timing, type constructors,
// and all math/vector/matrix module functions (also exposed globally)
// ============================================================
const builtinFunctions = [
    // ---- I/O ----
    {
        label: 'print',
        detail: 'print(...) -> void',
        documentation: 'Print one or more values to stdout without a trailing newline. Multiple arguments are separated by spaces.',
        signatures: [{ label: 'print(...values)', documentation: 'Print values without newline', parameters: [{ label: 'values', documentation: 'One or more values of any type' }] }]
    },
    {
        label: 'println',
        detail: 'println(...) -> void',
        documentation: 'Print one or more values to stdout followed by a newline. Multiple arguments are separated by spaces.',
        signatures: [{ label: 'println(...values)', documentation: 'Print values with newline', parameters: [{ label: 'values', documentation: 'One or more values of any type' }] }]
    },

    // ---- Iteration ----
    {
        label: 'range',
        detail: 'range(end: int) -> range | range(start: int, end: int) -> range',
        documentation: 'Create a lazy integer range for iteration. `range(n)` yields 0..n-1; `range(start, end)` yields start..end-1. Use in `for i in range(...)` loops.',
        signatures: [
            { label: 'range(end)', documentation: 'Yields integers 0, 1, ..., end-1', parameters: [{ label: 'end', documentation: 'Exclusive upper bound (int)' }] },
            { label: 'range(start, end)', documentation: 'Yields integers start, start+1, ..., end-1', parameters: [{ label: 'start', documentation: 'Inclusive lower bound (int)' }, { label: 'end', documentation: 'Exclusive upper bound (int)' }] }
        ]
    },

    // ---- Timing ----
    {
        label: 'get_tick',
        detail: 'get_tick() -> float64',
        documentation: 'Return a high-precision monotonic timestamp in seconds (based on `std::chrono::steady_clock`). Useful for benchmarking. Does not accept arguments.',
        signatures: [{ label: 'get_tick()', documentation: 'Returns current monotonic time in seconds', parameters: [] }]
    },

    // ---- Type constructors ----
    {
        label: 'make_complex',
        detail: 'make_complex(real: float64, imag: float64) -> complex',
        documentation: 'Create a complex number with the given real and imaginary parts.',
        signatures: [{ label: 'make_complex(real, imag)', documentation: 'Create complex number', parameters: [{ label: 'real', documentation: 'Real part (float64)' }, { label: 'imag', documentation: 'Imaginary part (float64)' }] }]
    },
    {
        label: 'make_set',
        detail: 'make_set(...) -> set',
        documentation: 'Create a set. Call with no arguments for an empty set, or with initial values: `make_set(1, 2, 3)`.',
        signatures: [{ label: 'make_set(...values)', documentation: 'Create set with optional initial values', parameters: [{ label: 'values', documentation: 'Zero or more initial values' }] }]
    },
    {
        label: 'make_tuple',
        detail: 'make_tuple(...) -> tuple',
        documentation: 'Create a heterogeneous tuple from the given values: `make_tuple(1, "hello", 3.14)`.',
        signatures: [{ label: 'make_tuple(...values)', documentation: 'Create tuple', parameters: [{ label: 'values', documentation: 'Values of any type' }] }]
    },
    {
        label: 'make_record',
        detail: 'make_record() -> record',
        documentation: 'Create an empty mutable record (key-value store with string keys). Add fields with `mapping.set`.',
        signatures: [{ label: 'make_record()', documentation: 'Create empty record', parameters: [] }]
    },

    // ====================================================================
    // Math module functions — also accessible as globals
    // ====================================================================

    // ---- Linear algebra ----
    {
        label: 'norm',
        detail: 'norm(v: vec) -> float64 | norm(m: mat) -> float64',
        documentation: 'Compute the Euclidean (L2) norm of a vector, or the Frobenius norm of a matrix. Also callable as `math.norm`.',
        signatures: [
            { label: 'norm(v)', documentation: 'Euclidean norm of vector', parameters: [{ label: 'v', documentation: 'Input vector (vec)' }] },
            { label: 'norm(m)', documentation: 'Frobenius norm of matrix', parameters: [{ label: 'm', documentation: 'Input matrix (mat)' }] }
        ]
    },
    {
        label: 'dot',
        detail: 'dot(a: vec, b: vec) -> float64',
        documentation: 'Compute the dot (inner) product of two vectors. Both vectors must have the same length. Also callable as `math.dot`.',
        signatures: [{ label: 'dot(a, b)', documentation: 'Dot product of two equal-length vectors', parameters: [{ label: 'a', documentation: 'First vector' }, { label: 'b', documentation: 'Second vector' }] }]
    },
    {
        label: 'transpose',
        detail: 'transpose(m: mat) -> mat | transpose(v: vec) -> mat',
        documentation: 'Transpose a matrix, or convert a column vector to a 1×N row matrix. Also callable as `math.transpose`.',
        signatures: [
            { label: 'transpose(m)', documentation: 'Transpose of matrix', parameters: [{ label: 'm', documentation: 'Input matrix (mat)' }] },
            { label: 'transpose(v)', documentation: 'Transpose column vector to row matrix', parameters: [{ label: 'v', documentation: 'Input vector (vec)' }] }
        ]
    },
    {
        label: 'inv',
        detail: 'inv(m: mat) -> mat',
        documentation: 'Compute the matrix inverse using full-pivot LU decomposition. Throws if the matrix is singular or non-square. Also callable as `math.inv`.',
        signatures: [{ label: 'inv(m)', documentation: 'Inverse of a square non-singular matrix', parameters: [{ label: 'm', documentation: 'Square invertible matrix (mat)' }] }]
    },

    // ---- Reductions ----
    {
        label: 'sum',
        detail: 'sum(v: vec) -> float64 | sum(m: mat) -> float64',
        documentation: 'Sum all elements of a vector or matrix. Also callable as `math.sum`.',
        signatures: [{ label: 'sum(v)', documentation: 'Sum of all elements', parameters: [{ label: 'v', documentation: 'Vector or matrix' }] }]
    },
    {
        label: 'mean',
        detail: 'mean(v: vec) -> float64 | mean(m: mat) -> float64',
        documentation: 'Arithmetic mean of all elements of a vector or matrix. Also callable as `math.mean`.',
        signatures: [{ label: 'mean(v)', documentation: 'Mean of all elements', parameters: [{ label: 'v', documentation: 'Vector or matrix' }] }]
    },

    // ---- Constructors ----
    {
        label: 'zeros',
        detail: 'zeros(n: int) -> vec | zeros(n: int, m: int) -> mat',
        documentation: 'Create a zero vector of size n, or a zero matrix of size n×m. Also callable as `math.zeros`.',
        signatures: [
            { label: 'zeros(n)', documentation: 'Zero vector of length n', parameters: [{ label: 'n', documentation: 'Length of vector (positive int)' }] },
            { label: 'zeros(n, m)', documentation: 'n×m zero matrix', parameters: [{ label: 'n', documentation: 'Rows (positive int)' }, { label: 'm', documentation: 'Columns (positive int)' }] }
        ]
    },
    {
        label: 'ones',
        detail: 'ones(n: int) -> vec | ones(n: int, m: int) -> mat',
        documentation: 'Create a vector or matrix filled with 1.0. Also callable as `math.ones`.',
        signatures: [
            { label: 'ones(n)', documentation: 'Vector of n ones', parameters: [{ label: 'n', documentation: 'Length (positive int)' }] },
            { label: 'ones(n, m)', documentation: 'n×m matrix of ones', parameters: [{ label: 'n', documentation: 'Rows' }, { label: 'm', documentation: 'Columns' }] }
        ]
    },
    {
        label: 'eye',
        detail: 'eye(n: int) -> mat',
        documentation: 'Create an n×n identity matrix. Also callable as `math.eye`.',
        signatures: [{ label: 'eye(n)', documentation: 'n×n identity matrix', parameters: [{ label: 'n', documentation: 'Size (positive int)' }] }]
    },

    // ---- Scalar / element-wise math ----
    {
        label: 'sqrt',
        detail: 'sqrt(x: float64) -> float64 | sqrt(v: vec) -> vec | sqrt(m: mat) -> mat',
        documentation: 'Square root. Works element-wise on vectors and matrices. Also callable as `math.sqrt`.',
        signatures: [{ label: 'sqrt(x)', documentation: 'Square root (scalar or element-wise)', parameters: [{ label: 'x', documentation: 'Scalar, vec, or mat' }] }]
    },
    {
        label: 'exp',
        detail: 'exp(x: float64) -> float64 | exp(v: vec) -> vec | exp(m: mat) -> mat',
        documentation: 'Natural exponential e^x. Works element-wise on vectors and matrices. Also callable as `math.exp`.',
        signatures: [{ label: 'exp(x)', documentation: 'e^x (scalar or element-wise)', parameters: [{ label: 'x', documentation: 'Scalar, vec, or mat' }] }]
    },
    {
        label: 'log',
        detail: 'log(x: float64) -> float64 | log(v: vec) -> vec | log(m: mat) -> mat',
        documentation: 'Natural logarithm ln(x). Works element-wise on vectors and matrices. Also callable as `math.log`.',
        signatures: [{ label: 'log(x)', documentation: 'Natural log (scalar or element-wise)', parameters: [{ label: 'x', documentation: 'Scalar, vec, or mat' }] }]
    },
    {
        label: 'abs',
        detail: 'abs(x: float64) -> float64 | abs(v: vec) -> vec | abs(m: mat) -> mat',
        documentation: 'Absolute value. Works element-wise on vectors and matrices. Also callable as `math.abs`.',
        signatures: [{ label: 'abs(x)', documentation: 'Absolute value (scalar or element-wise)', parameters: [{ label: 'x', documentation: 'Scalar, vec, or mat' }] }]
    },
    {
        label: 'pow',
        detail: 'pow(base: float64, exp: float64) -> float64',
        documentation: 'Raise `base` to the power `exp`. Also callable as `math.pow`.',
        signatures: [{ label: 'pow(base, exp)', documentation: 'base^exp', parameters: [{ label: 'base', documentation: 'Base value' }, { label: 'exp', documentation: 'Exponent' }] }]
    },
    {
        label: 'cbrt',
        detail: 'cbrt(x: float64) -> float64',
        documentation: 'Cube root of x. Also callable as `math.cbrt`.',
        signatures: [{ label: 'cbrt(x)', documentation: 'Cube root', parameters: [{ label: 'x', documentation: 'Scalar value' }] }]
    },

    // ---- Trigonometry ----
    {
        label: 'sin',
        detail: 'sin(x: float64) -> float64',
        documentation: 'Sine of x (radians). Also callable as `math.sin`.',
        signatures: [{ label: 'sin(x)', documentation: 'Sine in radians', parameters: [{ label: 'x', documentation: 'Angle in radians' }] }]
    },
    {
        label: 'cos',
        detail: 'cos(x: float64) -> float64',
        documentation: 'Cosine of x (radians). Also callable as `math.cos`.',
        signatures: [{ label: 'cos(x)', documentation: 'Cosine in radians', parameters: [{ label: 'x', documentation: 'Angle in radians' }] }]
    },
    {
        label: 'tan',
        detail: 'tan(x: float64) -> float64',
        documentation: 'Tangent of x (radians). Also callable as `math.tan`.',
        signatures: [{ label: 'tan(x)', documentation: 'Tangent in radians', parameters: [{ label: 'x', documentation: 'Angle in radians' }] }]
    },
    {
        label: 'asin',
        detail: 'asin(x: float64) -> float64',
        documentation: 'Arc sine (inverse sine) of x. Returns result in radians in [-π/2, π/2]. Also callable as `math.asin`.',
        signatures: [{ label: 'asin(x)', documentation: 'Arcsine in radians', parameters: [{ label: 'x', documentation: 'Value in [-1, 1]' }] }]
    },
    {
        label: 'acos',
        detail: 'acos(x: float64) -> float64',
        documentation: 'Arc cosine of x. Returns result in radians in [0, π]. Also callable as `math.acos`.',
        signatures: [{ label: 'acos(x)', documentation: 'Arccosine in radians', parameters: [{ label: 'x', documentation: 'Value in [-1, 1]' }] }]
    },
    {
        label: 'atan',
        detail: 'atan(x: float64) -> float64',
        documentation: 'Arc tangent of x. Returns result in radians in (-π/2, π/2). Also callable as `math.atan`.',
        signatures: [{ label: 'atan(x)', documentation: 'Arctangent in radians', parameters: [{ label: 'x', documentation: 'Scalar value' }] }]
    },
    {
        label: 'atan2',
        detail: 'atan2(y: float64, x: float64) -> float64',
        documentation: 'Two-argument arc tangent. Returns the angle in radians of the point (x, y) from the positive x-axis, in (-π, π]. Also callable as `math.atan2`.',
        signatures: [{ label: 'atan2(y, x)', documentation: 'atan(y/x) with correct quadrant', parameters: [{ label: 'y', documentation: 'Y coordinate' }, { label: 'x', documentation: 'X coordinate' }] }]
    },
    {
        label: 'sinh',
        detail: 'sinh(x: float64) -> float64',
        documentation: 'Hyperbolic sine of x. Also callable as `math.sinh`.',
        signatures: [{ label: 'sinh(x)', documentation: 'Hyperbolic sine', parameters: [{ label: 'x', documentation: 'Scalar value' }] }]
    },
    {
        label: 'cosh',
        detail: 'cosh(x: float64) -> float64',
        documentation: 'Hyperbolic cosine of x. Also callable as `math.cosh`.',
        signatures: [{ label: 'cosh(x)', documentation: 'Hyperbolic cosine', parameters: [{ label: 'x', documentation: 'Scalar value' }] }]
    },
    {
        label: 'tanh',
        detail: 'tanh(x: float64) -> float64',
        documentation: 'Hyperbolic tangent of x. Also callable as `math.tanh`.',
        signatures: [{ label: 'tanh(x)', documentation: 'Hyperbolic tangent', parameters: [{ label: 'x', documentation: 'Scalar value' }] }]
    },

    // ---- Rounding ----
    {
        label: 'floor',
        detail: 'floor(x: float64) -> float64',
        documentation: 'Round x down to the nearest integer. Also callable as `math.floor`.',
        signatures: [{ label: 'floor(x)', documentation: 'Round down', parameters: [{ label: 'x', documentation: 'Scalar value' }] }]
    },
    {
        label: 'ceil',
        detail: 'ceil(x: float64) -> float64',
        documentation: 'Round x up to the nearest integer. Also callable as `math.ceil`.',
        signatures: [{ label: 'ceil(x)', documentation: 'Round up', parameters: [{ label: 'x', documentation: 'Scalar value' }] }]
    },
    {
        label: 'round',
        detail: 'round(x: float64) -> float64',
        documentation: 'Round x to the nearest integer (half away from zero). Also callable as `math.round`.',
        signatures: [{ label: 'round(x)', documentation: 'Round to nearest integer', parameters: [{ label: 'x', documentation: 'Scalar value' }] }]
    },
    {
        label: 'trunc',
        detail: 'trunc(x: float64) -> float64',
        documentation: 'Truncate x toward zero. Also callable as `math.trunc`.',
        signatures: [{ label: 'trunc(x)', documentation: 'Truncate toward zero', parameters: [{ label: 'x', documentation: 'Scalar value' }] }]
    },

    // ---- Min / Max / Utilities ----
    {
        label: 'min',
        detail: 'min(v: vec) -> float64 | min(a, b, ...) -> float64',
        documentation: 'Minimum element of a vector, or minimum of multiple scalar arguments. Also callable as `math.min`.',
        signatures: [
            { label: 'min(v)', documentation: 'Minimum element of vector', parameters: [{ label: 'v', documentation: 'Input vector' }] },
            { label: 'min(a, b, ...)', documentation: 'Minimum of scalars', parameters: [{ label: 'a', documentation: 'First value' }, { label: 'b', documentation: 'Second value' }] }
        ]
    },
    {
        label: 'max',
        detail: 'max(v: vec) -> float64 | max(a, b, ...) -> float64',
        documentation: 'Maximum element of a vector, or maximum of multiple scalar arguments. Also callable as `math.max`.',
        signatures: [
            { label: 'max(v)', documentation: 'Maximum element of vector', parameters: [{ label: 'v', documentation: 'Input vector' }] },
            { label: 'max(a, b, ...)', documentation: 'Maximum of scalars', parameters: [{ label: 'a', documentation: 'First value' }, { label: 'b', documentation: 'Second value' }] }
        ]
    },
    {
        label: 'clamp',
        detail: 'clamp(value: float64, min: float64, max: float64) -> float64',
        documentation: 'Clamp `value` to the interval [min, max]. If value < min returns min; if value > max returns max. Also callable as `math.clamp`.',
        signatures: [{ label: 'clamp(value, min, max)', documentation: 'Restrict value to [min, max]', parameters: [{ label: 'value', documentation: 'Value to clamp' }, { label: 'min', documentation: 'Lower bound' }, { label: 'max', documentation: 'Upper bound' }] }]
    },
    {
        label: 'sign',
        detail: 'sign(x: float64) -> float64',
        documentation: 'Return 1.0 if x > 0, -1.0 if x < 0, or 0.0 if x == 0. Also callable as `math.sign`.',
        signatures: [{ label: 'sign(x)', documentation: 'Sign of x: -1, 0, or 1', parameters: [{ label: 'x', documentation: 'Scalar value' }] }]
    },
    {
        label: 'lerp',
        detail: 'lerp(a: float64, b: float64, t: float64) -> float64',
        documentation: 'Linear interpolation: returns a + t*(b-a). t=0 gives a, t=1 gives b. Also callable as `math.lerp`.',
        signatures: [{ label: 'lerp(a, b, t)', documentation: 'Linear interpolation between a and b', parameters: [{ label: 'a', documentation: 'Start value' }, { label: 'b', documentation: 'End value' }, { label: 't', documentation: 'Interpolation factor [0, 1]' }] }]
    },
    {
        label: 'deg2rad',
        detail: 'deg2rad(deg: float64) -> float64',
        documentation: 'Convert degrees to radians (deg * π / 180). Also callable as `math.deg2rad`.',
        signatures: [{ label: 'deg2rad(deg)', documentation: 'Degrees to radians', parameters: [{ label: 'deg', documentation: 'Angle in degrees' }] }]
    },
    {
        label: 'rad2deg',
        detail: 'rad2deg(rad: float64) -> float64',
        documentation: 'Convert radians to degrees (rad * 180 / π). Also callable as `math.rad2deg`.',
        signatures: [{ label: 'rad2deg(rad)', documentation: 'Radians to degrees', parameters: [{ label: 'rad', documentation: 'Angle in radians' }] }]
    },
    {
        label: 'factorial',
        detail: 'factorial(n: int) -> int',
        documentation: 'Compute n! for 0 ≤ n ≤ 20. Throws for negative n or n > 20. Also callable as `math.factorial`.',
        signatures: [{ label: 'factorial(n)', documentation: 'n! for 0 ≤ n ≤ 20', parameters: [{ label: 'n', documentation: 'Non-negative integer ≤ 20' }] }]
    },

    // ---- Shape information ----
    {
        label: 'rows',
        detail: 'rows(m: mat) -> int',
        documentation: 'Number of rows in a matrix. Also callable as `math.rows` or `matrix.rows`.',
        signatures: [{ label: 'rows(m)', documentation: 'Row count of matrix', parameters: [{ label: 'm', documentation: 'Input matrix (mat)' }] }]
    },
    {
        label: 'cols',
        detail: 'cols(m: mat) -> int',
        documentation: 'Number of columns in a matrix. Also callable as `math.cols` or `matrix.cols`.',
        signatures: [{ label: 'cols(m)', documentation: 'Column count of matrix', parameters: [{ label: 'm', documentation: 'Input matrix (mat)' }] }]
    },
    {
        label: 'size',
        detail: 'size(v: vec) -> int | size(m: mat) -> int | size(r: range) -> int',
        documentation: 'Total number of elements in a vector, matrix, or range. Also callable as `math.size`, `vector.size`, `matrix.size`.',
        signatures: [{ label: 'size(v)', documentation: 'Number of elements', parameters: [{ label: 'v', documentation: 'vec, mat, or range' }] }]
    },

    // ====================================================================
    // Vector module functions — also accessible as globals
    // ====================================================================
    {
        label: 'append',
        detail: 'append(v: vec, value: float64) -> vec',
        documentation: 'Return a new vector with `value` appended at the end. Does not modify the original. Also callable as `vector.append`.',
        signatures: [{ label: 'append(v, value)', documentation: 'New vector with value appended', parameters: [{ label: 'v', documentation: 'Source vector' }, { label: 'value', documentation: 'Scalar to append' }] }]
    },
    {
        label: 'reverse',
        detail: 'reverse(v: vec) -> vec',
        documentation: 'Return a new vector with elements in reversed order. Also callable as `vector.reverse`.',
        signatures: [{ label: 'reverse(v)', documentation: 'Reversed vector', parameters: [{ label: 'v', documentation: 'Source vector' }] }]
    },
    {
        label: 'insert',
        detail: 'insert(v: vec, index: int, value: float64) -> vec',
        documentation: 'Return a new vector with `value` inserted at `index`. Elements at and after `index` are shifted right. Also callable as `vector.insert`.',
        signatures: [{ label: 'insert(v, index, value)', documentation: 'Insert element at index', parameters: [{ label: 'v', documentation: 'Source vector' }, { label: 'index', documentation: 'Position (0-based)' }, { label: 'value', documentation: 'Scalar to insert' }] }]
    },

    // ====================================================================
    // Matrix module functions — also accessible as globals
    // ====================================================================
    {
        label: 'append_row',
        detail: 'append_row(m: mat, row: vec) -> mat',
        documentation: 'Return a new matrix with `row` appended as the last row. Row length must match matrix column count. Also callable as `matrix.append_row`.',
        signatures: [{ label: 'append_row(m, row)', documentation: 'New matrix with row appended', parameters: [{ label: 'm', documentation: 'Source matrix' }, { label: 'row', documentation: 'Row vector (length = cols(m))' }] }]
    },
    {
        label: 'append_col',
        detail: 'append_col(m: mat, col: vec) -> mat',
        documentation: 'Return a new matrix with `col` appended as the last column. Column length must match matrix row count. Also callable as `matrix.append_col`.',
        signatures: [{ label: 'append_col(m, col)', documentation: 'New matrix with column appended', parameters: [{ label: 'm', documentation: 'Source matrix' }, { label: 'col', documentation: 'Column vector (length = rows(m))' }] }]
    },
    {
        label: 'remove_row',
        detail: 'remove_row(m: mat, index: int) -> mat',
        documentation: 'Return a new matrix without the row at `index`. Also callable as `matrix.remove_row`.',
        signatures: [{ label: 'remove_row(m, index)', documentation: 'Remove row at index', parameters: [{ label: 'm', documentation: 'Source matrix' }, { label: 'index', documentation: 'Row index (0-based)' }] }]
    },
    {
        label: 'remove_col',
        detail: 'remove_col(m: mat, index: int) -> mat',
        documentation: 'Return a new matrix without the column at `index`. Also callable as `matrix.remove_col`.',
        signatures: [{ label: 'remove_col(m, index)', documentation: 'Remove column at index', parameters: [{ label: 'm', documentation: 'Source matrix' }, { label: 'index', documentation: 'Column index (0-based)' }] }]
    },

    // ====================================================================
    // Library functions (libs/math/core/core.rho) — require import
    // ====================================================================
    {
        label: 'safe_divide',
        detail: 'safe_divide(a: float64, b: float64) -> float64',
        documentation: '(libs/math/core) Divide a by b, guarding against division by very small numbers (|b| < 1e-6). Returns a/epsilon if b is near zero, printing a warning.',
        signatures: [{ label: 'safe_divide(a, b)', documentation: 'Safe division', parameters: [{ label: 'a', documentation: 'Numerator' }, { label: 'b', documentation: 'Denominator' }] }]
    },
    {
        label: 'power',
        detail: 'power(base: float64, exponent: int) -> float64',
        documentation: '(libs/math/core) Raise `base` to an integer `exponent`, including negative exponents. More predictable than `pow` for integer powers.',
        signatures: [{ label: 'power(base, exponent)', documentation: 'base^exponent (integer exponent)', parameters: [{ label: 'base', documentation: 'Base value' }, { label: 'exponent', documentation: 'Integer exponent (can be negative)' }] }]
    },
    {
        label: 'vec_abs',
        detail: 'vec_abs(v: vec) -> vec',
        documentation: '(libs/math/core) Element-wise absolute value of a vector. Returns a new vector.',
        signatures: [{ label: 'vec_abs(v)', documentation: 'Element-wise |v|', parameters: [{ label: 'v', documentation: 'Input vector' }] }]
    },
    {
        label: 'map_range',
        detail: 'map_range(value, from_min, from_max, to_min, to_max: float64) -> float64',
        documentation: '(libs/math/core) Map `value` from one range [from_min, from_max] to another [to_min, to_max] via linear interpolation.',
        signatures: [{ label: 'map_range(value, from_min, from_max, to_min, to_max)', documentation: 'Remap value from one range to another', parameters: [{ label: 'value', documentation: 'Value to remap' }, { label: 'from_min', documentation: 'Source range minimum' }, { label: 'from_max', documentation: 'Source range maximum' }, { label: 'to_min', documentation: 'Target range minimum' }, { label: 'to_max', documentation: 'Target range maximum' }] }]
    },
    {
        label: 'approx_equal',
        detail: 'approx_equal(a: float64, b: float64, epsilon: float64) -> bool',
        documentation: '(libs/math/core) Return true if |a - b| ≤ epsilon. Useful for floating-point comparisons.',
        signatures: [{ label: 'approx_equal(a, b, epsilon)', documentation: 'Floating-point equality check', parameters: [{ label: 'a', documentation: 'First value' }, { label: 'b', documentation: 'Second value' }, { label: 'epsilon', documentation: 'Tolerance' }] }]
    },
    {
        label: 'step',
        detail: 'step(x: float64, threshold: float64) -> float64',
        documentation: '(libs/math/core) Step function: returns 1.0 if x ≥ threshold, else 0.0.',
        signatures: [{ label: 'step(x, threshold)', documentation: 'Unit step function', parameters: [{ label: 'x', documentation: 'Input value' }, { label: 'threshold', documentation: 'Threshold' }] }]
    },
    {
        label: 'smoothstep',
        detail: 'smoothstep(x: float64, edge0: float64, edge1: float64) -> float64',
        documentation: '(libs/math/core) Smooth Hermite interpolation between 0 and 1 in [edge0, edge1]. Uses cubic formula 3t²-2t³ for smooth transitions.',
        signatures: [{ label: 'smoothstep(x, edge0, edge1)', documentation: 'Smooth step function', parameters: [{ label: 'x', documentation: 'Input value' }, { label: 'edge0', documentation: 'Lower edge' }, { label: 'edge1', documentation: 'Upper edge' }] }]
    },

    // ====================================================================
    // Library functions (libs/math/core/trigonometry.rho)
    // ====================================================================
    {
        label: 'degrees_to_radians',
        detail: 'degrees_to_radians(degrees: float64) -> float64',
        documentation: '(libs/math/core/trigonometry) Convert degrees to radians (degrees * π / 180).',
        signatures: [{ label: 'degrees_to_radians(degrees)', documentation: 'Convert degrees to radians', parameters: [{ label: 'degrees', documentation: 'Angle in degrees' }] }]
    },
    {
        label: 'radians_to_degrees',
        detail: 'radians_to_degrees(radians: float64) -> float64',
        documentation: '(libs/math/core/trigonometry) Convert radians to degrees (radians * 180 / π).',
        signatures: [{ label: 'radians_to_degrees(radians)', documentation: 'Convert radians to degrees', parameters: [{ label: 'radians', documentation: 'Angle in radians' }] }]
    },
    {
        label: 'wrap_angle',
        detail: 'wrap_angle(angle: float64) -> float64',
        documentation: '(libs/math/core/trigonometry) Wrap angle to [0, 2π) range.',
        signatures: [{ label: 'wrap_angle(angle)', documentation: 'Wrap to [0, 2π)', parameters: [{ label: 'angle', documentation: 'Angle in radians' }] }]
    },
    {
        label: 'wrap_angle_symmetric',
        detail: 'wrap_angle_symmetric(angle: float64) -> float64',
        documentation: '(libs/math/core/trigonometry) Wrap angle to [-π, π] range.',
        signatures: [{ label: 'wrap_angle_symmetric(angle)', documentation: 'Wrap to [-π, π]', parameters: [{ label: 'angle', documentation: 'Angle in radians' }] }]
    },

    // ====================================================================
    // Library functions (libs/math/linear_algebra/vectors.rho)
    // ====================================================================
    {
        label: 'vec_magnitude',
        detail: 'vec_magnitude(v: vec) -> float64',
        documentation: '(libs/math/linear_algebra/vectors) Euclidean magnitude (norm) of vector v. Equivalent to `norm(v)`.',
        signatures: [{ label: 'vec_magnitude(v)', documentation: 'Euclidean length of vector', parameters: [{ label: 'v', documentation: 'Input vector' }] }]
    },
    {
        label: 'vec_distance',
        detail: 'vec_distance(a: vec, b: vec) -> float64',
        documentation: '(libs/math/linear_algebra/vectors) Euclidean distance between vectors a and b: norm(a - b).',
        signatures: [{ label: 'vec_distance(a, b)', documentation: 'Distance between two vectors', parameters: [{ label: 'a', documentation: 'First vector' }, { label: 'b', documentation: 'Second vector' }] }]
    },
    {
        label: 'vec_project',
        detail: 'vec_project(a: vec, b: vec) -> vec',
        documentation: '(libs/math/linear_algebra/vectors) Orthogonal projection of vector a onto vector b.',
        signatures: [{ label: 'vec_project(a, b)', documentation: 'Projection of a onto b', parameters: [{ label: 'a', documentation: 'Vector to project' }, { label: 'b', documentation: 'Direction to project onto' }] }]
    },
    {
        label: 'vec_reject',
        detail: 'vec_reject(a: vec, b: vec) -> vec',
        documentation: '(libs/math/linear_algebra/vectors) Vector rejection: component of a perpendicular to b. Equals a - vec_project(a, b).',
        signatures: [{ label: 'vec_reject(a, b)', documentation: 'Component of a perpendicular to b', parameters: [{ label: 'a', documentation: 'Source vector' }, { label: 'b', documentation: 'Reference direction' }] }]
    },
    {
        label: 'vec_angle_radians',
        detail: 'vec_angle_radians(a: vec, b: vec) -> float64',
        documentation: '(libs/math/linear_algebra/vectors) Angle between vectors a and b, in radians.',
        signatures: [{ label: 'vec_angle_radians(a, b)', documentation: 'Angle between vectors (radians)', parameters: [{ label: 'a', documentation: 'First vector' }, { label: 'b', documentation: 'Second vector' }] }]
    },
    {
        label: 'vec_angle_degrees',
        detail: 'vec_angle_degrees(a: vec, b: vec) -> float64',
        documentation: '(libs/math/linear_algebra/vectors) Angle between vectors a and b, in degrees.',
        signatures: [{ label: 'vec_angle_degrees(a, b)', documentation: 'Angle between vectors (degrees)', parameters: [{ label: 'a', documentation: 'First vector' }, { label: 'b', documentation: 'Second vector' }] }]
    },
    {
        label: 'vec_normalize_safe',
        detail: 'vec_normalize_safe(v: vec, epsilon: float64) -> vec',
        documentation: '(libs/math/linear_algebra/vectors) Normalize vector v (divide by its norm). Returns v unchanged if norm ≤ epsilon, preventing division by zero.',
        signatures: [{ label: 'vec_normalize_safe(v, epsilon)', documentation: 'Safe unit vector', parameters: [{ label: 'v', documentation: 'Input vector' }, { label: 'epsilon', documentation: 'Minimum norm threshold' }] }]
    },
    {
        label: 'vec_cross_2d',
        detail: 'vec_cross_2d(a: vec, b: vec) -> float64',
        documentation: '(libs/math/linear_algebra/vectors) 2D cross product (scalar): a[0]*b[1] - a[1]*b[0].',
        signatures: [{ label: 'vec_cross_2d(a, b)', documentation: '2D cross product (scalar)', parameters: [{ label: 'a', documentation: '2D vector' }, { label: 'b', documentation: '2D vector' }] }]
    },
    {
        label: 'vec_cross_3d',
        detail: 'vec_cross_3d(a: vec, b: vec) -> vec',
        documentation: '(libs/math/linear_algebra/vectors) 3D cross product: returns a vector perpendicular to both a and b.',
        signatures: [{ label: 'vec_cross_3d(a, b)', documentation: '3D cross product', parameters: [{ label: 'a', documentation: '3D vector' }, { label: 'b', documentation: '3D vector' }] }]
    },
    {
        label: 'vec_reflect',
        detail: 'vec_reflect(v: vec, normal: vec) -> vec',
        documentation: '(libs/math/linear_algebra/vectors) Reflect vector v over the surface with given normal. Normalizes `normal` internally.',
        signatures: [{ label: 'vec_reflect(v, normal)', documentation: 'Reflection over normal', parameters: [{ label: 'v', documentation: 'Incident vector' }, { label: 'normal', documentation: 'Surface normal' }] }]
    },
    {
        label: 'vec_rotate_2d',
        detail: 'vec_rotate_2d(v: vec, angle: float64) -> vec',
        documentation: '(libs/math/linear_algebra/vectors) Rotate 2D vector by `angle` radians.',
        signatures: [{ label: 'vec_rotate_2d(v, angle)', documentation: '2D rotation', parameters: [{ label: 'v', documentation: '2D vector' }, { label: 'angle', documentation: 'Angle in radians' }] }]
    },
    {
        label: 'vec_hadamard',
        detail: 'vec_hadamard(a: vec, b: vec) -> vec',
        documentation: '(libs/math/linear_algebra/vectors) Element-wise (Hadamard) product of two vectors.',
        signatures: [{ label: 'vec_hadamard(a, b)', documentation: 'Element-wise multiplication', parameters: [{ label: 'a', documentation: 'First vector' }, { label: 'b', documentation: 'Second vector' }] }]
    },
    {
        label: 'vec_divide',
        detail: 'vec_divide(a: vec, b: vec) -> vec',
        documentation: '(libs/math/linear_algebra/vectors) Element-wise division using safe_divide.',
        signatures: [{ label: 'vec_divide(a, b)', documentation: 'Element-wise division', parameters: [{ label: 'a', documentation: 'Numerator vector' }, { label: 'b', documentation: 'Denominator vector' }] }]
    },
    {
        label: 'vec_clamp',
        detail: 'vec_clamp(v: vec, min_val: float64, max_val: float64) -> vec',
        documentation: '(libs/math/linear_algebra/vectors) Element-wise clamp of vector v to [min_val, max_val].',
        signatures: [{ label: 'vec_clamp(v, min_val, max_val)', documentation: 'Element-wise clamp', parameters: [{ label: 'v', documentation: 'Input vector' }, { label: 'min_val', documentation: 'Minimum value' }, { label: 'max_val', documentation: 'Maximum value' }] }]
    },

    // ====================================================================
    // Library functions (libs/math/linear_algebra/matrices.rho)
    // ====================================================================
    {
        label: 'mat_trace',
        detail: 'mat_trace(m: mat) -> float64',
        documentation: '(libs/math/linear_algebra/matrices) Sum of diagonal elements of matrix m.',
        signatures: [{ label: 'mat_trace(m)', documentation: 'Trace (sum of diagonal)', parameters: [{ label: 'm', documentation: 'Input matrix' }] }]
    },
    {
        label: 'mat_det_2x2',
        detail: 'mat_det_2x2(m: mat) -> float64',
        documentation: '(libs/math/linear_algebra/matrices) Determinant of a 2×2 matrix.',
        signatures: [{ label: 'mat_det_2x2(m)', documentation: '2×2 determinant', parameters: [{ label: 'm', documentation: '2×2 matrix' }] }]
    },
    {
        label: 'mat_det_3x3',
        detail: 'mat_det_3x3(m: mat) -> float64',
        documentation: '(libs/math/linear_algebra/matrices) Determinant of a 3×3 matrix using cofactor expansion.',
        signatures: [{ label: 'mat_det_3x3(m)', documentation: '3×3 determinant', parameters: [{ label: 'm', documentation: '3×3 matrix' }] }]
    },
    {
        label: 'mat_transpose',
        detail: 'mat_transpose(m: mat) -> mat',
        documentation: '(libs/math/linear_algebra/matrices) Transpose of matrix m. Equivalent to the built-in `transpose` function.',
        signatures: [{ label: 'mat_transpose(m)', documentation: 'Matrix transpose', parameters: [{ label: 'm', documentation: 'Input matrix' }] }]
    },
    {
        label: 'mat_hadamard',
        detail: 'mat_hadamard(a: mat, b: mat) -> mat',
        documentation: '(libs/math/linear_algebra/matrices) Element-wise (Hadamard) product of two matrices.',
        signatures: [{ label: 'mat_hadamard(a, b)', documentation: 'Element-wise multiplication', parameters: [{ label: 'a', documentation: 'First matrix' }, { label: 'b', documentation: 'Second matrix' }] }]
    },
    {
        label: 'mat_frobenius_norm',
        detail: 'mat_frobenius_norm(m: mat) -> float64',
        documentation: '(libs/math/linear_algebra/matrices) Frobenius norm: sqrt(sum of squares of all elements).',
        signatures: [{ label: 'mat_frobenius_norm(m)', documentation: 'Frobenius norm', parameters: [{ label: 'm', documentation: 'Input matrix' }] }]
    },
    {
        label: 'mat_is_symmetric',
        detail: 'mat_is_symmetric(m: mat, epsilon: float64) -> bool',
        documentation: '(libs/math/linear_algebra/matrices) True if m is square and m[i,j] ≈ m[j,i] within epsilon for all i,j.',
        signatures: [{ label: 'mat_is_symmetric(m, epsilon)', documentation: 'Symmetry check', parameters: [{ label: 'm', documentation: 'Input matrix' }, { label: 'epsilon', documentation: 'Tolerance' }] }]
    },
    {
        label: 'mat_outer_product',
        detail: 'mat_outer_product(u: vec, v: vec) -> mat',
        documentation: '(libs/math/linear_algebra/matrices) Outer product of vectors u and v: result[i,j] = u[i]*v[j]. Produces an m×n matrix.',
        signatures: [{ label: 'mat_outer_product(u, v)', documentation: 'Outer product', parameters: [{ label: 'u', documentation: 'Column vector (length m)' }, { label: 'v', documentation: 'Row vector (length n)' }] }]
    },
    {
        label: 'mat_row_means',
        detail: 'mat_row_means(m: mat) -> vec',
        documentation: '(libs/math/linear_algebra/matrices) Mean of each row; returns a vector of length rows(m).',
        signatures: [{ label: 'mat_row_means(m)', documentation: 'Per-row means', parameters: [{ label: 'm', documentation: 'Input matrix' }] }]
    },
    {
        label: 'mat_col_means',
        detail: 'mat_col_means(m: mat) -> vec',
        documentation: '(libs/math/linear_algebra/matrices) Mean of each column; returns a vector of length cols(m).',
        signatures: [{ label: 'mat_col_means(m)', documentation: 'Per-column means', parameters: [{ label: 'm', documentation: 'Input matrix' }] }]
    },
    {
        label: 'mat_diagonal',
        detail: 'mat_diagonal(m: mat) -> vec',
        documentation: '(libs/math/linear_algebra/matrices) Extract the main diagonal as a vector.',
        signatures: [{ label: 'mat_diagonal(m)', documentation: 'Extract diagonal', parameters: [{ label: 'm', documentation: 'Input matrix' }] }]
    },
    {
        label: 'mat_from_diagonal',
        detail: 'mat_from_diagonal(diag: vec) -> mat',
        documentation: '(libs/math/linear_algebra/matrices) Create a diagonal matrix from vector `diag`.',
        signatures: [{ label: 'mat_from_diagonal(diag)', documentation: 'Diagonal matrix', parameters: [{ label: 'diag', documentation: 'Diagonal values' }] }]
    },
    {
        label: 'mat_block',
        detail: 'mat_block(m: mat, start_row: int, start_col: int, block_rows: int, block_cols: int) -> mat',
        documentation: '(libs/math/linear_algebra/matrices) Extract a sub-block from matrix m starting at (start_row, start_col) with given dimensions.',
        signatures: [{ label: 'mat_block(m, start_row, start_col, block_rows, block_cols)', documentation: 'Extract sub-matrix block', parameters: [{ label: 'm', documentation: 'Source matrix' }, { label: 'start_row', documentation: 'Starting row index' }, { label: 'start_col', documentation: 'Starting column index' }, { label: 'block_rows', documentation: 'Number of rows to extract' }, { label: 'block_cols', documentation: 'Number of columns to extract' }] }]
    },
    {
        label: 'mat_clamp',
        detail: 'mat_clamp(m: mat, min_val: float64, max_val: float64) -> mat',
        documentation: '(libs/math/linear_algebra/matrices) Element-wise clamp of all matrix entries to [min_val, max_val].',
        signatures: [{ label: 'mat_clamp(m, min_val, max_val)', documentation: 'Element-wise clamp', parameters: [{ label: 'm', documentation: 'Input matrix' }, { label: 'min_val', documentation: 'Minimum value' }, { label: 'max_val', documentation: 'Maximum value' }] }]
    },

    // ====================================================================
    // Library functions (libs/math/statistics/descriptive.rho)
    // ====================================================================
    {
        label: 'variance',
        detail: 'variance(data: vec) -> float64',
        documentation: '(libs/math/statistics/descriptive) Population variance (divided by N).',
        signatures: [{ label: 'variance(data)', documentation: 'Population variance', parameters: [{ label: 'data', documentation: 'Data vector' }] }]
    },
    {
        label: 'std_dev',
        detail: 'std_dev(data: vec) -> float64',
        documentation: '(libs/math/statistics/descriptive) Population standard deviation.',
        signatures: [{ label: 'std_dev(data)', documentation: 'Population std deviation', parameters: [{ label: 'data', documentation: 'Data vector' }] }]
    },
    {
        label: 'sample_variance',
        detail: 'sample_variance(data: vec) -> float64',
        documentation: '(libs/math/statistics/descriptive) Sample variance (Bessel correction, divided by N-1).',
        signatures: [{ label: 'sample_variance(data)', documentation: 'Sample variance (N-1)', parameters: [{ label: 'data', documentation: 'Data vector' }] }]
    },
    {
        label: 'sample_std_dev',
        detail: 'sample_std_dev(data: vec) -> float64',
        documentation: '(libs/math/statistics/descriptive) Sample standard deviation (Bessel correction).',
        signatures: [{ label: 'sample_std_dev(data)', documentation: 'Sample std deviation (N-1)', parameters: [{ label: 'data', documentation: 'Data vector' }] }]
    },
    {
        label: 'median',
        detail: 'median(data: vec) -> float64',
        documentation: '(libs/math/statistics/descriptive) Median of a vector (sorts internally). Also in `stats.median` (built-in).',
        signatures: [{ label: 'median(data)', documentation: 'Median value', parameters: [{ label: 'data', documentation: 'Data vector' }] }]
    },
    {
        label: 'mode',
        detail: 'mode(data: vec) -> float64',
        documentation: '(libs/math/statistics/descriptive) Most frequent value in data (first occurrence for ties).',
        signatures: [{ label: 'mode(data)', documentation: 'Most frequent value', parameters: [{ label: 'data', documentation: 'Data vector' }] }]
    },
    {
        label: 'iqr',
        detail: 'iqr(data: vec) -> float64',
        documentation: '(libs/math/statistics/descriptive) Interquartile range: Q3 - Q1.',
        signatures: [{ label: 'iqr(data)', documentation: 'Q3 - Q1', parameters: [{ label: 'data', documentation: 'Data vector' }] }]
    },
    {
        label: 'coefficient_of_variation',
        detail: 'coefficient_of_variation(data: vec) -> float64',
        documentation: '(libs/math/statistics/descriptive) Coefficient of variation: std_dev / mean. Returns 0 if mean is 0.',
        signatures: [{ label: 'coefficient_of_variation(data)', documentation: 'CV = std / mean', parameters: [{ label: 'data', documentation: 'Data vector' }] }]
    },
    {
        label: 'quartiles',
        detail: 'quartiles(data: vec) -> vec',
        documentation: '(libs/math/statistics/descriptive) Returns [Q1, Q2, Q3] as a vector of length 3.',
        signatures: [{ label: 'quartiles(data)', documentation: '[Q1, Q2, Q3]', parameters: [{ label: 'data', documentation: 'Data vector' }] }]
    },
    {
        label: 'covariance',
        detail: 'covariance(x: vec, y: vec) -> float64',
        documentation: '(libs/math/statistics/descriptive) Population covariance between x and y (divided by N).',
        signatures: [{ label: 'covariance(x, y)', documentation: 'Population covariance', parameters: [{ label: 'x', documentation: 'First data vector' }, { label: 'y', documentation: 'Second data vector' }] }]
    },
    {
        label: 'correlation',
        detail: 'correlation(x: vec, y: vec) -> float64',
        documentation: '(libs/math/statistics/descriptive) Pearson correlation coefficient between x and y.',
        signatures: [{ label: 'correlation(x, y)', documentation: 'Pearson correlation', parameters: [{ label: 'x', documentation: 'First data vector' }, { label: 'y', documentation: 'Second data vector' }] }]
    },
    {
        label: 'z_score_normalize',
        detail: 'z_score_normalize(data: vec) -> vec',
        documentation: '(libs/math/statistics/descriptive) Z-score normalize: (data - mean) / std_dev. Returns zero vector if std_dev = 0.',
        signatures: [{ label: 'z_score_normalize(data)', documentation: 'Z-score normalization', parameters: [{ label: 'data', documentation: 'Data vector' }] }]
    },
    {
        label: 'min_max_normalize',
        detail: 'min_max_normalize(data: vec) -> vec',
        documentation: '(libs/math/statistics/descriptive) Scale data to [0, 1] using min-max normalization. Returns zero vector if range = 0.',
        signatures: [{ label: 'min_max_normalize(data)', documentation: 'Min-max scaling to [0, 1]', parameters: [{ label: 'data', documentation: 'Data vector' }] }]
    },
    {
        label: 'vec_sort',
        detail: 'vec_sort(data: vec) -> vec',
        documentation: '(libs/math/statistics/descriptive) Return a sorted copy of data vector (ascending, bubble sort).',
        signatures: [{ label: 'vec_sort(data)', documentation: 'Sorted copy of vector', parameters: [{ label: 'data', documentation: 'Data vector' }] }]
    },

    // ====================================================================
    // Library functions (libs/math/statistics/probability.rho)
    // ====================================================================
    {
        label: 'normal_pdf',
        detail: 'normal_pdf(x: float64, mean: float64, std_dev: float64) -> float64',
        documentation: '(libs/math/statistics/probability) Normal distribution PDF at x with given mean and standard deviation.',
        signatures: [{ label: 'normal_pdf(x, mean, std_dev)', documentation: 'Normal PDF', parameters: [{ label: 'x', documentation: 'Evaluation point' }, { label: 'mean', documentation: 'Distribution mean' }, { label: 'std_dev', documentation: 'Standard deviation' }] }]
    },
    {
        label: 'normal_cdf',
        detail: 'normal_cdf(x: float64, mean: float64, std_dev: float64) -> float64',
        documentation: '(libs/math/statistics/probability) Normal distribution CDF (cumulative probability up to x).',
        signatures: [{ label: 'normal_cdf(x, mean, std_dev)', documentation: 'Normal CDF', parameters: [{ label: 'x', documentation: 'Evaluation point' }, { label: 'mean', documentation: 'Distribution mean' }, { label: 'std_dev', documentation: 'Standard deviation' }] }]
    },
    {
        label: 'standard_normal_pdf',
        detail: 'standard_normal_pdf(x: float64) -> float64',
        documentation: '(libs/math/statistics/probability) Standard normal (μ=0, σ=1) PDF.',
        signatures: [{ label: 'standard_normal_pdf(x)', documentation: 'Standard normal PDF', parameters: [{ label: 'x', documentation: 'Evaluation point' }] }]
    },
    {
        label: 'standard_normal_cdf',
        detail: 'standard_normal_cdf(x: float64) -> float64',
        documentation: '(libs/math/statistics/probability) Standard normal (μ=0, σ=1) CDF.',
        signatures: [{ label: 'standard_normal_cdf(x)', documentation: 'Standard normal CDF', parameters: [{ label: 'x', documentation: 'Evaluation point' }] }]
    },
    {
        label: 'erf_approx',
        detail: 'erf_approx(x: float64) -> float64',
        documentation: '(libs/math/statistics/probability) Error function approximation using Horner polynomial evaluation.',
        signatures: [{ label: 'erf_approx(x)', documentation: 'Error function approximation', parameters: [{ label: 'x', documentation: 'Input value' }] }]
    },
    {
        label: 'poisson_pmf',
        detail: 'poisson_pmf(k: int, lam: float64) -> float64',
        documentation: '(libs/math/statistics/probability) Poisson PMF: probability of k events with rate λ.',
        signatures: [{ label: 'poisson_pmf(k, lam)', documentation: 'Poisson probability mass', parameters: [{ label: 'k', documentation: 'Number of events (int)' }, { label: 'lam', documentation: 'Rate λ' }] }]
    },
    {
        label: 'binomial_pmf',
        detail: 'binomial_pmf(k: int, n: int, p: float64) -> float64',
        documentation: '(libs/math/statistics/probability) Binomial PMF: probability of k successes in n trials with probability p.',
        signatures: [{ label: 'binomial_pmf(k, n, p)', documentation: 'Binomial probability mass', parameters: [{ label: 'k', documentation: 'Number of successes' }, { label: 'n', documentation: 'Number of trials' }, { label: 'p', documentation: 'Success probability' }] }]
    },
    {
        label: 'binomial_coefficient',
        detail: 'binomial_coefficient(n: int, k: int) -> float64',
        documentation: '(libs/math/statistics/probability) Binomial coefficient C(n, k) = n! / (k! * (n-k)!).',
        signatures: [{ label: 'binomial_coefficient(n, k)', documentation: 'n choose k', parameters: [{ label: 'n', documentation: 'Total items' }, { label: 'k', documentation: 'Items chosen' }] }]
    },
    {
        label: 'gamma_approx',
        detail: 'gamma_approx(x: float64) -> float64',
        documentation: '(libs/math/statistics/probability) Gamma function approximation using Lanczos method. Valid for x > 0.',
        signatures: [{ label: 'gamma_approx(x)', documentation: 'Gamma function (Lanczos)', parameters: [{ label: 'x', documentation: 'Positive real value' }] }]
    },
    {
        label: 'standard_normal_quantile',
        detail: 'standard_normal_quantile(p: float64) -> float64',
        documentation: '(libs/math/statistics/probability) Inverse CDF (quantile) of the standard normal distribution for probability p ∈ (0, 1).',
        signatures: [{ label: 'standard_normal_quantile(p)', documentation: 'Inverse normal CDF', parameters: [{ label: 'p', documentation: 'Probability in (0, 1)' }] }]
    },

    // ====================================================================
    // Library functions (libs/math/statistics/regression.rho)
    // ====================================================================
    {
        label: 'linear_regression',
        detail: 'linear_regression(x: vec, y: vec) -> (float64, float64)',
        documentation: '(libs/math/statistics/regression) Simple linear regression y = a + b*x. Returns tuple (intercept a, slope b).',
        signatures: [{ label: 'linear_regression(x, y)', documentation: 'Simple linear regression (a, b)', parameters: [{ label: 'x', documentation: 'Predictor vector' }, { label: 'y', documentation: 'Response vector' }] }]
    },
    {
        label: 'linear_regression_predict',
        detail: 'linear_regression_predict(x: vec, y: vec, x_new: float64) -> float64',
        documentation: '(libs/math/statistics/regression) Predict y at x_new using simple linear regression fitted on (x, y).',
        signatures: [{ label: 'linear_regression_predict(x, y, x_new)', documentation: 'Predict with linear regression', parameters: [{ label: 'x', documentation: 'Training predictor vector' }, { label: 'y', documentation: 'Training response vector' }, { label: 'x_new', documentation: 'New x value to predict' }] }]
    },
    {
        label: 'r_squared',
        detail: 'r_squared(x: vec, y: vec) -> float64',
        documentation: '(libs/math/statistics/regression) Coefficient of determination R² for a linear regression on (x, y). Values close to 1 indicate a good fit.',
        signatures: [{ label: 'r_squared(x, y)', documentation: 'R² goodness of fit', parameters: [{ label: 'x', documentation: 'Predictor vector' }, { label: 'y', documentation: 'Response vector' }] }]
    },
    {
        label: 'mse',
        detail: 'mse(y_true: vec, y_pred: vec) -> float64',
        documentation: '(libs/math/statistics/regression) Mean Squared Error between true and predicted values.',
        signatures: [{ label: 'mse(y_true, y_pred)', documentation: 'Mean Squared Error', parameters: [{ label: 'y_true', documentation: 'True values' }, { label: 'y_pred', documentation: 'Predicted values' }] }]
    },
    {
        label: 'rmse',
        detail: 'rmse(y_true: vec, y_pred: vec) -> float64',
        documentation: '(libs/math/statistics/regression) Root Mean Squared Error: sqrt(MSE).',
        signatures: [{ label: 'rmse(y_true, y_pred)', documentation: 'Root Mean Squared Error', parameters: [{ label: 'y_true', documentation: 'True values' }, { label: 'y_pred', documentation: 'Predicted values' }] }]
    },
    {
        label: 'mae',
        detail: 'mae(y_true: vec, y_pred: vec) -> float64',
        documentation: '(libs/math/statistics/regression) Mean Absolute Error.',
        signatures: [{ label: 'mae(y_true, y_pred)', documentation: 'Mean Absolute Error', parameters: [{ label: 'y_true', documentation: 'True values' }, { label: 'y_pred', documentation: 'Predicted values' }] }]
    },
    {
        label: 'logistic_function',
        detail: 'logistic_function(z: float64) -> float64',
        documentation: '(libs/math/statistics/regression) Sigmoid function: 1 / (1 + exp(-z)). Output in (0, 1).',
        signatures: [{ label: 'logistic_function(z)', documentation: 'Sigmoid / logistic activation', parameters: [{ label: 'z', documentation: 'Linear combination' }] }]
    },
    {
        label: 'logistic_predict',
        detail: 'logistic_predict(coefficients: vec, x: vec) -> float64',
        documentation: '(libs/math/statistics/regression) Logistic regression prediction: sigmoid(dot(coefficients, x)). Returns probability in (0, 1).',
        signatures: [{ label: 'logistic_predict(coefficients, x)', documentation: 'Logistic regression prediction', parameters: [{ label: 'coefficients', documentation: 'Model coefficients' }, { label: 'x', documentation: 'Feature vector' }] }]
    }
];

// ============================================================
// Module descriptors — functions callable with module.func() syntax
// Used for autocomplete when user types "module." and hover docs
// ============================================================
const moduleDescriptors = {
    math: {
        constants: [
            { label: 'PI',     detail: 'float64',  documentation: 'π = 3.141592653589793' },
            { label: 'E',      detail: 'float64',  documentation: 'e = 2.718281828459045' },
            { label: 'PHI',    detail: 'float64',  documentation: 'Golden ratio φ = 1.618033988749895' },
            { label: 'SQRT_2', detail: 'float64',  documentation: '√2 = 1.414213562373095' },
            { label: 'SQRT_3', detail: 'float64',  documentation: '√3 = 1.732050807568877' }
        ],
        functions: [
            { label: 'norm',      detail: 'math.norm(v: vec|mat) -> float64',          documentation: 'Euclidean or Frobenius norm' },
            { label: 'dot',       detail: 'math.dot(a: vec, b: vec) -> float64',       documentation: 'Dot product of two vectors' },
            { label: 'transpose', detail: 'math.transpose(m: mat|vec) -> mat',         documentation: 'Matrix/vector transpose' },
            { label: 'inv',       detail: 'math.inv(m: mat) -> mat',                   documentation: 'Matrix inverse (full-pivot LU)' },
            { label: 'sum',       detail: 'math.sum(v: vec|mat) -> float64',           documentation: 'Sum of all elements' },
            { label: 'mean',      detail: 'math.mean(v: vec|mat) -> float64',          documentation: 'Arithmetic mean of all elements' },
            { label: 'zeros',     detail: 'math.zeros(n) -> vec | math.zeros(n, m) -> mat', documentation: 'Zero vector or matrix' },
            { label: 'ones',      detail: 'math.ones(n) -> vec | math.ones(n, m) -> mat',   documentation: 'Vector or matrix of ones' },
            { label: 'eye',       detail: 'math.eye(n: int) -> mat',                   documentation: 'n×n identity matrix' },
            { label: 'sqrt',      detail: 'math.sqrt(x) -> float64|vec|mat',           documentation: 'Square root (scalar or element-wise)' },
            { label: 'exp',       detail: 'math.exp(x) -> float64|vec|mat',            documentation: 'e^x (scalar or element-wise)' },
            { label: 'log',       detail: 'math.log(x) -> float64|vec|mat',            documentation: 'Natural log (scalar or element-wise)' },
            { label: 'abs',       detail: 'math.abs(x) -> float64|vec|mat',            documentation: 'Absolute value (scalar or element-wise)' },
            { label: 'pow',       detail: 'math.pow(base, exp: float64) -> float64',   documentation: 'base^exp' },
            { label: 'cbrt',      detail: 'math.cbrt(x: float64) -> float64',          documentation: 'Cube root' },
            { label: 'sin',       detail: 'math.sin(x: float64) -> float64',           documentation: 'Sine (radians)' },
            { label: 'cos',       detail: 'math.cos(x: float64) -> float64',           documentation: 'Cosine (radians)' },
            { label: 'tan',       detail: 'math.tan(x: float64) -> float64',           documentation: 'Tangent (radians)' },
            { label: 'asin',      detail: 'math.asin(x: float64) -> float64',          documentation: 'Arcsine (radians)' },
            { label: 'acos',      detail: 'math.acos(x: float64) -> float64',          documentation: 'Arccosine (radians)' },
            { label: 'atan',      detail: 'math.atan(x: float64) -> float64',          documentation: 'Arctangent (radians)' },
            { label: 'atan2',     detail: 'math.atan2(y, x: float64) -> float64',      documentation: '2-argument arctangent' },
            { label: 'sinh',      detail: 'math.sinh(x: float64) -> float64',          documentation: 'Hyperbolic sine' },
            { label: 'cosh',      detail: 'math.cosh(x: float64) -> float64',          documentation: 'Hyperbolic cosine' },
            { label: 'tanh',      detail: 'math.tanh(x: float64) -> float64',          documentation: 'Hyperbolic tangent' },
            { label: 'floor',     detail: 'math.floor(x: float64) -> float64',         documentation: 'Round down' },
            { label: 'ceil',      detail: 'math.ceil(x: float64) -> float64',          documentation: 'Round up' },
            { label: 'round',     detail: 'math.round(x: float64) -> float64',         documentation: 'Round to nearest integer' },
            { label: 'trunc',     detail: 'math.trunc(x: float64) -> float64',         documentation: 'Truncate toward zero' },
            { label: 'min',       detail: 'math.min(v: vec) -> float64 | math.min(a, b, ...) -> float64', documentation: 'Minimum element or minimum of scalars' },
            { label: 'max',       detail: 'math.max(v: vec) -> float64 | math.max(a, b, ...) -> float64', documentation: 'Maximum element or maximum of scalars' },
            { label: 'clamp',     detail: 'math.clamp(value, min, max: float64) -> float64', documentation: 'Clamp value to [min, max]' },
            { label: 'sign',      detail: 'math.sign(x: float64) -> float64',          documentation: 'Sign of x: -1, 0, or 1' },
            { label: 'lerp',      detail: 'math.lerp(a, b, t: float64) -> float64',    documentation: 'Linear interpolation' },
            { label: 'deg2rad',   detail: 'math.deg2rad(deg: float64) -> float64',     documentation: 'Degrees to radians' },
            { label: 'rad2deg',   detail: 'math.rad2deg(rad: float64) -> float64',     documentation: 'Radians to degrees' },
            { label: 'factorial', detail: 'math.factorial(n: int) -> int',             documentation: 'n! for 0 ≤ n ≤ 20' },
            { label: 'rows',      detail: 'math.rows(m: mat) -> int',                  documentation: 'Number of rows' },
            { label: 'cols',      detail: 'math.cols(m: mat) -> int',                  documentation: 'Number of columns' },
            { label: 'size',      detail: 'math.size(v: vec|mat) -> int',              documentation: 'Total number of elements' }
        ]
    },

    stats: {
        functions: [
            { label: 'std',       detail: 'stats.std(v: vec|mat) -> float64',          documentation: 'Sample standard deviation (N-1 correction). Requires ≥ 2 elements.' },
            { label: 'var',       detail: 'stats.var(v: vec|mat) -> float64',          documentation: 'Sample variance (N-1 correction). Requires ≥ 2 elements.' },
            { label: 'cov',       detail: 'stats.cov(a: vec, b: vec) -> float64',      documentation: 'Sample covariance between two equal-length vectors (N-1 correction).' },
            { label: 'mean',      detail: 'stats.mean(v: vec|mat) -> float64',         documentation: 'Arithmetic mean of all elements.' },
            { label: 'median',    detail: 'stats.median(v: vec) -> float64',           documentation: 'Median value (sorts internally).' },
            { label: 'percentile',detail: 'stats.percentile(v: vec, p: float64) -> float64', documentation: 'p-th percentile (p in [0, 100]) using linear interpolation.' },
            { label: 'min',       detail: 'stats.min(v: vec) -> float64',              documentation: 'Minimum element of vector.' },
            { label: 'max',       detail: 'stats.max(v: vec) -> float64',              documentation: 'Maximum element of vector.' },
            { label: 'corr',      detail: 'stats.corr(a: vec, b: vec) -> float64',     documentation: 'Pearson correlation coefficient. Returns 0 if either vector has zero variance.' },
            { label: 'skewness',  detail: 'stats.skewness(v: vec) -> float64',         documentation: 'Sample skewness (Fisher). Requires ≥ 3 elements.' },
            { label: 'kurtosis',  detail: 'stats.kurtosis(v: vec) -> float64',         documentation: 'Excess kurtosis (Fisher, -3 from normal). Requires ≥ 4 elements.' },
            { label: 'zscore',    detail: 'stats.zscore(v: vec) -> vec',               documentation: 'Z-score standardize vector: (v - mean) / std. Returns zeros if std = 0.' }
        ]
    },

    numerical: {
        functions: [
            { label: 'solve_quadratic', detail: 'numerical.solve_quadratic(a, b, c: float64) -> vec', documentation: 'Real roots of ax²+bx+c=0. Returns empty vec for no real roots, length-1 for double root, length-2 for two roots.' },
            { label: 'linspace',   detail: 'numerical.linspace(start, end: float64, n: int) -> vec',  documentation: 'n evenly spaced points from start to end inclusive. Requires n ≥ 2.' },
            { label: 'logspace',   detail: 'numerical.logspace(start, end: float64, n: int) -> vec',  documentation: 'n logarithmically spaced points from 10^start to 10^end. Requires n ≥ 2.' },
            { label: 'cumsum',     detail: 'numerical.cumsum(v: vec) -> vec',                          documentation: 'Cumulative sum of vector elements.' },
            { label: 'cumprod',    detail: 'numerical.cumprod(v: vec) -> vec',                         documentation: 'Cumulative product of vector elements.' },
            { label: 'diff',       detail: 'numerical.diff(v: vec) -> vec',                            documentation: 'First-order finite differences: result[i] = v[i+1] - v[i]. Output length = input length - 1.' },
            { label: 'gradient',   detail: 'numerical.gradient(v: vec, h: float64) -> vec',            documentation: 'Numerical gradient using central differences (forward/backward at endpoints). h is the step size.' },
            { label: 'trapz',      detail: 'numerical.trapz(y: vec, x: vec) -> float64',               documentation: 'Trapezoidal numerical integration of y over x. x and y must have the same length ≥ 2.' },
            { label: 'simps',      detail: 'numerical.simps(y: vec, x: vec) -> float64',               documentation: "Simpson's rule integration of y over x. Requires ≥ 3 points." },
            { label: 'interp1d',   detail: 'numerical.interp1d(x: vec, y: vec, x_new: float64) -> float64', documentation: 'Linear interpolation at x_new given data (x, y). Extrapolates using nearest endpoint.' },
            { label: 'polyval',    detail: 'numerical.polyval(coeffs: vec, x: float64) -> float64',    documentation: 'Evaluate polynomial at x. coeffs = [a0, a1, a2, ...] for a0 + a1*x + a2*x² + ...' },
            { label: 'polyfit',    detail: 'numerical.polyfit(x: vec, y: vec, degree: int) -> vec',    documentation: 'Least-squares polynomial fit. Returns coefficients [a0, a1, ..., a_degree]. Use with polyval.' }
        ]
    },

    vector: {
        functions: [
            { label: 'append',  detail: 'vector.append(v: vec, value: float64) -> vec',              documentation: 'New vector with value appended at the end.' },
            { label: 'remove',  detail: 'vector.remove(v: vec, index: int) -> vec',                  documentation: 'New vector without the element at index.' },
            { label: 'reverse', detail: 'vector.reverse(v: vec) -> vec',                             documentation: 'Reversed copy of vector.' },
            { label: 'insert',  detail: 'vector.insert(v: vec, index: int, value: float64) -> vec',  documentation: 'New vector with value inserted at index.' },
            { label: 'size',    detail: 'vector.size(v: vec) -> int',                                documentation: 'Number of elements in vector.' }
        ]
    },

    matrix: {
        functions: [
            { label: 'append_row', detail: 'matrix.append_row(m: mat, row: vec) -> mat',  documentation: 'New matrix with row vector appended as last row.' },
            { label: 'append_col', detail: 'matrix.append_col(m: mat, col: vec) -> mat',  documentation: 'New matrix with column vector appended as last column.' },
            { label: 'remove_row', detail: 'matrix.remove_row(m: mat, idx: int) -> mat',  documentation: 'New matrix without row at index.' },
            { label: 'remove_col', detail: 'matrix.remove_col(m: mat, idx: int) -> mat',  documentation: 'New matrix without column at index.' },
            { label: 'rows',       detail: 'matrix.rows(m: mat) -> int',                  documentation: 'Number of rows.' },
            { label: 'cols',       detail: 'matrix.cols(m: mat) -> int',                  documentation: 'Number of columns.' },
            { label: 'size',       detail: 'matrix.size(m: mat) -> int',                  documentation: 'Total number of elements.' }
        ]
    },

    string: {
        functions: [
            { label: 'length',     detail: 'string.length(s: string) -> int',                          documentation: 'Number of characters in string.' },
            { label: 'lower',      detail: 'string.lower(s: string) -> string',                        documentation: 'Convert string to lowercase.' },
            { label: 'upper',      detail: 'string.upper(s: string) -> string',                        documentation: 'Convert string to uppercase.' },
            { label: 'substr',     detail: 'string.substr(s: string, start: int, length: int) -> string', documentation: 'Extract substring starting at `start` with given `length`.' },
            { label: 'find',       detail: 'string.find(s: string, sub: string) -> int',               documentation: 'First index of `sub` in `s`, or -1 if not found.' },
            { label: 'replace',    detail: 'string.replace(s: string, old: string, new: string) -> string', documentation: 'Replace all occurrences of `old` with `new` in `s`.' },
            { label: 'trim',       detail: 'string.trim(s: string) -> string',                         documentation: 'Remove leading and trailing whitespace.' },
            { label: 'startswith', detail: 'string.startswith(s: string, prefix: string) -> bool',     documentation: 'True if `s` starts with `prefix`.' },
            { label: 'endswith',   detail: 'string.endswith(s: string, suffix: string) -> bool',       documentation: 'True if `s` ends with `suffix`.' },
            { label: 'contains',   detail: 'string.contains(s: string, sub: string) -> bool',          documentation: 'True if `sub` appears anywhere in `s`.' },
            { label: 'split',      detail: 'string.split(s: string, delimiter: string) -> vec',        documentation: 'Split `s` by `delimiter` into a vector of substrings (stored as additional values).' },
            { label: 'join',       detail: 'string.join(sep: string, v: vec) -> string',               documentation: 'Join elements of `v` into a string using `sep` as separator.' },
            { label: 'reverse',    detail: 'string.reverse(s: string) -> string',                      documentation: 'Reverse the characters in string `s`.' },
            { label: 'repeat',     detail: 'string.repeat(s: string, n: int) -> string',               documentation: 'Repeat string `s` exactly `n` times.' },
            { label: 'at',         detail: 'string.at(s: string, index: int) -> string',               documentation: 'Character at position `index` (0-based) as a single-char string.' },
            { label: 'concat',     detail: 'string.concat(a: string, b: string) -> string',            documentation: 'Concatenate two strings. Equivalent to `a + b`.' },
            { label: 'isempty',    detail: 'string.isempty(s: string) -> bool',                        documentation: 'True if string has length 0.' },
            { label: 'count',      detail: 'string.count(s: string, sub: string) -> int',              documentation: 'Number of non-overlapping occurrences of `sub` in `s`.' },
            { label: 'to_int',     detail: 'string.to_int(s: string) -> int',                          documentation: 'Parse string as integer. Throws on invalid format.' },
            { label: 'to_float',   detail: 'string.to_float(s: string) -> float64',                   documentation: 'Parse string as float64. Throws on invalid format.' },
            { label: 'slice',      detail: 'string.slice(s: string, start: int, end: int) -> string',  documentation: 'Extract characters from `start` to `end-1` (exclusive).' }
        ]
    },

    mapping: {
        functions: [
            { label: 'create',  detail: 'mapping.create() -> mapping',                                       documentation: 'Create an empty mapping (dictionary). Keys are strings.' },
            { label: 'set',     detail: 'mapping.set(m: mapping, key: string, value) -> void',               documentation: 'Set `key` to `value` in mapping. Overwrites if key exists. Alias: `mapping.put`.' },
            { label: 'get',     detail: 'mapping.get(m: mapping, key: string) -> value',                     documentation: 'Get the value for `key`. Throws if key not found.' },
            { label: 'has',     detail: 'mapping.has(m: mapping, key: string) -> bool',                      documentation: 'True if `key` exists in mapping.' },
            { label: 'delete',  detail: 'mapping.delete(m: mapping, key: string) -> void',                   documentation: 'Remove `key` from mapping. Alias: `mapping.remove`.' },
            { label: 'size',    detail: 'mapping.size(m: mapping) -> int',                                   documentation: 'Number of key-value pairs in mapping.' },
            { label: 'isempty', detail: 'mapping.isempty(m: mapping) -> bool',                               documentation: 'True if mapping has no entries.' },
            { label: 'clear',   detail: 'mapping.clear(m: mapping) -> void',                                 documentation: 'Remove all entries from mapping.' },
            { label: 'keys',    detail: 'mapping.keys(m: mapping) -> vec',                                   documentation: 'Return all keys as a vector of strings.' },
            { label: 'values',  detail: 'mapping.values(m: mapping) -> vec',                                 documentation: 'Return all values as a vector.' },
            { label: 'update',  detail: 'mapping.update(target: mapping, source: mapping) -> void',          documentation: 'Merge all entries from `source` into `target`, overwriting duplicates.' },
            { label: 'copy',    detail: 'mapping.copy(m: mapping) -> mapping',                               documentation: 'Shallow copy of the mapping.' },
            { label: 'put',     detail: 'mapping.put(m: mapping, key: string, value) -> void',               documentation: 'Alias for `mapping.set`.' },
            { label: 'remove',  detail: 'mapping.remove(m: mapping, key: string) -> void',                   documentation: 'Alias for `mapping.delete`.' }
        ]
    },

    array: {
        functions: [
            { label: 'create',  detail: 'array.create(size: int, value) -> array',     documentation: 'Create an array of given size filled with `value`.' },
            { label: 'empty',   detail: 'array.empty() -> array',                      documentation: 'Create an empty dynamic array.' },
            { label: 'push',    detail: 'array.push(a: array, value) -> void',         documentation: 'Append `value` to the end of the array (in-place).' },
            { label: 'pop',     detail: 'array.pop(a: array) -> value',                documentation: 'Remove and return the last element of the array.' },
            { label: 'size',    detail: 'array.size(a: array) -> int',                 documentation: 'Number of elements in the array.' },
            { label: 'isempty', detail: 'array.isempty(a: array) -> bool',             documentation: 'True if array has no elements.' },
            { label: 'clear',   detail: 'array.clear(a: array) -> void',               documentation: 'Remove all elements from the array.' },
            { label: 'resize',  detail: 'array.resize(a: array, size: int) -> void',   documentation: 'Resize array to given size. New elements are zero-initialized.' }
        ]
    },

    io: {
        functions: [
            { label: 'input',    detail: 'io.input(prompt: string) -> string',                       documentation: 'Read a line from stdin after displaying `prompt`. Returns the line without the trailing newline.' },
            { label: 'open',     detail: 'io.open(filename: string, mode: string) -> handle',        documentation: 'Open a file and return a file handle. Mode: "r" (read), "w" (write), "a" (append), "rw" (read/write).' },
            { label: 'read',     detail: 'io.read(handle) -> string',                                documentation: 'Read the entire content of an open file as a string.' },
            { label: 'write',    detail: 'io.write(handle, content: string) -> void',                documentation: 'Write a string to an open file.' },
            { label: 'readline', detail: 'io.readline(handle) -> string',                            documentation: 'Read one line from an open file. Returns empty string at EOF.' },
            { label: 'seek',     detail: 'io.seek(handle, offset: int, origin: int) -> void',        documentation: 'Seek to position in file. origin: 0=start, 1=current, 2=end.' },
            { label: 'tell',     detail: 'io.tell(handle) -> int',                                   documentation: 'Return current file position (byte offset from start).' },
            { label: 'close',    detail: 'io.close(handle) -> void',                                 documentation: 'Close an open file handle and flush any pending writes.' },
            { label: 'flush',    detail: 'io.flush(handle) -> void',                                 documentation: 'Flush write buffer to disk without closing the file.' },
            { label: 'exists',   detail: 'io.exists(filename: string) -> bool',                      documentation: 'True if the file at `filename` exists on disk.' },
            { label: 'remove',   detail: 'io.remove(filename: string) -> void',                      documentation: 'Delete the file at `filename`. Throws if the file does not exist.' },
            { label: 'lines',    detail: 'io.lines(filename: string) -> vec',                        documentation: 'Read all lines of a file into a vector of strings (without newlines).' },
            { label: 'read_csv', detail: 'io.read_csv(filename: string) -> mat',                     documentation: 'Parse a CSV file of numeric values into a matrix. Rows = lines, columns = comma-separated values.' },
            { label: 'stdin',    detail: 'io.stdin() -> string',                                     documentation: 'Read one line from standard input (no prompt). Returns the line without newline.' }
        ]
    },

    datetime: {
        functions: [
            { label: 'now',            detail: 'datetime.now() -> datetime',                                    documentation: 'Current local date and time.' },
            { label: 'today',          detail: 'datetime.today() -> datetime',                                  documentation: 'Current local date (time set to midnight).' },
            { label: 'current_time',   detail: 'datetime.current_time() -> datetime',                           documentation: 'Current local time as a datetime value.' },
            { label: 'make',           detail: 'datetime.make(year, month, day, hour, minute, second: int) -> datetime', documentation: 'Create a datetime from components (local time).' },
            { label: 'make_date',      detail: 'datetime.make_date(year, month, day: int) -> datetime',         documentation: 'Create a date-only datetime (time = midnight).' },
            { label: 'make_time',      detail: 'datetime.make_time(hour, minute, second: int) -> datetime',     documentation: 'Create a time-only datetime (date = epoch).' },
            { label: 'timestamp',      detail: 'datetime.timestamp(dt: datetime) -> float64',                   documentation: 'Convert datetime to Unix timestamp (seconds since 1970-01-01 UTC).' },
            { label: 'from_timestamp', detail: 'datetime.from_timestamp(ts: float64) -> datetime',              documentation: 'Convert Unix timestamp to local datetime.' },
            { label: 'format',         detail: 'datetime.format(dt: datetime, fmt: string) -> string',          documentation: 'Format datetime using strftime-style format string (e.g., "%Y-%m-%d %H:%M:%S").' },
            { label: 'diff_seconds',   detail: 'datetime.diff_seconds(a: datetime, b: datetime) -> float64',    documentation: 'Signed difference in seconds: a - b.' },
            { label: 'diff_days',      detail: 'datetime.diff_days(a: datetime, b: datetime) -> float64',       documentation: 'Signed difference in days: a - b (may be fractional).' }
        ]
    }
};

const { buildModuleDescriptors, buildBuiltinFunctions } = require('./doxygenParser');

module.exports = {
    builtinFunctions: buildBuiltinFunctions(builtinFunctions),
    moduleDescriptors: buildModuleDescriptors(moduleDescriptors)
};
