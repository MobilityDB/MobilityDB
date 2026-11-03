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
| Version | v0.2.0 |
| Scope | What the native geometry engine of MEOS answers, how it computes the answer, and what lies outside it; this version covers distances, predicates and relationships |
| Reference | `meos/src/geo/geo_funcs.c` (the exact kernels), `meos/src/geo/tgeo_distance.c` (the distances of moving values), `meos/test/distance_point_test.c` (the witnesses quoted below) |

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
judges check (section 8).

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
two dimensions is the exact answer. In MobilityDB, `dwithin(geometry, geometry,
float)` answers it unless both geometries have Z; in MEOS, `geom_dwithin2d`.
The distance given is itself a rational, and the answer compares the exact
distance with it, whatever double the exact distance rounds to.

| Operands | Distance | Answer | Why it needs care |
|---|---|---|---|
| `POINT(0 0)`, `POINT(2 3)` | `3.605551275463989` | false | the distance, the square root of 13, lies just above this double, which is the nearest double to it |
| `POINT(0 0)`, `POINT(2 3)` | `3.6055512754639896` | true | the next double, above the distance |
| `POINT(0 0)`, `POINT(1 2)` | `2.23606797749979` | true | the square root of 5 lies just below the nearest double to it |
| `POINT(0.5 5e-324)`, `LINESTRING(0 0,1 0)` | `0` | false | the two are apart by the least double there is |
| a square, a point inside it | `0` | true | the point is on no edge, and the area holds it |

A distance read in three dimensions, or a geometry carrying an arc, compares the
given distance with a distance in doubles, so the answer can differ from the
exact one where the two lie within its rounding.

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

## 7. What lies outside

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

## 8. How the answers are checked

The reference for every exact answer of the engine is CGAL, an independent
library of exact geometry, computing on the same doubles in exact rational
arithmetic. A CGAL judge accepts an answer of the engine only where it is the
exact answer, and refuses the same answer moved by one ulp, so it tells a
correct double from its neighbour. The witnesses of the tables above are tests
of the tree (`meos/test/distance_point_test.c`): each expected value is a
constant the CGAL judge accepts, never the result of a formula.
