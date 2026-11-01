<!--
  MobilityDB — Exactness: every MEOS value is a finite number
  Copyright(c) MobilityDB Contributors

  This documentation is licensed under a Creative Commons Attribution-Share
  Alike 3.0 License: https://creativecommons.org/licenses/by-sa/3.0/
-->

# Every MEOS value is a finite number

This note is the plan of the campaign that makes every MEOS and MobilityDB value a finite
number: the rule, the census that measures where MEOS does not apply it, the pull requests that
apply it, in order, the oracles that judge them, and the topics they leave, each a pull request of
its own. Master read: `91f8d676e7`.

## 1. The rule

1. **No MEOS or MobilityDB value holds a NaN, +Infinity or -Infinity.** Every entry point refuses
   one with an error: the text, WKB and MF-JSON readers, the constructors, and every public
   function taking a `double`. A text literal beyond the doubles, such as `1e400`, reads as an
   infinity and is refused. A missing value is a gap of a temporal value, or NULL.
2. **A result of finite operands that leaves the doubles raises "value out of range"**, as
   PostgreSQL's float8 arithmetic raises it: the integral of a temporal float, a distance, a span
   or box bound, a tile bound.
3. **A result that lies within the doubles is answered**, whatever its intermediate values: the
   time-weighted average of values near the largest double, the interpolation between `1e308` and
   `-1e308`, the distance of two coordinates whose difference exceeds the largest double, the
   rounding of the largest double. Where the computation cannot answer it, the function raises
   "not supported", never a value in place of the answer.
4. **A division by zero raises "Division by zero"**, as the division of a temporal number
   (`arithop_tnumber_number`, `arithop_tnumber_tnumber`) and PostgreSQL's `float8div` raise it:
   the density of a point cloud patch whose XY bounds have no area is one.
5. **`-0.0` equals `0.0`** in every comparison, equality, hash and sort key.

**Mechanism** (`meos-public-validates-internal-asserts-idiom`): a public function tests each
`double` argument with `ensure_finite` and each geometry with `ensure_finite_geo` at entry
(`meos/include/temporal/temporal.h`); an internal function asserts the same; a public function
computing a float tests its result with `ensure_finite_result`; a span computed from another one
tests its bounds with `ensure_finite_span_bounds`. `tools/scripts/check_nan_guards.py` checks that
every public function tests each double it takes.

**Why.** A NaN has no order, and an infinity no exact value: CGAL's `Gmpq(double)` asserts
`is_finite`, so the exact oracle of every geometric answer is defined on the finite doubles alone
([Exact distance](EXACT-DISTANCE.md), MobilityDB #3287). GEOS reports a non-finite coordinate as invalid
(`IsValidOp`), PostGIS a non-finite box (`gbox_is_valid`); WKB reads `POINT(NaN NaN)` as
`POINT EMPTY` (`lwin_wkb.c`); JSON, and so MF-JSON, has neither. A time span already refused
`infinity`; the rule makes the value dimension say the same.

**Decisions of the owner.**

- **2026-10-30.** The NaN work is one pull request (section 3).
- **2026-10-31.** A float span refuses ±Infinity like every input, as `tstzspan` does; spans gain
  no unbounded flag. An unbounded span is written with a finite bound.
- **2026-11-01.** The documented error value of the 107 functions returning a double moves from
  `DBL_MAX` to +Infinity (section 5).

**Kept as they are.** A raster pixel holding NaN is no value: the pixel readers skip it (MobilityDB
#3322), and no MEOS value read from a raster holds one. The float8 comparator `float8_cmp` keeps
PostgreSQL's order, NaN above infinity, since it runs inside sort comparators; every MEOS entry
point refuses the values it orders. The index-internal "no distance" keys and unbounded node boxes,
and the `INFINITY` seeds of the geometry kernels' running minima and boxes, cross no public entry.

## 2. The census

The census calls every public function of the installed headers, from harnesses generated from the
MEOS-API catalog, each case in a child process so that a crash is recorded. The value it puts in
is NaN, +Infinity, -Infinity or the text literal `1e400`; the census of results calls each
function returning a float over finite operands near the largest double: a temporal float from
-1.7e308 to 1.7e308, such a span, span set, set and box, and every double the largest double.

| Path | Cases | On master `307597251b`, NaN | On PR 1, NaN, ±Infinity and `1e400` |
|---|---|---|---|
| text readers | 49 readers, 6124 cases (master); 64, 6140 (PR 1) | the pose and circular buffer families and `box3d_in` accept NaN; `jsonpath_in` aborts on 3 invalid paths | every reader refuses each value; what is accepted is text and JSON content (a string holding `Infinity`, the arbitrary-precision jsonb number `1e400`) and an S2 cell token |
| hex WKB readers | 56 readers, 6062 / 6080 | the pose and circular buffer families, `tbox_from_hexwkb` and `spanset_from_hexwkb` of floats accept NaN; three readers abort | every patched value refused |
| MF-JSON readers | 20 readers, 4058 | `tcbuffer`, `tpose`, `tposechain`, `trgeometry` accept NaN; `trgeometry` aborts (22) | every token refused |
| a double among the arguments | 290 calls | 153 accept NaN, 26 abort on an assertion | every call refuses each value |
| scalar functions | 66 / 31 calls | `gbox_make`, `float_round`, `float_degrees`, `float_get_bin`, `float_angular_difference` accept NaN | every call refuses each value but `float8_cmp` |
| float results | 384 calls over infinity (master); 368 calls over 243 functions near the largest double (PR 1) | 30 calls return NaN; `tfloat_exp`, `tbox_get_value_tile`, `tbox_get_value_time_tile`, `nad_tpcpoint_geo` abort | no result is non-finite without an error, and nothing aborts |

On master, and fixed by PR 1: `float_round` answers infinity for the largest double and NaN for
400 decimal places; `tfloatbox_expand` leaves the doubles with no error; `tbox_get_value_tile`
returns an unfilled box; `tfloat_exp` dates a turning point outside its segment, and has none
where its two values differ by more than 709; the interval multiplication and division take an
infinite factor.

## 3. The NaN work, MobilityDB #3291

MERGED (2026-10-31) as `b313e49372`, seven commits, its receipts on the head (strict-ci, Windows,
cppcheck, valgrind, speed, and the CGAL oracle: 8375 of 8375 circular buffer distances and 2100 of
2100 buffers of geometries).

1. `Test that no MEOS value holds a NaN`: `nan_refusal_test.c`;
2. `Refuse a NaN in the readers of poses and circular buffers`: `cbuffer_make`, `pose_make_2d`,
   `pose_make_3d`, the WKB reader (`type_from_wkb` drops a value whose reading raised an error)
   and the plural MF-JSON parsers;
3. `Refuse a NaN in the readers of geometries and in the box constructors`: every public geometry
   reader ends with `geo_read_result`, and `gbox_make`, `box3d_make`;
4. `Refuse a NaN in each double a public MEOS function takes`: the test at entry, by shape, in 53
   files;
5. `Raise an error where a float operation would give a NaN`;
6. `Refuse a NaN in the reference geometry of a temporal rigid geometry`;
7. `Check that every public MEOS function tests each double it takes for NaN`:
   `tools/scripts/check_nan_guards.py` and its job in `check-code.yml`.

Found by the census and not caused by NaN, each a pull request of its own, each with a test that
fails on the file of master and passes on the fix:

| Pull request | Defect | Test |
|---|---|---|
| MobilityDB #3301, MERGED as `e0f9f95714` | a HexWKB string of an odd length, with a character that is not a hexadecimal digit, or empty, is parsed from the bytes it does not encode | `hexwkb_input_error_test.c` |
| MobilityDB #3298, MERGED as `00b566cedb` | `span_parse` ignores the refusal of `span_set` | `text_input_error_test.c` |
| MobilityDB #3303, MERGED as `939e78ded4` | `nad_tpcpoint_geo` and `eintersects_tpcpoint_geo` test only for null | `tpointcloud_test.c` |
| MobilityDB #3324, MERGED as `3680bfbd1d` | `parsejsonpath` returns its uninitialized result after a syntax error | `json_inout_validity_test.c` |
| MobilityDB #3322, MERGED as `8d62a25e15` | a pixel holding NaN, refused by `tinstant_make`, left a missing instant the raster walk read | `raster_nan_pixel_test.c` |
| MobilityDB #3332, MERGED as `7de78631f5` | the pose sort keys | `pose_sortkey_test.c` |

## 4. PR 1, the finite rule

MobilityDB #3361, branch `fix/meos-refuses-non-finite`, head `5ab1ba4d0d` on master
`91f8d676e7`. Seven commits:

1. `Refuse an infinite value where MEOS refuses a NaN`: the helpers test `isfinite`, the geometry
   walk refuses a non-finite coordinate, the raster pixel readers test `isfinite`; the interval
   multiplication and division refuse an infinite factor.
2. `Name the tests of a finite number ensure_finite`: `ensure_finite`, `ensure_finite_result`,
   `ensure_finite_geo`, `geo_walk_has_non_finite`, `ensure_finite_span_bounds`, and the comments
   that state the rule.
3. `Test that MEOS refuses an infinite value where it refuses a NaN`: `nan_input_test.c` reads
   infinite inputs, and `distance_point_test.c` asserts the refusal of two `1e400` geometries.
4. `Raise an error where a result of finite operands leaves the doubles`: the span distance, the
   temporal float sum, the delta, the circular buffer distance, the float bin count, the
   exponential and its turning point, `geom_to_cbuffer`, the rounding of floats, the expansion of
   float boxes, the value tile, the integral and the averages of temporal numbers (one power of
   two scales the values), the interpolation where `end - start` overflows, the average,
   weighted average and centroid aggregates (their state holds a mean where a sum overflows), and
   the density of a point cloud patch without area (rule 4).
5. `Answer the distances of coordinates near the largest double`: the orientation sign scales the
   coordinates by a power of two or reads `WideDyadic`, its double filter decides only products
   clear of the subnormal range; the 3D segment kernel scales back only a normal scaled distance
   and reads an unscalable pair as it is; the analytic distances of temporal points and temporal
   circular buffers read operands from 2^1019 scaled by 2^-5; the foot of a shortest line scales
   its two vectors apart; the temporal distance of a temporal point to a geometry of segments,
   whose kernel compares squares in doubles, raises "not supported" from 2^510. The magnitude
   of a polygonal geometry reads every ring, since a liblwgeom polygon box covers its exterior
   ring alone; the length and the perimeter measure each segment as its own vector, halving
   where a difference leaves the doubles; the area alone scales the geometry, to 2^500 (at most
   2^1022), and raises "not supported" where the scaling is not exact.
6. `Test that every MEOS value is a finite number`: `nan_refusal_test.c` reads NaN and ±Infinity
   through every entry class and checks the results of finite operands near the largest double,
   95 cases.
7. `State the rule of finite values for the users of MEOS`: `doc/specs/meos-finite-values.md`,
   its examples run against the head.

**The oracle.** Every corpus is judged by CGAL's exact rationals on the doubles the engine holds,
on the branch's answers alone; an answer "error" is right exactly where the exact distance rounds
to infinity. Each corpus has an accept control, which agrees, and a refuse control, refused on
every row it moves. The measures are judged within 1e-9 of their scale (a length of itself, an
area of the sum of the magnitudes of its shoelace terms), "not supported" right exactly where the
scaling of the area is not exact; the worst row is within 3.5e-16.

| Corpus | Rows | Agree |
|---|---|---|
| far coordinates, distance of two geometries (2D) | 3000 | 3000 |
| far coordinates, nearest approach of a temporal point | 3000 | 3000 |
| far coordinates, nearest approach of a temporal circular buffer | 2999 | 2999 |
| far coordinates, distance of a circular buffer at rest | 1000 | 1000 |
| far coordinates, distance of two geometries (3D) | 3000 | 3000 |
| the same with a fifth of the coordinates subnormal | 3000 | 3000 |
| 3D pairs no power of two scales near 1 exactly | 2000 | 2000 |
| the distance of two geometries of segments, at projected coordinates | 4405 | 4405 |
| the nearest approach of a temporal point to the census geometries | 4560 | 4560 |
| the nearest approach of a moving circular buffer | 2100 | 2100 |
| the 3D distance of points and segments | 2300 | 2300 |
| the length, perimeter and area of geometries from subnormal to near the largest double | 3000 | 3000 |

**SQL.** The regression tests gain the cases of each result: `003_span`, `021_tbox`,
`022_temporal`, `025_temporal_tile`, `026_tnumber_mathfuncs`, `040_temporal_aggfuncs`,
`042_temporal_waggfuncs`, `064_tpoint_distance`, `068_tpoint_aggfuncs`, `210_tcbuffer_distance`,
`441_tpc_aggfuncs_tbl`; the two `round` cases of `003_span` over an infinite bound become
refusals, and the `round` example of `doc/set_span_types.xml` takes a finite bound.

**The user document** `doc/specs/meos-finite-values.md` states the rule for the users of MEOS and
of the bindings.

**Receipts** on the head: strict-ci, Windows, cppcheck, valgrind, the CGAL oracle (12 witnesses,
24 controls) and the speed probe (26 cells, the distance of geometries at 0.435 of GEOS called
directly). Its merged-lines approval is the row of #2861 naming its branch.

## 5. PR 2, the double error value

The rule that an error value lies outside the value domain of its function (owner, 2026-09-09) is
met by `INT_MAX` and `INT64_MAX`, and not by `DBL_MAX`, which 107 functions document as their
error value although it is a value they can answer: `float_round(DBL_MAX, 0)` answers `DBL_MAX`.
Once PR 1 refuses ±Infinity as a value, +Infinity is the one double no function answers; it sorts
after every distance, strictly, and lies in the return type, as the typed bindings require.

Branch `fix/double-error-value-infinity`, whose base is the branch of PR 1; its pull request opens
once PR 1 is green. Two commits:
the double error value is named once, beside `distance_sentinel`, and the 88 `result == DBL_MAX`
tests of the PostgreSQL wrappers and the 12 of MEOS call it, no answer changing; then its value
moves to +Infinity with the 107 `@errval` tags, the `EXPECTED` and `ALIASES` maps of
`check_error_sentinels.py` and `distance_sentinel(T_FLOAT8)`, the bindings regenerating from the
catalog. The three nearest approach distances of float boxes and `line_locate_point` keep -1.0.
The user document gains its section on the error value of a failed call.

## 6. Topics the campaign leaves, each a pull request of its own

| Topic | Measured |
|---|---|
| The exactness guards read the lowest set bit | `double_lowest_bit` answers the unit in the last place, not the lowest set bit, so the guards of the exact distance comparisons decline from a skew of about 2^128: `segment_distance_square` of the path (1e40, 0)–(1e40, 4) and the point (-1e40, 2) declines, and the nearest approach instant of a point passing a point at 1e50 is the start of the segment, not its middle |
| The float overflow errors name their class | the 34 overflow and underflow errors of `pgtypes/utils/float.h` raise `MEOS_ERR_INTERNAL_ERROR`, not `MEOS_ERR_VALUE_OUT_OF_RANGE` |
| The integer sums and deltas raise on overflow | the sum and the delta of a temporal integer and a temporal big integer overflow with no error |
| The area of a geometry is exact at every magnitude | `geom_area` raises "not supported" where scaling the geometry into 2^-500 to 2^500 is not exact, such as coordinates near the largest double beside coordinates the scaling by 2^-523 rounds; the exact area sums the shoelace terms in expansions, then `WideDyadic` |
| The temporal distance of a temporal point to a geometry of segments is exact | `tpoint_linear_distance_geom` takes the root of a square rounded in doubles behind an absolute tolerance of 1e-12; an entry of [Exact distance](EXACT-DISTANCE.md), MobilityDB #3287 |
