<!--
   ****************************************************************************
    MobilityDB / MEOS Specification — Exact Geometry
    Copyright(c) MobilityDB Contributors

    This documentation is licensed under a Creative Commons Attribution-Share
    Alike 3.0 License: https://creativecommons.org/licenses/by-sa/3.0/
   ****************************************************************************
-->

# Exact Geometry — Engine Specification

| | |
|---|---|
| Version | v0.4.2 |
| Scope | What the native geometry engine of MEOS answers, how it computes the answer, and what lies outside it; this version covers distances, predicates, relationships, areas and buffers |
| Reference | `meos/src/geo/geo_funcs.c` (the exact kernels), `meos/src/geo/tgeo_distance.c` (the distances of moving values), `meos/src/geo/geo_buffer.c` and `meos/src/geo/geo_arrange.c` (the buffer and the arrangement it is read off), `meos/test/distance_point_test.c`, `meos/test/geo_test.c` and `mobilitydb/test/geo/sql/049_geo_funcs.sql` (the witnesses quoted below) |

## 1. Scope and intent

MEOS computes geometry with its own engine, written in C inside MEOS: MEOS and
MobilityDB build and run without GEOS. Every answer the engine gives about a
geometry is the exact answer about the numbers the geometry holds, or the
double nearest that exact answer where the answer is a number. Its rule is
that a question about the coordinates of the input is decided with no
tolerance, so that no answer depends on two values being "close enough";
section 3 states where the engine still reads one.

This document describes that guarantee from the point of view of a user of
MEOS, of MobilityDB or of a binding generated from MEOS: what an exact answer
is, how the engine reaches it at the speed of a floating-point engine, which
operations it covers, and what lies outside it. It is descriptive of the
engine and prescriptive for every operation it covers.

## 2. Notation

A *double* is an IEEE 754 binary64 number, the type of every coordinate MEOS
holds. Every finite double is an exact rational number: `0.1` written in a
query is read as the double nearest one tenth, which is the rational
`3602879701896397 / 2^55`, and from then on that rational is the coordinate.

The *exact answer* of a question about doubles is the answer of the same
question about those rationals, computed without rounding.

The *nearest double* of a real number is the double closest to it; a number
exactly between two doubles goes to the one whose last bit is even, as IEEE
754 rounds. A *unit in the last place* (ulp) of a double is the gap between it
and the next double.

## 3. The rule

MEOS answers a geometric question about the doubles it holds.

- A **predicate** (does a point lie on a segment, do two segments cross, is a
  ring oriented counterclockwise, is one value within a distance of another)
  answers true or false as the exact rationals do.
- A **measure** (a distance, a length of time, a nearest approach) answers the
  double nearest the exact value.
- Where the engine cannot give either answer, it raises an error. It never
  answers an approximation in their place.

Two consequences matter in practice.

**The answer does not depend on the scale of the coordinates.** Two points a
metre apart and the same two points scaled by `2^500` give the same answer
scaled by `2^500`, and a question asked in projected metres near `5e6` is
decided as exactly as one asked near the origin. A tolerance cannot do this:
any fixed tolerance is too large at some scale and too small at another.

**The answer is about the input, not about an intermediate value.** A value
that MEOS would have to construct and round, such as a point placed between
two instants or a disc drawn as a polygon, is not built and then measured. The
engine computes from the doubles that define it, and rounds only the answer
(section 5.3).

**A tolerance remains only at a point the engine constructs.** Where an
operation must build a point and then ask a question of it, such as a corner
of a buffer computed from two offset edges, the built point is rounded, so a
question asked of it exactly would be about that rounding. There the engine
compares within a distance sized to the rounding of the construction, in one
of two formulas, with `m` the largest absolute coordinate of the points
compared:

    coordinate_rounding   = 4 * DBL_EPSILON * m
    coordinate_tolerance  = MEOS_GEOM_TOLERANCE + 4 * DBL_EPSILON * m

`coordinate_rounding` is the distance a point constructed from coordinates of
that size misses by, with no absolute term; `coordinate_tolerance` adds
`MEOS_GEOM_TOLERANCE`, equal to `1e-12`.

| Band | Where | The constructed point it judges |
|---|---|---|
| `coordinate_rounding` | the lift of the Z and M of an overlay's answer (`meos_lift_ordinates`, `lift_point_read`, `arc_ordinates_at`); the relate engine: two constructed points read as one (`relate_same_coordinate`) and a constructed point read on a boundary within the band each edge carries (`edge_set_tolerance`, `relate_point_on_boundary_scan`) | a vertex the overlay constructs, read on a vertex, a segment or an arc of an input; a point the relate engine constructs |
| `coordinate_tolerance` | the buffer (`buffer_join_is_point`, `buffer_piece_contains_point`, `buffer_add_arc`, `buffer_ring_rebuild_at_nodes`); the ordinates an overlay's vertex takes from its inputs once an arc gave the first (`lift_point_take`); a constructed point read on a segment of the relate engine where an arc meets a segment or lies along a boundary (`point_on_segment` in `relate_point_in_linear` and `relate_linear_area`) | a corner, a join, a piece end or the midpoint of a piece the buffer computes; a vertex an overlay constructs, read along the angle of an arc; a point the relate engine constructs |
| `coordinate_tolerance`, as a filter | the candidates of the buffer's node index (`buffer_node_index_cand`); the edges a disc can reach (`tcbuffer_disc_boundary_sign`); two extents or two members of the relate engine apart (`relate_extents_apart`, `relate_members_apart_within`); the segments of two areal boundaries an index admits (`geom_areal_touching_stretches`) | none: the band only admits candidates, and an exact test decides each, so a wider band costs time and never an answer |
| `tnpoint_meeting_band` | the ever and always overlaps of two temporal network points (`eaoverlaps_tnpoint_tnpoint`) | the positions of the two values at one instant, each within the bound of construction of its route, the band being the sum of the two bounds, with no absolute term |

The band is a distance in the units of the coordinates. It decides whether two
constructed points are one point, never a question about the input, which the
engine decides exactly. The cast of a geometry to a network point decides
exactly whether a segment of the route meets the box of the bound of the
point's construction (`segment_meets_point_box`), and the meeting of two
temporal geography points is decided where the ends of their segments prove
it; neither reads a band. Nor does the sewing of the lines a merge of temporal
geometries unites, whose ends are input vertices and are one point where their
coordinates are equal (`sew_same_point`), nor whether the crossing of two
straight edges of the relate engine is in the boundary of a line, decided on
the input vertices that could be that crossing (`relate_crossing_in_linear`),
nor whether a straight line runs along a straight boundary, decided by the
exact collinearity of their input segments (`relate_boundary_piece_covered`,
`relate_line_piece_on_boundary`), nor the normalization of a sequence: whether
three instants are collinear is a question about the input, decided exactly
(`float_collinear`, and `geopoint_collinear`, `pose_collinear` and
`cbuffer_collinear` on it).

The absolute term of `coordinate_tolerance` is not the rounding of a
construction: below a largest coordinate of about `1.1e3`, where
`4 * DBL_EPSILON * m` falls under `1e-12`, it widens the band beyond the
rounding it stands for. An answer that a band of `coordinate_tolerance`
decides, in the second row of the table, is therefore outside this document's
guarantee where that term rules, as the comparisons with a fixed tolerance
below are; in the filter row it costs time and never an answer.

**A tolerance is never an accuracy of the output.** `MEOS_GEOM_TOLERANCE`
and `MEOS_EPSILON` (`1e-6`, the equality of two modelled values, which the
microsecond of a timestamp pins) are the two tolerances MEOS owns. A value
MEOS would have to build to a stated accuracy, such as a polygon following a
curve the geometry cannot state, would need a third, and MEOS does not build
it: the geography of a geodetic box is the polygon of its four corners, whose
edges are great circles where the box's horizontal edges are parallels.

**Some comparisons still read a fixed tolerance.** Parts of the engine compare
a length or a parameter with `MEOS_GEOM_TOLERANCE` or `MEOS_EPSILON`, among
them the ray crossing of a point in a polygon (`dist_poly_seg_raycross`), the
closest point on an arc (`dist_geom_closest_on_arc`) and the turning points of
the distance of a moving point (`tgeogpointsegm_distance_turnpt`). Their answers
are not covered by this document's guarantee. The answers this document
states are the ones the CGAL judges check (section 10).

## 4. How the engine reaches an exact answer

Most questions are easy, and a few are hard: two segments that clearly cross
are told apart from two that clearly do not in a few floating-point
operations, while two segments that meet at an endpoint, or a point one ulp
from a line, need more. The engine answers in two layers.

1. **A filter in doubles.** The question is first computed in ordinary
   floating-point arithmetic together with a proven bound on its rounding
   error. Where the bound shows the computed value cannot be on the wrong side,
   that answer is the exact answer, and nothing else runs. This is the path of
   nearly every call, and it costs about what a plain floating-point
   computation costs.
2. **An exact computation** runs only where the filter cannot decide: sums and
   products of doubles kept without rounding (expansions of doubles), and
   wide integers where those leave the range of a double. A distance is found
   by comparing the exact squared distance with the exact squares of the
   midpoints between neighbouring doubles, so no square root of a rounded
   value decides it.

Coordinates far from 1, such as `1e200` or `1e-200`, are first scaled by a
power of two, which is exact, so that their squares and products neither
overflow nor underflow. The same answer comes out at every scale.

A filter never changes an answer: a filter that cannot decide only costs time.
The speed of the engine is measured against GEOS called directly, operation by
operation, and a slower cell is answered by adding a filter in front of the
exact layer, never by giving up exactness.

## 5. Distances

### 5.1 Two static geometries

The distance between two geometries of points and straight edges is the
double nearest the exact distance, in two and three dimensions. In MobilityDB,
`distance(geometry, geometry)` answers it; in MEOS, `geom_distance2d` and
`geom_distance3d`.

| Operands | Answer | Why it needs care |
|---|---|---|
| `POINT(0 0)`, `POINT(1e39 0)` | `1e39` | the distance exceeds the largest single-precision float, which an engine starting its search from that value answers instead |
| `POINT(0 0)`, `POINT(1e200 1e200)` | `1.414213562373095e+200` | the squared distance, `2e400`, is not a double: a formula that squares answers infinity |
| `POINT(0 0)`, `POINT(1e-200 1e-200)` | `1.414213562373095e-200` | the squared distance underflows to 0: a formula that squares answers 0 |
| `POINT(5 5)`, a square with a hole about it | `1.0` | the point lies in the hole, so the distance is to the hole's ring |

Two geometries that meet are at distance `0`, decided by exact orientations,
whatever their scale.

### 5.2 Moving values over continuous time

MEOS reads time as continuous. Between two instants a moving point moves along
a segment, and its nearest approach to another value is the least distance
over every time of that interval, not over the microseconds that a timestamp
can name. In MobilityDB, `nearestApproachDistance` and the operator `|=|`
answer it; `tDistance` and `<->` answer the distance at each time.

| Operands | Answer |
|---|---|
| a point crossing from `(0 0)` to `(1000000 0)` in a day, and a point at rest at `(123456.789 1)` | `1`, reached at a time between two microseconds, where the distance at every microsecond is above 1 |
| a point going east from `(0 0)` to `(7 0)` in a day, and a point falling from `(0 1)` to `(7 -6)` | `0`, the two meeting at a seventh of the day, which is no whole number of microseconds |

Whether two moving values are ever or always within a distance (`eDwithin`,
`aDwithin`) is decided by the same exact computation over continuous time.

The instant MEOS returns for such an event, as `nearestApproachInstant` does,
is a timestamp, a whole number of microseconds: the exact time truncated to
the microsecond. The distance reported is the one at the exact time.

### 5.3 Values that MEOS does not construct

Some values are defined by doubles from which a geometry would have to be
constructed and rounded before it could be measured. The engine measures from
the defining doubles instead.

| Value | Not constructed | Measured from |
|---|---|---|
| a point between two instants | the interpolated point | the two positions and the two timestamps |
| a circular buffer | the disc as a polygon | the centre and the radius: the distance to the centre less the radius |
| two circular buffers | the sum of the radii rounded to a double | the two radii, their sum kept exactly |

A circular buffer of radius `0.2271214713895495` moving along the line `y =
0.22712147138954955`, just above the line `y = 0`, is at distance
`5.5511151231257827e-17` from it: the exact difference of the two doubles.

Because the answer is about the defining doubles, it can differ in its last
bits from the distance measured on a value MEOS returns rounded: the point
`valueAtTimestamp` returns between two instants has rounded coordinates, so
the distance of that point to a geometry can differ by a few ulps from the
temporal distance at that time.

## 6. Predicates and relationships

### 6.1 Questions about the input

A question that reads only the doubles of the input is decided exactly, at any
scale, by the filter and the exact layer of section 4: the side of a line a
point lies on (`cross_product_sign`), whether two segments meet
(`linesegm_intersect`), and whether the distance of a point to a segment is
less than, equal to or more than a given distance
(`point_segment_offset_sign`).

### 6.2 Two static geometries within a distance

Whether two geometries of points and straight edges are within a distance in
two dimensions is the exact answer, and so is whether two geometries of points
and segments with Z are within a distance in three. In MobilityDB,
`dwithin(geometry, geometry, float)` answers both; in MEOS, `geom_dwithin2d`
and `geom_dwithin3d`.
The distance given is itself a rational, and the answer compares the exact
distance with it, whatever double the exact distance rounds to.

| Operands | Distance | Answer | Why it needs care |
|---|---|---|---|
| `POINT(0 0)`, `POINT(2 3)` | `3.605551275463989` | false | the distance, the square root of 13, lies just above this double, which is the nearest double to it |
| `POINT(0 0)`, `POINT(2 3)` | `3.6055512754639896` | true | the next double, above the distance |
| `POINT(0 0)`, `POINT(1 2)` | `2.23606797749979` | true | the square root of 5 lies just below the nearest double to it |
| `POINT(0.5 5e-324)`, `LINESTRING(0 0,1 0)` | `0` | false | the two are apart by the least double there is |
| a square, a point inside it | `0` | true | the point is on no edge, and the area holds it |
| `LINESTRING Z(0 0 0,2 0 0)`, `LINESTRING Z(1 -1 3,1 1 3)` | `3` | true | the two are nearest inside both, at a distance held by no end |
| `LINESTRING Z(-1 0 -2,1 0 2)`, `LINESTRING Z(0 -1 5e-324,0 1 5e-324)` | `0` | false | the least double over the root of 5, which rounds to 0 |

A geometry carrying an arc is answered exactly in two dimensions, and the table
holds whichever double the distance rounds to:

| Geometries | Distance | Within | Why |
|---|---|---|---|
| `POINT(1 1)`, `CIRCULARSTRING(5 0,3 4,0 5)` | `3.585786437626905` | true | the least double over `5 - sqrt(2)`, the radius less the distance to the centre |
| `POINT(1 1)`, `CIRCULARSTRING(5 0,3 4,0 5)` | `3.5857864376269046` | false | the double under it |
| `POINT(3.0000000000000004 4)`, `CIRCULARSTRING(5 0,3 4,0 5)` | `0` | false | the least double off the circle |
| `POINT(1.0000000000000002 0)`, `CURVEPOLYGON(CIRCULARSTRING(1 0,-1 0,1 0))` | `0` | false | the least double outside the disk |

So is a triangle, a TIN and a polyhedral surface, each face a surface as a polygon is:

| Geometries | Distance | Within | Why |
|---|---|---|---|
| `POINT(3 3)`, `TRIANGLE((0 0,4 0,0 3,0 0))` | `1.7999999999999998` | false | the foot on the hypotenuse lies `9/5` from the point, above this double |
| `POINT(3 3)`, `TRIANGLE((0 0,4 0,0 3,0 0))` | `1.8` | true | the double nearest `9/5` lies above it |
| `POINT(4.5 4.5)`, a surface whose square has a hole about the point | `1.5` | true | the hole holds the point, `1.5` from its edge |
| `POINT(4.5 4.5)`, that surface with a face filling the hole | `0` | true | another face holds the point |

In three dimensions, a triangle, a TIN, and a polygon or a polyhedral surface whose faces each lie
exactly in a plane are answered exactly. A face is the region its rings bound in its plane, holes
included. Two geometries are within the distance where two of their segments are, where a vertex of
one lies within it of a face of the other with its foot inside the face, or where a segment of one
crosses a face of the other; each is decided by exact signs on the input coordinates, and no point is
constructed.

| Geometries | Distance | Within | Why |
|---|---|---|---|
| `POINT Z(0 0 0)`, `TRIANGLE Z((1 0 0,0 1 0,0 0 1,1 0 0))` | `0.5773502691896257` | false | the foot lies inside the face on the plane `x + y + z = 1`, `1/sqrt(3)` away, just above this double |
| `POINT Z(0 0 0)`, `TRIANGLE Z((1 0 0,0 1 0,0 0 1,1 0 0))` | `0.5773502691896258` | true | the next double |
| a square at height 0, a larger triangle parallel to it at height 3 | `3` | true | the faces are parallel, and the distance is reached at every vertex of the square |
| `POINT Z(4.5 4.5 1)`, a square with a hole about the point's foot | `1.8027756377319946` | false | the foot falls in the hole, so the nearest point is on its edge, `sqrt(3.25)` away, above this double |
| `LINESTRING Z(1 1 -5,1 1 5)`, `TRIANGLE Z((0 0 0,4 0 0,0 4 0,0 0 0))` | `0` | true | the segment crosses the face, far from its edges |

A polygon whose vertices do not lie exactly in one plane, and a solid, compare the given distance with
a distance in doubles, so the answer can differ from the exact one where the two lie within its
rounding. A geometry carrying an arc raises an error, as its distance in three dimensions does.

### 6.3 Relationships

The relationships of two geometries, `intersects`, `contains`, `covers`,
`touches`, `disjoint` and the DE-9IM matrix of `relate`, are not exact answers
of this engine. A relationship of two geometries is about the points where
their boundaries meet, and two edges crossing cross at a point that is in
general no pair of doubles. The engine builds that point, rounded, and classifies
it against the other geometry within a tolerance sized to the construction
(`coordinate_tolerance`, `MEOS_GEOM_TOLERANCE`, section 3), in the functions of
`meos/src/geo/geo_funcs.c` named `relate_*`.

Asking an exact question of that rounded point would answer about its rounding,
not about the input: the point is not on the edge it was built from. A decision
with no tolerance therefore needs the arrangement of the two geometries, whose
vertices are held exactly and never located by a rounded coordinate, and the
relationship of two areal operands read off that arrangement.

A collection of areal members, such as a multipolygon or the multisurface a
buffer answers, is read as the union of its members. An edge of one member is
cut only where an edge of another member meets it: the boundary of one member
meets itself only at the vertices its rings share, where its edges already
end, so two adjacent edges of one member, an arc and the segment it turns into
included, cut nothing. A point lying in one member therefore lies in the union
whatever the other members are.

The temporal relationships inherit this: at an instant they read the same
engine.

## 7. Areas

### 7.1 The answer

The area of a geometry of straight edges is half the sum of the shoelace sums
of its rings, the exterior counted positive and each hole negative. It is a
rational of the coordinates, and `geom_area` (`area` in SQL) answers the double
nearest it, a tie going to the even one, whatever the magnitudes of the
coordinates, from the subnormal doubles to `±DBL_MAX`. An area beyond
`DBL_MAX` raises `value out of range`. A geometry holding a circular arc keeps
its area in doubles, since its circular segments are not rationals of its
coordinates (see [Finite Values](meos-finite-values.md)).

A sum of the shoelace terms in doubles rounds at every term, and the terms of
a ring cancel: a thin hole, a sliver or a vertex far from the others leaves an
area much smaller than the terms that make it. The witnesses below, tests of
the tree (`meos/test/geo_test.c`), set the answer of the engine beside that of
a sum in doubles, as GEOS 3.14's `GEOSArea` computes it:

| geometry | `geom_area` (the nearest double) | a sum in doubles |
|---|---|---|
| a square of side about 100 at projected coordinates, less a hole within `1e-9` of its edges | `3.927212674136797e-07` | `3.9272163121495396e-07` |
| a triangle reaching from subnormal coordinates to `1e307` | `3.3118017121875465e-17` | `3.3118017121875471e-17` |
| a rectangle of `1e154` by `1.7e154` | `1.7e308` | infinity |
| a unit square and a triangle of legs `2^-26`, an area of `1 + 2^-53` | `1`, the tie to the even double | `1` |
| a square of side `1.5e154`, an area beyond `DBL_MAX` | raises `value out of range` | infinity |

How often a sum in doubles differs from the nearest double, judged by CGAL
(section 10) on three sets of geometries; `geom_area` answers the nearest double
on every one of their rows:

| geometries | a sum in doubles differs on | by |
|---|---|---|
| 192 multipolygons of real data at projected coordinates | 145 of 192 | at most 29 ulps, 1 ulp on 53 rows |
| 2000 shapes built to cancel (thin holes, slivers, degenerate rings) | 978 of 2000 | 1 ulp on 462 rows, more than `1e9` ulps on 76 |
| 1000 geometries from the subnormal doubles to `±DBL_MAX` | 236 of 1000 | more than `1e9` ulps on 48, infinity on some finite areas |

On data at ordinary magnitudes the two agree to about 14 significant digits,
and differ in the last ones: two systems, or two orders of the same rings, may
not give the same double. Where the terms cancel or the magnitudes are
extreme, the sum in doubles may differ from the first digits on.

### 7.2 How the engine reaches it

Each ring is read once, in the layers of section 4, the first that applies
answering:

1. **Integers.** Where the binary exponents of all the coordinates of a ring
   lie within 4 of each other, every coordinate is an integer multiple of one
   power of two. The exponents are bounded once per geometry by its stored
   box, whose corners are rounded outwards to floats, wherever the box keeps
   clear of 0 and of the largest float on an axis; elsewhere they are read
   once per ring from the bitwise AND and OR of the bit patterns. The ring is
   summed in 128-bit integers, one product per vertex in the form
   `x_i (y_(i-1) - y_(i+1))` GEOS uses, exactly and so on the coordinates
   themselves, four vertices a step. This is the path of the geometries data
   holds.
2. **A compensated sum in doubles.** Elsewhere, the same one product per
   vertex, about the first vertex, is split into its rounded value and its
   exact error (Dekker's product), the rounded values are summed exactly, and
   the errors are summed with a bound on their rounding. The one search for the
   nearest double places half that sum against the midpoints of the doubles;
   where the bound shows which double is nearest, that is the answer.
3. **Wide integers.** Where the bound cannot decide, or a product leaves the
   doubles, the shoelace sums are computed exactly in wide integers and
   rounded once.

### 7.3 Its cost

The speed of a MEOS operation is stated against two references, each as the
cost of the reference divided by the cost of MEOS, counted in instructions, so
that above `1` MEOS is faster and below `1` slower:

- the **kernel speed**, against GEOS called directly on geometries already
  converted to it;
- the **route speed**, against GEOS called on a MEOS geometry, the conversion
  to GEOS and back included.

| geometries | kernel speed | route speed |
|---|---|---|
| 192 multipolygons of real data | 0.82 | 8.9 |
| the 1871 polygons and multipolygons among the 2000 shapes | 0.70 | 6.4 |

The area is the one measure of this engine whose exact layer runs on nearly
every call: a sum that cancels has no bound in doubles that decides its nearest
double cheaply, so the integers are the first layer, not a filter in front of
them. The answer is the nearest double on every call. The integer layer takes
about 14 instructions a vertex, three of them converting each coordinate to its
integer, where the sum in doubles of GEOS takes about 13, and a
ring whose coordinates span more than 4 binades, as degrees across an axis do,
takes the compensated sum.

## 8. Buffers

### 8.1 The answer

The buffer of a geometry at a distance `r` is the set of the points whose
distance to the geometry is at most `r`. A negative distance asks for the
erosion of an areal geometry: the points it holds that lie at least `r` from
its boundary. In MobilityDB, `buffer(geometry, float)` answers both; in MEOS,
`geom_buffer`.

The engine reads the buffer off the exact arrangement of the offset curves of
the geometry, for the inputs and styles below; the answer keeps its circular
arcs as arcs (a `CURVEPOLYGON`, or a `MULTISURFACE` of them).

| Input | Distance | Style | Answer |
|---|---|---|---|
| polygon, multipolygon | positive | round joins | the region the offset curves wind around at least once |
| line, multiline | positive | round joins, any cap | the same, each line walked out to its end and back under a round cap; under a flat or a square cap, the strips of its end segments and the buffer with round caps of the line between them |
| polygon, multipolygon | negative | round joins | the region the offset curves of the complement wind around clockwise |
| multipoint | positive | none | the region the circles about its points wind around at least once, a point repeating another dropped |
| circular string, compound curve, multicurve | positive | round joins, any cap | the region the offset curves wind around at least once, each curve walked out to its end and back, or, under a flat or a square cap, the strips of its end elements and the curve between them |
| curve polygon, multisurface | positive | round joins | the region the offset curves of its rings wind around at least once |
| geometry collection of points, lines, polygons, curve polygons and curves, a multisurface of polygons and curve polygons | positive | round joins, any cap | the region the offset curves of its members wind around at least once, each member offset as it is alone |

Round joins and round caps are the default style.

**Each piece of the curves is fixed by the input and the distance.** The offset
of an edge is the segment between the points where the common tangent of the
circles of radius `r` about its two ends touches them, and the join at a vertex
is the arc of the circle about that vertex. No point of the curves is
constructed and then asked about: every crossing of two pieces is decided by
the arrangement from the doubles that define them, and only the vertices of
the answer are rounded, each once.

**An arc is offset by an arc about its own centre.** The circle of an input
arc through its start, middle and end, or on the diameter of a whole circle's
half, has a centre `C` that is a rational of those doubles and a radius `R`
whose square is one. Its offset to the right of the walk is the arc about `C`
of radius `R + r` where it runs counterclockwise and `R - r` where it runs
clockwise, between the points of the circles of radius `r` about its ends on
the lines through `C`; where `R - r` is negative the offset runs on the far
side of `C`. Its circle, its ends and every crossing it makes are held in the
square root of `R^2` and the roots their meetings add, decided exactly. The
join at a vertex turns as the walk does there, the direction of an arc being
its tangent, and a whole circle is walked as its two halves. A ring of a curve
polygon turns counterclockwise where the shoelace sum of the ring through
every point fixing it, and the sense of every arc, agree on it: each arc adds
to that sum two circular segments of its own sign.

**The region is decided by the winding number.** The curves wind around a
point as many times as the parts of the geometry within the distance of it,
less the holes lying within that distance whole, so the buffer is the set of
points they wind around at least once, however the offsets of a narrow or
self-touching geometry cross; the buffers of the polygons of a multipolygon,
and of the lines of a multiline, are united in the one arrangement. A hole
whose smallest enclosing circle is no larger than `r` lies within the
distance of its own ring and is covered whole, the radius of that circle
compared with `r` exactly from the points that define it.

**A flat or a square cap is read off strips.** A flat cap leaves out the half
disc about an end, part of which the disc about the vertex before it may
cover; a curve turning back across the end would take that part from the
winding number too. The buffer is instead the union of three curves, each
winding a positive number of times around every point it covers: the strip of
each end segment, bounded by its two offsets and the chords across its ends,
and the buffer with round caps of the line between them, whose caps are the
discs of the joins at its second and its last but one vertices. A square cap
stretches the strip by `r` past the free end; its corners lie past the points
where the offsets touch the circle about the end by `r` along the segment,
`Ck + r (d + side rot(d)) / |d|`, which the arrangement holds as exactly as the
points where a tangent touches a circle. A closed line has no end and keeps
its round walk whatever the cap. A curve is read the same way, its end
elements segments or arcs. The strip of an arc of centre `C` and radius `R`
is the region its normal segment of half length `r` sweeps, bounded by its
two offsets about `C` and the segments across its ends on the lines through
`C`; a square cap moves the segment across the free end by `r` along the
tangent of the circle there, its corners `A + r (side n + along rot(n))`,
`n` the unit vector from `C` to the end `A`, in the root of the circle as the
ends of the offsets are. Where `R` does not exceed `r` the normal segment
sweeps through `C` into points beyond it that no one curve winds around with
the rest of the strip.

**The erosion is what the buffer of the complement leaves.** The exterior of
every polygon is offset clockwise into it and every hole counterclockwise out
of it; the unbounded part of the complement adds one to the winding number
everywhere, so the erosion is the set of points the curves wind around
clockwise at least once. That holds however a grown hole meets the contracted
exterior: inside it, around it, or crossing it. A polygon whose smallest
enclosing circle is no larger than `r` holds no point of its erosion.

**Where the arrangement leaves a question open**, the buffer of a polygon is
built on the older path, whose answers read the tolerances of section 3 and
are outside this document's guarantee; the buffer of a line is declined, since
no other path answers it exactly; the erosion of a polygon holding a ring that
encloses no area is built on the older path. So is the buffer of a lone point,
whose disc that path writes as its whole circle, of a curve under
a flat or a square cap ending in an arc whose radius is not shown to exceed
the distance, of a curve polygon ring whose arcs turn against its
shoelace sum, and of a curve polygon holding a hole of arcs, other than a whole
circle, that the distance may cover.

**The rings of a line's buffer carry a certificate.** The rings are written in
doubles, and the written end of every offset segment, chord or corner of a
strip must lie strictly on the side of the edge at that end that its exact
point takes: where the distance falls below the rounding of
the coordinates, an end rounds onto the edge or across it, and the buffer is
declined rather than answered. The rings of a polygon's buffer are written
with no such certificate.

### 8.2 Witnesses

| Query | Answer | Why |
|---|---|---|
| the area of the square of side 10 buffered by 1 | `143.141593` | `140 + pi`, the four corners being quarter circles read as arcs |
| that square with the hole from 4 to 6, buffered by 1 | `143.141593` | the hole is as wide as twice the distance and closes |
| the square of side 10 with the hole from 3.75 to 6.25, eroded by 2 | four surfaces; `(2.1 2.1)` in it, `(2.1 5)` not | the grown hole reaches 1.75 from the sides, past the contracted exterior at 2, and leaves its four corners, which lie `1.75 sqrt(2)` from the hole's corners |
| the line from `(2^40, 2^40)` to the corners of a unit square, buffered by `sqrt(1/2)` | holds `(2^39, 2^39)`, not the point 1024 above it | the exact distances decide both, at coordinates near `1e12` |
| the same line from `(2^60, 2^60)` | declined | the coordinates round by 256, more than the distance |
| `LINESTRING(0 0,10 0)` buffered by 1 with a flat cap, and with a square cap | the rectangles from `(0 -1)` to `(10 1)` and from `(-1 -1)` to `(11 1)` | the strip of the one segment, stretched by 1 past both ends for the square cap |
| `LINESTRING(0 0,10 0,0 0.5)` buffered by 1 with a flat cap, and with a square cap | one curve polygon each | the line turns back; the disc about the turn and the two strips unite in one ring |
| `LINESTRING(10 10,3 5,3 6)` buffered by 1.5 with a flat cap, the point `(3.66 6.24)` past its end | in the buffer | the point lies within the distance of the vertex before the end |
| a multipolygon of two polygons buffered by `316.42868675352696`, a point within the distance of the first | in the buffer | the answer is two curve polygons, and the point is located in the union of section 6.3 |

### 8.3 What CGAL measured

A buffer is judged as the set it is (section 10): a point lies in it exactly
where its distance to the geometry is at most the radius, and in an erosion
exactly where it lies inside the geometry at least the radius from its
boundary. Unless its row says otherwise, each query point stands clear of the
boundary by a factor of two, at half and at twice its distance, so no verdict
rests on a rounding.

| Buffers | Queries | CGAL agrees with |
|---|---|---|
| polygons with round joins | sharp shapes and 1313 points about natural areas | 3979820 of 3979820 and 1313 of 1313 |
| polygons with round joins, points about multipolygons | 15402 points about 192 natural-area multipolygons | 15402 of 15402 |
| lines with round joins and caps | 36960 straight queries; 336 lines with a first vertex from `2^20` to `2^60`; 1576 queries along vessel tracks | 36960 of 36960; 274 answered, 62 declined; 1530 answered, 11 declined, 35 past 60 s |
| lines with round joins and round, flat or square caps | 1712 lines at two distances, a grid about each and points along and across each end at 0.999 and 1.001 of the distance | 1021872 of 1021872, with no decline |
| multipoints | 300 multipoints of two to eight points at two distances, a grid about each and points about every crossing of two circles at 0.999 and 1.001 of the distance | 44728 of 44728, with no decline |
| erosions | 62800 points about 400 eroded squares and squares with holes; 12366 points about 192 natural areas | 62800 of 62800 and 12366 of 12366, with no decline |
| circular strings, compound curves and curve polygons | 200 geometries of one to three arcs, whole circles among them, at three distances, a grid about each and points at 0.999 and 1.001 of the distance from every arc and every end | 41268 of 41268, with no decline |
| curve polygons bounded by whole circles, with and without a circular hole | 60 discs at distances below and above the radius of the hole, points at 0.999 and 1.001 of the distance from either circle | 11087 of 11087, with no decline |
| circular strings and compound curves under a flat or a square cap | 120 curves of one to three arcs, ending in segments or arcs, at 0.3 and 0.97 of the smallest radius of their end arcs, a grid about each, points at 0.999 and 1.001 of the distance from every arc, segment and end, and on both sides of every cut and of every side and corner of a square | 57760 of 57760, with no decline |
| geometry collections of points, multipoints, lines, multilines, polygons with and without a hole, curve polygons and curves, nested at times | 80 collections of two to five members about one centre, 41 holding a line, under the round, flat and square caps at 0.3 and 0.9 of the smallest radius of their curves' end arcs, a grid about each and points at 0.999 and 1.001 of the distance from every edge, arc and point | 46392 of 46392, with no decline |

Every query a line's certificate declines has a distance below the rounding of
its coordinates, at most 0.32 of an ulp; answered instead, five of them are
wrong.

### 8.4 Its cost

Against GEOS called directly, in the kernel speed of section 7.3 (above `1`
MEOS is faster):

| Buffers | Distance 1 | Distance 50 | Distance 500 |
|---|---|---|---|
| polygons of natural areas | 1.05 | 1.11 | 1.76 |
| lines | 1.57 | 3.52 | 2.22 |

The joins of polygons read 1.34.

### 8.5 Measuring a buffer

A buffer with round joins holds circular arcs, and its area is `140 + pi` only
where the arcs are read as arcs. `area` measures the circular segment each arc
encloses, in doubles (section 7.1); a measure that first replaces each arc
with chords answers a smaller
area, and a buffer far smaller than its coordinates, where the chords leave
the arcs furthest in proportion, reads off its closed form. On the eroded
squares of the ledger at the scales `2^-24` and `2^-27`, 48 areas measured on
chords miss the closed form by up to 2.6%; measured by `area`, none of the
1400 closed-form areas does, the largest relative difference being
`1.34e-12`.

## 9. What lies outside

- **Geodetic distances.** A distance on the spheroid is not a rational of the
  coordinates; MEOS computes it by iteration, accurate to the iteration and
  not the nearest double.
- **Sums of distances.** The length of a trajectory adds one distance per
  segment, each rounded, so the length is not the nearest double of the exact
  length.
- **Temporal rigid geometries.** The temporal distance of a temporal rigid
  geometry is the distance of the body placed at each pose, the double nearest
  the exact distance of the placed geometries, and between instants the value
  the walk of the rigid body computes in doubles.
- **Values that are not finite.** NaN and infinity are not numbers MEOS holds
  (see [Finite Values](meos-finite-values.md)); the exact answer is defined
  for finite doubles.
- **Buffers in other styles and of other inputs.** Bevel and mitre joins,
  a geometry holding circular arcs, a single point, and a
  collection of mixed dimension are buffered on the older path, as is a
  polygon whose arrangement leaves a question open (section 8.1); their
  answers are outside this document's guarantee.

## 10. How the answers are checked

The reference for every exact answer of the engine is CGAL, an independent
library of exact geometry, computing on the same doubles in exact rational
arithmetic. A CGAL judge accepts an answer of the engine only where it is the
exact answer, and refuses the same answer moved by one ulp, so it tells a
correct double from its neighbour. The witnesses of the tables above are tests
of the tree (`meos/test/distance_point_test.c`): each expected value is a
constant the CGAL judge accepts, never the result of a formula.

A buffer is judged without building one: whether a point lies within a
distance of a geometry is a comparison of the squared distance from the point
to each segment, a rational of the doubles, with the squared distance, which
CGAL decides in exact rationals; for an erosion, whether the point lies inside
the geometry is decided by the exact parity of the edges a ray crosses. Each
judge is run with two controls: the verdict read in doubles, which stands a
factor of two clear of the boundary and must be accepted, and the same verdict
with every ninth answer flipped, which must be refused on exactly those lines.
