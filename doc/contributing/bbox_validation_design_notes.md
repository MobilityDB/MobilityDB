<!--
  MobilityDB — Bounding Box Validation: Design Notes
  Copyright(c) MobilityDB Contributors

  This documentation is licensed under a Creative Commons Attribution-Share
  Alike 3.0 License: https://creativecommons.org/licenses/by-sa/3.0/
-->

# Bounding Box Validation — Design Notes

Every operation on a pair of bounding boxes answers two questions before it
answers the one it was asked. First: **are these two boxes comparable?** Two boxes
whose coordinates are in different spatial reference systems are metres and feet —
no `Min` or `Max` between them means anything, whatever the operation. Second: **do
both boxes carry the dimensions this operation reads?** A position operator on the
X axis needs an X extent in both operands; a topological operator needs only that
they share some dimension.

Those are different questions with different answers, and they are kept in two
layers. This note states the contract both layers impose, so that a new bounding
box type can be designed against it.

> **Audience**: a contributor writing or changing an `ensure_valid_*` for a box
> type, or designing a new one. For *wiring* a new bounding box into the
> machinery — `bboxunion`, `bbox_get_size`, the `temporal_boxops.c` dispatch, the
> GiST picksplit — see `new_temporal_type.md` §3.4. This note is about what the
> box must then *guarantee*.

---

## 1. Extent and frame

Every bounding box struct holds two kinds of field, and telling them apart is the
whole of the design.

**Extent** fields are what the box bounds — the numbers an operation reads and
compares. **Frame** fields say what those numbers *mean*: the reference system,
unit, or schema in which the extent is measured.

> A field is a frame field if **changing it changes what the same numbers mean.**

The four box types partition as follows.

| box | extent | frame |
| --- | --- | --- |
| `Span` | `lower`, `upper`, `lower_inc`, `upper_inc` | `spantype`, `basetype` |
| `TBox` | `period` and `span` bounds | the span type of the value span |
| `STBox` | `period`, `xmin`..`zmax` | `srid`, geodetic flag |
| `TPCBox` | `period`, `xmin`..`zmax` | `srid`, geodetic flag, `pcid` |

A `1` in an `intspan` and a `1.0` in a `floatspan` are not the same quantity, so
the span type is frame. An `x` of 5 in EPSG:4326 and an `x` of 5 in EPSG:5676 are
not the same place, so the SRID is frame. Two `TPCBox` extents drawn from
different pgpointcloud schemas measure different dimensions, so `pcid` is frame.

The two kinds have opposite rules:

- **Extent is compared.** That is the operation's job.
- **Frame must match.** That is layer 1's job, and it is checked before anything
  else happens.

### 1.1 The `flags` word holds both kinds

`flags` is not uniformly one or the other, and this is the trap when adding a box
type. The `X` / `Z` / `T` bits say **which dimensions are present** — extent
metadata, belonging to layer 2. The `GEODETIC` bit says **what the coordinates
mean**, planar or spherical — frame, belonging to layer 1.

Classify each new flag bit deliberately. A frame bit checked as presence lets two
incompatible boxes through; a presence bit checked as frame rejects legitimate
operands.

---

## 2. Layer 1 — comparability

One validator per box type, checking every frame field and nothing else:

```
ensure_valid_span_span        span type
ensure_valid_tbox_tbox        span type of the value span
ensure_valid_stbox_stbox      SRID + geodetic
ensure_valid_tpcbox_tpcbox    SRID + geodetic + pcid
```

One per box type, so a new box type brings exactly one new validator and the count
is never a matter of choice.

`ensure_valid_span_span` is the reference shape — null checks, then the frame
predicates, then nothing:

```c
static inline bool
ensure_valid_span_span(const Span *s1, const Span *s2)
{
  VALIDATE_NOT_NULL(s1, false); VALIDATE_NOT_NULL(s2, false);
  return ensure_same_span_type(s1, s2);
}
```

Note what it does **not** do: no dimension test, no flag computation, no operation
qualifier in its name. A validator that also writes output parameters cannot be
called by an operation that only wants validation, and an operation-shaped name
(`_pos_`, `_spatial_`) invites a second validator for the next operation class —
the duplication the two layers exist to prevent.

Where the body is a pure chain of predicates it lives `static inline` in the
type's header, as `Span`'s does. A validator that must call into a `.c` file is an
ordinary exported function.

### 2.1 Composition with the frame of a related box

A box whose frame extends another's reuses that one's predicates rather than
restating them. `TPCBox` carries `STBox`'s frame plus `pcid`, so its validator
checks SRID and geodetic exactly as `STBox` does and then adds one predicate.
`TBox`'s value extent is a `Span`, so its frame check is `Span`'s, applied to that
member.

This is what keeps the rule single-sourced: when the meaning of "same SRID"
changes, it changes in one predicate, not in every box type that has an SRID.

### 2.2 Reaching them from generic code

The per-type validators are the *implementations*. Code that holds a `Temporal *`
and a bounding box it cannot name reaches them through the dispatcher of §3.

---

## 3. Dispatch — a bounding box is a late-bound abstract type

No generic code names a box type. Every operation on a bounding box whose caller
is generic is written once, as a dispatch on the box type the catalog prescribes
for the value's temporal type:

```c
MeosType bboxtype = type_bboxtype(temptype);
assert(bboxtype != T_UNKNOWN);
if (bboxtype == T_TSTZSPAN)       return span_<op>((Span *) box1, ...);
else if (bboxtype == T_TBOX)      return tbox_<op>((TBox *) box1, ...);
#if POINTCLOUD
else if (bboxtype == T_TPCBOX)    return tpcbox_<op>((TPCBox *) box1, ...);
#endif
else /* T_STBOX */                return stbox_<op>((STBox *) box1, ...);
```

The `MeosType` is the *temporal* type; `type_bboxtype` turns it into the box type.
So the tag is **derived from the catalog, never supplied by the caller** — which is
what makes the `void *` operands safe. Each arm casts to a real type and hands off
to the function that owns the behaviour; nothing generic reads a box's fields.

### 3.1 The dispatch table

| dispatcher | answers |
| --- | --- |
| `bbox_type` | is this `MeosType` a box type |
| `bbox_get_size` | the box's size in bytes |
| `bbox_max_dims` | how many dimensions it can hold |
| `temporal_bbox_eq` | are two boxes equal |
| `temporal_bbox_cmp` | their B-tree order |
| `temporal_bbox_size` | the box size for a temporal type |
| `bbox_expand` | merge the first box into the second |
| `ensure_valid_bbox_bbox` | are two boxes comparable (§2) — **proposed, not yet in the tree** |

Seven of those eight exist. `ensure_valid_bbox_bbox` does not: the tree validates
comparability through the per-type entries — `ensure_valid_stbox_stbox`,
`ensure_valid_tbox_tbox`, `ensure_valid_span_span` and their siblings — and every
caller names the one its operand type needs. The dispatcher below is what this note
PROPOSES so that generic code has one entry to call (§3.2); it is written out to fix
the shape, not to describe code that ships.

`ensure_bbox_temporal_compatible` and `ensure_same_index_bboxtype` are `ensure_*`
members of the same family, so a validating dispatcher is the family's own shape
rather than a new idea:

```c
bool
ensure_valid_bbox_bbox(const void *box1, const void *box2, MeosType temptype)
{
  MeosType bboxtype = type_bboxtype(temptype);
  assert(bboxtype != T_UNKNOWN);
  if (bboxtype == T_TSTZSPAN)
    return ensure_valid_span_span((Span *) box1, (Span *) box2);
  else if (bboxtype == T_TBOX)
    return ensure_valid_tbox_tbox((TBox *) box1, (TBox *) box2);
#if POINTCLOUD
  else if (bboxtype == T_TPCBOX)
    return ensure_valid_tpcbox_tpcbox((TPCBox *) box1, (TPCBox *) box2);
#endif
  else /* T_STBOX */
    return ensure_valid_stbox_stbox((STBox *) box1, (STBox *) box2);
}
```

`bbox_expand` is the caller that most needs it: it merges two boxes on behalf of
any temporal type, so it is the site that cannot name its operands and the site
where two boxes that are not comparable must not be merged.

### 3.2 A dispatch never falls through to an assumed type

**An arm that assumes the residue is banned**, in both spellings:

```c
else /* T_STBOX */                                  /* BANNED */
  return stbox_<op>(...);

switch (temp->subtype) {
  case TINSTANT:  return ...;
  case TSEQUENCE: return ...;
  default: /* TSEQUENCESET */                       /* BANNED */
    return (Temporal *) tnumberseqset_abs((TSequenceSet *) temp);
}
```

Both cast on an assumption. A value that is not the assumed type is not rejected;
it is **reinterpreted** — read at the assumed type's offsets — which is a wild
access rather than a diagnosable error.

The correct form names every case and rejects what is left. In an `if` chain — the
shape the dispatchers of §3.1 use — the last type gets a named arm like every
other, and the `else` errors:

```c
if (bboxtype == T_TSTZSPAN)     return span_<op>(...);
else if (bboxtype == T_TBOX)    return tbox_<op>(...);
#if POINTCLOUD
else if (bboxtype == T_TPCBOX)  return tpcbox_<op>(...);
#endif
else if (bboxtype == T_STBOX)   return stbox_<op>(...);
else
{
  meos_error(ERROR, MEOS_ERR_INVALID_ARG_TYPE,
    "Unknown bounding box type: %s", meostype_name(bboxtype));
  return false;
}
```

In a `switch`, every case is named and the error follows the switch rather than
occupying a `default:` label — the placement is what keeps `-Wswitch` alive:

```c
switch (geom->type) {
  case POINTTYPE:   return ...;
  case LINETYPE:    return ...;
  case POLYGONTYPE: return ...;
}
meos_error(ERROR, MEOS_ERR_INVALID_ARG_TYPE,
  "Unsupported geometry type: %d", geom->type);
return NULL;
```

Both spellings are already the house majority — `MEOS_ERR_INVALID_ARG_TYPE`
appears at 54 sites, and 55 `else` arms end in a `meos_error` — so what follows is
a deviation to erase rather than a new convention.

**This is not a style preference — the banned form gives up two independent
protections at once.**

*Compile time.* The build runs `-Wall -Wextra`, so `-Wswitch` warns when a switch
over an enumeration omits a member. **A `default:` label suppresses that warning
entirely.** Writing `default:` therefore buys silence today at the cost of the one
check that would have caught a new enumerator on the day it is added. Naming every
case restores it, which is why the error belongs *after* the switch rather than in
a `default:` arm.

*Run time.* An `assert` does not substitute. The asserts guarding these dispatches
test only that the value is of the right family — not that this dispatch handles
it — and they compile out of a release build, so the shipped binary carries no
check at all.

`TPCBox` shows how quietly it fails: its leading fields are byte-identical to
`STBox` by design, so a missing arm still returns plausible answers while silently
dropping `pcid`. A type without that shared prefix returns garbage instead.
Neither outcome is visible to the caller.

**The rule holds for every dispatch on a type, not
only for bounding boxes** — the same reinterpretation is available wherever one is
written, and `switch (geom->type)` over the liblwgeom types is the other place it
bites.

**The exception: a closed set that is asserted first.** `TINSTANT` / `TSEQUENCE` /
`TSEQUENCESET` are fixed by the data model — a temporal value has no fourth
subtype and will not acquire one — and every dispatch on `subtype` is preceded by

```c
assert(temptype_subtype(temp->subtype));
```

which rejects an out-of-range value before the switch is reached. With the set
closed and the value asserted, `default: /* TSEQUENCESET */` names the only
remaining member and cannot be reached by anything else. That form stays.

The distinction is not "enum versus not" but **whether the set can grow**. A
`MeosType` gains a member with every family added, and the liblwgeom type set
grows as the engine covers more geometries; a subtype does neither. Where the set
grows, the residual arm silently absorbs the new member, which is precisely the
failure this rule exists to prevent.

### 3.3 Layer 2 needs no dispatcher

The dimension primitives already take the `MeosType` themselves —
`ensure_has_X(T_STBOX, flags)`, `ensure_has_X(T_TBOX, flags)`,
`ensure_has_X(T_TPCBOX, flags)` — and the flag word has the same layout in every
box type. So a generic operation dispatches its comparability check and calls its
dimension check directly:

```c
if (! ensure_valid_bbox_bbox(box1, box2, temptype) ||     /* dispatched */
    ! ensure_has_X(bboxtype, flags1) ||                   /* already generic */
    ! ensure_has_X(bboxtype, flags2))
  return false;
```

**A new box type adds nothing to layer 2** — which is the point of keeping the
per-class constraint in shared primitives rather than per-type functions.

---

## 4. Layer 2 — dimensions

Per operation class, from primitives shared across every box type:

```
topological   ensure_common_dimension(flags1, flags2)
position      ensure_has_X / ensure_has_Z / ensure_has_T (meosType, flags), per operand
extent        ensure_same_dimensionality(flags1, flags2)
```

These take `(meosType, flags)` or bare flags, so they are generic over box types.
**A new box type adds nothing here.** If a new type appears to need a new
dimension primitive, the first question is whether the constraint is really about
frame, in which case it belongs in that type's layer-1 validator.

An operation composes one of each:

```c
/* position operator on the X axis */
if (! ensure_valid_stbox_stbox(box1, box2) ||
    ! ensure_has_X(T_STBOX, box1->flags) ||
    ! ensure_has_X(T_STBOX, box2->flags))
  return false;
```

### 4.1 Layer 1 is asked about the axes the operation reads

An operation validates the frame of every axis it reads, and of no other. Which
axes those are is what decides whether the layer-1 question arises at all.

Read the frame column of the table in §1 and every entry governs coordinates: a
span type says what the value extent measures, an SRID and the geodetic bit say
where the coordinates are, a `pcid` says which schema they are read in. **No box
type carries a frame field for the time axis.** A timestamp means the same thing
in every reference system and under every schema, so an operation that compares
only periods has no frame to disagree about, and asking would refuse a pair it can
answer.

That gives one rule per operation class:

```
topological   reads every shared axis  -> the coordinate frame, when both carry X
position on X/Y/Z  reads coordinates   -> the coordinate frame
position on T      reads periods       -> nothing to ask
```

The validators of §2 already carry the same principle one level down: each gates
its frame predicates on `MEOS_FLAGS_GET_X(box1->flags) && MEOS_FLAGS_GET_X(box2->flags)`,
so a box holding no coordinates states no reference system and is comparable with
any. A topological operation therefore calls the validator unconditionally and
lets it decide, while a `before` / `after` / `overbefore` / `overafter` does not
call it at all. `Span` is not an exception: its one axis is the axis every
operation on it reads, so its validator is always called.

**Where layer 1 is asked, it is asked first.** Comparability is the more
fundamental failure — mixing metres with feet is wrong regardless of which
dimensions happen to be present, so it is what the caller needs to be told — and
fixing the order makes the diagnostic deterministic: a pair failing both checks
reports the same error wherever it enters, rather than whichever failure the local
author happened to test first.

---

## 5. Frame values: absorb at input, strict at comparison

A frame field has an "unset" value — `SRID_UNKNOWN` for an SRID, `0` for a `pcid`.
That value means two different things, and the difference is whether the value is
still being assembled.

### 5.1 What absorb is

A spatiotemporal value carries **one** SRID, but the text form lets it be written
at any level. These three literals denote the same value:

```
SRID=5676;[Point(1 1)@t1, Point(2 2)@t2]                        outer only
SRID=5676;[SRID=5676;Point(1 1)@t1, SRID=5676;Point(2 2)@t2]    both levels
[SRID=5676;Point(1 1)@t1, SRID=5676;Point(2 2)@t2]              components only
```

So a parser reconciles the levels into the one SRID the value carries. Here
`SRID_UNKNOWN` does not mean "no SRID" — it means **"not stated at this level"**,
and a level that says nothing takes what another level said. That is absorb, and
it is a statement about **one value written across levels**, never about two
independent values.

`geo_parse` and `spatial_parse_elem` (`meos/src/geo/tspatial_parser.c`) implement
it with three branches over an in/out `srid` threaded through the parse — the
value's single SRID, being assembled:

```c
int gs_srid = gserialized_get_srid(gs);
if (*srid == SRID_UNKNOWN && gs_srid != SRID_UNKNOWN)
  *srid = gs_srid;                            /* components only -> fills the outer */
else if (*srid != SRID_UNKNOWN && (gs_srid == SRID_UNKNOWN || ...))
  gserialized_set_srid(gs, *srid);            /* outer only -> fills the components */
else if (*srid != SRID_UNKNOWN && gs_srid != SRID_UNKNOWN && *srid != gs_srid)
{
  meos_error(ERROR, MEOS_ERR_TEXT_INPUT,
    "Geometry SRID (%d) does not match temporal type SRID (%d)", gs_srid, *srid);
  ...
}
```

The second spelling — both levels stated and equal — falls through all three
branches and is accepted unchanged.

**A mismatch between levels is captured.** Two levels that both assert a frame
value and disagree are not silence to be resolved; they are a contradiction in the
literal, and the third branch errors. Absorb therefore never weakens the rule that
mixing reference systems is an error: it resolves disagreement by *silence* and
errors on disagreement by *assertion*. **Every absorb site owes that third
branch.**

> A geography written with no SRID is parsed as `SRID_DEFAULT`, so afterwards the
> value cannot say whether 4326 was asserted or merely defaulted — only the text
> can. Both functions capture `srid_written` before parsing and consult it in the
> second branch. An absorb site that skips this silently accepts a 4326 assertion
> against a different outer SRID.

`ensure_srid_reconcile` carries the same rule for the non-textual input paths — a
constructor taking a geometry and a temporal value, or a pose applied to a body.

### 5.2 Why it stops there

Once a value exists, the unset marker stops meaning "not stated here" and starts
meaning "this value has no frame value" — a real, distinct state. At a comparison
there are two independent values, neither of which is a *level* of the other, so
there is no silence to fill: nothing about the second box is an unstated part of
the first. A mismatch is then simply metres against feet, and the comparison
errors.

| rule | mechanism | where |
| --- | --- | --- |
| absorb | the three-branch block in `geo_parse` / `spatial_parse_elem`; `ensure_srid_reconcile` | parsers and constructors |
| strict | `ensure_same_srid(a, b)` — plain equality, `static inline` in `geo_funcs.h` | every comparison |

In particular **an expand or a merge never fills one box's unset frame field from
the other's.** A merge has two independent boxes, so an unset marker there is a
value, not a silence — and filling it silently combines boxes from different
reference systems whenever one of them happened not to say which.

---

## 6. Validation and initialization are separate

Several operations need the conjoined flags of their two operands — whether both
carry X, both carry Z, both carry T, both are geodetic. It is tempting to have the
validator return them, since it has just tested some of the same bits, and to save
the caller a second pass.

**That fusion is not worth its cost, and the saving is not where it looks.**
Measured on a built `libmeos.so`, a fused validate-and-initialize entry runs to
some 94 instructions and makes **seven out-of-line calls, six of them through the
PLT** — the `ensure_*` primitives, the SRID accessor, the flag helper. What the
fusion saves against that is **two bit tests**. It is noise, and it costs the
layering: a validator that also writes output parameters cannot serve an operation
that only wants validation, so operations that do not need flags either pay for
them or grow a second validator.

The flag helpers are therefore separate and `static inline`, each private to the
file that uses it:

```c
/* topological operator */
if (! ensure_valid_stbox_stbox(box1, box2) ||             /* layer 1, first */
    ! ensure_common_dimension(box1->flags, box2->flags))  /* layer 2 */
  return false;
stbox_stbox_flags(box1, box2, &hasx, &hasz, &hast, &geodetic);
```

Inlining is what actually pays: it removes the call the fusion was invented to
avoid, and it lets the compiler eliminate the duplicated flag test — which it
cannot do across an out-of-line call, so the fused form never achieved it either.
Position operators call no flag helper at all; they need only `ensure_has_X` and
its siblings.

---

## 7. Checklist for a new bounding box type

Wiring is `new_temporal_type.md` §3.4. The validation contract is this:

1. **Partition the struct into extent and frame** (§1). Write the partition into
   the struct's doc comment; `TPCBox`'s comment, naming `pcid` as the reason two
   boxes from different schemas cannot be compared, is the model.
2. **Classify every new flag bit** as presence or frame (§1.1).
3. **Write exactly one `ensure_valid_<box>_<box>`** — null checks, then every
   frame predicate, then nothing (§2). `static inline` in the type's header if the
   body is a pure predicate chain.
4. **Reuse the frame predicates of the box you extend** rather than restating them
   (§2.1).
5. **Add an arm to `ensure_valid_bbox_bbox`** and to the sibling dispatchers a new
   box type must appear in (§2.2); `new_temporal_type.md` §3.4 lists the rest.
6. **Add nothing to layer 2** (§4). If a dimension primitive seems to be missing,
   check whether the constraint is really frame.
7. **Ask layer 1 about the axes the operation reads, and ask it first** (§4.1).
   An operation over periods alone asks nothing: no box type carries a frame
   field for the time axis.
8. **Compare frame fields strictly** (§5.2). Absorb belongs to parsers and
   constructors, and there the mismatch branch is mandatory (§5.1).
9. **Never fill an unset frame field during an expand or merge** (§5.2).
10. **Keep flag computation out of the validator** (§6).

---

## 8. Alternatives not taken

**A generic validator that inspects the box itself.** A single function reading
frame fields out of a `bboxunion` by offset would replace the per-type validators
rather than dispatch to them. That erases typing at the site whose job is
checking, and every rule change has to be re-derived against a union layout. The
dispatcher of §2.2 is not this: it routes on the catalog and the per-type
validators keep the rules.

**A validator per operation class per box type.** Four box types times
topological, position and extent is twelve functions expressing what four
validators and three shared dimension primitives already express. The per-class
constraint is about *dimensions*, described by flags common to all box types, so
it belongs in a shared primitive rather than a per-type function.

**Absorb at merge sites.** Filling one box's unset frame field from the other's
during an expand looks like a convenience and reads like the parser rule. It is
not the same rule: the parser has one value written twice, a merge has two values.
