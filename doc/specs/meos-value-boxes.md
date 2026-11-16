<!--
   ****************************************************************************
    MobilityDB / MEOS Specification — The Boxes of a Value
    Copyright(c) MobilityDB Contributors

    This documentation is licensed under a Creative Commons Attribution-Share
    Alike 3.0 License: https://creativecommons.org/licenses/by-sa/3.0/
   ****************************************************************************
-->

# The Boxes of a Value — Behaviour Specification

| | |
|---|---|
| Version | v0.1.0 |
| Scope | The boxes MEOS gives a value for an index: one box per piece of the value, a split of its pieces into a number of boxes, and a merge of its piece boxes into a number of boxes, for the sets, the span sets, the temporal types and the lines of the geometry types; and the box of a temporal rigid geometry |
| Reference | `tools/codegen/inherited/templates/boxsplit.c.tmpl` (the splits and merges of the temporal types), `meos/src/geo/tgeo_boxops.c` (the temporal points and geometries, the lines of a geometry), `meos/src/rgeo/trgeo_boxops.c` (the box of a temporal rigid geometry), `meos/src/temporal/temporal_boxops.c` (`bbox_merge_n`, the merge), `meos/src/temporal/span_index.c`, `meos/src/temporal/tbox_index.c`, `meos/src/geo/stbox_index.c` (the size of a box) |

## 1. Scope and intent

An index keyed by the bounding box of a value reads a moving object through
one box, whose empty space every query near the object pays for. The boxes
of this document key a value by several boxes instead, each tight around a
part of it. MEOS gives three forms of them: the boxes of the pieces of the
value, a split of its pieces into a given number of boxes, and a merge of its
piece boxes into a given number of boxes. This document states what each
answers. It is prescriptive: MEOS, MobilityDB and every binding generated
from MEOS answer by it.

Each section states what master guarantees, and section 10 lists what the
rule does not yet cover.

## 2. Notation

A **piece** of a value is an instant of a set, a span of a span set, an
instant of a temporal instant or of a sequence with discrete
interpolation, and a segment, the part between two consecutive instants, of
a sequence with step or linear interpolation; a sequence of one instant has
that instant as its one piece. The pieces of a sequence set are the pieces
of its sequences in their order. The pieces of a line are its segments.

The **size** of a box is the product of its extents: the width of a span,
the width of the value span of a `tbox` times its duration, and the product
of the X, Y and, when present, Z extents of an `stbox` or a `tpcbox` times
its duration. A box flat in one of its dimensions has size 0. The unit of
time differs between box types, which scales every size of a value by the
same factor and changes no answer below.

## 3. The box types of a value

Each type answers in the box of its kind, under the names below, where
`<Box>` is that kind:

| Box | Types |
|---|---|
| `Spans` (the time span; the value span for a set or span set of numbers) | the sets and span sets; every temporal type but `tposechain` |
| `Stboxes` | `geometry` and `geography` lines; every spatial temporal type but `tnpoint` |
| `Tboxes` | `tint`, `tbigint`, `tfloat` |
| `Tpcboxes` | `tpcpoint`, `tpcpatch` |

The functions are `spans`, `stboxes`, `tboxes` and `tpcboxes` (the boxes of
the pieces), `splitN<Box>` and `splitEachN<Box>` (section 5), and
`mergeN<Box>` and `mergeEachN<Box>` (section 6); MEOS names them
`<type>_<boxes>`, `<type>_split_n_<boxes>`, `<type>_split_each_n_<boxes>`,
`<type>_merge_n_<boxes>` and `<type>_merge_each_n_<boxes>`.

## 4. The boxes of the pieces

The boxes of a value are one box per piece, in the order of the pieces.
Each holds the value over its piece: at every instant of the piece, the
value lies in the box. A bound inside a sequence is inclusive, since the
instant it names belongs to both pieces meeting there, and a bound at an end
of a sequence carries the inclusivity the sequence gives that end, so every
box of a value, of its pieces, its splits and its merges, lies in the
bounding box of the value:

```sql
SELECT stboxes(tgeompoint '{[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03],[Point(3 3)@2001-01-04, Point(3 3)@2001-01-05]}');
-- {"STBOX XT(((1,1),(2,2)),[2001-01-01, 2001-01-02])","STBOX XT(((1,1),(2,2)),[2001-01-02, 2001-01-03])","STBOX XT(((3,3),(3,3)),[2001-01-04, 2001-01-05])"}
SELECT splitNSpans(tfloat '(1.5@2001-01-01, 3.5@2001-01-02, 2@2001-01-04)', 1);
-- {"(2001-01-01, 2001-01-04)"}
```

## 5. Splits

`splitN<Box>(value, n)` answers at most `n` boxes, each the box of
consecutive pieces. A value of `p` pieces answers one box per piece when
`n >= p`. Otherwise each box takes `p / n` consecutive pieces, rounded
down, and the first `p mod n` boxes one more:

```sql
SELECT splitNStboxes(geometry 'Linestring(1 1,2 2,3 1,4 2,5 1)', 3);
-- {"STBOX X((1,1),(3,2))","STBOX X((3,1),(4,2))","STBOX X((4,1),(5,2))"}
SELECT splitNStboxes(geometry 'Linestring(1 1,2 2,3 1,4 2,5 1)', 6);
-- {"STBOX X((1,1),(2,2))","STBOX X((2,1),(3,2))","STBOX X((3,1),(4,2))","STBOX X((4,1),(5,2))"}
```

A sequence set of `s` sequences splits by sequence. When `n >= s`, each
sequence takes a share of the `n` boxes proportional to its number of
instants, at least one, and is split as above, so the answer can hold fewer
than `n` boxes; when `n < s`, consecutive whole sequences share a box, `s /
n` of them rounded down and the first `s mod n` boxes one more.

`splitEachN<Box>(value, n)` cuts each sequence into boxes of `n`
consecutive pieces, the last one of a sequence holding the pieces left. No
box of a split holds pieces of two sequences.

## 6. Merges

`mergeN<Box>(value, n)` answers the least of `n` and the number of pieces
of boxes. It starts from the boxes of the pieces and merges, each time, the
two consecutive boxes whose union adds the least size to theirs, until `n`
boxes remain; among pairs adding the same size, the earlier merges first.
Every box of a piece lies in one box of the answer, and the boxes keep the
order of the pieces. For the same `n`, a merge and a split can answer
different boxes:

```sql
SELECT splitNStboxes(geometry 'Linestring(1 1,2 2,3 1,4 2,5 1)', 2);
-- {"STBOX X((1,1),(3,2))","STBOX X((3,1),(5,2))"}
SELECT mergeNStboxes(geometry 'Linestring(1 1,2 2,3 1,4 2,5 1)', 2);
-- {"STBOX X((1,1),(4,2))","STBOX X((4,1),(5,2))"}
```

A sequence set is merged as one array of the boxes of all its pieces, so a
box of a merge can hold pieces of two sequences and the gap between them,
whose duration counts in its size:

```sql
SELECT mergeNStboxes(tgeompoint '{[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(3 1)@2001-01-03,
Point(4 2)@2001-01-04, Point(5 1)@2001-01-05, Point(6 2)@2001-01-06], [Point(7 1)@2001-01-07,
Point(8 2)@2001-01-08, Point(9 1)@2001-01-09,Point(10 2)@2001-01-10]}', 2);
-- {"STBOX XT(((1,1),(5,2)),[2001-01-01, 2001-01-05])","STBOX XT(((5,1),(10,2)),[2001-01-05, 2001-01-10])"}
```

`mergeEachN<Box>(value, n)` merges in the same way down to the number of
pieces divided by `n`, rounded up.

## 7. The box of a temporal rigid geometry

A temporal rigid geometry is a geometry carried by a pose. The box of a
segment where the pose keeps its orientation, the angle of a planar pose or
the quaternion of a 3D one, is the union of the boxes of the geometry at the
two ends of the segment: the geometry only translates, each of its
coordinates moving linearly with time. The box of a segment where the pose
turns is the box of the positions of the pose grown on every side by the
radius of the geometry, the greatest distance from its reference point to
one of its points. Two orientations are the same only when they are equal;
a quaternion and its opposite, which state the same rotation, are read as
two, which keeps the box of a turning segment for them. The box of a
sequence is the union of the boxes of its segments:

```sql
SELECT stbox(trgeometry 'Polygon((0 0,1 0,1 1,0 1,0 0));[Pose(Point(0 0), 0.0)@2001-01-01, Pose(Point(10 0), 0.0)@2001-01-02]');
-- STBOX XT(((0,0),(11,1)),[2001-01-01, 2001-01-02])
```

## 8. Arguments

A count `n` that is not positive is refused:

```sql
SELECT mergeNStboxes(tgeompoint 'Point(1 1)@2001-01-01', -1);
-- ERROR:  The value must be strictly positive: -1
```

In MEOS the refusal is `MEOS_ERR_INVALID_ARG_VALUE`. A MEOS function of
this document returns its boxes as an array and their number through its
last argument, `count`: a null `count` is refused with
`MEOS_ERR_INVALID_ARG` before anything is written through it. The splits,
the merges and the boxes of the pieces of a temporal type or a line set
`count` to 0 once it is known not null, before any other argument is
tested, so a refused call leaves it at 0.

## 9. Why the rules are what they are

### 9.1 A split counts, a merge measures

A split reads only the number of pieces, so its boxes are known before any
box is read, and two values with the same number of pieces split alike. A
merge reads the boxes, so its boxes hold less empty space where the value
turns back or stops; the size a merge adds is the empty space the union
holds and neither box did, since two consecutive boxes of a value meet at
an instant at most. A box flat in a dimension measures 0, so a merge of two
such boxes can add the size of their union and nothing more.

### 9.2 A merge crosses the gaps of a sequence set

An index reads boxes, not sequences. Merging the boxes of a sequence set as
one array lets a merge join two short sequences close in space and time
rather than keep a box per sequence whatever its size; the gap between them
is time the box covers, and its duration counts in the size of the union.

### 9.3 The box of a rigid geometry holds it at every instant

A query answered through a box misses a value that leaves its box. Between
two poses of the same orientation each bound of the geometry is reached at
an end of the segment, so the boxes at the ends hold it; between two poses
of different orientations the geometry can reach any angle about its
reference point, which the radius bounds.

## 10. Versioning and stability

The minor number rises with each part of the rule master gains, the patch
number with corrections of wording.

| Version | Content |
|---|---|
| v0.1.0 | The boxes of the pieces of a value, its splits by count and its merges by size, the box of a temporal rigid geometry, and the arguments |

## 11. Out of scope

Future: the rule extends to these, each added here with its version:

- a merge that stops at the boxes a query of a given extent reads least,
  in place of a count;
- the index operator classes keyed by the boxes of this document.

Not specified: the boxes of a value read by the GiST and SP-GiST operator
classes keyed by its bounding box, which hold one box per value.

## 12. References

- [Finite values](meos-finite-values.md), whose doubles every bound of a
  box is.
- [Empty geometries](meos-empty-geometries.md), the boxes of an empty
  geometry.

## 13. Consumers

Every binding generated from the MEOS catalog inherits the rule without code
of its own: the boxes of a value are answered by MEOS and reach the binding
unchanged.

## 14. References to the implementation

The source files of the reference line are authoritative. A disagreement
between this document and the code is a defect of one of them, reported as an
issue of MobilityDB.
