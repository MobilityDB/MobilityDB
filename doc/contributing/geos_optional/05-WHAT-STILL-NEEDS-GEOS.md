<!--
  MobilityDB — Making GEOS optional: what still needs GEOS
  Copyright(c) MobilityDB Contributors

  This documentation is licensed under a Creative Commons Attribution-Share
  Alike 3.0 License: https://creativecommons.org/licenses/by-sa/3.0/
-->

# 5. What still needs GEOS

**Measured at** MobilityDB master `d9b9c11985`. Every result below is the output of a program in
[`tools/`](tools/), kept in [`tools/results/`](tools/results/), and can be re-run.

## 5.1 The question, and the two halves of its answer

"Which MEOS functions still need GEOS?" has two halves, and each needs its own kind of evidence.

1. **Which functions have a path to GEOS in their code?** This is a fact about the source: a
   function needs GEOS only if some chain of calls leads from it to a place that calls GEOS. It is
   answered by reading the code, and every chain can be checked by hand.
2. **Which inputs actually take that path?** A path is often guarded: the code tries its own
   routes first and reaches GEOS only when they all decline. Only running the function says
   whether an input gets through. In a MEOS built without GEOS, an input that takes the path
   raises an error instead of an answer, so the run shows it directly.

A path without an input that takes it is harmless; an input that takes a path is a function that
does not work without GEOS. This note gives both halves, then what they mean.

## 5.2 The build without GEOS

MEOS configured with `-DMEOS=ON -DALL=ON -DGEOS=OFF` at the master above compiles with 0
warnings ([`tools/results/build_nogeos.txt`](tools/results/build_nogeos.txt)). Its `libmeos.so` lists no GEOS library among the libraries it needs, and
`nm -D --undefined-only` finds 0 undefined GEOS symbols in it
([`tools/results/libmeos_nogeos_links.txt`](tools/results/libmeos_nogeos_links.txt)). Every run
below uses that library.

GEOS is nevertheless **loaded** into a process using it. `-DALL=ON` builds the raster family,
which links GDAL, and the system GDAL 3.8.4 (`libgdal.so.34`) itself links `libgeos_c.so.1` and
`libgeos.so.3.12.1`; `ldd` lists both. MEOS calls none of GEOS's functions — its fall-backs are
compiled out and raise their own error, which is what the runs below detect — but a process free
of GEOS needs the raster family off, or a GDAL built without GEOS.

**No continuous-integration workflow builds MobilityDB with `-DGEOS=OFF`**: no file under
`.github/` contains the option. A change that breaks the build without GEOS, or that sends a new
input to GEOS, is therefore not detected today.

## 5.3 Where GEOS is called

In a build without GEOS, two kinds of code stand in GEOS's place:

- **The four fall-backs** in `meos/src/geo/postgis_funcs.c`, inside `#if GEOS` blocks. A
  *fall-back* is the code after the last native route of an operation; without GEOS it raises
  "… is answered by the GEOS library, which this build excludes". They sit at the end of
  `geom_intersection2d_route`, `geom_difference2d_route`, `geom_array_union_shared` and
  `geom_unary_union`.
- **The stubs** of `meos/src/geo/geo_lwgeom_none.c`, which stand in for the GEOS-calling
  functions of the vendored PostGIS library and raise the same error. Inside that library they
  are called from four places: `lwgeom_cluster_kmeans` (through `lwgeom_centroid`, which the stub
  file answers natively with `meos_centroid`), `lwgeom_subdivide`, `lwgeom_wrapx`, and
  `lwgeom_clip_to_ordinate_range`, which offsets a curve only when asked for a non-zero offset.
  MEOS calls `lwgeom_cluster_kmeans` and none of the others; the one library function reaching
  `lwgeom_clip_to_ordinate_range` that MEOS can call, the 3D distance in `measures3d.c`, passes
  an offset of 0. So no stub that raises is ever reached.

What needs GEOS is therefore exactly what reaches one of the four fall-backs.

## 5.4 Which functions have a path: 134 of 3051

[`tools/geos_reach.py`](tools/geos_reach.py) reads every function defined in `meos/src`, records
which functions each one refers to (a call, or a function passed as a pointer), and walks back
from the four fall-backs to every public function that can reach one. A function is *public* when
its documentation carries an `@ingroup` outside the internal groups.

Of the **3051** public functions of `meos/src`, **134** have a path. The other 2917 have none:
nothing they call, at any depth, leads to a fall-back. The 134 are listed one by one in
[appendix A](A-INVENTORY.md), each with its chain.

Their paths meet the fall-backs through nine *gateways*: the functions that call a
fall-back-holding function directly. A public function usually has several: 97 of the 134 can pass
through three gateways or more, and only 37 through exactly one. The table counts, for each
gateway, the public functions able to pass through it, so the counts overlap
([`tools/results/geos_reach.txt`](tools/results/geos_reach.txt), last column):

| Gateway | What it does | Public functions able to pass through it |
|---|---|---|
| `geom_array_union` | the union of an array of geometries, which merges instants at the same timestamp and builds a network point's trajectory | 91 |
| `tpointseq_linear_trajectory` | the trajectory of a temporal point sequence | 74 |
| `geom_intersection2d` | the intersection of two geometries, used by restriction to a geometry or a box and by the temporal relationships | 55 |
| `geom_difference2d` | the difference of two geometries, used by the same restrictions | 55 |
| `geo_values_collect` | the union of the values of a temporal geometry, its traversed area | 38 |
| `tpointseqset_linear_trajectory` | the trajectory of a temporal point sequence set | 37 |
| `tcbufferseq_traversed_area` | the area a moving disc sweeps over one sequence | 27 |
| `trgeo_geoms_merge` | the union of the placements of a rigid geometry | 14 |
| `tcbufferseqset_traversed_area` | the area a moving disc sweeps over a sequence set | 13 |

A gateway reaches GEOS on certain inputs only, so the runtime half is measured gateway by
gateway; functions sharing a gateway may still differ, since each hands it its own arguments
(§5.6 shows one such pair). The unary union itself is a fall-back-holding function and is listed
as its own entry.

Every one of the 134 can reach the array-union fall-back, because the overlay and the unary union
both hand a collection to the array union, part by part.

## 5.5 Which inputs take the path: the sweep

[`tools/geos_fallback_sweep.c`](tools/geos_fallback_sweep.c) runs the four operations in the
build without GEOS on one representative geometry of each of 15 types: point, multipoint, line,
multiline, polygon, multipolygon, triangle, circular string, compound curve, multicurve, curve
polygon, multisurface, TIN, polyhedral surface and geometry collection. It recognizes a fall-back
by its own message, since other refusals share its error code. Two controls run first and both
pass: a unary union on a precision grid, which can only be answered by GEOS, is caught; and a
geodetic array of lines is refused by the native code before any fall-back ("The union of a
geodetic array is answered for positions only").

The table counts the calls that reach a fall-back. A *Z mode* says which operands carry a third
coordinate: neither, both, only the first, only the second.

| Operation, inputs | Reach a fall-back |
|---|---|
| intersection of two geometries, every ordered pair of types, each Z mode | 0 of 225 |
| difference of two geometries, every ordered pair of types, each Z mode | 0 of 225 |
| unary union of one geometry of each type, flat and with Z | 0 of 30 |
| **array union of two geometries, every ordered pair of types, each Z mode** | **104 of 225** |
| **unary union of a collection of two geometries, every ordered pair of types** | **104 of 225** |
| **intersection of such a collection with a geometry of each type, both orders** | **409 of 6750** |
| **difference of such a collection and a geometry of each type, both orders** | **953 of 6750** |
| fifteen geometric special cases: coinciding arcs, a fork, a line doubling back, polygons sharing an edge or a corner, discs tangent at one point, and others; each under all four operations | 0 |

**The array union, and the unary union of a collection, follow one exact rule.** They reach the
fall-back if and only if one member is a `MULTIPOLYGON`, a `MULTISURFACE`, a `TIN` or a
`POLYHEDRALSURFACE`. Checked line by line against the output: the 104 pairs are exactly the pairs
holding one of those four types, in every Z mode, and the 121 pairs holding none of them are
answered. A single geometry of those types is answered by the unary union; it is as a *member*
of an array or a collection that it is not.

The smallest examples ([`tools/arrunion_min.c`](tools/arrunion_min.c),
[`tools/coll_multi.c`](tools/coll_multi.c), [`tools/coll_detail.c`](tools/coll_detail.c)):

| Call | Without GEOS |
|---|---|
| array union of `POLYGON((0 0,1 0,1 1,0 1,0 0))` and `MULTIPOLYGON(((5 5,6 5,6 6,5 6,5 5)))` | reaches the fall-back |
| array union of `POINT(9 9)` and that `MULTIPOLYGON` | reaches the fall-back |
| array union of two disjoint `POLYGON`s | answers their `MULTIPOLYGON` |
| unary union of `GEOMETRYCOLLECTION(POINT(9 9),MULTIPOLYGON(((5 5,6 5,6 6,5 6,5 5))))` | reaches the fall-back |
| intersection of `MULTIPOLYGON(((0 0,2 0,2 2,0 2,0 0)),((3 3,4 3,4 4,3 4,3 3)))` with `GEOMETRYCOLLECTION(POINT(2 2),POLYGON((0 0,4 0,4 4,0 4,0 0)))` | reaches the fall-back |
| difference of `GEOMETRYCOLLECTION(LINESTRING(0 2,4 2),POLYGON((0 0,4 0,4 4,0 4,0 0)))` and `TRIANGLE((0 0,4 0,2 4,0 0))` | reaches the fall-back |

The error these calls raise does not name their cause. The array union's says "The union of a
geodetic array, and of one whose linework coincides over a curve, is answered by the GEOS
library"; the input is neither geodetic nor curved. The unary union's names "a precision grid"
among its causes, and none is asked for.

## 5.6 Which public functions this reaches

[`tools/gateway_reach.c`](tools/gateway_reach.c) runs public functions of every gateway on the
inputs above: temporal points (one sequence, one sequence set), temporal geometries whose value is
a `MULTIPOLYGON`, a `MULTISURFACE`, a `TIN`, a `POLYHEDRALSURFACE` or a collection, and moving
discs; each against seven geometry arguments, from a plain `POLYGON` to
`GEOMETRYCOLLECTION(POINT, MULTIPOLYGON)`. Of its **337 calls, 322 answer, 11 reach a fall-back,
and 4 give another outcome**, which the output names: `tgeo_traversed_area` refuses a temporal
point by its own type check ("The temporal value must be a temporal geometry or geography"), and
`tgeo_minus_geom` returns no value, without an error, for the two temporal points (a sequence and
a sequence set) against the polyhedral surface whose face is the square from (0 0) to (4 4); both
lie within that square. With the
targeted probes [`tools/temporal_reach.c`](tools/temporal_reach.c),
[`tools/trgeo_reach.c`](tools/trgeo_reach.c), [`tools/tcbuffer_reach.c`](tools/tcbuffer_reach.c)
and [`tools/tmerge_min.c`](tools/tmerge_min.c), the public functions that do not work without GEOS
on ordinary input are these:

| Public function (SQL) | Input that reaches GEOS |
|---|---|
| `tgeo_traversed_area` (`traversedArea`) | a temporal geometry one of whose values is a `MULTIPOLYGON`, a `MULTISURFACE`, a `TIN`, a `POLYHEDRALSURFACE` or a `GEOMETRYCOLLECTION`, even `GEOMETRYCOLLECTION(POINT, POLYGON)` |
| `tgeo_at_geom` (`atGeometry`) | a temporal geometry one of whose values is a `MULTIPOLYGON`, a `MULTISURFACE`, a `POLYHEDRALSURFACE` or a `GEOMETRYCOLLECTION`, restricted to `GEOMETRYCOLLECTION(POINT, MULTIPOLYGON)`; with the same argument, `tgeo_minus_geom` answers |
| `trgeometry_traversed_area` (`traversedArea` of a rigid geometry) | a rigid geometry whose reference shape is a `POLYHEDRALSURFACE` |
| `temporal_merge` (`merge`) | two instants of a temporal geometry at the same timestamp, one valued `POLYGON` and the other `MULTIPOLYGON`: the union reaches GEOS, and the merge reports the refusal as an error (§5.7). `temporal_merge_array`, `temporal_insert`, `temporal_update` and `temporal_append_tsequence` reach the same merge step; they are not run |
| `geom_array_union`, `geom_unary_union`, `geom_intersection2d`, `geom_difference2d` | the inputs of §5.5 |

The functions run on the same inputs and answering every one of them: the temporal relationships
`tintersects_tgeo_geo` and `tcontains_geo_tgeo`, the ever relationships `eintersects_tgeo_geo`
and `econtains_geo_tgeo`, `tpoint_at_geom`, `tgeo_at_stbox`, `tgeo_convex_hull`,
`trgeometry_convex_hull`, and for moving discs `tcbuffer_traversed_area`, `tcbuffer_at_geom`,
`tintersects_tcbuffer_geo`, `tdwithin_tcbuffer_geo`, `eintersects_tcbuffer_geo` and
`tcontains_cbuffer_tcbuffer`. `tgeo_minus_geom` answers every input but the two above, where it
returns no value without an error.

Not run, by family, with the gateways [appendix A](A-INVENTORY.md) gives each:

- **network points**: the restrictions to a box go through `geom_intersection2d`,
  `geom_difference2d` and `geom_array_union`, those to a geometry through these and the trajectory
  of a point sequence; the trajectory and the distances built on it go through `geom_array_union`
  alone, over the lines of the road network;
- **poses**: the restrictions go through the same gateways as those of network points; the
  trajectory and the distances go through the trajectories of the pose's position and
  `geo_values_collect`;
- **rigid geometries**: the twelve ever and always relationships with a geometry go through
  `trgeo_geoms_merge`, the gateway through which `trgeometry_traversed_area` reaches GEOS for a
  polyhedral reference shape while `trgeometry_convex_hull` answers; `trgeometry_length` goes
  through the trajectory of the body's position; `trgeometry_merge`, `trgeometry_merge_array` and
  `trgeometry_append_tsequence` go through `geom_array_union`;
- **raster**: `raster_tile_value_array` goes through `geom_array_union`, by way of the merge.

## 5.7 A refused union in the merge

`tgeoinst_merge_array_iter` (`meos/src/temporal/temporal_modif.c`) merges the instants of a
temporal geometry or geography that share a timestamp by taking the union of their values, through
`geoarr_merge`. When that union is refused, the merge answers no value and leaves the union's
error standing, as for any pair of instants that cannot be merged (MobilityDB #2946, master
`0bf0dab28c`). At master `d9b9c11985`, which precedes it, the merge passes the absent value to
`tinstant_make`, which reads it, and the process ends inside `ensure_not_empty`
([`tools/results/tmerge_min.gdb.txt`](tools/results/tmerge_min.gdb.txt)). Two unions are refused: the planar array
union of a multi-part member in a build without GEOS (§5.5), and, in every build, the union of a
geodetic array holding anything but positions ("The union of a geodetic array is answered for
positions only").

[`tools/merge_sweep.c`](tools/merge_sweep.c) merges two instants at one timestamp for every
ordered pair of the 15 types, as temporal geometries and as temporal geographies, each case in a
process of its own so that a crash is counted rather than ending the run. In the build without
GEOS, at master `d9b9c11985` ([`tools/results/merge_sweep.master.txt`](tools/results/merge_sweep.master.txt))
and with #2946 ([`tools/results/merge_sweep.with_fix.txt`](tools/results/merge_sweep.with_fix.txt)):

| | Answered | Refused with an error | Crash |
|---|---|---|---|
| temporal geometry, 225 pairs, `d9b9c11985` | 121 | 0 | 104 |
| temporal geography, 225 pairs, `d9b9c11985` | 4 | 0 | 221 |
| temporal geometry, 225 pairs, with #2946 | 121 | 104 | 0 |
| temporal geography, 225 pairs, with #2946 | 4 | 221 | 0 |

The 104 refused geometry pairs are the pairs of §5.5 holding a multi-part member; the 4 answered
geography pairs are the four made only of points and multipoints. The regression test of #2946,
`meos/test/merge_validity_test.c`, merges two temporal geographies holding lines at one timestamp,
a case refused in every build, and checks the error and the absence of a value; the same test
checks that two geography points and two geometry polygons still merge.

## 5.8 What this means

The four fall-backs cannot be deleted yet. Three of them are reached by ordinary data: a temporal
geometry valued with a multipolygon, a building footprint stored as a `MULTIPOLYGON`, a 3D model
stored as a `POLYHEDRALSURFACE`, or a collection, is enough. The work it leaves, in order:

1. **The array union answers multi-part members.** A `MULTIPOLYGON`, `MULTISURFACE`, `TIN` or
   `POLYHEDRALSURFACE` member is what every reaching input has in common, in the array union and
   in the unary union of a collection alike.
2. **The overlay of a collection answers** the 409 and 953 calls of §5.5 that remain after that,
   or the sweep is re-run to see how many remain.
3. **A workflow builds without GEOS and runs the sweep**, so that the counts of §5.5 cannot grow
   unseen.

Only then does the fourth fall-back — the unary union on a precision grid, which no MEOS caller
asks for and no SQL function exposes — and with it the other three, become dead code that can be
deleted.
