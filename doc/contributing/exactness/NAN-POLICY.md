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

**Owner, 2026-10-30:** all the NaN work goes in a single PR, with one commit per step. Branch
`fix/meos-refuses-nan` (`estebanzimanyi/MobilityDB`), test `meos/test/nan_refusal_test.c`
(39 cases, each in a child process; 35 fail on master, 0 on the branch).

Two choices of the branch for the owner to confirm:

- **A linear temporal float holds no infinite value.** Its interpolation adds infinities of
  opposite signs or multiplies one by zero, so it has a NaN at every interior instant;
  `ensure_valid_tinstarr` refuses it, and a step or discrete temporal float keeps infinity. This
  one check removes the NaN of the integral, `twavg`, `avg_value`, Frechet, DTW, the shift and
  scale and the `tfloat_exp` crash.
- **Two kinds of double keep NaN, pending the owner:** a raster pixel value (nodata, map algebra constant),
  which GDAL writes as NaN for a float band with no data, and the float8 functions of `pgtypes`,
  which keep PostgreSQL's order (NaN above infinity) since they run inside sort comparators.

Found by the census and not caused by NaN, so left to PRs of their own: `jsonpath_in` aborts on 3
invalid paths; `tbox_from_hexwkb` aborts on the hex "1"; `floatspan_in` of `(1234.5, 10)` returns
a value together with its error and `floatspanset_in` of it aborts in `datum_cmp`;
`nad_tpcpoint_geo` given a temporal point aborts on an assertion.

In the order of the commits, each measured by the harness of its row before and after:

1. the pose and circular buffer readers (text, WKB, MF-JSON) and `box3d_in`, `gbox_make`, the
   float `tbox` and span set WKB readers: `ensure_not_nan_geo` and `ensure_not_nan` where the
   value is read;
2. the public functions taking a `double`: `ensure_not_nan` at entry, by shape;
3. the float results: a NaN result raises an error;
4. the crashes of the census, each refused with an error;
5. `tools/scripts/check_nan_guards.py`, which refuses a public function whose `double` parameter
   is not tested, and a catalog-driven test refusing a NaN through every entry point.

The work in flight that depends on this: the pose sort keys (D9.13, `meos/sortkey-pose`) and the
box order of NaN bounds (`fix/stbox-order-nan-bounds`), which the rule makes unnecessary, since no
box holds a NaN.
