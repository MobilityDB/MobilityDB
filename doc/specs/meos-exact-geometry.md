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
| Version | v0.3.0 |
| Scope | What the native geometry engine of MEOS answers, how it computes the answer, and what lies outside it; this version covers distances, predicates, relationships and areas |
| Reference | `meos/src/geo/geo_funcs.c` (the exact kernels), `meos/src/geo/tgeo_distance.c` (the distances of moving values), `meos/test/distance_point_test.c` and `meos/test/geo_test.c` (the witnesses quoted below) |

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
compares within a distance sized to the rounding of the construction, a fixed
`MEOS_GEOM_TOLERANCE` plus a few ulps of the coordinates
(`coordinate_tolerance`).

**Some comparisons still read a fixed tolerance.** Parts of the engine compare
a length or a parameter with `MEOS_GEOM_TOLERANCE` or `MEOS_EPSILON`, among
them the ray crossing of a point in a polygon (`dist_poly_seg_raycross`), the
closest point on an arc (`dist_geom_closest_on_arc`) and the turning points of
the distance of a moving point (`tpoint_geo_distance_turnpt`). The exactness
plan of the project, MobilityDB #3287, lists them and the changes that decide
each one exactly. The answers this document states are the ones the CGAL
judges check (section 9).

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
vertices are held exactly and never located by a rounded coordinate. Step 16 of
the exact buffer plan of the project, MobilityDB #3287, reads the relationship
of two areal operands off that arrangement.

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
(section 9) on three sets of geometries; `geom_area` answers the nearest double
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
   lie within 4 of each other, read once per ring from the bitwise AND and OR
   of their bit patterns, every coordinate is an integer multiple of one
   power of two. The ring is summed in 128-bit integers, one product per
   vertex in the form `(x_i - x_0)(y_(i-1) - y_(i+1))`, the form GEOS uses,
   which is exact. This is the path of the geometries data holds.
2. **A compensated sum in doubles.** Elsewhere, each product is split into its
   rounded value and its exact error (Dekker's product), the rounded values
   are summed exactly, and the errors are summed with a bound on their
   rounding. Where the bound shows which double is nearest, that is the answer.
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
| 192 multipolygons of real data | 0.37 | 4.0 |
| the 1871 polygons and multipolygons among the 2000 shapes | 0.33 | |

The area is the one measure of this engine whose exact layer runs on nearly
every call: a sum that cancels has no bound in doubles that decides its nearest
double cheaply, so the integers are the first layer, not a filter in front of
them. The answer is the nearest double on every call; the cost is about 2.7
times that of a sum in doubles, and a quarter of that of calling GEOS on a MEOS
geometry.

## 8. What lies outside

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

## 9. How the answers are checked

The reference for every exact answer of the engine is CGAL, an independent
library of exact geometry, computing on the same doubles in exact rational
arithmetic. A CGAL judge accepts an answer of the engine only where it is the
exact answer, and refuses the same answer moved by one ulp, so it tells a
correct double from its neighbour. The witnesses of the tables above are tests
of the tree (`meos/test/distance_point_test.c`): each expected value is a
constant the CGAL judge accepts, never the result of a formula.
