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

**The `ever(tR)` / `always(tR)` projections define correctness; they are not the
implementation.** `eR ≡ ever(tR)` and `aR ≡ always(tR)` are the oracle a test
uses. Computing them by materialising the full temporal Boolean and projecting
∃/∀ resolves every sub-period of every segment only to ask whether one exists.
The running reduction answers the same question and stops as soon as it is
decided.

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
| `disjoint` prefilter | **no** | a box miss would prove disjointness and prune true rows |
| exact `nad`, `minDistance`, `|=|` | **no** | no threshold exists; the nearest pair can sit just outside the box |

Soundness of the threshold prune depends on the box enclosing the whole value.
For `tcbuffer` this holds because the box is **radius-aware** — the extent is
expanded by the radius, so it encloses the swept disc, not merely the centre
path. A family whose box covered only a centre would need that expansion before
the prune became valid.

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
