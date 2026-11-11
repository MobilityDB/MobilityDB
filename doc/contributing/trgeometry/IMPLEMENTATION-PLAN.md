<!--
  MobilityDB — The temporal rigid geometries: implementation plan
  Copyright(c) MobilityDB Contributors

  This documentation is licensed under a Creative Commons Attribution-Share
  Alike 3.0 License: https://creativecommons.org/licenses/by-sa/3.0/
-->

# The temporal rigid geometries: the implementation plan

[Back to the index](00-INDEX.md) · [Assessment](ASSESSMENT.md)

Each gap of [the assessment](ASSESSMENT.md) and the pull request that closes it, in order.

## The manual

| Row | Task | State |
|---|---|---|
| TG1 | State every refusal and every wrong answer of a 3D value in `doc/temporal_rigid_geometries.xml` and its Spanish twin, each where its function is documented, state what the type accepts in place of "2D" there and in `doc/contributing/trgeometry_design_notes.md`, and make the examples of the chapter run | merged as #3443 |

## Wrong answers

| Row | Task | State |
|---|---|---|
| TG2 | The ever and always relationships of a 3D value: `eIntersects`, `aDisjoint`, `eDisjoint`, `eContains`, `eCovers`, `eTouches` | #3347 merged (`a5efd7d54e`), by reading its kernel; not yet run on a 3D value |
| TG3 | The temporal relationships of a 3D value: `tIntersects`, `tDisjoint`. A body bounded by a closed polyhedral surface is read as its faces: at `3aa3c58046`, a tetrahedron at the origin and `Point(0.1 0.1 0.1)` strictly inside it answer `tIntersects` false and `tDisjoint` true (172_trgeo_tempspatialrels), since a surface parsed from text carries no solid flag and the 3D distance of liblwgeom measures to the faces. A point on a face of a body in step interpolation answers true, and a flat body at z = 1 with `Point(1 1 1)` on it answers true then false as its 2D control, since #3450 (TG25). Linear interpolation in 3D is TG19 | #3572 open as a draft (`95cd6bf8dc`), not at parity, every receipt held: CGAL on 400 rows of the solid corpus; reads Z by decision 1 and the solid by decision 5; branch `fix/trgeometry-3d-relationships-read-z`: the constructors flag a closed reference surface as a solid (`geom_solid_from_closed`), and the face kernel of `geom_intersects3d` reads the inside of a solid by the parity of a ray whose line meets no edge (`geom_faces_solid_point`). A corpus of 400 bodies (box, cube, two tetrahedra, an L-prism that is not convex) at poses exact in doubles, judged by an exact oracle in rationals, agrees on 400 at the branch and 296 at `3aa3c58046`, every difference a point inside read as outside; 400 points on, above and beside tilted triangles, which are no solids, agree at both. Its cost on those 400 rows, both arms reading the serialized operands under callgrind, is 15.5 million instructions against 10.2 million for `lwgeom_mindistance3d_tolerance` at 0, 1.52 times, the exact signs of `vector_triple_sign` 40 percent of it and the reading of the faces 30, so the pull request opens as a draft. Replayed on the head of MobilityDB #3484 (`8c99c3c711`), whose signs of faces read doubles first and whose faces are read in place on the serialized form, with `geom_faces_init` reading a solid at the top, the same rows cost 9.8 million against 10.2, 0.97, and CGAL agrees with the kernel on 400: TG3 reaches parity stacked on #3484, which carries its solid into the reader of #3484 |
| TG4 | The restrictions by a box of a 3D value: `atStbox`, `minusStbox` | open, reads Z by decision 1 |
| TG5 | The merged `traversedArea` of a 3D value answers the ground its 2D projection covers, which it does not in a turn | open |
| TG26 | The merged `traversedArea` of a 2D body that turns joins placements 0.05 rad apart by their hull, which is the ground it covers only when it does not turn: on rows 1094 and 1153 of the corpus of the exact rigid distance it reaches a geometry the body stays 0.042 and 0.045 from, and `trgeometry_convex_sweep` states the hull as what the body covers. A translating body is exact: a contact through a vertex the union constructs reads as apart in doubles, as any region of rounded vertices does | merged as #3515 (`f3929f747b`); CGAL judges its 400 turning rows of the corpus equal to the exact union of its pieces |
| TG23 | `eIntersects` and `tIntersects` of two 2D values agree on the car of nuScenes that touches the ego vehicle | #3347 merged (`a5efd7d54e`), whose witness shows the lifting compares poses; not yet run on that car |

## Defects

| Row | Task | State |
|---|---|---|
| TG6 | An SRID stated before the pose in a literal is held rather than dropped, and `atStbox`, `minusStbox` take a box of time alone, as `atTime` does | merged as #3521 and #3522 |
| TG7 | `stops` over a `trgeometry` and a `tpose` answers, or its declaration goes | merged as #3540, by decision 2 |
| TG8 | `tIntersects`, `tDisjoint` of a 3D surface answer, or refuse in words of the type | merged as #3543 (`f6eb676881`): a polyhedral surface, a TIN or a curve is forced to 2D without the geodetic walk, so `tIntersects` and `tDisjoint` answer in step interpolation and are refused in linear as having Z |
| TG9 | `round` of a 3D rotation keeps the quaternion of unit norm | merged as #3520 |
| TG10 | `tDistance` of a 3D value and a 3D `tgeompoint` names the 3D refusal | merged as #3519 |
| TG11 | The relationships and `tDwithin`, `eDwithin`, `aDwithin` of a 2D value read a 3D operand in 2D, as `tgeo_different_dimensions` states | #3569 open (`1f6269c7e6`), its receipts held: CGAL on 1107 rows of the exact rigid distance corpus, each geometry given with Z, `distance_geo` 0.174 of GEOS; branch `fix/trgeometry-temporal-intersects-reads-z`: every entry relating a 2D value and a geometry reads a geometry with Z in 2D through `trgeometry_geo_common_dims`, as the distances do; at `3aa3c58046` each raised "Operation on mixed 2D/3D dimensions" while `\|=\|` of the same operands answered 0 |

## Refusals to lift

| Row | Task | State |
|---|---|---|
| TG12 | The distance of a concave, holed or surface body and to a concave or holed polygon | [its note](NON-CONVEX-DISTANCE.md) |
| TG13 | The relationships of a linear rigid geometry | [its note](RIGID-SWEPT-RELATIONSHIPS.md); `intersects` and `disjoint` merged as #3497 (`266f7ed2bb`, `59f3e1305a`), `contains`, `covers` and `touches` open |
| TG14 | The distance to a collection and to a curve | open; to reconcile with [the note](NON-CONVEX-DISTANCE.md) |
| TG15 | `atGeometry`, `minusGeometry` over a geometry with holes | open |
| TG16 | `transform`, `transformPipeline` | merged as #3541 (`67771c197f`), by decision 3 |
| TG17 | `asMFJSON`, `round` over a surface | open |
| TG18 | The relationships and restrictions of a 3D value against a geometry with Z | open, reads Z by decision 1 |
| TG19 | The distance of a value in 3D | open, decision 4 |
| TG25 | A point at the mean of the vertices of a polygon reads as away from it in 3D, in `eIntersects` and the distance, since `project_point_on_plane` of the vendored liblwgeom returns without setting its output; every temporal type measuring through it, and PostGIS itself | merged as #3450 |
| TG24 | `force2D(trgeometry)` and `force2D(tpose)`, in MEOS `trgeometry_force2d` and `tpose_force2d`, in place of `EgoBody2D` and `ObjectTrip2D` of the companion, refusing a body with real height; decided by the owner | merged as #3448; the book and its companion read it |

## Regularity

| Row | Task | State |
|---|---|---|
| TG20 | The array forms of the relationships, and the relationships at the instants of their inputs | merged as #3347 (`a5efd7d54e`): an always relationship of two step values reads every instant, and `contains` and `covers` take their array forms |
| TG21 | The portable names `geoRound`, `geoTransform`, `geoLength`, `geoTranslate`, `geoRotate` by the geometry a `trgeometry` answers at an instant | open |
| TG22 | The index test `162_trgeo_indexes_tbl` at the level of every operator of the three classes | merged as #3444 |
| TG28 | `ensure_same_geom`, which tells whether two values share their reference geometry in a merge, a sequence and a sequence set, compares the points of the two geometries within `MEOS_EPSILON` (`same_lwgeom`), so two bodies that differ by less than it are taken as one | open |
| TG27 | A generic temporal function answers a `trgeometry` as the SQL function of its name does, in every binding: the dispatch to the `trgeometry_*` function lives in MEOS, as `temporal_merge`, `temporal_merge_array` and `temporal_append_tinstant` hold it, not in the PostgreSQL wrapper, and the `trgeometry_*` functions it reaches are internal | DECIDED BY THE OWNER 2026-11-11 ("ok", to the 31 `trgeometry_*` functions leaving the public API): each public `temporal_*` function of these signatures tests its arguments and dispatches a `trgeometry` to its `trgeometry_*` function, the PostgreSQL wrapper calls the `temporal_*` function alone, and the `trgeometry_*` functions are internal (`@ingroup meos_internal_*`), asserting the conditions they test today. Branch `meos/trgeometry-dispatch-in-meos` (`12542627b8`, over `898a08c288`), in its receipts: the generic functions strip and attach the reference geometry, as `temporal_round` does; the 24 wrappers call them alone; 27 `trgeometry_*` functions and the five `trgeometry_restrict_*` they called are gone; `trgeometry_merge`, `trgeometry_merge_array`, `trgeometry_append_tinstant` and `trgeometry_append_tsequence` are internal, the generic functions testing the same body. The 93 answers of the generic functions on a linear sequence, a step sequence and a sequence set equal those of the twins before; `meos/test/trgeo_generic_test.c` makes 69 checks, none failing, and aborts over the library before. It found two defects of the generic step code, the twins reaching them as well, each fixed apart: `temporal_minus_timestamptz` and `temporal_delete_timestamptz` of `tfloat 'Interp=Step;[1@2001-01-01, 2@2001-01-02]'` at 2001-01-02 read past the last instant (`tcontseq_minus_timestamp_iter` takes the instant after the n-th where t is the last one) and end on a segmentation fault; `temporal_delete_tstzspanset` of `tfloat 'Interp=Step;[1@2001-01-01, 2@2001-01-03, 3@2001-01-05]'` and `{[2001-01-02, 2001-01-02 12:00], [2001-01-04, 2001-01-05]}` connecting what remains stops with `Invalid end value for temporal sequence with step interpolation`, and answers without connecting. At MobilityDB `3a4e3add5f`, 24 PostgreSQL wrappers of `mobilitydb/src/temporal/temporal.c` choose `temp->temptype == T_TRGEOMETRY ? trgeometry_X(...) : temporal_X(...)` (`Temporal_instants`, `Temporal_start_instant`, `Temporal_after_timestamptz`, `Temporal_round`, the restrictions through `trgeometry_restrict_*`, ...), so the SQL answers, while a binding calling the public `temporal_*` function does not dispatch: the typed SQL surface of Spark registers `instants`, `startInstant` and `atTime` of a `trgeometry` through `temporal_instants`, `temporal_start_instant` and `temporal_at_timestamptz`. Over libmeos at that commit and two values (a linear polygon of three poses, a step square of two), `temporal_instants` and `temporal_start_instant` stop with `Cannot access geometry from temporal rigid geometry` and `temporal_after_timestamptz` crashes where the `trgeometry_*` twin answers; `round`, `atTime`, `minusTime`, `deleteTime` and `sequences` answer as the twin on both. The catalog gives the 31 `trgeometry_*` functions the SQL signatures of the generic ones, so a generator registering a signature once takes the generic function |

## Decisions

1. **What a relationship of two 3D values reads: Z, where a kernel in three dimensions exists.** Decided by the owner. A
   `trgeometry` has three dimensions, as Chapter 4 uses it, and the rule of
   `tgeo_different_dimensions` reads Z where both operands have it, so `eIntersects` and `tIntersects`
   agree. The nuScenes bodies at z = 0 and z = 0.8 then never meet in 3D, and a question about the road
   goes through `force2D`, TG24. Containing, covering and touching have no such kernel, so they refuse Z
   as `ea_touches_tgeo_geo` does.
2. **`stops`: over the positions of the poses.** A temporal pose stops where its positions stay
   within the area, as a temporal circular buffer stops where its centre does, so a pose turning in
   place is one stop, and a temporal rigid geometry stops where its temporal pose does, keeping its
   reference geometry. TG7.
3. **`transform`: the poses, and the body in its own frame.** The reference geometry is the shape
   of the body in its body frame, as AIS and GeoPose give it, so a change of the reference system
   transforms the poses, each orientation turning by the convergence of the meridians at its
   position, and keeps the reference geometry. The body is placed in the plane of its poses, so a
   geographic target is refused. TG16. A circular buffer, planar for the same reason, refuses a
   geographic target too (#3542), and the build requires PROJ 7.1 for `proj_degree_output`.
4. **The distance in 3D.** Bodies gain faces, and a quaternion replaces the one angle the solver of
   `trgeo_distance.txt` is built on, so this is a new algorithm and a change to Chapter 4.
   **The recommendation** is to write its algorithm as a note of this plan before any code, and to
   state the limit in the manual meanwhile.
5. **The body a closed polyhedral surface bounds: the solid.** Decided by the owner. A reference
   geometry that is a closed polyhedral surface is the solid it encloses, so a point inside
   it meets the body in the ever, temporal and distance relationships alike, as PostGIS reads a
   value of `ST_MakeSolid`; `ST_3DIntersects` reads the same text, which carries no solid flag, as
   its faces alone, and the manual states the difference. An open surface is its faces. The
   containment is exact: a test of a point in a solid by exact signs, beside `geom_faces_meet`,
   since the one of liblwgeom, `lwgeom_solid_contains_lwgeom`, clips the solid, which constructs
   vertices, retries at a boundary after a random skew, and reads the first point of the other
   geometry alone. TG3.
   The solid flag and the MEOS flags are separate fields: the MEOS flags are `Temporal->flags`,
   and PostGIS keeps the solid flag in the serialized geometry, in the 8 bytes of extended flags
   of its version 2. MEOS reads the Z and geodetic flags of a geometry through the PostGIS macros
   and writes its own through `MEOS_FLAGS_SET_*`, and reads no geometry at an offset of its header,
   so the extended flags shift nothing. Four consequences are TG3's to meet: text and WKB do not
   carry the flag, so the constructor sets it from the closedness of the reference geometry and
   never takes it from the input, and every input and rebuild agrees; a solid reference geometry
   is 8 bytes longer than the same geometry without the flag, so the reference geometry a value
   returns is tested against the geometry of its constructor; `lwgeom_force_2d` and the placement
   at a pose are tested to keep the flag; and `geom_faces_add` refuses a solid, so the exact test
   of a point in a solid sits in the face kernel.

## The order

TG1 first, since it only states what holds. TG24 next, since the use case of the book needs it and it
decides nothing. TG2 to TG5 and TG23 then, since they are the wrong answers. TG6 to TG11 in any order. The refusals as each is taken up, each removing its
own note from the manual.

## Merged

| Subject | Pull request |
|---|---|
| Operands of different dimensions in the distance | #3439 |
| The exact temporal distance at instants and in segments | #3375 |
| The value at a timestamptz | #3336 |
| The generated ever and always relationships, distances and temporal relationships | #3341, #3350, #3359 |
| The generated nearest approach distance | #3335 |
| The internal functions named after the type | #3396 |
| The constructors named in the MEOS functions they call | #3166 |
| The exact merged traversed area | #3097 |
| The aggregates keep the reference geometry, and the distance of a gap | #3100, #3102 |
