<!--
  MobilityDB — Adjacency: Design Notes
  Copyright(c) MobilityDB Contributors

  This documentation is licensed under a Creative Commons Attribution-Share
  Alike 3.0 License: https://creativecommons.org/licenses/by-sa/3.0/
-->

# Adjacency — Design Notes

Adjacency is the one bounding-box operator whose answer is not forced. Containment,
overlap and the position operators each have a single defensible reading; `-|-` asks
where the *boundary* of a value is, and a value with holes has more than one boundary
worth naming. This note records the rule the span family answers by, the evidence for
it, and the rule the two box types answer by — one rule for both, and §5 records why the
two obvious alternatives to it are each indefensible.

> **Audience**: a contributor or binding author working at the MEOS C level.
> This is design rationale, not user documentation — the user manual
> (`doc/set_span_types.xml`, `doc/box_types.xml`) states the rules these notes
> derive, and intentionally does **not** carry the derivations.

Every table below transcribes live output, and §7 carries the reproduction recipe. The
one exception is the `R1` and `R2` columns of §5: those two rules are the alternatives
R3 is argued against, so their columns are read from the definitions given there rather
than from a build.

---

## 1. The one-dimensional rule

Two spans are adjacent when the upper bound of one **is** the lower bound of the other:

```c
datum_eq(upper1, lower2) || datum_eq(upper2, lower1)
```

The inclusivity bits are **not** consulted. That is not an omission — it is what makes
the rule work for both kinds of base type, because the bits have already done their job
by the time the comparison runs.

### 1.1 Canonicalisation is what carries the discrete case

A discrete base type (`int`, `bigint`, `date`) canonicalises every span to the half-open
form `[lower, upper)`. A continuous one (`float`, `timestamptz`) keeps the bounds as
written:

| literal | stored as |
|---|---|
| `intspan '[1,3]'` | `[1, 4)` |
| `intspan '(1,3)'` | `[2, 3)` |
| `intspan '[3,3]'` | `[3, 4)` |
| `floatspan '[1,3]'` | `[1, 3]` |
| `floatspan '(1,3)'` | `(1, 3)` |
| `floatspan '[3,3]'` | `[3, 3]` |

So for an integer span the closure question and the bound-equality question are the same
question, and for a float span the bits are already inside the bounds the comparison
reads. One rule, two type classes, no special case.

### 1.2 The same literals, two different answers

This is the pair worth keeping in mind, because it looks like an inconsistency and is
not:

| query | answer |
|---|---|
| `intspan '[1,3]' -|- intspan '[4,5]'` | **t** |
| `intspan '[1,3]' -|- intspan '[3,5]'` | **f** |
| `floatspan '[1,3]' -|- floatspan '[4,5]'` | **f** |
| `floatspan '[1,3]' -|- floatspan '[3,5]'` | **t** |

Read through the canonical forms it is immediate. `intspan '[1,3]'` is `[1,4)` and
`intspan '[4,5]'` is `[4,6)` — they meet at 4, so **t**; `intspan '[3,5]'` is `[3,6)`,
which *overlaps* `[1,4)`, and an overlap is not an adjacency, so **f**. The float pair
answers the other way because no canonicalisation applies: `[1,3]` and `[3,5]` meet at 3,
and `[1,3]` and `[4,5]` have a gap.

⛔ The lesson generalises: **an integer example and a float example of "the same" query
are not the same query.** Any adjacency argument stated without naming the base type is
under-specified.

Further float cases, all **t**, showing the bits genuinely do not participate:

```
floatspan '[1,3]' -|- floatspan '(3,5]'    → t     (closed meets open)
floatspan '[1,3)' -|- floatspan '(3,5]'    → t     (open meets open — nothing is at 3)
floatspan '[2,5)' -|- floatspan '(5,6)'    → t
```

⭐ The third one is the decisive case against any rule of the form *"intersect, then
require the intersection to be degenerate"*: `[2,5)` and `(5,6)` do not intersect at all,
and they are adjacent. Keep it in hand for §5.

---

## 2. The representation identities

A value, a singleton span and a singleton span set are three spellings of one thing:

```
3  ≡  intspan '[3,3]'  ≡  intspanset '{[3,3]}'
[a,b]  ≡  {[a,b]}
```

These are identities of the type system, not conveniences. `span(v, v, true, true)` and
`spanset(ARRAY[...])` construct exactly what the literal parses to. Any operator that
answers differently for the three is answering about the *representation* rather than
about the value, and that is a defect regardless of which answer one prefers.

⇒ **Every proposed adjacency rule must be checked against these identities before it is
checked against anything else.** It is the cheapest filter available and it eliminates
most candidates.

---

## 3. Span sets: three tiers, and only one of them is a choice

The manual titles this section **"Bounding Box Operations"** in every family, span types
included. That is already a statement: for these operators the bounding box is the
contract. What needs a rule is when an implementation may answer *more precisely* than
that contract.

> **The rule.** A bounding-box operator is defined by the bounding box of its operands.
> An implementation may return the exact answer instead **only when the exact answer is
> contained in the box answer**. Where it is not contained, the box answer *is* the
> semantics.

It decides all three groups without a per-operator convention.

**Containment, overlap, equality — refine.** For `@>`, `<@`, `&&`, `~=` the exact answer
is a *subset* of the box answer: the box can only say "maybe", and reading the components
removes false positives. That is what an index recheck does, so refining changes no
contract. Span sets carry their components, so they refine —
`contains_spanset_span` and `overlaps_spanset_span` are written as "bounding-box test,
then binary search over the components", and `intspanset '{[1,3),[5,8)}' @> 4` is
correctly **false**. Temporal types do *not* refine: `contains_tnumber_tnumber` is
`contains_tbox_tbox` on the two boxes, gaps ignored. Both behaviours are legal under the
rule, because refinement is permitted, not required.

**Position — nothing to refine.** `S << x` asks whether *every* element is left of `x`,
and a universally quantified statement over a set reduces to its extreme. The exact
answer *equals* the hull answer; it is a theorem, not a choice. That is why
`left_spanset_value` reads only the last component and is hull-equivalent by
construction.

**Adjacency — the box answer is the semantics.** Here the per-component answer is **not**
contained in the box answer: it *adds* trues at the inner boundaries. So it is not a
refinement at all, it is a different question — and one no bounding-box index can answer.
Under the rule, the box answer is therefore the definition.

For a plain span all three tiers coincide, its bounding box being itself, so §1 is the
one-dimensional case of everything above.

---

## 4. What the sweep shows

`S = intspanset '{[1,3),[5,8),[10,12)}'`, hull `[1,12)`. Each value `v` asked three
ways — as the value, as `[v,v]`, as `{[v,v]}` — which by §2 must agree.

**Integer, current master:**

| `v` | `S -|- v` | `S -|- [v,v]` | `S -|- {[v,v]}` |
|---|---|---|---|
| 0 | t | t | t |
| 1–11 | f | f | f |
| 12 | t | t | t |
| 13 | f | f | f |

**Float** (`floatspanset '{[1,3),[5,8),[10,12)}'`), current master:

| `v` | `S -|- v` | `S -|- [v,v]` | `S -|- {[v,v]}` |
|---|---|---|---|
| 0 | f | f | f |
| 1 | t | t | t |
| 2–11 | f | f | f |
| 12 | t | t | t |
| 13 | f | f | f |

Three columns, zero disagreements, in both base types — the identities hold.

⭐ **The two tables differ, and that difference is the correct answer, not noise.** The
integer sweep fires at 0 and the float sweep at 1, for the same span set written the same
way. `0` canonicalises to `[0,1)`, whose upper bound 1 is the hull's lower bound, so an
integer 0 is adjacent to a span set starting at 1; the float `0` is `[0,0]` and 1 is
`[1,1]`, and only the latter meets the hull. This is §1.2 again, one dimension up.

⛔ A per-component reading breaks both properties at once. It fires at 0, 3, 9, 12 for
the integer value column and at 1, 3, 10, 12 for the float one, while the two span
columns fire only at the hull ends — so `S -|- 3` answers **t** where
`S -|- intspan '[3,3]'` answers **f**, on one value written two ways. The extra hits are
*interior* boundaries: 3 ends the first component, 9 and 10 sit at a gap edge. Neither
the set of values nor the disagreement between the columns is predictable from the
value.

### 4.1 Why the index cannot follow the per-component reading

A GiST/SP-GiST key for a span set is its **bounding span** — one span, the components are
not in the key. An index member is sound only when

```
OP(A, B)  ⟹  Q(key(A), key(B))
```

Under the per-component reading `S -|- 3` is true while `hull(S) -|- 3` is false, so the
implication fails: an index that filtered on the key would drop a qualifying row and
return fewer rows than a sequential scan. A false positive is removable by recheck; a
false negative is not, and silently wrong results are the worst failure mode a bounding
box operator has.

⇒ Under the per-component reading `-|- (spanset, value)` is **not indexable at all**. Under
the hull reading it is *exactly* indexable, and the operator classes carry it:
`OPERATOR 17 -|- (intspanset, integer)` sits in each of the five span-set types across
`rtree`, `quadtree` and `kdtree`. That is a capability the rule buys, not a side effect.

The soundness is exactness rather than mere safety, and it needs no new C: the GiST and
SP-GiST leaf key *is* the bounding span (`Spanset_gist_compress` and
`Spanset_spgist_compress` both call `spanset_span_slice`), `span_gist_get_span` turns a
base-type scan key into `[v,v]`, and `adjacent_span_value(s,v)` is literally
`adjacent_span_span(s,[v,v])` (`meos/src/temporal/span_ops.c`). So the leaf answers the
operator exactly, and both inner consistents already write `adjacent || overlaps`
(`span_index.c`, `span_spgist.c`) — the widening belongs there, never in the operator.

---

## 5. The boxes: one rule, and the two alternatives it rules out

Both box types answer **R3**: *the extents meet in every dimension the boxes share, and
touch at a single value in at least one of them.* `adjacent_tbox_tbox` puts a
per-dimension conjunct in front of its disjunction:

```c
bool adjx = false, adjt = false;
if (hasx)
{
  adjx = adjacent_span_span(&box1->span, &box2->span);
  if (! adjx && ! overlaps_span_span(&box1->span, &box2->span))
    return false;
}
if (hast)
{
  adjt = adjacent_span_span(&box1->period, &box2->period);
  if (! adjt && ! overlaps_span_span(&box1->period, &box2->period))
    return false;
}
return (adjx || adjt);
```

`adjacent_stbox_stbox` asks the same question dimension by dimension — `meet_extent_extent`
for each closed spatial extent, `adjacent_span_span`/`overlaps_span_span` for the period —
and returns whether some dimension touches.

The rest of this section is the argument for R3, because the two alternatives are each
defensible on first reading and each provably wrong:

- **R1** — *adjacent in at least one dimension*: a bare `return (adjx || adjt)`, which is
  what `adjacent_tbox_tbox` reduces to without the two conjuncts above.
- **R2** — *intersect, then require the intersection to be flat*: compute
  `inter_stbox_stbox`, answer false when the boxes do not intersect at all, and otherwise
  ask whether the intersection is degenerate in some dimension
  (`inter.xmin == inter.xmax || … || inter.period.lower == inter.period.upper`).

### 5.1 The divergence R1 and R2 produce, and what R3 answers

Same shape, both types, X touching at 5 and the two periods five months apart:

| query | R1 / R2 | R3 |
|---|---|---|
| `tbox 'TBOXFLOAT XT([1,5],[2000-01-01,2000-01-05])' -|- tbox 'TBOXFLOAT XT([5,9],[2000-06-01,2000-06-05])'` | **t** | **f** |
| `stbox 'STBOX XT(((1,1),(5,5)),[2000-01-01,2000-01-05])' -|- stbox 'STBOX XT(((5,1),(9,5)),[2000-06-01,2000-06-05])'` | **f** | **f** |

Under the two former rules, two boxes that share nothing but an X coordinate and lie half
a year apart in time are adjacent as a `tbox` and not adjacent as an `stbox` — the same
shape, two answers. R1 accepts a pair whose closures never meet; R3 answers **f** for both,
because the time dimension alone separates them.

⚠️ The integer twin of that `tbox` query answers **f** —
`tbox 'TBOXINT XT([1,5],…)' -|- tbox 'TBOXINT XT([5,9],…)'` — because `[1,5]` is `[1,6)`
and `[5,9]` is `[5,10)`, which overlap in X rather than meeting. §1.2 reaches here too:
a `tbox` example proves nothing until its base type is named.

### 5.2 The rest of the measured behaviour

`stbox`, no time dimension — R3 agrees with R2 on every one of these:

| configuration | answer |
|---|---|
| share an edge — `X((1,1),(5,5))` vs `X((5,1),(9,5))` | t |
| share a corner — `X((1,1),(5,5))` vs `X((5,5),(9,9))` | t |
| overlapping area — `X((1,1),(5,5))` vs `X((3,3),(9,9))` | f |
| gap — `X((1,1),(5,5))` vs `X((6,1),(9,5))` | f |

`stbox`, with time:

| configuration | R2 | R3 |
|---|---|---|
| X touches, periods touch (closed) | t | t |
| XY identical, periods touch (closed) | t | t |
| XY identical, periods touch **half-open** `[…,Jan05)` / `[Jan05,…)` | **f** | **t** |
| X overlaps, periods touch (closed) | t | t |
| X touches, periods far apart | f | f |

⭐ The half-open row is the sharpest thing in this section. Two boxes covering the same
square, whose periods meet exactly at Jan 05 with the first excluding it, are adjacent —
as the same two periods are as bare `tstzspan` values, and as a `tbox` too. R2 loses them
to its intersect-first construction: half-open periods that meet do not intersect, so
`inter_stbox_stbox` fails before any flatness test runs. Compare
`floatspan '[2,5)' -|- '(5,6)'` = **t** from §1.2 — intersect-then-flatten contradicts the
one-dimensional rule on its own terms, which is why R3 asks each dimension directly.

⭐ The other thing R2 gets wrong points the same way, and it is the shape to recognise on
sight. A box lying **strictly inside** another intersects it in a degenerate box, so R2
calls the pair adjacent:

| configuration | R2 | R3 |
|---|---|---|
| `X((1,1),(5,5))` vs the point `X((3,3),(3,3))` | **t** | **f** |

A point inside a box is contained, not adjacent, and §1 says so one dimension down:
`floatspan '[1,5]' -|- floatspan '[3,3]'` = **f**. ⛔ This is also why R2 cannot be
repaired by intersecting *closures* instead: a degenerate intersection is not the same
predicate as a touch, and only the per-dimension question distinguishes them.

`tbox`, single dimension:

| configuration | answer |
|---|---|
| X touch only, no T | t |
| X overlap only, no T | f |
| T touch only, no X | t |

⇒ the `tbox` anomaly appears **only with two dimensions present**. With one, `adjx || adjt`
degenerates to the correct one-dimensional rule.

### 5.3 The asymmetry that explains — but does not excuse — the difference

The two structs are not parallel:

```c
typedef struct { Span span;   Span period; … } TBox;    /* both dims carry inc/exc bits */
typedef struct { Span period; double xmin, xmax, ymin, ymax, zmin, zmax; int32 srid; … } STBox;
```

An `STBox` spatial dimension is a pair of doubles with **no open/closed distinction** —
every spatial extent is closed — while its period is a full `Span`. A `TBox` carries a
`Span` in both dimensions. So the two types genuinely have different information
available, and a spatial extent has no half-open case to get wrong.

That justifies the two types differing in *how a dimension is compared*. It does not
justify them differing in *how the dimensions are combined*, which is what §5.1 measures:
one type takes the disjunction, the other takes an intersection-and-flatten. The
combination rule is a semantic decision and it is shared, or it should be.

### 5.4 The three candidate combination rules

| | rule | fails |
|---|---|---|
| **R1** | adjacent in ≥1 dimension | §5.1 — accepts boxes whose closures never meet |
| **R2** | intersect, then intersection degenerate in ≥1 dimension | §5.2 half-open row, §5.2 point-inside row, and the 1-D case `[2,5) -|- (5,6)` |
| **R3** | closures meet in every common dimension **and** touch in ≥1 | — (the rule both boxes carry) |

R3 is the *n*-dimensional statement of §1: two boxes are adjacent when they share a face,
edge or corner and are separated by nothing. It reduces to §1 in one dimension, it accepts
the corner and edge cases §5.2 gets right, and it rejects both the far-apart pair of §5.1
and the half-open loss of §5.2.

Read against the §3 rule, R1 and R2 fail in *opposite directions*, which is why one note
covers both:

- **R1 is a widening** — it accepts more than the true box answer. A widening may never
  be the semantics; its correct home is the index's inner-node filter, and
  `tbox_index.c` already writes exactly `adjacent_tbox_tbox(...) || overlaps_tbox_tbox(...)`
  there. The leaf test in the same file is the *exact* function, which is the proof that
  the operator itself is meant to be exact.
- **R2 is a narrowing** — it rejects pairs that are genuinely adjacent, i.e. it produces
  false negatives, the same failure class §4.1 rules out for the index.

⇒ One rule governs both halves: the span half rejects an answer that is not a
refinement, the box half an answer that is a widening. Same principle, two directions.

### 5.5 How far the rule reaches

The rule is not confined to the boxes. `temporal_boxops_meos.c:695/712/728` passes
`&adjacent_tbox_tbox` to `adjacent_tnumber_tbox`, `adjacent_tbox_tnumber` and
`adjacent_tnumber_tnumber`, so every temporal-number adjacency answer follows what `tbox`
decides; `tspatial_topops_meos.c:247/261/274` wires `&adjacent_stbox_stbox` the same way
for every temporal spatial type.

Ten test files discriminate the rule, over **33 expected-output lines** — the set a
change to it has to answer for:

| test | lines | | test | lines |
|---|---|---|---|---|
| `021_tbox` | 1 | | `162_trgeo_indexes_tbl` | 1 |
| `032_temporal_topops` | 1 | | `208_tcbuffer_topops` | 4 |
| `046_tnumber_selfuncs_tbl` | 10 | | `216_tcbuffer_indexes_tbl` | 6 |
| `110_tpose_indexes_tbl` | 2 | | `317_tnpoint_indexes_tbl` | 3 |
| `156_trgeo_topops` | 4 | | `558_tposechain_topops_tbl` | 1 |

⭐ **Every one of the 33 is a removal** — a count going down, or `t`→`f`. Not one answer
turns from false to true, which says something the diff alone does not: the suite
exercises R1's widening and R2's point-inside artifact, and covers the half-open period
case of §5.2 **nowhere**. That half of the rule would ship untested, so the discriminating
cases are pinned directly — `021_tbox` for the widening, `051_stbox` for the half-open and
point-inside rows.

⭐ **The way to check a box rule is an oracle, not the diff.** Re-express the candidate in
the settled 1-D operators of §1 and require `operator == oracle` over the fixtures:

```sql
-- R3 for a tbox pair, written only in terms of span adjacency and overlap
WHERE (t1.b::intspan  -|- t2.b::intspan  OR t1.b::intspan  && t2.b::intspan)
  AND (t1.b::tstzspan -|- t2.b::tstzspan OR t1.b::tstzspan && t2.b::tstzspan)
  AND (t1.b::intspan  -|- t2.b::intspan  OR t1.b::tstzspan -|- t2.b::tstzspan)
```

Both sides answer 0 on `tbl_tboxint`, and the conjuncts show that zero is a real answer
rather than a stuck predicate: *X touch only* = 98 — exactly what R1 returns — while
*X meet* = 705, *T meet* = 99 and *X touch AND T meet* = 0. Every one of those 98 pairs
shares a value bound while its periods lie apart, which is §5.1 at fixture scale. The
`stbox` side agrees the same way: operator 0, oracle 0, over 1000 rows of which 664 meet
in all three dimensions while none touches in any.

⛔ A cross-type change like this one must sweep **every** family's `*topops` expecteds,
including families newer than the change — all 21 such test files, `460_tjsonb_topops`,
`436_tpc_topops`, `308_tnpoint_topops` and `108_tpose_topops` among them.

---

## 6. What the whole note settles

**The one-dimensional rule of §1** — bound equality, bits not consulted, discrete types
carried by canonicalisation. **A span set answers `-|-` as its bounding span answers**, for
all three operand shapes, and the identities of §2 hold across value, span and span set.

**R3 is the box rule**, and both box types carry it. Each of the four questions this note
opened has an answer, and each answer falls out of the §3 rule rather than out of taste:

1. **Is R3 the rule?** Yes. It is the only candidate that survives every measurement here,
   it is the *n*-dimensional statement of §1, and R1 and R2 fail it in opposite
   directions — a widening and a narrowing.
2. **Is the `stbox` spatial dimension a deliberate exception?** It is an exception in *how
   a dimension is compared* and not in *how the dimensions are combined*. Its extents are
   closed by construction, so `meet_extent_extent` compares two `double` pairs where the
   period uses `adjacent_span_span`; the combination is the same question in both types.
3. **What is the blast radius?** 33 lines over 10 files, all of them removals — §5.5.
4. **Should `adjacent(tnumber, tnumber)` answer on the box at all?** Yes. §3 settles it:
   the per-component answer *adds* trues at inner boundaries, so it is not a refinement of
   the box answer and no bounding-box index can answer it. For adjacency the box answer is
   the definition.

⭐ The one asymmetry worth carrying away: the span half of this note rejects an answer that
is not a *refinement*, and the box half rejects an answer that is a *widening*. Both follow
from the single §3 rule, applied in the two directions it can fail.

---

## 7. Reproducing every measurement

Every query behind every table appears verbatim in the section that quotes it, so the
tables are reproducible by pasting them into `psql` against a build of master. The sweeps
of §4 come from

```sql
SELECT v,
       intspanset '{[1,3),[5,8),[10,12)}' -|- v                                   AS "-|- v",
       intspanset '{[1,3),[5,8),[10,12)}' -|- span(v,v,true,true)                 AS "-|- [v,v]",
       intspanset '{[1,3),[5,8),[10,12)}' -|- spanset(ARRAY[span(v,v,true,true)]) AS "-|- {[v,v]}"
FROM generate_series(0,13) v;
```

and the float table from the same statement over `floatspanset` and `v::float`. §4.1 is
read from the plan: with the operator-class member in place, `tbl_intspanset` gives
`Index Scan using tbl_intspanset_{rtree,quadtree,kdtree}_idx` for `i -|- 96`.

The box tables of §5 need no fixtures — every query is a pair of literals. The oracle of
§5.5 does: it runs over `tbl_tboxint` and `tbl_tcbuffer_big`, and its value is that it
never mentions a box operator, so it cannot inherit the bug it is checking for.

---

Related: `doc/set_span_types.xml` § "Topological Operations" carries the user-facing
statement of §1–§3, and `doc/box_types.xml` § "Topological Operations" the box rule of §5,
both in English and Spanish.
