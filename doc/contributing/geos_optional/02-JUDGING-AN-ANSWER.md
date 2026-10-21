<!--
  MobilityDB — Making GEOS optional: how an answer is judged
  Copyright(c) MobilityDB Contributors

  This documentation is licensed under a Creative Commons Attribution-Share
  Alike 3.0 License: https://creativecommons.org/licenses/by-sa/3.0/
-->

# 2. How an answer is judged

**Measured at** MobilityDB master `d9b9c11985`.

Replacing GEOS means writing a second implementation of operations that GEOS has answered for
twenty years. A committer reviewing such a change asks two questions — *is the new answer right?*
and *is it fast enough?* — and this note gives the campaign's answer to both, then the rules that
follow from them. Every pull request of the campaign is judged this way.

## 2.1 Correctness: an exact reference, on the branch's own answers

**The question has one exact answer.** A coordinate stored in a geometry is a double-precision
floating-point number, and every such number is exactly a fraction (a whole number divided by a
power of two). So a question about the stored geometry — do these two segments cross, does this
point lie inside that polygon — has exactly one true answer, the one exact rational arithmetic
gives on those fractions.

**CGAL computes that answer.** CGAL is a C++ geometry library that can compute in exact rational
arithmetic. Given the same doubles MEOS reads, it decides the question exactly, so its verdict is
*the* answer, not a third opinion. CGAL is used only in test probes; it is never linked into the
shipped library.

**A MobilityDB build is never the reference for another.** A change is proposed on a *branch*
(the code of master plus the change). Comparing a branch with master shows
only that something *changed*. Where master is wrong and the branch is wrong in the same way, the
two agree and the comparison reports nothing. Where the branch is right and master wrong, the
comparison reports a "difference" that is in fact a fix. So a branch is judged by the exact
reference on its own answers alone, and "no answer moved against master" is not evidence of
correctness.

**Which reference judges what:**

| The shapes being compared carry | The reference |
|---|---|
| straight edges only, a predicate or a relationship matrix, and MEOS and GEOS agree | the agreement of two independent implementations, accepted without a further check |
| straight edges only, and MEOS and GEOS disagree | CGAL, exact arithmetic on the doubles |
| any circular arc | a closed-form formula first, then CGAL's exact circular-arc support, then a conservation identity such as *area(A ∩ B) + area(A − B) = area(A)*; never GEOS |
| a union or another constructed shape | the measure of the set of points it covers, computed exactly by CGAL (§2.4); GEOS is not a reference here, see note 7 §7.3 |

## 2.2 Curved shapes: the witness is a closed form or CGAL, because GEOS polygonizes arcs

GEOS has no circular arcs. When a geometry carrying one is handed to GEOS, PostGIS's conversion
routine (`LWGEOM2GEOS` in `postgis/liblwgeom/lwgeom_geos.c`) first replaces every arc with a
chain of straight chords, 32 for each quarter circle — it *polygonizes* the arc, which is why GEOS
is never a witness for an arc. GEOS then answers correctly about that polygon, which is a
different shape.

A case whose truth is a closed-form formula shows the size of the effect. Take the square from
(−2, −2) to (2, 2) and remove the disc of radius 2 centred at the origin, written as a curve
polygon:

| | area of the remainder |
|---|---|
| exact, by the closed form 16 − 4π | 3.433629386 |
| the same shape once the disc is the 128-sided polygon GEOS computes on | 3.438675372 |

The second line is what any GEOS-based answer to this question measures. MEOS answers the
difference as four corner regions, each bounded by a `CIRCULARSTRING` lying on the original
circle, so the arc survives into the answer and nothing is approximated
([`tools/square_minus_disc.c`](tools/square_minus_disc.c), output in
[`tools/results/square_minus_disc.txt`](tools/results/square_minus_disc.txt)).

Two consequences follow for judging a curved answer, where the closed form or CGAL decides:

- **Agreement with GEOS proves nothing** beyond the size of the chord error, and would read the
  same for a native answer wrong by exactly that much.
- **Disagreement with GEOS convicts nothing**: the exact answer is *supposed* to differ.

## 2.3 Why MEOS cannot afford to polygonize an arc

Turning arcs into chords is acceptable for GEOS, whose data model has no arcs. It is not for
MEOS, for three reasons.

- **Arcs are part of MEOS's own types.** A temporal circular buffer is a disc moving over time;
  the rounded joins of every buffer are arcs; PostGIS's curve types are accepted throughout. An
  answer made of chords is a different shape from the one the user stored, and a second operation
  applied to it compounds the error.
- **The error lands where predicates are asked.** The chord error matters most at exactly the
  cases a spatial predicate is about: tangency, touching, a trajectory grazing a circle. A
  predicate answered on chords can turn "touches" into "disjoint".
- **It costs memory and time.** Each quarter circle becomes 32 segments, so every arc multiplies
  the vertices an algorithm walks, and on a moving disc that multiplication is paid at every
  instant. The campaign exists to compute less, not more.

## 2.4 What a correct answer is: the point set

PostGIS and MEOS can describe the same region differently: as one line or as three pieces
meeting end to end, starting a ring at another vertex, listing parts in another order. The
committers ruled (2026-09-22) that **a native answer is correct when it covers the same set of
points as the answer a PostGIS user receives from the same call.** How that set is written down is
free, with one condition: every point is covered once, so that the length of a union is the
length of the set and not of a path walking a stretch twice.

Where a choice changes the point set itself, MEOS follows PostGIS: the method of `make_valid`, and
how a single-sided buffer ends.

Two places where MEOS answers differently from GEOS by design follow from this rule, and both
are decisions of the committers:

- **A union keeps lines whole.** GEOS cuts every line at every crossing; MEOS keeps each line in
  one piece unless the point set requires a cut. The point set is the same. A visible
  consequence is in the SQL regression test `056_tpoint_spatialfuncs_tbl`
  (`mobilitydb/test/geo/queries/`), which reports the largest number of vertices among the
  trajectories of a test table: 56 and 52 natively, where the GEOS answer has 848 and 996, every
  crossing having become a vertex (#2818).
- **Elevations are never averaged.** Where two shapes carrying Z meet at a point, MEOS gives the
  answer the elevation the inputs determine, and drops Z where they determine two different ones.
  GEOS averages them (1 and 7 give 4), which yields a value no input holds and makes the union of
  an array depend on its order.

## 2.5 A decline counts as a failure

A native route may *decline*: raise an error saying it cannot answer, rather than answer
wrongly. That is better than a wrong answer, but it is not an answer. So:

- a change that turns an answer into a decline is a regression, however correct the remaining
  answers are;
- a test corpus made only of queries the engine already answers cannot see a new decline, and
  every corpus states how many of its queries are declined (its *decline floor*).

## 2.6 Speed: parity with GEOS called directly

**The bar is GEOS, not master.** Master is itself many times slower than GEOS on some operations,
so "no slower than master" would accept a native engine eight times slower than the library it
replaces. An operation leaves GEOS only when its native cost is at most GEOS's on every data set
it is measured on.

**GEOS is measured called directly**, on geometries already converted to its format, so the
comparison is against GEOS's own algorithm and not against MobilityDB's conversion code.

**The unit is instructions, counted by callgrind, in one process.** The probe `perf_geos` runs the
native engine and GEOS on the same inputs inside one program, under the instruction counter
`callgrind`. A *cell* is one operation on one data set, for example "buffer, natural areas,
distance 1", and its figure is the ratio native instructions / GEOS instructions. Several
sessions and builds run on the measuring machine at once, so wall-clock times move by a factor of
four or five from one run to the next; a ratio of instruction counts from one process does not.

**A speed comparison is valid only when both sides answer the same rows.** If a change alters
which inputs are declined, the two sides time different work and no claim can be made.

**Distance on curves is not a speed comparison at all.** GEOS cannot measure an arc, so the GEOS
side times a polygon with the chord conversion left outside the timed loop, while the native side
times the exact computation. Those are different questions; the figure is information, never a
target.

## 2.7 How a geometric question is decided: exactly

The rule behind every native kernel in the campaign:

1. **A question about stored vertices is the sign of one small computation** — whether a point
   lies left of, right of or on the line through two others is the sign of
   (b − a) × (c − a), a 2 × 2 determinant; collinearity and crossings follow from it. It is
   decided with a *filtered* sign: compute in floating point, and fall back to exact arithmetic
   only when the result is too close to zero to trust, by a bound on the rounding error that
   Jonathan Shewchuk published for exactly this computation. A filter decides whether the floating-point result can be trusted; it never decides
   the answer.
2. **A position along a stored segment is its parameter** *t* between 0 and 1 on that segment.
   Take the segment from (0, 0) to (3, 1) and its point at *t* = 1/3: its exact coordinates are
   (1, 1/3), but 1/3 has no exact double, so the stored point is (1, 0.333…3) and lies a tiny
   distance off the segment. Asking "is it on the segment?" then answers no. Comparing parameters
   on the same segment never asks that question.
3. **A degenerate case is exactly degenerate.** A segment of length zero has length zero, not
   "less than a tolerance".
4. **A tolerance survives only at a point the engine constructs**, and the first move is to stop
   constructing it. Where one must remain, it is a distance sized to the rounding of that
   construction. MEOS owns two such constants: `MEOS_EPSILON` (1e-6, for the modelled equality of
   values) and `MEOS_GEOM_TOLERANCE` (1e-12, for planar geometry).
5. **Changing the value of a tolerance is never a fix.** A tolerance in the right units still
   moves the failure to another scale.

GEOS needs few tolerances because it first *nodes* the geometry: it cuts every edge at every
intersection, so "is this point on that edge" is answered by identity and never computed. MEOS
answers it analytically instead, which is why the rules above matter so much.
