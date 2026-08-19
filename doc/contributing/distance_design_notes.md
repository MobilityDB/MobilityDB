<!--
  MobilityDB — Distance and Spatial Relationships: Design Notes
  Copyright(c) MobilityDB Contributors

  This documentation is licensed under a Creative Commons Attribution-Share
  Alike 3.0 License: https://creativecommons.org/licenses/by-sa/3.0/
-->

# Distance and Spatial Relationships — Design Notes

Every temporal spatial type answers the same family of questions: how far apart
two values are, whether they ever meet, whether they always stay apart. The
answers differ between types for exactly one reason — **the geometry of the
value**. A `tgeompoint` carries a point; a `tcbuffer` carries a disc. A point has
no interior and no boundary, so predicates that distinguish interior from closure
collapse for it; a disc has both, so they do not.

This note records the algebra those questions reduce to, which bounding-box
filters are sound and which are automatic, and how a new family inherits the
surface without new code.

> **Audience**: a contributor working at the MEOS C level, or a binding author
> deciding which functions a new type must expose. This is design rationale, not
> user documentation.

---

## 1. Point versus area: the value's geometry sets the surface

| | `tgeompoint`, `tpose` | `tcbuffer`, `tgeometry` |
| --- | --- | --- |
| value | a point | an area (disc, general geometry) |
| extent | zero | positive |
| interior vs boundary | indistinguishable | distinct |
| radius in the value | none | `tcbuffer` only |

**The applicability of `contains` and `covers` is set by the container — the
first argument — not by the temporal operand.** `contains(A, B)` and
`covers(A, B)` differ only in how they treat `B` touching the boundary of `A`.
That distinction needs `A` to have a boundary, so `A` must be areal.

For any geometry `A` and point `p`:

```
covers(A, p)   ≡  intersects(A, p)     -- p lies in closure(A) either way
contains(A, p)                         -- meaningful: excludes the boundary
covers(p, A), contains(p, A)           -- degenerate: a point contains nothing
```

A point-valued family therefore exposes the **areal-container direction only**:
`eContains(geometry, tgeompoint)` and `eCovers(geometry, tgeompoint)` are
meaningful; `eCovers(tgeompoint, geometry)` is not. An area-valued family exposes
both directions, because its own value can be the container.

`covers(A, p) ≡ intersects(A, p)` being numerically true does **not** make the
`covers` surface redundant for a point family — `contains` is exposed for the
same operand pair and is not omitted. Applicability is decided per predicate
semantics, not by matching another family's function list.

**`covers` and `contains` are two-dimensional by design.** No `covers3d` exists:
PostGIS and MEOS provide these only in 2D, while `intersects`, `disjoint` and
`dwithin` carry `_3d` variants. A Z separation is invisible to `covers`, exactly
as in `ST_Covers`. This is not a gap to fill.

---

## 2. The unifying scalar and the reduction algebra

Every distance-family predicate is a statement about one scalar function of time,
the **signed gap** — negative inside, zero on the boundary, positive outside:

```
g(t) = dist(centre(t), gs) − r(t)
```

For a point-valued type `r ≡ 0`, so `g` is plain distance. This single scalar is
why a point family is a special case of an area family rather than a different
problem.

Each predicate is a running **minimum** or **maximum** of `g` with an early exit:

| predicate | statement | reduction | kernel | early exit |
| --- | --- | --- | --- | --- |
| `eIntersects` | ∃ g ≤ 0 | `min g ≤ 0` | running MIN (`nad`) | first `g ≤ 0` → true |
| `aDisjoint` | ∀ g > 0 | `min g > 0` | running MIN (`nad`) | first `g ≤ 0` → false |
| `eDwithin(d)` | ∃ g ≤ d | `min g ≤ d` | running MIN | first `g ≤ d` → true |
| `aIntersects` | ∀ g ≤ 0 | `max g ≤ 0` | running MAX (farthest approach) | first `g > 0` → false |
| `eDisjoint` | ∃ g > 0 | `max g > 0` | running MAX | first `g > 0` → true |
| `aDwithin(d)` | ∀ g ≤ d | `max g ≤ d` | running MAX | first `g > d` → false |

Two reductions cover the whole table, so a family needs at most two distance
kernels plus its own per-segment turning-point function. Everything else derives.

**Containment reuses the same scalar with the radius sign flipped.** `contains`
and `covers` between two areas are the same signed-gap question with the
threshold changed from the radius **sum** to the radius **difference**. Disc A
contains disc B when the farthest point of B from A's centre stays inside A,
`dist(centre_A, centre_B) + r_B ≤ r_A`, so the containment gap is

```
g⊃(t) = dist(centre_A(t), centre_B(t)) − (r_A(t) − r_B(t))
```

— `covers` at `g⊃ ≤ 0`, strict `contains` at `g⊃ < 0`, the two differing only on
the measure-zero boundary where the disc is internally tangent. The intersects
gap subtracts the sum `r_A + r_B`; the containment gap subtracts the difference
`r_A − r_B`. One scalar, one sign, and every containment predicate is again a
running reduction of it: `eContains ≡ ∃ g⊃ < 0`, `aCovers ≡ ∀ g⊃ ≤ 0`. Testing
the four extreme points of one disc against the other is a strictly weaker
approximation that reports a disc poking out along a diagonal as contained; the
scalar gap is the exact statement.

**The gap is convex over a segment, so the two quantifiers need different
evidence.** `dist` of two affinely-moving centres is a norm of an affine
function, hence convex; subtracting the affine threshold keeps it convex. A
convex function on a segment attains its **maximum at an endpoint**, so `always`
(`∀ g ≤ 0`) is decided by the two segment endpoints alone — no interior scan. Its
**minimum** may fall in the interior, so `ever` (`∃ g < 0`) needs the sub-interval
between the two roots of the squared gap `dist² = (r_A − r_B)²`; the
turning-point function returns exactly that sub-interval, and the temporal Boolean
is true across it rather than only at the closest-approach instant. This is the
same turning-point machinery the bounded `dwithin` uses, with the radius sum
replaced by the difference and the distance set to zero. The endpoint test that
decides `always` must carry the strictness of the predicate — `g < 0` for
contains, `g ≤ 0` for covers — so two identical discs, whose gap is zero for the
whole segment, cover each other throughout but never strictly contain.

**The `ever(tR)` / `always(tR)` projections define correctness; they are not the
implementation.** `eR ≡ ever(tR)` and `aR ≡ always(tR)` are the oracle a test
uses. Computing them by materialising the full temporal Boolean and projecting
∃/∀ resolves every sub-period of every segment only to ask whether one exists.
The running reduction answers the same question and stops as soon as it is
decided.

**The reduction earns its keep from the early exit, so it needs a threshold to
exit on.** `eDwithin(d)` has one in `d` and reduces to the nearest approach
profitably. `eIntersects` has the threshold `0`, but the nearest approach is
computed exactly, and an exact minimum cannot stop at the first zero — it walks
every segment to prove no smaller value exists. So `nad ≤ 0` is not the early
exit the table describes; it is the same full walk wearing a threshold.

The scan that stops at the first crossing is the target shape and no primitive
implements it for this pair. Until one does, the ever intersects of a temporal
point resolves through the clip engine and projects, which is the expensive
form this section warns about and is still the cheapest available: measured on
the tcbuffer join, the projection runs at 1.16× of the trajectory path it
replaces and `aDisjoint`, which derives from it, at 2.01×, while the `nad`
reduction runs at 0.95× and 1.70×. Neither number licenses the projection as
the design: they measure two ways of walking every segment against each other.
A scan that stops at the first crossing beats both, and remains the shape this
relationship wants.

**`disjoint` is the complement of `intersects` with the quantifier swapped:**
`eDisjoint = ¬aIntersects`, `aDisjoint = ¬eIntersects`,
`tDisjoint = ¬tIntersects`. A `disjoint` path that does not reduce to a negated
`intersects` is a laggard.

> **The complement identity proves consistency, not correctness.** Computing
> always-intersects as `spatialrel(intersects, ALWAYS)` over a trajectory is
> **wrong**: a moving point whose trajectory crosses a target intersects it at
> one instant, not always, yet the trajectory-level test answers true. `covers`
> is the correct ALWAYS primitive, which is why always-intersects derives as
> `¬eDisjoint`. An identity such as `eDisjoint + aIntersects = total` can hold
> while both sides are wrong together.

---

## 3. Bounding-box filters: sound, unsound, and automatic

An `stbox` carries a period, `xmin/ymin/zmin/xmax/ymax/zmax`, an SRID, and flags
recording which dimensions are present.

**`&&` compares only the dimensions present in *both* operands.** Overlap is
required in every common dimension; a dimension absent from either operand is not
consulted. So `tpoint && geometry` is a purely spatial test — a plain geometry
has no period, so time is not a common dimension.

**`&&` is an index accelerator, not a semantic operator.** It belongs in a GiST
prefilter, never in a predicate's definition. A predicate that spells part of its
meaning as `NOT (stbox(a) && b) OR …` conflates the box's space-and-time test
with its own semantics and answers wrongly wherever the two disagree.

**Comparability is a question about time alone.** When time is a common dimension
and the periods do not overlap, there is no shared instant, the values are not
comparable, and the answer is NULL. Sharing time while far apart in space is
perfectly comparable — the answer there is `disjoint = true`.

**An empty geometry is not unknown.** It is a valid, known, empty point set:
∅ ∩ X = ∅. It intersects nothing (FALSE) and is disjoint from everything (TRUE),
and `disjoint = ¬intersects` holds through it. NULL is reserved for genuinely
unknown, which here means no shared instant.

### Which prunes are sound

| use | sound | why |
| --- | --- | --- |
| fixed-threshold `dwithin(d)`, `intersects` | yes | box distance > `d` ⟹ every point pair is farther than `d` |
| ordering key for a pair loop | yes | the box distance is a valid lower bound to sort on |
| `disjoint` as an index condition | **no** | a box miss proves disjointness, so `&&` prunes exactly the true rows |
| `disjoint` as an early answer inside the function | **yes** | the same box miss that makes it unusable as a prefilter answers the relationship outright |
| exact `nad`, `minDistance`, `\|=\|` — whole-pair box | **no** | no external threshold exists; the nearest pair can sit just outside the box |
| exact `nad` — per-segment lower bound vs the running min | **yes** (planar) | the running minimum is a live threshold; a segment whose centre-box-minus-radius lower bound ≥ the running min cannot lower it |
| `touches` prefilter via exact `nad` | **yes** | contact needs the gap `= 0` exactly; a strictly positive `nad` proves disjoint, so the pair can neither ever nor always touch |
| `always dwithin(d)` — value's radius-aware box inside the geometry box grown by `d` | **no** | `dwithin` is the distance to the NEAREST point of the disc, so a large disc parked on a small geometry is always within it while reaching far outside the grown box |
| `always` anything — one *unit* whose CENTRE box is farther than `d + rmax` | **yes** | the disc reaches exactly its radius beyond the centre, so that unit holds no instant within `d`, and one unit refutes an `always` |
| `ever`/`always contains`/`covers` — no unit admits a disc inside the geometry box | **yes** | a disc inside the geometry is inside its box, so its centre lies in that box shrunk by the radius |
| box restriction (`atStbox`) — value's radius-aware box | sound but **inert** | the restriction reads the CENTRE path, and the widened box of a whole trajectory overlaps nearly everything |

The two `disjoint` rows are the same fact read in opposite directions, and the
direction is what decides soundness. A box miss proves that the values never
meet. Fed to an index as `col && stbox(arg)`, that turns into a filter which
keeps only the pairs whose boxes overlap and therefore drops every pair the
miss has just proved disjoint — the rows the query wants. Read inside the
relationship instead, the same miss is a complete answer: ever disjoint is
true, ever intersects is false, and neither needs the trajectory the paths
below would build. So the box belongs in the ever intersects and ever disjoint
branches as an early answer, and never in their `SUPPORT` clause.

A box test compares every dimension the two boxes share, so it answers in the
dimensionality of the operands rather than of the relationship. That matters
wherever the two differ: the ever disjoint of a temporal point resolves through
a 2D covers, so a 3D pair separated in Z alone is *not* disjoint, while their
boxes do not overlap. An early answer taken from `overlaps_stbox_stbox` would
contradict the relationship there. Either restrict the early answer to planar
2D operands, or take the box miss in 2D — the `gserialized_get_gbox_p` plus
`gbox_overlaps_2d` reject that `geom_spatialrel` and `geo_covers2d` apply — but
never clear the Z flag by hand.

Soundness of the threshold prune depends on the box enclosing the whole value.
For `tcbuffer` this holds because the box is **radius-aware** — the extent is
expanded by the radius, so it encloses the swept disc, not merely the centre
path. A family whose box covered only a centre would need that expansion before
the prune became valid.

An exact `nad` has no external threshold, so the whole-pair box cannot prune it —
yet the synchronised running-min walk manufactures one. The running minimum found
so far is itself a live threshold, so a per-segment lower bound on the signed gap
(the separation of the two centre boxes over the segment, minus the sum of the
maximum radii, signed) is sound: when that lower bound is at or above the running
minimum, the segment — turning point and per-instant endpoints alike — cannot
lower it and is skipped. The bound is planar; a planar box separation is not a
lower bound on a geodesic distance, so the geodetic path takes no prune.

Contact is the measure-zero boundary of intersection: two values touch only where
the signed gap is exactly zero. A strictly positive `nad` therefore rejects a
pair from `touches` without removing any touching instant, and the result is
unchanged down to the byte. The same lower-bound reject does **not** transfer to
`contains` or `covers`, which hold across an open sub-interval rather than at a
single instant — a pair whose exact discs are already apart can still have their
polygonal approximations overlap, so a containment prefilter built on the exact
gap disagrees with a containment kernel built on the approximation. Containment
is made exact by computing the gap directly (§2), not by pruning an approximate
kernel.

### The radius is radial, and a box is not

Enclosing the value is where the box has to expand by the radius. Testing it does
not, and the difference is the whole prune margin for a family whose radius is
comparable to what it is measured against.

A swept-capsule unit — the disc of one `tcbuffer` segment as its centre moves
from one instant to the next — carries `rmax = max(r₁, r₂)` as the largest radius
it ever has. Every point of the unit therefore lies within `rmax` of the unit's
**centre segment**, so for any set `S`

```
dist(unit, S) ≥ dist(centre segment, S) − rmax
```

and a centre distance of at least `best + rmax` proves the unit cannot lower a
running minimum of `best`. That is the **radial** form of the bound, and the
comparison it wants is against the threshold, not against the box.

The tempting alternative is to grow the centre's bounding box by `rmax` on each
axis and compare the grown box against `best` directly. It is sound — the grown
box does contain the unit — but it subtracts the radius **once per axis**, so a
separation lying diagonally has the radius taken out of it twice. Radially that
gives away a factor of up to √2:

```
axis-wise:  √( max(dx − rmax, 0)² + max(dy − rmax, 0)² )  ≥  best
radial:     √( dx² + dy² )                                ≥  best + rmax
```

The radial test is never weaker than the axis-wise one, is strictly stronger
wherever both axes separate, and with `rmax = 0` — every point family — the two
coincide exactly. A disc is round and a box is square; expanding a box is how you
*contain* a disc, and it is not how you *measure* one.

### The three levels of the geometry traversal

Against a plain geometry the nearest approach, the nearest approach instant and
the shortest line share one traversal. The geometry is decomposed once into
straight and circular-arc edges, Morton-ordered, and grouped into √n buckets;
each temporal unit is then filtered through three nested levels before any exact
solve runs.

| level | test | what a rejection saves |
| --- | --- | --- |
| geometry | centre box against the geometry's overall box | the whole unit, point-in-polygon test included |
| bucket | centre box against the bucket's box | a √n-sized group of edges |
| edge | centre box against the edge's box, then the exact centre-**segment** to edge distance | one edge's exact solve |

Every one of those tests reads the radial bound above against the same squared
threshold `(best + rmax)²`, which is recomputed only when the running minimum
moves, never per edge. The last test replaces the edge's box with the edge
itself, so it rejects the survivors the box levels leave; it earns its cost only
for a moving disc, because with `rmax = 0` the box levels already carry the bound
exactly and the exact solve is no dearer than the test that would skip it.

The levels matter in proportion to the radius. Measured on a 100 × 100 join of
vessel trajectories against natural-area polygons, where the disc radius averages
5 km against polygons of comparable size, reading the three box levels radially
instead of axis-wise makes the whole trio — `nad`, `nai`, `shortestLine` — **five
times faster** and puts the `tcbuffer` cost below its `tgeompoint` counterpart on
the same query. Results are unchanged to the last digit: a lower bound that is
tightened while remaining a lower bound cannot change which edge attains the
minimum. What is left is the box arithmetic of the bucket level itself; the exact
solve, which once dominated, now runs a few hundred times per pair.

### Which box, and read against what

The radial reading of the previous subsections is one instance of a rule that
decides every filter an area-valued family puts in front of a kernel, and
getting it wrong is sound in one direction and unsound in the other.

**The box of a `tcbuffer` is radius-aware.** It is widened so that it encloses
the swept discs, which is what a distance or a relationship needs (§3 above). It
is therefore the wrong box for anything that reads the *centre*, and it is a
weak box for anything whose threshold the radius already carries.

Three things follow, and each is a filter that pays for itself.

**An `always` relationship is refuted by one unit, and the centre is what
refutes it.** A disc is within `d` of a set exactly when its centre is within
`d + r` of it. So a unit whose centre box stands farther than `d + rmax` from the
geometry box holds no instant within `d`, and one such unit ends the question —
no kernel, no edge decomposition. Taking units rather than instants keeps this
independent of which bounds the sequence carries, the interior of a unit always
belonging to the definition time. `always touches` is the same test at `d = 0`,
and there it *replaces* the exact `nad` reject: a running minimum over the whole
value answers whether contact happens anywhere, which is the **ever** question,
while the always question falls to the first unit that fails to reach.

**The mirror-image filter is unsound, and the temporal point sibling has it.**
For a point value, always-within-`d` puts every point of the value inside the
geometry box grown by `d`, so `contains(box_geom ⊕ d, box_temp)` is a valid
`always` prefilter and the point families use it. It does **not** carry over to a
disc: `dwithin` is the distance to the *nearest* point of the disc, so a disc of
radius 100 parked on a point is always within 1 of it while its box exceeds the
grown box a hundredfold. Copying the sibling's prefilter across the radius
boundary silently answers false on exactly the pairs that are always within.
⇒ **Before reusing a point family's box filter for an area family, ask which
points of the value the relationship constrains. `dwithin` constrains one.**

**Containment adds a fit test the overlap cannot express.** A disc inside the
geometry is inside the geometry's box, so its centre lies in that box shrunk by
the radius, and a shrunk box that comes out empty admits no disc at all. A value
no unit of which admits such a disc is inside the geometry at no instant, so
neither `ever` nor `always` contains or covers it. The existing overlap test
cannot see this, the widened box overlapping wherever the discs merely reach.

**A restriction reads the centre, so its filter must too.** `atStbox` and
`minusStbox` clip the centre path and slice the value to what survives. Filtering
that with the value's own box is sound and nearly inert: widened by the radius
and spanning a whole trajectory, it passes almost every pair in a join, and each
one pays for a conversion and a clip that returns nothing. The per-unit centre
test settles it instead — and note that the *global* centre extent is not enough
either, one trajectory box covering most of a region. Per unit is where the test
bites.

### A witness must measure the distance it reports

A scalar distance and its witness — the shortest line, the nearest approach
instant — answer the same question, and the witness is wrong whenever its own
measurement disagrees with the scalar.

For a disc family the trap is the centre. Reducing two temporal circular buffers
to their centre paths and taking the shortest line between those is a line whose
length exceeds `nearestApproachDistance` on the same pair by exactly the two
radii, because it leaves from a centre rather than from a boundary. The line the
question asks for runs along the line of centres, from the boundary of one disc
to the boundary of the other, at the instant where the temporal distance is
least; overlapping or concentric discs meet, and it degenerates to the point
where the boundary of the first reaches the second.

The same reasoning governs a fallback. When a geometry has no edge decomposition
the nearest approach distance falls back to the exact traversed area, so the
nearest approach *instant* must fall back to something that agrees with it —
the instant of least exact temporal distance — and not to the centre path, whose
argmin moves away from it as soon as the radius varies.

⇒ **The self-consistency check is cheap and it is the test to write: the length
of the shortest line equals the nearest approach distance, and the distance at
the nearest approach instant equals it too.** Both hold by construction when the
witness is read from the same signed gap as the scalar (§2), and both fail
loudly when a centre has been substituted for a value.

### Where a zero radius stops being a temporal point

A `tcbuffer` all of whose radii are zero converts to a temporal point without
loss, and several relationships take that conversion. It is tempting to read the
delegation as scaffolding to be removed once the disc kernels are complete, but
the boundary it marks is real: **a disc of zero radius has no interior, and the
disc kernels read contact and containment from a disc boundary.**

Measured on a 100 by 100 join with every radius set to zero, the delegating path
and the native disc path agree on `eIntersects`, `eDwithin`, `aDwithin`,
`eCovers`, `aCovers`, `aContains` and `aTouches`, and part company on
`eTouches` (12 pairs of 10000) and on every temporal relationship —
`tIntersects`, `tDisjoint`, `tTouches`, `tContains`, `tCovers` — at roughly 250
pairs of 10000 each. The split is where it should be: the relationships that
distinguish an interior from a closure are the ones a zero-radius disc cannot
answer for itself.

⇒ The delegation is removable exactly where it is redundant, and removing it
elsewhere is a change of answer, not a cleanup. The measurement, not the shape
of the code, is what tells the two apart.

### Why bounded predicates get the box for free and distance does not

This is the asymmetry that explains why box filtering is uniform in one half of
the surface and patchy in the other.

A **bounded** predicate carries a radius, so a support function can rewrite it
into an indexable form. `tspatial_supportfn` turns a named predicate into
`t2.trip && expandSpace(t1.trip, r)` as an index condition, and the query text
stays operator-free and portable. The box filter arrives automatically, for every
family, with no per-call-site work.

An **unbounded** distance has no radius, so there is no expanded box to test.
Support functions inject scan *conditions* only — never `ORDER BY` — so the
rewrite that saves `eDwithin` cannot save `minDistance`. Each distance function
must therefore opt into its prune explicitly, at its own call site, with its own
soundness argument from the table above. Nothing forces that set to be uniform,
and it is not: the prune is present in some distance paths and absent in their
siblings.

### Two traps in the distance operators

`|=|` is `nearestApproachDistance`, a scalar, and is KNN-indexable. `<->` on a
temporal point is **temporal distance** returning a `tfloat`, not a KNN scalar.

`|=|` between two temporal values is **time-synchronised**: it returns NULL for
temporally disjoint operands. Against a plain geometry it is **time-agnostic**,
because the time test applies only when both boxes carry a period — so
`trip |=| geometry` is the correct spatial distance ignoring time, while
`trip1 |=| trip2` is not a substitute for it.

KNN acceleration additionally needs an `OPERATOR … FOR ORDER BY` member in the
opclass for that operand pair; a missing member degrades silently to a scan and
sort. Its payoff also depends on probe selectivity — a compact probe object
prunes well, while a whole-trajectory probe overlaps most boxes and leaves the
exact recheck to do.

---

## 4. API minimisation: the box classes and the cast

Each function added to the MEOS public API multiplies across every binding —
PyMEOS, JMEOS, GoMEOS, MEOS.NET, MEOS.js, MobilityDuck, the JVM engines. One new
function is one new instantiation in each. Keeping the surface small is a
first-order design objective, not tidiness.

**Bounding-box operators are implemented per box class, not per temporal type.**

| box class | carries | serves |
| --- | --- | --- |
| `tstzspan` | time | every temporal type |
| `tbox` | value + time | the number types |
| `stbox` | space + time | every spatial type |
| `tpcbox` | point-cloud extent + time | the point-cloud types |

The topological and positional operators live once per box class. A temporal type
does not reimplement them; it declares how to produce its box. The base `span`
and `spanset` types carry their own topological and positional operators on a
separate span-type axis; they are the primitives the box operators delegate to,
not a fifth box class.

**The cast is the inheritance vehicle.** In the SQL dialect `a::b`, the
constructor form `b(a)`, and the MEOS function `a_to_b` are three faces of one
function. A cast defined once in MEOS C is present in every binding
automatically — no per-binding wrapping, no per-type operator set.

Defining `T → stbox` for a new temporal type therefore makes **the entire
topological and positional operator surface available with no further code**:
`overlaps`, `contains`, `contained`, `same`, `adjacent`, and the positional
family (`left`/`right`, `below`/`above`, `front`/`back`, `before`/`after` with
their `over*` forms) in every operand direction. This is why the box classes are
few and the temporal types many.

The same mechanism carries the spatial relationships: a derived family reaches
the geo implementation through a lossless MEOS C cast rather than reimplementing
it. The cast target follows the value's geometry — point-like values cast to
`tgeompoint`; area-like values cast to the boundary geometry, so the predicate
stays exact rather than centroid-lossy.

**What cannot travel through a cast, and stays native per type:**

1. the box materialised in the type header — it is what the cast returns;
2. the box operators and the GiST / SP-GiST opclasses — an opclass binds
   operators on the **native** column, so it cannot route through a cast;
3. genuinely type-semantic functions with no geometry equivalent.

A cast written in PG SQL does not satisfy this: SQL-level casts never reach a
binding such as Spark. The cast must exist in MEOS C for the inheritance to work.
For the same reason a SQL-level wrapper is acceptable only as a convenience
projection over MEOS C primitives, never as the place a gap is closed. A
`LANGUAGE SQL` wrapper is also inlined by the planner before a support function
fires, so an overload written that way forfeits index acceleration that its
sibling overloads keep.

---

## 5. Adding a family

For a new temporal spatial type, in order:

1. **Classify the value** — point or area. This fixes which predicates are
   meaningful (§1) and whether the `contains`/`covers` surface is
   one-directional.
2. **Define the box** and its MEOS C cast `T → stbox`, extent-aware if the value
   has extent. The topological and positional operators and the indexes follow
   with no further code (§4).
3. **Supply one per-segment turning-point function** for the signed gap `g`. The
   six predicates of §2 derive from it through the two running reductions.
4. **Reach the geo spatial relationships through the cast**, choosing the cast
   target by the value's geometry.
5. **Bind every operand direction to its native symbol.** An overload left as a
   SQL cast while a native symbol exists diverges from its own mirror direction
   and loses index support (§4).
6. **Opt each distance function into its box prune** explicitly, against the
   soundness table of §3. This step is per function; the framework cannot supply
   it implicitly (§3).

A point-valued family added this way needs no new distance mathematics: with
`r ≡ 0` the signed gap is plain distance, and every reduction is already present.

---

## 6. The temporal type inventory

The surface a temporal type gets is fixed by **two independent axes**, and the
whole point of this section is that they do not coincide.

- The **family axis** (§1) is the *value's geometry* — scalar, point, area, cloud,
  or non-metric. It decides which predicates are meaningful and whether the
  `contains`/`covers` surface is one-directional.
- The **kernel axis** (§2) is the *turning-point strategy for the signed gap* —
  whether the interior extremum of `g(t)` is analytic (closed form) or must be
  found by adaptive bisection. It decides the **exactness** of the answer and
  which reduction machinery applies.

These are orthogonal. An **area** type can share the **point** type's closed-form
kernel — `tcbuffer` does, because its gap `dist(centre, gs) − r(t)` is still
analytic with at most two turning points. And an area type can need a *different*
kernel — `trgeometry` does, because a rotating rigid body has no closed-form
distance. So "point versus area" predicts the *surface* but not the *strategy*;
"analytic versus adaptive" predicts the *strategy* but not the *surface*. The two
tables below record each axis for every temporal type, exhaustively.

### 6.1 Family axis — the value's geometry

| temporal type | value geometry | signed-gap radius `r(t)` | distance surface |
| --- | --- | --- | --- |
| `tbool`, `ttext`, `tjsonb` | non-metric | — | none — no distance is defined |
| `tint`, `tbigint` | scalar (ℝ, step) | n/a | number-line `\|a − b\|` |
| `tfloat` | scalar (ℝ, linear) | n/a | number-line `\|a − b\|` |
| `tgeompoint` | point (planar) | `0` | point — container direction one-way |
| `tgeogpoint` | point (geodetic) | `0` | point — container direction one-way |
| `tnpoint` | point (network, reduces to `tgeompoint`) | `0` | point — container direction one-way |
| `tpose` | point (+ orientation carried, unused by distance) | `0` | point — container direction one-way |
| `tcbuffer` | area — disc | `r(t)` | area — both container directions |
| `tgeometry` | general (point ∪ area, per value) | boundary | follows the value's geometry |
| `tgeography` | general (geodetic) | boundary | follows the value's geometry |
| `trgeometry` | area — rigid body (translate + rotate) | boundary | area |
| `tpointcloud` (`tpcpoint`, `tpcpatch`) | point set | extent | cloud |
| `th3index`, `tquadbin` | discrete cell index | — | none — no continuous distance |

`tnpoint` and `tpose` are same-family as `tgeompoint`: all three are points, so
all three expose the identical one-directional container surface. `tnpoint` is a
fraction-delta along a reference edge and `tpose` is a point plus an orientation,
but the geometry distance sees is a point in every case — they reach it through
the MEOS C casts `tnpoint_to_tgeompoint` and `tpose_to_tpoint` (§4). `tcbuffer` is
*not* their family: its value is a disc, so it exposes both container directions
and its box is radius-aware.

### 6.2 Kernel axis — turning-point strategy, exactness, and reduction

| temporal type | turning-point strategy | exactness | `nad` reduction |
| --- | --- | --- | --- |
| `tint`, `tbigint` | none — step, extremum at endpoints | exact | materialise `tdistance` + `min` |
| `tfloat` | closed-form unary (linear gap) | exact | materialise `tdistance` + `min` |
| `tgeompoint` | closed-form unary | exact (planar) | synchronised running-min fast-path |
| `tgeogpoint` | closed-form unary, chordal | approximate — interior turning point linearised, not geodetic | synchronised running-min fast-path |
| `tnpoint` | closed-form unary (via `→ tgeompoint`) | exact (planar) | materialise `tdistance` + `min` |
| `tpose` | closed-form unary (via `→ tpoint`) | exact (planar) | materialise `tdistance` + `min` |
| `tcbuffer` | closed-form unary, radius-aware | exact (planar) | synchronised running-min fast-path (shared radius-aware kernel) |
| `tgeometry` | closed-form per-segment geo | exact (planar) | materialise `tdistance` + `min` |
| `tgeography` | closed-form per-segment geo, chordal | approximate — as `tgeogpoint` | materialise `tdistance` + `min` |
| `trgeometry` | adaptive ε-bisection (n ≥ 2 turning points, no closed form) | ε-bounded — converges within `MEOS_EPSILON` | vs geometry: materialise `tdistance` + `min`; vs `tpoint` / vs `trgeometry`: raises `NOT_IMPLEMENTED` |
| `tpointcloud` | none — bounding-box distance (`nad_stbox_stbox`) | box lower bound, not point-exact | box-level, no `tdistance` |
| `tbool`, `ttext`, `tjsonb`, `th3index`, `tquadbin` | — | — | none |

Three facts this table records.

**The synchronised running-min reduction is carried by `tgeompoint`,
`tgeogpoint`, and `tcbuffer`** through one shared radius-parameterised kernel
(`nad_tcont_tcont_sync`), the disc radius `0` for the point types and `r(t)` for
`tcbuffer` — the `g(t) = dist(centre, ·) − r(t)` unification of §2 realised as a
single walk. Every other family computes `nad` by materialising the whole
`tdistance` temporal float and taking `temporal_min_value` of it — the same
answer, built as the entire temporal float before reducing, the pattern §2 names
as the laggard. `tnpoint` and `tpose` reduce to a point, so the running-min
scaffold applies to them after their cast with no new mathematics.

**The radius lives on the kernel axis, not the family axis.** The closed-form
unary turning-point function is one function parameterised by `r(t)` — `0` for the
point types, `r(t)` for `tcbuffer`. It is exact in both cases. This is why a
point family is the `r ≡ 0` special case of the area kernel (§2) rather than a
separate problem, and why sharing the kernel does not merge the families.

**`trgeometry` carries the reduction but not the two-moving-body kernel.** The
generic adaptive turning-point strategy (`tfunc_tlinearseq_adaptive`, depth-bounded
by `MEOS_ADAPTIVE_MAX_DEPTH`) and the running-min reduction are present, and the
one-moving-body path `tdistance_trgeometry_geo` carries the pattern end to end
through the `solve_s_tpoly_point` bisection. What raises `NOT_IMPLEMENTED` is the
*two-moving-body* per-segment kernel — sampling `dist(trgeo₁@t, trgeo₂@t)` and
bisecting the segment while both operands move — named in a code comment as
`trgeo_pair_dist_adaptive` but not defined. It clones the one-moving bisection to
sample both sequences. Its result is ε-bounded, so it is admissible only as a
doc-marked convergent approximation, never silently, and it does not join the exact
closed-form kernel of the point and disc types.

**`tgeogpoint` and `tgeography` interior extrema are chordal, not geodetic.** The
distance *values* at synchronised instants are exact geographic distances, but the
*location* of an interior turning point comes from linearising the two segments in
3-D Cartesian space (`point3d_min_dist`), not on the sphere, so the reported
minimum can sit slightly off the true geodesic minimum. The source marks this
`TODO`; a geodetically exact turning point uses the chordal value as the seed of an
iterative refinement.
