<!--
  MobilityDB — Exactness: no NaN in MEOS and MobilityDB
  Copyright(c) MobilityDB Contributors

  This documentation is licensed under a Creative Commons Attribution-Share
  Alike 3.0 License: https://creativecommons.org/licenses/by-sa/3.0/
-->

# No NaN in MEOS and MobilityDB

**Decided by the owner, 2026-10-30.** Master read: `307597251b`.

## 1. The rule

1. **No MEOS or MobilityDB value holds a NaN.** Every entry point refuses one with an error: the
   text, WKB and MF-JSON readers, the constructors, and every public function taking a `double`.
   A missing value is a gap of a temporal value, or NULL.
2. **Infinity stays a value of the floats.** `floatspan '[1.123456789, inf)'` is the documented
   way to write an unbounded span (`doc/set_span_types.xml`, `round`). Spatial coordinates are
   finite, as the point and box constructors already require.
3. **An operation whose result would be NaN raises an error**, as PostgreSQL raises "value out of
   range": `inf - inf`, `0 * inf` and the interpolation between infinities have no value.
4. **`-0.0` equals `0.0`** in every comparison, equality, hash and sort key.

The rule is the one `ensure_not_nan` (`meos/include/temporal/temporal.h`) already states: "A NaN
has no position among the numbers, so a value, a span bound or a coordinate holding one cannot be
compared, bounded or indexed, and MEOS refuses it where a number enters it." The census measures
where MEOS does not apply it.

**Mechanism** (`meos-public-validates-internal-asserts-idiom`): a public function tests each
`double` argument with `ensure_not_nan`, and each geometry with `ensure_not_nan_geo`, at entry; an
internal function asserts the same; a public function computing a float tests its result.

**Why NaN, and not infinity:** a NaN has no order, no exact value (CGAL's `Gmpq(double)` asserts
`is_finite`), and a meaning of its own in WKB, where `POINT(NaN NaN)` reads as `POINT EMPTY`
(`lwin_wkb.c`). GEOS uses NaN for "absent" (`Coordinate` z, a null `Envelope`) and reports a NaN
coordinate as invalid (`IsValidOp`). JSON, and so MF-JSON, has no NaN.

## 2. The census

Tools in `~/cpwork/nan-census` on P-ULB-PF59GXL3: the MEOS-API catalog (`c791520db`) of the
installed headers, then harnesses generated from it that call each public function, each case in
a child process so that a crash is recorded.

| Path | Cases | Refuse NaN | Accept NaN | Crash |
|---|---|---|---|---|
| text readers | 49 readers, 6124 | floats, temporal points and geometries, network points, `tbox`, `stbox`, `tpcbox` | `pose_in`, `poseset_in`, `tpose_in`, `posechain_in`, `posechainset_in`, `tposechain_in`, `trgeometry_in`, `cbuffer_in`, `cbufferset_in`, `tcbuffer_in`, `box3d_in` | `jsonpath_in` on 3 invalid paths |
| hex WKB readers | 56 readers, 6062 | `tfloat`, `floatset`, `geomset`, `stbox`, `tpcbox`, `tgeompoint`, `tgeometry` | the pose and circular buffer families; `tbox_from_hexwkb`, `spanset_from_hexwkb` of floats | `span_from_hexwkb` ("out of memory", exit), `spanset_from_hexwkb` (assertion in `datum_cmp`), `temporal_from_hexwkb` of a `trgeometry` (SIGSEGV) |
| MF-JSON readers | 20 readers, 4058 | `tfloat`, `tgeompoint`, `tgeometry`, `tgeogpoint`, `tgeography` | `tcbuffer`, `tpose`, `tposechain`, `trgeometry` | `trgeometry` (22) |
| scalar functions | 40, 66 calls | every value builder | `gbox_make`, `float_round`, `float_degrees`, `float_get_bin`, `float_angular_difference`, the exported PostgreSQL float primitives | none |
| a double and pointers | 290 calls | 111 | 153 | 26 assertions: `teq`/`tne`/`tlt`/`tle`/`tgt`/`tge` of a tfloat and a float, four ever/always comparisons, `tdwithin` (3), `geom_dwithin3d`, `line_interpolate_point`, `line_substring`, `tpose_rotate_x`/`_y`, `trgeometry_rotate_x`/`_y` |
| float results from infinity | 251 functions, 384 calls | | 30 calls return NaN: `degrees` and `shift_scale` of float sets, spans and span sets, `floatspan_expand`, `tfloat_shift`/`scale_value`, `tfloatbox_shift_scale`, `tnumber_integral`, `tnumber_twavg`, `tnumber_avg_value`, `temporal_frechet_distance`, `temporal_dyntimewarp_distance` | `tfloat_exp`, `tbox_get_value_tile`, `tbox_get_value_time_tile`, `nad_tpcpoint_geo`; `tbox_from_hexwkb` on an invalid hex string |

Defects the census finds besides NaN: `temporal_as_mfjson` of a `tgeometry` holding a circular
string, and of a `ttext` holding a double quote, is not read back by its reader.

Not yet measured: 57 functions with an argument the harness has no canned value for, and 16 text
readers whose types have no literal in the regression queries.

## 3. The fixes, one PR

**Owner, 2026-10-30:** all the NaN work goes in a single PR, with one commit per step: MobilityDB
#3291, MERGED (2026-10-31) as `b313e49372`, seven commits, its receipts on the head (strict-ci, Windows, cppcheck, valgrind, speed, and the CGAL
oracle: 8375 of 8375 circular buffer distances and 2100 of 2100 buffers of geometries). Test `meos/test/nan_refusal_test.c`: 39 cases, each
in a child process; 35 fail on the master before it, 0 after, valgrind clean in every child.

The commits, in order:

1. `Test that no MEOS value holds a NaN`: `nan_refusal_test.c`, readers, functions taking a NaN and
   results over infinite floats;
2. `Refuse a NaN in the readers of poses and circular buffers`: `cbuffer_make`, `pose_make_2d`,
   `pose_make_3d`, the WKB reader (`type_from_wkb` drops a value whose reading raised an error)
   and the plural MF-JSON parsers;
3. `Refuse a NaN in the readers of geometries and in the box constructors`: every public geometry
   reader ends with `geo_read_result`, and `gbox_make`, `box3d_make`; `nan_input_test.c` and
   `distance_point_test.c` read a NaN geometry as refused;
4. `Refuse a NaN in each double a public MEOS function takes`: `ensure_not_nan` at entry, by shape,
   in 53 files (the census: 290 calls, 290 refused);
5. `Raise an error where a float operation would give a NaN`: `ensure_not_nan_result`, the
   linear temporal float refusing infinity, and the shift, scale and expansion of float sets,
   spans, span sets and boxes tested in `numset_shift_scale`, `numspan_shift_scale`,
   `numspan_expand`, `numspanset_shift_scale` and `tbox_shift_scale_value`, which the SQL
   functions call directly (the census: 384 calls, no NaN);
6. `Refuse a NaN in the reference geometry of a temporal rigid geometry`;
7. `Check that every public MEOS function tests each double it takes for NaN`:
   `tools/scripts/check_nan_guards.py` and its job in `check-code.yml` (409 untested doubles on
   master, none on the branch).

Two choices of the branch for the owner to confirm, each with the recommendation:

- **A linear temporal float holds no infinite value.** Its interpolation adds infinities of
  opposite signs or multiplies one by zero, so it has a NaN at every interior instant, even
  between two equal infinities; `ensure_valid_tinstarr` refuses it, and a step or discrete
  temporal float keeps infinity. This one check removes the NaN of the integral, `twavg`,
  `avg_value`, Frechet, DTW, the shift and scale and the `tfloat_exp` crash. Recommended: keep
  the refusal, the error being raised where the value is built rather than in each function
  reading it; a set, span, box, step or discrete value still states an unbounded value. A linear
  temporal point with an infinite coordinate has the same interpolation and is outside the branch.
- **Two kinds of double keep NaN, pending the owner:** a raster pixel value (nodata, map algebra
  constant), which GDAL writes as NaN for a float band with no data, and the float8 functions of
  `pgtypes`, which keep PostgreSQL's order (NaN above infinity) since they run inside sort
  comparators. Recommended: keep both. A raster holds a NaN pixel as no data, and no MEOS value
  read from it holds one, a NaN pixel read as no value (to verify on every path from a raster to
  a value); the float8 functions are PostgreSQL's `double precision`, and every MEOS entry point
  refuses the NaN they can return.

Found by the census and not caused by NaN, each a PR of its own on master `4b5f1d6d84`, each with
a test that fails on the file of master and passes on the fix:

| Branch | Defect | Test |
|---|---|---|
| MobilityDB #3301, MERGED as `e0f9f95714` | a HexWKB string of an odd length, with a character that is not a hexadecimal digit, or empty, is parsed from the bytes it does not encode: `tbox_from_hexwkb("1")` and `("")` abort | `hexwkb_input_error_test.c`, nine readers |
| MobilityDB #3298, MERGED as `00b566cedb` | `span_parse` ignores the refusal of `span_set`: `floatspan_in("(1234.5, 10)")` returns a value with its error, `floatspanset_in` of it aborts | `text_input_error_test.c` |
| MobilityDB #3303, MERGED as `939e78ded4` | `nad_tpcpoint_geo` and `eintersects_tpcpoint_geo` test only for null: a temporal point aborts, a sequence is read as an instant | `tpointcloud_test.c` |
| MobilityDB #3324, `fix/jsonpath-syntax-error-returns-no-value` | `parsejsonpath` returns its uninitialized result after a syntax error: `free(): invalid pointer` | `json_inout_validity_test.c` |

A JSON path the grammar refuses leaves no parse behind in #3324: `yydestruct` frees the value of each symbol the grammar discards by its `%type` in `jsonpath_gram.y`, `parsejsonpath` drains the free list, and `freeJsonPathParseItem` frees a subscript; valgrind finds no block lost over nine refused paths.

A pixel holding NaN reads as no value in MobilityDB #3322 (`fix/raster-nan-pixel-reads-no-value`): both pixel readers skipped only a pixel equal to the nodata value, which a NaN never is, and the NaN, refused by `tinstant_make` since #3291, left a missing instant that `raster_run_close` read, a segmentation fault. The vendored map algebra of `raster_rtcore.c` keeps its pixel values as they are.

The work that depends on this: the pose sort keys (D9.13, `meos/sortkey-pose`), rebased on master
`cde4a92732` with `pose_cmp` and `posechain_cmp` keeping `<` and `>`, and the box order of NaN
bounds (`fix/stbox-order-nan-bounds`, on the fork), set aside, since no box holds a NaN.

## 4. Next: every MEOS value is a finite number, and the double sentinel

**State:** planned, to start once the NaN work of section 3 is closed (owner, 2026-10-31). Two PRs,
in this order.

**PR 1, the finite rule.** Every input refuses a NaN and ±Infinity; an operation whose result
leaves the doubles from finite operands raises "value out of range", as PostgreSQL's float8
arithmetic does (`float8_pl` already raises it for `datum_add`). Measured on master `089206e021`,
the inputs disagree today: float spans, sets, `tbox` value spans and temporal float values accept
Infinity; `tstzspan`, `tstzset` and a temporal value at `infinity` refuse it; `POINT(Infinity 1)` is
refused while `POINT(1e400 1)` and `geompoint_make2d` with an infinite coordinate are accepted;
`STBOX X((1,1),(Infinity,2))` and a circular buffer of radius Infinity are accepted; a pose refuses
an infinite position and infinite angles. The rule closes these, and it is the domain of the CGAL
oracle, which reads every double as the rational it holds. The work extends what section 3 built:
`ensure_not_nan` (352 calls in 57 files) tests `isfinite` and is named `ensure_finite`;
`ensure_not_nan_geo` (8 calls) and its coordinate walk likewise; `ensure_not_nan_result` (13
calls) becomes the overflow test; `check_nan_guards.py` reads the new name; `nan_refusal_test.c`
and the census gain ±Infinity in every input class. Left to measure: the results that overflow
from finite operands outside the generic arithmetic (about 10 raw double sites in the temporal code
and the distance kernels for coordinates near ±1e308). Unaffected: the 68 `INFINITY`/`HUGE_VAL`
uses of the geometry kernels (interval bounds, `nextafter` directions) and the index-internal
"no distance" keys and unbounded node boxes, none of which crosses a public entry. Three SQL test
files name infinity, each as an error case whose message changes, harvested from strict-ci. The
manual names infinity in 4 lines. Receipts: strict-ci, Windows, cppcheck, valgrind, the CGAL
oracle and the speed probe. Owner to decide: an unbounded float span, `[0, Infinity)`, is then
written with a large finite bound, or spans gain an unbounded flag as PostgreSQL ranges have.

**PR 2, the double sentinel.** The rule that an error sentinel lies outside the value domain of
its function (decided by the owner, 2026-09-09) is met by `INT_MAX` and `INT64_MAX` (comparators
answer -1, 0 or 1, and a temporal integer refuses the maximum of its type since #3210), and is not
met by `DBL_MAX`, which 107 functions document as their error value although it is a value they
can answer: `float_round(DBL_MAX, 0)` answers `DBL_MAX`. Once PR 1 refuses ±Infinity as a value
and raises on overflow, +Infinity is the one double no function answers, so it is the sentinel the
rule derives; it sorts after every distance, strictly, and lies in the return type, as the typed
bindings require. Two commits: the double sentinel is named once, beside `distance_sentinel`, and
the 88 `result == DBL_MAX` tests of the PostgreSQL wrappers (18 files) and the 12 of MEOS call it,
no answer changing; then its value moves to +Infinity with the 107 `@errval` tags, the `EXPECTED`
and `ALIASES` maps of `check_error_sentinels.py` and `distance_sentinel(T_FLOAT8)`, the bindings
regenerating from the catalog. Owner to approve: the change of the documented error value of the
107 functions.
