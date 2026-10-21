<!--
  MobilityDB — Making GEOS optional: lessons, and approaches ruled out
  Copyright(c) MobilityDB Contributors

  This documentation is licensed under a Creative Commons Attribution-Share
  Alike 3.0 License: https://creativecommons.org/licenses/by-sa/3.0/
-->

# 7. Lessons, and approaches ruled out

**Measured at** MobilityDB master `d9b9c11985`.

The campaign spent weeks on work that did not converge before it found the method of
[note 2](02-JUDGING-AN-ANSWER.md). This note records what it learned, so nobody pays for it twice:
first the method, then the rules about numbers, then the approaches measured and rejected, then
the traps. Each figure is the measurement that settles its question.

## 7.1 Method

**Build the instrument before the fix.** Repairs driven by a single example do not accumulate, for a
specific reason: a reproducer proves only that *that* shape moved. It says nothing about the shapes that did not. So each fix landed, the next session
met another shape, and the engine moved sideways. The guesses, what refuted each, and how long the
refutation took once an instrument existed:

| The guess | What refuted it | Time to refute |
|---|---|---|
| the closing test must read a scaled tolerance | a 48-cell gap × distance map | 30 s |
| drop the degenerate ring where it is built | the row still declines | 1 min |
| treat a hole like the no-area test treats a ring | the row still declines | 1 min |
| node identity in the chain walk | the node is already exact, at distance 0 | 2 min |

The replacement is a generated, scored space of cases — the buffer ledger of
[note 6](06-BUFFER.md) — which answers "is it fixed?" with a number in a tenth of a second. Any
engine defect without such a ledger gets one first.

**An instrument needs a control that turns it red.** The ledger's *census* is its table of
disagreeing groups per *arm* — move, scale, closed form (note 6 §6.5). Deliberately breaking one
function the ledger reaches (a *fault injection*) takes its three arms from 64 / 113 / 120 to 151 / 152 / 140 disagreements, the
declines from 196 to 576, and moves 380 of 1840 answers; reverting returns the census exactly. A
judge never seen to fail proves nothing.

**A decline must count as a disagreement.** Under the weaker rule, where a group in which every
member declines "agrees", the injected fault *improves* the census.

**A generator's own bugs read as engine defects.** The ledger's first census reported a 1170 %
closed-form deviation that is a hole larger than its own square, and a 2.3 % one that is a
formula applied outside its precondition (a reflex polygon). Every closed form states its
precondition, and the space is restricted to it.

**A type sweep must sweep the containers too.** The intersection, the difference and the unary
union reach GEOS for none of the 15 geometry types taken one or two at a time. The same types
placed *inside* a collection, or given to the array union as members, reach it for 104 of 225
pairs and for thousands of overlay calls ([note 5](05-WHAT-STILL-NEEDS-GEOS.md) §5.5). A sweep of
single geometries reads clean while `traversedArea` and `merge` reach GEOS on a temporal geometry
valued with a multipolygon. A sweep states what it nests, and nests every container the operation
recurses into.

**Separate a path from an input.** That a function *has* a path to GEOS is read from the code;
that an input *takes* it is read from a run. Neither stands in for the other: 134 public functions
have a path, and most inputs to them answer; the claim "no input reaches the fall-back" holds only
over inputs a run includes, and a run that omits the inputs that reach it proves nothing.

**Judge every answer of the branch, not only the ones that changed.** A census judged only where
answers changed against master missed a regression twice.

**An evidence set built only from cases a fix improves cannot refute the fix.** A fix to the
instant at which two moving objects first touch, validated on four configurations, none of which could fail, shipped a regression (#2303: a
grazing touch becomes a 0.17 s span) and #2304 reverts it. Name the cases that must not
degrade, and measure them before writing the code.

**Read PostGIS and GEOS first, then derive the correct answer.** Neither is authoritative alone,
and each has a measured blind spot. Consult both as references, then ship the mathematically
correct answer and carry neither one's quirks.

**Classify a tolerance by its call site, never by its function.** One function often serves both
a question about stored vertices and a question about constructed points; making it exact
wholesale breaks the second kind (§7.2).

## 7.2 Numbers

**An exact predicate needs a noded topology.** Dropping an exact orientation test into the
relationship engine made it far worse: 922 of 931 H3 cells wrong against 372 before. The engine
asked whether *constructed* midpoints lie on their own segments, and a rounded midpoint exactly
never does. GEOS never asks that question, because it nodes first. The fix is to stop
constructing the point (note 2 §2.7, rule 2), not to make the test exact.

**The engine can be asked about its own rounding.** With the exact segment kernel, one real buffer
came out invalid: at a hole vertex turning 4.6e-9 radians, the round join at distance 1 is 4.6e-9
long, a few units in the last place at y ≈ 6.1e6. The buffer computed the edge end, the join and
the next edge start separately, they rounded into a 2.2e-9 stub and a 2.5e-9 gap, and it asked
whether they meet. The exact answer, no, is correct; the defect is the question, asked of pieces
the engine built to touch. The fix is to build each joint once and share it.

**An absolute floor defeats a scaled tolerance.** Adding a constant to a size-aware tolerance
(`ABS + rel · scale`) re-imposes the unscaled failure in the opposite regime: a point-on-segment
fix correct on 220 synthetic pairs still lost the interior of 360 of 931 real H3 cells.

**A relative epsilon built from two cancelling terms vanishes where both do**, which is exactly
where the degeneracy is worst: a segment starting on a circle and tangent to it there gave two
roots 6e-8 apart instead of one touch. Scale the rounding by the magnitudes the terms are
differences *of*.

**A cross product is an area.** Comparing it to a length constant such as 1e-12 is too tight at
large coordinates and too loose at short edges. Dimensional analysis locates such a defect; it does
not prescribe the fix.

**A filtered sign's zero is two answers**, "zero" and "cannot tell". Every caller states which it
takes: an exact fallback, or a decline. Reading it as "collinear" is a guess.

**A total cannot answer a per-part question.** `area(subject) > 0` passes a subject that mixes a
region with a part of no area, so one point set answers two ways depending on how it is
written. Ask the engine's own per-part classification instead.

**`lwgeom_is_collection()` is true for a curve polygon and a compound curve**, whose rings and
pieces are sub-geometries. Walking their members as components turns a valid curve polygon into an
invalid multisurface. Dispatch on the type.

**Never move a computed instant to make room for it.** A crossing that lands on or before the
previous instant is not a crossing; drop it.

**An empty region is a statement, not an absence.** When two polygons only touch — along an edge
or at a corner — their intersection has no area, so an engine that builds regions answers
"empty". But the true intersection is that shared edge or point, which is not empty. MEOS
therefore answers such a pair by clipping the first polygon's boundary to the second, which
yields the line or point they share.

**Where two polygon boundaries run along each other, the direction of their edges decides the
answer.** Orient both polygons the same way (say, counter-clockwise). If the shared stretch is
traversed in *opposite* directions, the polygons lie on opposite sides of it, so it is part of
their intersection only as a boundary they share; if in the *same* direction, they overlap
there. The rule needs no constructed point and no tolerance beyond deciding that the edges are
collinear.

**Two engines at two resolutions cannot be combined.** Clipper2 works at 1e-7 of a unit and the
exact kernels to about 5.6e-9; a shape built by one is not clipped against a shape built by the
other.

**The minimum rotated rectangle is not one rectangle** (note 4 §4.6), so any measure read from it
needs a stated tie-break.

## 7.3 GEOS as a reference: where it holds and where it does not

GEOS is the reference for straight-edged shapes where it and MEOS agree (note 2 §2.1). Beyond that:

- **On a union, measured by the point set:** over 300 unions, the native answer has the exact
  measure 300 times, the GEOS answer 294 times, counting a stretch a line walks twice. The native
  answers agree with GEOS on 297 rows without the fix and on 294 with it: agreement with GEOS
  falls as correctness rises.
- **On a tangency:** over 7 rotations of a pair of discs touching at one point, GEOS answers the
  point once, `POLYGON EMPTY` five times, and a polygon of five identical vertices once.
- **On a TIN** (a surface made of triangles) **or a face of zero area**, GEOS's reading is not the
  point set and cannot judge one.
- **The installed GEOS version decides a GEOS answer.** The system GEOS 3.12.1 disagrees with its
  own suite's assertions on 18 collection records that 3.14.1 answers as asserted, so the suites
  are read with 3.14.1.

## 7.4 Approaches measured and rejected

Each row is a measured refutation. The measurement is the reason; repeating it is not.

| Approach | Why it is rejected |
|---|---|
| Tuning any tolerance constant of the buffer | Each revision moves the failure to another scale. This is the project's standing ruling. |
| Node identity in the buffer's chain walk | The needed piece sits at distance exactly 0 and is already used; scaling the node tolerance from 1e-6 to 50 leaves the node count at 162 |
| A computed answer-side flag for the buffer path | Moves no answer on 1840 ledger queries and 192 real polygons, and loses 3 of 1313 containment queries |
| An edge R-tree built per call for relate | Slower: from 0.67–0.72 of GEOS to 1.02–1.59 on areal pairs. A predicate that answers at its first witness usually stops after a few tests, so the index build is paid in full and the walk it saves never runs. An index pays only when amortised over many calls on the same geometry. |
| Monotone chains for relate (splitting each boundary into runs that only go one way in x) | Measured: no gain where the cost lies |
| Region descent on both trees for the index join | 2.6–3.0 times slower than reading the entries: node pairs are the product of the two levels, entries their sum |
| Guarding the two linear passes of `relate_edges_init` | Changes no answer, but buys 3 points of the 10 it targets |
| `relate_same_point` as the cause of the self-matrix cost | Replaced behind a switch: identical output, and a call counter reads 0 |
| Copying both operands to rescale them for relate | +3.08 % on every pair smaller than a unit, which longitude/latitude data at city scale always is. The scale factor belongs where coordinates are read. |
| The C library functions `fmin`, `fmax`, `ldexp` in arithmetic run per pair | +154 M and +146 M instructions on one clustering run; the `Min`/`Max` macros and `scalbn` cost nothing measurable |
| Cutting a moving rigid body into convex pieces to compute the area it sweeps | The average body splits into about 2.6 pieces, each needing as much work as the present method's 10 strips, so roughly 4 times the work on a computation already exact |

## 7.5 Where the cost hides

- **Edge extraction, not the matrix, is the cost of a relationship test**: 99.3 % of a native
  intersects call on 1444 areal pairs. A data set reuses its geometries 81–99 % of the time, which is
  why a per-geometry cache pays where a per-call index does not.
- **The early exit decides whether native beats GEOS.** Predicates that answer at a first witness
  (intersects, touches) win; those needing most of the matrix (contains, covers) did not, until
  the engine computed only the cells a question reads (#2706).
- **A validating accessor in a hot loop.** The public array accessor checked its index on each of
  200 million calls, 16 % of a buffer; the loops whose index comes from the array's own count read
  an internal twin that asserts instead (#2811).
- **Allocation shows only in the call graph.** `meos_array_create` has a self time of 1.19 %; the
  `malloc` it performs is 17.55 % of one overlay.

## 7.6 Traps

- **A WKT printed with 17 decimals rounds small coordinates.** Dump answers as hex EWKB.
- **A probe that calls a liblwgeom kernel directly links it statically** and keeps the kernel of
  its link time; relink it after every kernel change. A probe calling only MEOS entry points
  follows the rebuilt library.
- **`gbox_merge` merges nothing when the two boxes' Z/M flags differ**, and reports failure.
- **The installed `meos_geo.h` is written at configure time**, so a stale install prefix makes a
  test abort early and read "0 lost" — void, not green.
- **`callgrind_annotate` needs `--inclusive=yes`** or the GEOS column comes back empty.
- **A data set file may carry a third field** (`areal_pairs.txt` holds an expected matrix), and a
  two-field reader then reads no rows at all.
- **A checker counts a name in a comment**: `check_liblwgeom_geos.py` matches text, so read its
  hits before believing them.
- **Never rebaseline the GEOS checker to make it pass**: its list only shrinks, and a call it
  catches is a call that reaches GEOS invisibly.
