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
| Version | v1.1.0 |
| Scope | The numbers a MEOS or MobilityDB value holds, the answer to an operation whose result exceeds them, and the class of the error it raises |
| Reference | `meos/include/temporal/temporal.h` (`ensure_finite`, `ensure_finite_result`), `meos/src/geo/geo_funcs.c` (`ensure_finite_geo`), `meos/src/temporal/meos_error.c` (`meos_errcode_sqlstate`) |

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

The integers follow the same rule. The sum, the weighted sum and the delta of
a temporal integer or a temporal big integer whose exact result leaves its
type raise `integer out of range` or `bigint out of range`, with the code
`MEOS_ERR_VALUE_OUT_OF_RANGE`, as the `int4` and `int8` arithmetic of
PostgreSQL does, and never wrap around:

```sql
SELECT deltaValue(tint '[2147483646@2001-01-01, -3@2001-01-02]');
-- ERROR:  integer out of range
SELECT wsum(temp, interval '1 day') FROM (VALUES
  ('{5000000000000000000@2001-01-01, 5000000000000000000@2001-01-02}'::tbigint)) t(temp);
-- ERROR:  bigint out of range
```

### 4.3 Every magnitude of the doubles

Holding finite numbers only does not narrow the numbers a value holds: from
the subnormal doubles to `±DBL_MAX`, an answer is computed about the doubles
the value holds and is the same at every magnitude, as
[Exact geometry](meos-exact-geometry.md) states for the geometric answers. A
squared difference, a shoelace product or a product of six coordinates that
leaves the doubles is carried on in exact arithmetic, never rounded to 0 or to
an infinity: a temporal point moving inside the square of side `2e154`, or of
side `2e-210`, centred on the origin, is at distance 0 from it, the square
bounding a region whose area no double holds. The area of a geometry is the
one answer that still declines at such magnitudes (section 4.2).

### 4.4 A division by zero

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

## 6. The error value of a failed call

### 6.1 How a failed call is reported

A MEOS function that cannot answer, because an argument is refused (section
3), a result leaves the doubles (section 4) or any other error occurs,
reports the error to the error handler of the program with `meos_error`. The
default handler ends the program. In PostgreSQL the error is raised as an
`ERROR`, so no error value ever reaches a query. Where the handler returns,
as the one `meos_initialize_noexit_error_handler` installs does, the function
returns its error value and `meos_errno()` holds the error code.

Each function states its error value in its `@errval` tag:

```c
/**
 * @ingroup meos_temporal_accessor
 * @brief Return the start value of a temporal float
 * @param[in] temp Temporal value
 * @errval DBL_MAX
 * @csqlfn #Temporal_start_value()
 */
double
tfloat_start_value(const Temporal *temp)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TFLOAT(temp, DBL_MAX);
  return DatumGetFloat8(temporal_start_value(temp));
}
```

`VALIDATE_TFLOAT` reports a `NULL` argument or one that is not a temporal
float, and the function then returns `DBL_MAX`:

```c
meos_initialize_noexit_error_handler();
double r = tfloat_start_value(temp);
if (meos_errno() != 0)
{
  /* The call failed, r is DBL_MAX and meos_errno() holds the code */
}
```

The bindings read the tag from the MEOS catalog they are generated from, and
`tools/scripts/check_error_sentinels.py` checks that each function documents
and returns the error value its return type and domain call for.

### 6.2 The error value of each function

The error value is the maximum of the return type, and a value outside the
answers of the function where the maximum is one of them:

| the function returns | its error value | example |
|---|---|---|
| a pointer | `NULL` | `cbufferset_start_value` |
| `bool` | `false` | `geom_contains` |
| `int`, `int64`, `uint32`, `uint64` | `INT_MAX`, `INT64_MAX`, `UINT32_MAX`, `UINT64_MAX` | `cbuffer_cmp`, which answers -1, 0 or 1; `tpcpatch_npoints` |
| `double` | `DBL_MAX` | `tfloat_start_value` |
| an `int` answering 1, 0 or -1 for true, false or unknown | `-1` | `spatialrel_geo_geo` |
| an `int` answering a count | `-1` | `geo_num_points` |
| a `double` answering a fraction along a line | `-1.0` | `line_locate_point` |
| a number answering a distance | `-1`, `-1.0` for a `double` | `distance_cbuffer_cbuffer`, `nad_tgeo_tgeo`, `distance_set_int`, `temporal_frechet_distance` |

The last four rows follow from the domain. A comparator answering -1 takes
the maximum, and a relationship answering -1 for unknown takes -1. A count
of `INT_MAX` is a count, so its error is -1. A fraction lies in [0, 1]. A
distance answers the maximum where its operands have no distance (section
6.3), and a distance of finite values can itself be the largest double, so
its error takes -1, a value no distance has. A distance is a function of a
distance group of the reference manual, or one whose note states that a
distance is never negative.

### 6.3 A missing answer is not an error

Where the operands are valid and have no answer, a function answers without
error, and a `@note` of the function states that answer: the distance to an
empty geometry, which has no point to measure from, is `DBL_MAX`, as is the
nearest approach distance of two temporal values that share no time. A
distance raising an error answers -1.0 instead, so its value alone tells the
two apart. The PostgreSQL function answers `NULL` there:

```sql
SELECT nearestApproachDistance(tgeompoint '[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02]',
  geometry 'Point empty');
-- NULL
```

### 6.4 At the SQL level and in C

At the SQL level a failed call is an `ERROR` and a missing answer is `NULL`,
so every value a query reads is an answer, `DBL_MAX` included:

```sql
SELECT startValue(tfloat '[1.7976931348623157e+308@2001-01-01]');
-- 1.7976931348623157e+308
```

In C, `tfloat_start_value` of that value answers `DBL_MAX` with
`meos_errno()` 0. A caller of a function whose answers include its error
value, or its missing answer, reads `meos_errno()` to tell them apart, as the
example of section 6.1 does.

### 6.5 The class of an error

Each MEOS error code names the class of its error, and the errors MEOS raises
from the PostgreSQL code it carries take the code of the class PostgreSQL
gives the same message. In PostgreSQL an error is raised with the SQLSTATE of
its class, so a client tells a value out of range from a malformed input by
the state alone, as it does for the built-in types:

| MEOS code | SQLSTATE |
|---|---|
| `MEOS_ERR_VALUE_OUT_OF_RANGE` | 22003 |
| `MEOS_ERR_DATETIME_OUT_OF_RANGE` | 22008 |
| `MEOS_ERR_DIVISION_BY_ZERO` | 22012 |
| `MEOS_ERR_INVALID_ARG_VALUE` | 22023 |
| `MEOS_ERR_FEATURE_NOT_SUPPORTED` | 0A000 |
| `MEOS_ERR_INDETERMINATE_COLLATION` | 42P22 |
| `MEOS_ERR_INVALID_REGULAR_EXPRESSION` | 2201B |
| `MEOS_ERR_SYNTAX_ERROR` | 42601 |
| `MEOS_ERR_TEXT_INPUT`, `MEOS_ERR_MFJSON_INPUT`, `MEOS_ERR_GEOJSON_INPUT` | 22P02 |
| `MEOS_ERR_WKB_INPUT` | 22P03 |
| the SQL/JSON codes, such as `MEOS_ERR_NON_NUMERIC_SQL_JSON_ITEM` | the SQL/JSON class of the same name, 22030 to 2203C, such as 22036 |
| `MEOS_ERR_UNDEFINED_OBJECT` | 42704 |
| `MEOS_ERR_MEMORY_ALLOC_ERROR`, `MEOS_ERR_OUT_OF_MEMORY` | 53200 |
| a code with no class of its own, such as `MEOS_ERR_INTERNAL_ERROR` | XX000 |

```sql
\set VERBOSITY sqlstate
SELECT tsum(temp) FROM (VALUES
  ('2000000000@2001-01-01'::tint), ('2000000000@2001-01-01'::tint)) t(temp);
-- ERROR:  22003
SELECT tint '[1@2001-01-01, 2@2001-01-02]' / 0;
-- ERROR:  22012
SELECT tintFromBinary('\x00');
-- ERROR:  22P03
```

A jsonpath applied to a `tjsonb` or a `jsonbset` raises the errors, and the
SQLSTATEs, that the same path raises on a `jsonb`. An item method that cannot
convert its argument raises the SQL/JSON class of a non-numeric item, a call
that suppresses the errors of its path answers nothing and raises none, and
the largest value of a type is a value:

```sql
SELECT jsonbsetPathQueryFirst(jsonbset '{"\"x\""}', '$.bigint()');
-- ERROR:  argument "x" of jsonpath item method .bigint() is invalid for type bigint
SELECT jsonbsetPathQueryFirst(jsonbset '{"\"9223372036854775807\""}', '$.bigint()');
-- {9223372036854775807}
```

## 7. Versioning and stability

The rule is part of the value domain of every MEOS type. A later version may
widen what an input accepts only by a new type, never by admitting a
non-finite number into an existing one.

## 8. Out of scope

The internal search keys of the indexes, such as an unbounded box at the root
of a tree or the "no distance yet" key of a nearest-neighbour search, are
never MEOS values and cross no public function. The error value a function
documents for a failed call is not a value either (section 6). The types and operators of
PostgreSQL itself keep the semantics of PostgreSQL inside MobilityDB: the SQL
product of an `interval` and a `float` is the PostgreSQL operator, which
answers an infinite interval for an infinite factor, while the MEOS functions
of intervals refuse one.

## 9. References

- IEEE 754-2019, the binary64 format and its special values.
- PostgreSQL, `float8` arithmetic and the `value out of range: overflow`
  error.
- [MEOS-WKB](meos-wkb-0.9.md), whose double fields carry finite numbers only.

## 10. Consumers

Every binding generated from the MEOS catalog inherits the rule without code
of its own: the refusal and the overflow error are raised by MEOS and reach
the binding through its error handler.

## 11. References to the implementation

The tests of a number are `ensure_finite` and `ensure_finite_result`
(`meos/include/temporal/temporal.h`), the test of a geometry is
`ensure_finite_geo` (`meos/src/geo/geo_funcs.c`), and
`tools/scripts/check_nan_guards.py` lists any public function that does not
test each of its double arguments. `meos/test/nan_input_test.c` and
`meos/test/nan_refusal_test.c` exercise each input class. Corrections are
filed as issues of the MobilityDB repository.
