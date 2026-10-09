<!--
  MobilityDB — Making GEOS optional: the buffer
  Copyright(c) MobilityDB Contributors

  This documentation is licensed under a Creative Commons Attribution-Share
  Alike 3.0 License: https://creativecommons.org/licenses/by-sa/3.0/
-->

# 6. The buffer

**Measured at** MobilityDB master `d9b9c11985`.

The *buffer* of a geometry at a distance *r* is the region of all points no farther than *r*
from it: a disc around a point, a sausage around a line, a polygon grown outwards with rounded
corners. `geom_buffer` in `meos/src/geo/geo_buffer.c` computes it natively in every build.

The buffer has no path to GEOS ([note 5](05-WHAT-STILL-NEEDS-GEOS.md) §5.4), so what is open
here is not GEOS but the buffer's own answer and speed. Two things are open: its answer depends on
where the shape lies (§6.1–§6.6), and it is between about 2.8 and 4.4 times slower than GEOS on
large real polygons (§6.9). Most of this note concerns the repair of the first, and why it
cannot be merged on its own.

**Where each figure comes from.** The figures of §6.1 are measured on master for these notes, by
[`tools/square_buffer.c`](tools/square_buffer.c), and the table of §6.4 by the same program run
against the branch carrying the repair (§6.5 names it), output in
[`tools/results/square_buffer.repair_branch.txt`](tools/results/square_buffer.repair_branch.txt).
The census of §6.5 is re-run for these notes with the campaign's buffer ledger, whose programs are
kept outside the tree; its output is
[`tools/results/buffer_ledger_census.txt`](tools/results/buffer_ledger_census.txt). Every other figure of this note — the
real-polygon declines, the CGAL counts, the hand-moved polygons and the tolerances of §6.6, the
node counts and the open-line map of §6.7, the tables of §6.8 — is recorded from the campaign's
runs of 2026-09-30, with programs kept outside the tree, and is not re-run here. The speed figures
of §6.9 are attributed to the pull request or commit that states them.

## 6.1 The symptom

Take a square 0.02 units on a side and buffer it by 0.001. Then take the same square, moved to
a projected coordinate such as 6 400 000 — an ordinary easting in metres — and buffer it by the
same 0.001. The answer should be the same shape in a different place, so its area should be
identical.

The correct area is known exactly: the buffer of a square is the square, plus a strip along
each side, plus four quarter-discs at the corners. With *s* = 0.02 the side and *r* = 0.001 the
distance, its area is s² + 4sr + πr² = **0.000483141592654**.

| Where the square sits | Area of the buffer on master |
|---|---|
| at the origin | 0.000483140331157 |
| at 1 000 000 | 0.000483140335918 |
| at 6 400 000 | **0.000323999972105** |

The first two rows are within 1.3e-9 of the formula; that small difference is not attributed
here, and §6.8 explains why an area is weak evidence for a buffer. The
third is **33 % too small** — smaller even than the 0.0004 square it must contain.

With a larger distance, the same square at 6 400 000 is not answered at all: the buffer
*declines*: it refuses with an error instead of answering.

| Distance | at the origin | at 6 400 000 |
|---|---|---|
| 0.001 | 0.000483 | 0.000324, wrong |
| 1 | 3.2207 | declines |
| 1000 | 3 140 411 | declines |

These figures are the output of [`tools/square_buffer.c`](tools/square_buffer.c) on master
([`tools/results/square_buffer.txt`](tools/results/square_buffer.txt)), run with an error
handler that returns so that every case prints.

## 6.2 What ought to be true

Moving a shape and then buffering it gives the same result as buffering it and then moving it.
The same holds for scaling, provided the distance is scaled with the shape:

```
buffer(move(G), r)          =  move(buffer(G, r))
buffer(scale(G, k), k · r)  =  scale(buffer(G, r), k)
```

This is not a tolerance question; it is what the operation means. It is also convenient to test,
because it needs no reference implementation: the same shape, moved or resized, is its own
answer key.

**Both tests are needed.** The error above comes from adding and subtracting numbers of very
different sizes — a shape 0.02 across sitting at 6 400 000. That *ratio* does not change when
the shape and its position are scaled together, so scaling the whole picture reproduces the same
wrong number and the scale test reports agreement. Moving the shape changes the ratio, and the
move test catches it at once. The converse also exists: a comparison against a fixed constant
somewhere in the code is invisible to moving and caught by scaling.

## 6.3 Where the error comes from

To build a buffer the engine makes many decisions of the form "are these two computed points the
same point?" and "is this point on this line?". Each compares against a small fixed quantity, a
*tolerance*, which grows with the size of the coordinates. A tolerance that suits coordinates
near 1 is wrong for coordinates near 6 400 000, and the reverse. Tuning any one of them moves the
failure to another scale, which is why it is ruled out ([note 7](07-LESSONS.md)).

## 6.4 The repair: choose the frame, not the constants

Rather than choosing better constants, the engine can choose better coordinates to work in.
Before constructing anything it:

1. **moves the shape near the origin**, by subtracting a point of the shape itself;
2. **scales it so that the distance is about 1**, by a power of two;
3. builds the buffer;
4. **undoes both** on the result.

Every internal decision is then made in one regime, where one constant is right, instead of
across every magnitude a user may supply. Multiplying by a power of two only changes a number's
exponent, so it is exact in both directions and adds no error of its own.

With this change, the square of §6.1 reads, at the origin, at 1 000 000 and at 6 400 000:

| Distance | at the origin | at 1 000 000 | at 6 400 000 |
|---|---|---|---|
| 0.001 | 0.000483140331157 | 0.000483140335918 | 0.000483140324619 |
| 1 | 3.22073115695475 | 3.22073115702247 | 3.22073115508071 |
| 1000 | 3 140 411.157355 | 3 140 411.157355 | 3 140 411.157353 |

## 6.5 What the repair achieves, and what it costs

**Where the work is.** The repair is commit `289dcbb8d3`, "Buffer in a frame chosen from the
geometry and the distance", on a branch `topology` kept on the campaign's machine; nothing of it
is pushed. The branch is five commits on master `a362004728`: four that move the edge topology
the buffer shares with the overlay into a file of its own, `geo_topology.c`, then the frame. The
measurements compare the frame commit with its *parent* (the four-commit state), which carries
master's buffer algorithm.

**The measurement.** The buffer ledger of [note 3](03-TEST-DATA.md) generates 1840 buffers: six
shape families, at four origins and five power-of-two scales. It groups them so each group must
agree with itself, and scores three *arms*:

- **move**: a group is one shape at several origins; every member must answer, with the same
  number of rings and the same area;
- **scale**: a group is one shape at several scales; every member must answer, with the same
  rings and the same area once divided by the scale squared;
- **closed form**: where the family has a formula for its area, the answer must match it within
  1 % (the area function itself approximates arcs, so this arm cannot be sharper).

A group in which some member declines counts as a disagreement: the same shape cannot be
buffered in one place and not in another.

| Arm (groups that disagree) | parent | with the frame |
|---|---|---|
| move, of 460 groups | 64 | **40** |
| scale, of 368 groups | 113 | **32** |
| — of which differ in *shape*, not merely in declining | 46 | **0** |
| closed form, of 880 | 120 | 124 |
| declined queries, of 1840 | 196 | **160** |

On 192 real protected-area polygons of the natural-area set buffered at distance 1, the parent
declines **5** and the frame **12**.

So the frame removes every case where the engine returns a *differently shaped* answer, and it
costs seven real polygons that answered before. The closed-form arm does not improve. Of its 120 and 124 disagreeing
groups, 44 on each side are areas off the formula by more than 1 % (the rest are declines);
both arms count 44, and their cause is not yet found.

CGAL, deciding for points placed well inside or well outside the buffer whether they belong to
it, reads 4 disagreements of 32 668 queries on the parent and 0 of 33 600 with the frame. The
two totals differ because only the buffers the engine answers can be queried, and the frame
answers more of them. All four are
one family at the smallest scale, with the probe point at the centre of a hole whose half-width
is exactly r/2, where the judge (which measures the distance to the boundary lines and has no
notion of inside) is weakest. They are not attributed to the engine until the judge is ruled out.

## 6.6 The seven extra declines come from the coordinates, not from the frame

This is measurable rather than arguable. Take five of the real polygons that decline only with
the frame. Move them to the origin by hand and give them to the *parent*, which has none of the
frame code: it declines all five. Give it the same five unmoved, and it answers all five. At
distance 1 the frame's scale factor is exactly 1, so the frame here *is* that move and nothing
else. The decline follows the coordinates.

The reason is the tolerances of §6.3. They grow with the size of the coordinates, so moving a
polygon spanning 12 000 m from 6 400 000 to the origin tightens them sharply:

| Tolerance | at 6 400 000 | at the origin | |
|---|---|---|---|
| deciding whether two computed points are one point | 7.42e-05 | 3.27e-06 | 22.7× tighter |
| deciding whether a point lies on a line | 5.51e-09 | 1.17e-11 | 472× tighter |

A generous tolerance lets the next stage treat near-misses as matches, and that is what carries
these polygons through on master. **The buffer's success on real projected data depends on the
very sensitivity to position the frame removes.** With tighter tolerances a separate, older defect
shows: the ring assembly, §6.7. Hence the rule for the plan: **the frame and the ring-assembly
repair are one change**, delivered together. Neither merges alone: the frame loses answers
without the assembly repair, and the assembly repair has no witness without the frame.

## 6.7 The ring assembly strands pieces

**What the assembly does.** The buffer computes many boundary *pieces* — straight segments and
arcs — cuts them where they cross, keeps the ones on the outside, and finally chains them into
closed rings. The chaining starts a ring at the first piece not yet used, follows the piece whose
start matches the current end, and marks each piece *used* as it goes.

**The defect.** A boundary piece separates two faces, one on each side. The walk spends the whole
piece the first time any ring passes along it, so the face on the other side finds a gap where
that piece should be. Which ring gets there first depends on the order the pieces happen to be
stored in.

**Proven on a real polygon.** Tracing the point where one real projected polygon's ring gets
stuck at distance 1: the nearest unused piece lies 0.601 away, far beyond any tolerance; the piece
the ring needs lies at distance **exactly 0**, and an earlier ring has already used it.

**Node identity is not the cause**, though it is the obvious suspect. A split point is written
from the same stored coordinates into both pieces, so it is shared by construction; scaling the
node tolerance from 1e-6 to 50 leaves the number of nodes at 162 (it moves only at 1e6, the
control showing the switch is live). With the frame the engine finds 162 nodes where the parent
finds 114, on the same 310 raw pieces: the 48 extra are crossings the better-conditioned
arithmetic resolves. The frame hands the assembly a *more correct* topology, and the assembly
cannot cope with it.

**The same mechanism on synthetic input.** A square drawn as an open line whose last vertex
misses the first by a small gap declines in 14 of 48 cells of a gap × distance map, at unit
coordinates as well as projected ones, and not monotonically in either. There the walk first
closes a degenerate two-piece ring of area −2.6e-23, which consumes the real ring's corner arcs,
and the real ring then reaches a point nothing continues. So the defect has two faces: a ring
that should not close takes pieces, and a ring that should close cannot find them.

**The repair.** Spend a *side*, not a piece. Each piece carries two directed sides, and the walk
marks only the side it traverses, so every piece stays available to both of its faces. This is the
classical traversal of a planar subdivision: walking every directed side exactly once yields
every face, whatever the starting point.

**A prerequisite, measured.** The chaining function `buffer_chain_ring_infos` is shared by the
buffer and the overlay. It expects each piece to carry one extra field, a flag saying on which
side of the piece the answer lies. The overlay gives it pieces with that field; the buffer gives
it pieces of a smaller record type without it, so on the buffer path the function reads the flag
from the first bytes of the *next* piece in memory. Supplying a correctly computed flag changes no answer on the 1840 ledger queries
or the 192 real polygons, and loses 3 of the 1313 containment queries, so on the buffer path the
flag carries no information and is not itself the fix. A side traversal needs per-side state, so
the buffer path must first pass the right element type, guarded by an assertion on the element
size.

## 6.8 Three measurements that mislead

Each of these reads a wrong number for a reason that has nothing to do with the buffer, and each
has to be separated from the engine before it counts as evidence.

**The ring area is taken about the origin.** The buffer's two routines computing a ring's area
(`buffer_ring_area`, `buffer_ring_signed_area`) use the shoelace formula, summing products of
coordinates about (0, 0). At projected coordinates each product is about 10^12, and the area is
the small difference of such numbers. Against exact rational arithmetic, on a small ring:

| Ring near | True area | About the origin | About the ring's first point |
|---|---|---|---|
| 0 | 1.000000e-10 | 1.000000e-10 | 1.000000e-10 |
| 10 | 9.999823e-11 | 9.998757e-11 | 9.999823e-11 |
| 1e6 | 1.000008e-06 | **0** | 1.000008e-06 |
| 1e6 | 1.000001e-04 | **2.441406e-04**, 144 % wrong | 1.000001e-04 |
| 6.4e6 | 1.000000e-03 | **0** | 1.000000e-03 |

Every user of that number — which ring is a shell and which a hole, whether a ring has no area,
whether a hole survives — reads a value whose error exceeds the quantity. liblwgeom's own
`ptarray_signed_area` subtracts the first point for exactly this reason, so the repair copies a
sibling. Fixing it is correct and wins answers, but it uncovers the same assembly defect, so it
ships with §6.7 too.

**A hole that shrinks to exactly a point.** A 10 × 10 square at 6 400 000 with a centred square
hole, buffered by 0.001: the buffer answers correctly when the hole's half-width is below or
above 0.001, and declines when it is exactly 0.001, where the shrunken hole is a single point.
The surviving-hole areas on either side are exact, so the defect is that one degenerate case.

**The area function approximates arcs.** `lwgeom_area` measures an arc by its chords, so across
power-of-two scales a buffered disc carrying one identical geometry reads 826 437.81 (scaled back)
at large scales and 526 338 at 2^−24 and below: about 2/π of it, the square inscribed in the
circle. The geometry is the same; the measurement changes. An area difference is therefore
evidence about the buffer only once the geometry itself is compared, which is why the scale arm
compares geometries exactly and only the closed-form arm reads an area, at 1 %.

**A containment corpus cannot see a decline.** The 1313-query corpus over 172 protected areas
holds only queries the engine answers. It reads 1313 of 1313 across the frame change, which costs
seven answers on the same areas at distance 1. It measures whether answers stay right, not whether
they keep existing.

## 6.9 Speed

The buffer is the one operation still slower than GEOS: at commit `f8f5d0b1a3`, 3.99 times GEOS
on the natural areas at distance 1, 4.35 at 50, 2.83 at 500, as its message states. Three later
commits lower the distance-1 cell's native instructions further against an unchanged GEOS count,
by 3.61 %, 3.03 % and 1.87 % (`4416773c99`, `167648b2a3`, `0315c48101`), which puts it near 3.7.
Small shapes run at 0.548, faster than GEOS (#2815). Before #2803 the distance-1 figure stood at 48.3. A series of changes that move no
answer brings it to the figures above: looking pieces up through indexes and sweeps instead of
comparing every pair (#2803, #2813), skipping a bounds check the loop already guarantees
(#2811), writing a straight run as one line rather than one line per segment (#2815), a cheaper
sort, and reading each ring's edges once instead of twice.

No speed claim is made for the frame: with it, the ledger compares 187 polygons against 180, and
a valid comparison needs both sides to answer the same rows (note 2, §2.6). The first step of the
remaining speed work is a fresh profile of master's buffer on the natural areas, after the
correctness change, because that change alters which rows are timed.

## 6.10 What is left, in one list

1. The frame (§6.4) together with the side traversal (§6.7), its prerequisite element type, and
   the ring area about a point of the ring (§6.8), as one change. Its finishing line: the move and
   scale arms of the ledger at 0, no more declines than the parent on the ledger and on the real
   polygons, and CGAL agreeing on every query.
2. The hole shrinking to exactly a point (§6.8).
3. Speed parity on the natural areas (§6.9).
4. The single-sided buffer, as the committers specified it (note 4 §4.11).
