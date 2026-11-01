<!--
   ****************************************************************************
    MobilityDB / MEOS Specification — Finite Values
    Copyright(c) MobilityDB Contributors

    This documentation is licensed under a Creative Commons Attribution-Share
    Alike 3.0 License: https://creativecommons.org/licenses/by-sa/3.0/
   ****************************************************************************
-->

# Finite Values — Value Domain Specification

| | |
|---|---|
| Version | v1.0.0 |
| Scope | The numbers a MEOS or MobilityDB value holds, and the answer to an operation whose result exceeds them |
| Reference | `meos/include/temporal/temporal.h` (`ensure_finite`, `ensure_finite_result`), `meos/src/geo/geo_funcs.c` (`ensure_finite_geo`) |

## 1. Scope and intent

Every number held by a MEOS value is a finite double. A NaN, `+Infinity` and
`-Infinity` are refused wherever a number enters MEOS, and an operation whose
result over finite values exceeds the largest double raises an error. This
document states that rule, what it covers, the errors it raises, and how to
write the conditions an infinity would otherwise stand for. It is prescriptive:
every input and every operation of MEOS, and therefore of MobilityDB and of
every binding generated from MEOS, follows it.

The rule exists because a value without a position among the numbers cannot
be ordered, bounded or indexed. A NaN is neither less than, equal to nor
greater than any number, and an infinite bound or coordinate makes the
arithmetic of a span, a box or an interpolation answer a NaN or a wrong
value. Holding finite numbers only, every comparison is a total order, every
box bounds its value, and every geometric answer is computed exactly about
the doubles the value holds.

## 2. Notation

`DBL_MAX` is the largest finite double, `1.7976931348623157e+308`. An *input*
is any way a number enters MEOS: a text, WKB, hex-WKB or MF-JSON reader, a
constructor taking a C double, or a conversion from another type.

## 3. Inputs

### 3.1 The rule

An input holding a NaN or an infinity is refused, and no value is built.

| value | numbers tested |
|---|---|
| float sets, float spans and span sets | every value and every bound |
| `tbox` | the bounds of its value span |
| `stbox`, `box3d` | every coordinate bound |
| temporal floats | every value |
| geometries and geographies, the points of temporal points and geometries | every coordinate, X, Y, Z and M |
| circular buffers | the centre and the radius |
| poses and pose chains | the position and the orientation |
| network points | the position on the route |
| a C double argument of any public function | the argument itself |

The timestamp inputs refuse `infinity` and `-infinity`: a `tstzspan`, a
`tstzset` and a temporal value hold finite timestamps only.

### 3.2 The error

In MEOS the error is raised through the error handler with the code
`MEOS_ERR_INVALID_ARG_VALUE` (12), and the function returns its documented
error value (`NULL` for a pointer). The messages are:

| input | message |
|---|---|
| a number | `The value must be finite` |
| a geometry | `The coordinates of a geometry must be finite` |

In PostgreSQL the same messages are raised as an `ERROR`.

```sql
SELECT floatspan '[0, Infinity)';
-- ERROR:  The value must be finite
SELECT tfloat 'NaN@2001-01-01';
-- ERROR:  The value must be finite
SELECT tgeompoint 'POINT(1e400 1)@2001-01-01';
-- ERROR:  The coordinates of a geometry must be finite
```

`1e400` is read as `+Infinity`, since it exceeds `DBL_MAX`.

### 3.3 Rasters

A raster pixel holding a NaN or an infinity carries no value, as a pixel
equal to the nodata value of its band does. The temporal value read from a
raster along a trajectory therefore has no instant where the trajectory
crosses such a pixel.

## 4. Results

### 4.1 The rule

An operation over finite values whose exact result exceeds `DBL_MAX` in
magnitude raises an error, as the `float8` arithmetic of PostgreSQL does. It
never answers an infinity, a NaN, `DBL_MAX` or any other stand-in.

### 4.2 The error

The code is `MEOS_ERR_VALUE_OUT_OF_RANGE` (3), and the message
`value out of range: overflow`. The operations that can leave the doubles
include the arithmetic of temporal floats, the sum of temporal floats, the
delta, the integral and the exponential of a temporal float, the distance of
two float spans, the distance of two circular buffers or geometries whose
coordinates are near `±DBL_MAX`, the area, the volume and the perimeter of a
box or a geometry, the expansion of a float box, a value tile, and the number
of bins of a float span:

```sql
SELECT floatspan '[-1e308, -1e308]' <-> floatspan '[1e308, 1e308]';
-- ERROR:  value out of range: overflow
SELECT deltaValue(tfloat '[-1e308@2001-01-01, 1e308@2001-01-02]');
-- ERROR:  value out of range: overflow
SELECT integral(tfloat '[1e308@2001-01-01, 1e308@2001-01-02]');
-- ERROR:  value out of range: overflow
SELECT bins(floatspan '[-1e308, 1e308]', 1.0);
-- ERROR:  value out of range: too many bins
SELECT area(stbox 'STBOX X((-1e308,-1e308),(1e308,1e308))');
-- ERROR:  value out of range: overflow
```

A result that is a finite double is answered, however large its
intermediate values: the bins of `[-1e308, 1e308]` of size `1e305` are 2002
spans, a circle whose diameter spans `[-1e308, 1e308]` is the circular buffer
of centre `(0 0)` and radius `1e308`, the time-weighted average of
`[1e308@2001-01-01, 1e308@2001-01-02]` is `1e308`, and the integral of
`{[1e308@2001-01-01, 1e308@2001-01-02], [-1e308@2001-01-03, -1e308@2001-01-04]}`
is 0. The value of `[1e308@2001-01-01, -1e308@2001-01-03]` at `2001-01-02` is
0, and the temporal average (`tavg`, `wavg`) and the temporal centroid
(`tcentroid`) of values near `±DBL_MAX` are answered as well: the `tavg` of
two temporal floats valued `1e308` is `1e308`. The rounding of `DBL_MAX` is
`DBL_MAX`, and the area of a box whose extent exceeds `DBL_MAX` while its area
does not is answered:

```sql
SELECT round(1.7976931348623157e308::float, 15);
-- 1.7976931348623157e+308
SELECT area(stbox 'STBOX X((-1e308,0),(1e308,1e-10))');
-- 2.0000000000000002e+298
```

The length and the perimeter of a geometry are answered whatever the
magnitudes of its coordinates, from the subnormal doubles to `±DBL_MAX`, and
raise `value out of range` only where they exceed `DBL_MAX`. The area of a
geometry is read scaled by a power of two where its coordinates lie outside
`2^-500` to `2^500`; `geom_area` raises `MEOS_ERR_FEATURE_NOT_SUPPORTED` where
that scaling is not exact, as for a geometry mixing coordinates near
`DBL_MAX`, scaled by `2^-523`, with coordinates so small that this scaling
rounds them:

```sql
SELECT perimeter(stbox 'STBOX X((0,0),(5e-324,1e-323))') = 3e-323;
-- t
```

### 4.3 A division by zero

A division by zero raises `Division by zero`, with the code
`MEOS_ERR_DIVISION_BY_ZERO`, as the division of temporal numbers and the
`float8` division of PostgreSQL raise it. The density of a point cloud patch
whose XY bounds have no area, a single point or points on a line, is one:

```sql
SELECT tdensity(v) FROM (VALUES (tpcpatch(pcpatch(pcpoint(1, 1, 1, 1)),
  '2001-01-01'::timestamptz))) t(v);
-- ERROR:  Division by zero
```

A query over patches that may have no area reads those whose box has one,
`area(stbox(tpcbox(patch))) > 0`.

## 5. Writing an unbounded condition

An unbounded float span is not a value. The query states the condition on
the values instead, or a finite bound:

| intent | written as |
|---|---|
| the instants where a temporal float is at least 0 | `atSpan(temp, floatspan '[0, 1.7976931348623157e+308]')`, or the comparison `temp #>= 0.0` |
| a distance with no limit | the distance function itself, with no threshold |
| a missing measurement | the absence of an instant, or SQL `NULL`, never a NaN |

## 6. Versioning and stability

The rule is part of the value domain of every MEOS type. A later version may
widen what an input accepts only by a new type, never by admitting a
non-finite number into an existing one.

## 7. Out of scope

The internal search keys of the indexes, such as an unbounded box at the root
of a tree or the "no distance yet" key of a nearest-neighbour search, are
never MEOS values and cross no public function. The error value a function
documents for a failed call is not a value either. The types and operators of
PostgreSQL itself keep the semantics of PostgreSQL inside MobilityDB: the SQL
product of an `interval` and a `float` is the PostgreSQL operator, which
answers an infinite interval for an infinite factor, while the MEOS functions
of intervals refuse one.

## 8. References

- IEEE 754-2019, the binary64 format and its special values.
- PostgreSQL, `float8` arithmetic and the `value out of range: overflow`
  error.
- [MEOS-WKB](meos-wkb-0.9.md), whose double fields carry finite numbers only.

## 9. Consumers

Every binding generated from the MEOS catalog inherits the rule without code
of its own: the refusal and the overflow error are raised by MEOS and reach
the binding through its error handler.

## 10. References to the implementation

The tests of a number are `ensure_finite` and `ensure_finite_result`
(`meos/include/temporal/temporal.h`), the test of a geometry is
`ensure_finite_geo` (`meos/src/geo/geo_funcs.c`), and
`tools/scripts/check_nan_guards.py` lists any public function that does not
test each of its double arguments. `meos/test/nan_input_test.c` and
`meos/test/nan_refusal_test.c` exercise each input class. Corrections are
filed as issues of the MobilityDB repository.
