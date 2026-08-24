<!--
  MobilityDB — GiST Sorted Build and the Spatiotemporal Sort Key: Design Notes
  Copyright(c) MobilityDB Contributors

  This documentation is licensed under a Creative Commons Attribution-Share
  Alike 3.0 License: https://creativecommons.org/licenses/by-sa/3.0/
-->

# GiST Sorted Build and the Spatiotemporal Sort Key — Design Notes

When a GiST operator class supplies support function 11, PostgreSQL stops inserting
entries one at a time and instead sorts every entry, then packs the pages in that
order. MobilityDB supplies it on all 36 of its GiST operator classes, and the build
is faster, the index smaller, and the queries cheaper for it.

The sort needs a **total order over boxes**. That is where a spatiotemporal type
parts company with a spatial one, and the whole of this note is about the parting.
A `BOX2D` has two coordinates of the same kind, so ordering them together needs no
justification. An `STBox` has a position and a time interval, and any single
ordering of `(x, y, t)` must decide **how many seconds are worth one degree**.
There is no construction that avoids the decision; there are only constructions
that hide where it sits.

This note records what ships, every key shape measured against it, the one that
measures best and why it is nevertheless **not shipped**, and the structural reason
the obvious generalisation of the PostGIS approach does not work.
It is written to be picked up cold.

> **Audience**: a contributor who wants to improve the sorted build, or who is
> about to propose an *n*-dimensional space-filling curve for it. Read §6 and §12
> before writing any code. This is design rationale and a research record, not user
> documentation.

---

## 1. What ships today

| commit | subject | what it carries |
| --- | --- | --- |
| `ce58c6a8c4` | Keep the inclusivity of a bound in a GiST node key | the prerequisite: a wrong-answer fix this work exposed |
| `8eb4985626` | Build a GiST index by sorting the entries | the sorted build itself |

Master carries `FUNCTION 11` on **36 of 36** GiST operator classes, over four key
types and three comparators. The sort key functions live in
`mobilitydb/src/temporal/index_sortsupport.c` and
`mobilitydb/pg_include/pg_temporal/index_sortsupport.h`, with the spatiotemporal
key in `mobilitydb/src/geo/tspatial_gist.c`.

Measured on 496254 bounding boxes of vessel track segments from 14 days of Danish
AIS, three interleaved rounds, control being the treatment's own parent commit so
the arms differ by exactly one change:

| metric | control | sorted | ratio |
| --- | --- | --- | --- |
| `CREATE INDEX` | 3762 ms | 2648 ms | **0.70** |
| index size | 97.7 MB | 67.3 MB | **0.69** |
| buffers, space+time selective | 3400 | 479 | **0.14** |
| buffers, narrow space wide time | 11373 | 1409 | **0.12** |

All six runs answer 5075 / 0 / 24789 rows through the same `Index Only Scan`.

**The 31% size drop corroborates the mechanism independently of any clock.** A
sorted build packs denser, so the size is evidence the path actually engaged —
useful because a build that silently fell back to insertion would still be green.

### 1.1 The prerequisite: what a sorted build exposes

The sorted build cannot land alone, because it changes which physical order the
tree is built from and that surfaces a latent wrong-answer defect.

`stbox_adjust` / `tbox_adjust` / `tpcbox_adjust` write the union of a node key's
period themselves:

```c
box1->period.lower = TimestampTzGetDatum(Min(...));
box1->period.upper = TimestampTzGetDatum(Max(...));
```

The bound **values** move; `lower_inc` / `upper_inc` do not. A key expanded to end
exactly on another entry's instant therefore keeps an exclusive upper bound,
excludes that instant, and the inner consistent prunes the page holding it — a
plain `CREATE INDEX` then answers 5049 where a sequential scan answers 5050.

MEOS owns the correct primitive: `span_expand()` moves the bound **and** its
inclusivity. So **a bounding-box union in the PostgreSQL layer delegates to the
MEOS span kernel and never writes the bounds itself** — guarded on the dimension
being present, because a raquet `STBox` carries no time and an unguarded
`span_expand` dispatches on an unset basetype and kills the backend.

**The union is only half.** `gistgetadjusted` sets `neednew` only where
`!gistKeyIsEQ(...)`, so the opclass `same` method decides whether a node is
rewritten at all. A `same` that compares a period by bound value alone counts a
key whose only needed change is an inclusivity flip as unchanged, and the fix never
reaches the parent — leaving the defect alive on the insert path.

Two lessons worth carrying beyond this defect:

- **One binary yields both build modes.** `CREATE INDEX ... WITH (buffering = on)`
  takes `GIST_BUFFERING_STATS` and so never reaches the sortsupport check. Reading
  seq 5050 / sorted 5049 / insert 5050 isolates the build mode from the opclass, the
  data and the key.
- **A single-size witness is shape luck.** A 400-row case passes on PostgreSQL 17
  and 18 with the union half alone and fails on 16 and 19, because which node ends
  on a bound follows from how the pages split. A regression case for this class
  **sweeps sizes** and asserts zero disagreements across all of them.
- `enable_seqscan = off` only *discourages* a sequential scan. A count taken without
  `EXPLAIN` can come from one and prove nothing. Confirm the plan node every time.

**Naming needed no decision.** Support functions 2, 5, 6 and 7 are declared once
per *key* type while only 1 and 3 are per *column* type, because only those name
the column type in their signature. `sortsupport` is `(internal) RETURNS void`, so
it follows the key-type form. `TPCBox` begins with a whole `STBox` (enforced by
per-field `static_assert`s), so four key types need only three comparators.

---

## 2. The key that shipped, and why it invents nothing

```c
/* mobilitydb/src/geo/tspatial_gist.c */
uint64
stbox_sort_hash(const STBox *box)
{
  uint32 space = 0, time = 0;
  if (MEOS_FLAGS_GET_X(box->flags))
  {
    GBOX gbox;
    stbox_set_gbox(box, &gbox);
    FLAGS_SET_GEODETIC(gbox.flags, false);
    space = (uint32) (gbox_get_sortable_hash(&gbox, box->srid) >> 32);
  }
  if (MEOS_FLAGS_GET_T(box->flags))
    time = sortsupport_rank_span_center(&box->period);
  return sortsupport_hilbert(space, time);
}
```

Every piece is borrowed rather than invented:

- **space** — `gbox_get_sortable_hash()` is the function PostGIS itself sorts a
  geometry column by, carrying its geodetic branch and its `[1,2)` normalisation
  for SRIDs 4326, 3857 and 3395.
- **time** — a `TimestampTz` is int64 microseconds, so a shift plus the
  signed→unsigned map is a rank with no floating-point pathology.
- **composition** — `uint32_hilbert()`, the same PostGIS primitive a second time,
  so that neither dimension leads the other.
- **spans** — no curve at all: for a one-dimensional value the lower bound *is* the
  packing order, and `span_cmp` already is it.

Ties break on `stbox_cmp` / `tbox_cmp` / `span_cmp` / `tpcbox_cmp`, **never**
`memcmp` — `Span` carries a declared `char padding[4]` that nothing guarantees is
zeroed.

A geodetic `STBox` holds longitude and latitude in **degrees** where a geodetic
`GBOX` holds unit-sphere xyz, so the box is hashed as planar and the SRID
normalisation then fits the degrees it actually holds. This is what
`FLAGS_SET_GEODETIC(gbox.flags, false)` is for.

### 2.1 The constant hiding inside it

```c
/* mobilitydb/pg_include/pg_temporal/index_sortsupport.h:71 */
#define MEOS_SORT_TIME_SHIFT 24
```

Dropping 24 bits of a microsecond timestamp makes the time unit ≈16.8 seconds,
which keeps roughly 1100 years either side of the epoch inside the 32 bits the
curve is given. **No measurement backs this number.** It is the seconds-per-degree
decision of the introduction, taken implicitly. Any discussion
that frames the shipped key as constant-free and a candidate as constant-bearing
has mis-stated the comparison.

---

## 3. The crux: a constant no purely spatial key needs

PostGIS has the same mechanism and needs no such constant, and the reason is worth
stating precisely, because it is what makes this a genuinely new problem rather
than a port.

Its key is two-dimensional and **both dimensions are the same kind of quantity**.
Treating one unit of *x* as worth one unit of *y* is canonical; it needs no
defence, and no dataset can make it wrong.

Adding time makes a distance comparable to a duration. Any single total order over
`(x, y, t)` must fix that exchange rate somewhere — in a bit shift, in a bucket
width, in a bits-per-dimension allocation, or in a "characteristic velocity". The
forms differ; the commitment does not.

This is the sense in which the work is parked. It is not blocked for want of a key
that performs — §7 gives one that beats master on two independent corpus shapes.
It is blocked on the question of whether MobilityDB is willing to ship a constant
it cannot derive, while generalising PostGIS work that has none.

---

## 4. The measurement harness

Reproduce this before believing any new number.

**Corpus.** `~/gistbench/boxes_full.csv`: **1843774** trip bounding boxes built from
153084638 lines of ten Danish AIS day files, all 24 hours evenly covered
(≈76000/hour). A second shape, `~/gistbench/boxes_wide.csv`, holds **278909** boxes
from the same feed with trips cut at 7200 s instead of 900 s.

An earlier `boxes.csv` came from a self-imposed 4M-line-per-file cap and covers only
00:00–07:00, which makes the wide-space narrow-time query return **zero rows**.
Every conclusion about time drawn from it is void. This is the first trap of §11.

**Probe.** `~/probes/gistsort_locality.c` reads the AIS CSVs, groups fixes per MMSI
into trips, and takes the bounding box of each trip — exactly the `STBox` a
`tgeompoint` column would carry. `EMIT=<file>` writes `STBox` literals for a
`\copy`. Its build needs a `postgis_revision.h` stub in the build directory and
links `libpostgis.a` (**not** any `liblwgeom.a` — `gbox_get_sortable_hash` is only
in the former) plus `-lgeos_c -lproj`.

**Four query shapes**, which are the whole oracle:

| | shape |
| --- | --- |
| Q1 | selective in space **and** time |
| Q2 | **wide in space, narrow in time** |
| Q3 | **narrow in space, wide in time** |
| Q4 | both tight, one vessel-sized window |

Q2 and Q3 are the orthogonal pair of §6 and are what every candidate is really
being tested on. A sweep that reports only an average hides exactly the failure
that matters.

**Discipline.** Arms are interleaved per round; ratios are the only valid timing
under a machine that may be running peer builds. **Read the row counts before the
clock** — all runs must answer identically, or an arm is fast because it answers
short. Buffer counts are deterministic and reproduce bit-identically across rounds;
`CREATE INDEX` time is not, and should be quoted only from a quiet machine.

---

## 5. The ledger — every key shape measured

All ratios are against shipped master on the 1843774-box corpus, and all arms
answer 19678 / 23390 / 99539 / 82 rows.

| arm | key | Q1 | Q2 | Q3 | Q4 | build |
| --- | --- | --- | --- | --- | --- | --- |
| ctl | shipped: `hilbert(gbox_hash >> 32, timerank32)` | 1818 buf | 460 buf | 5587 buf | 59 buf | 10454 ms |
| 3-D | **interleaved** Hilbert over (x, y, t), 21 bits each | **0.36** | **6.51** | **0.36** | 1.37 | 1.12 |
| C | composed shape, space ranked as integers, 16 b/axis | 1.13 | 1.03 | 1.17 | 1.14 | 1.01 |
| D21 | lexicographic, time 21 bits (50 min buckets) | 0.40 | **1.19** | 0.53 | 0.24 | — |
| D26 | lexicographic, time 26 bits (94 s buckets) | **1.09** | 0.95 | **1.14** | 0.93 | — |
| **D23** | **lexicographic, time 23 / space 20 (12.5 min)** | **0.65** | **0.99** | **0.85** | **0.53** | 1.02 |

The criterion throughout is **domination on all four**, not the mean. A key that
halves three shapes and multiplies the fourth by 6.5 is not a 0.72-average
improvement; it is a regression for anyone whose workload is that fourth shape.

**Arm C is the informative negative.** The composed shape — the shipped form with
its space half given real resolution instead of the degenerate hash — loses on all
four. That is the diagnosis of the shipped key: it is not a spatiotemporal key with
a fixable space half; it is a **time order** whose space half is inert, and giving
that half genuine resolution while keeping the 32/32 weighting makes it worse. See
§11.2.

**The bucket width is a sharp dial and its mechanism is rows-per-bucket against
page capacity** (a page holds ≈135 entries). 94-second buckets hold ≈200 rows, 1.5
pages, so space cannot cluster inside one and the key degenerates to master. 50-
minute buckets hold ≈6400 rows, 47 pages, so space clusters superbly but a 3-hour
query straddles only ≈3.6 buckets and the partial ones cost 19%. 12.5-minute
buckets hold ≈1600 rows, 12 pages — enough to cluster, fine enough that edge waste
vanishes.

---

## 6. The structural bound on interleaving curves

**Read this before proposing an *n*-dimensional Hilbert curve.** The proposal is
natural, it is implemented on branch `index/sortsupport-nd-hilbert`, and its result
is the 3-D row above.

A total order linearises three-dimensional space. Consider the two query shapes:

```
Q2   a thin slab in t, spanning all of x,y
Q3   a thin box in x,y, spanning all of t
```

These are **orthogonal**. A key whose iteration is contiguous for one is scattered
for the other. A curve that mixes the dimensions **at every order of the grid** —
which is what interleaving means — must therefore trade one against the other, and
no allocation of bits per dimension removes the trade. It only slides where the
trade sits.

The measured 3-D arm shows this at its sharpest: **0.36 on Q1 and Q3, 6.51 on Q2**.
It is not badly tuned. It is a balanced curve, and a balanced curve is the worst
possible answer for Q2 because the shipped key is good at Q2 precisely by being
*unbalanced* — degenerate, in fact (§11.2).

Two honest qualifications:

1. **The measured allocation is 21/21/21**, not every allocation. "No allocation
   helps" is an argument from the geometry above, not an exhaustive sweep.
2. **The argument has a measured endpoint.** Push the weighting until *all* time
   bits sit above *all* space bits, and it stops being an interleaved curve and
   becomes the lexicographic key of §7 — which does dominate on all four. So the
   interleaved family is not merely untuned; it is **dominated by a family already
   in hand**.

**The correctness of a Hilbert transform is not the issue.** Skilling's transform
(*Programming the Hilbert curve*, AIP Conf. Proc. 707, 2004) holds over every cell
of ten grids at 2, 3 and 4 dimensions, exhaustively checked: bijective onto
`0..n-1`, and consecutive indices differ by 1 in exactly one coordinate. The claim
that "a *d*-dimensional curve is untestable" is **false and must not be quoted**. A curve's correctness is unit-testable; what has no oracle
in this repository is a sort order's *clustering* (§11.1).

---

## 7. The lexicographic key — the candidate that escapes the bound

```
key = (time_rank << 2*nbits) | hilbert2d(x_rank, y_rank)
```

Time in the high bits, a two-dimensional spatial Hilbert curve in the low ones.

The §6 bound governs curves that mix the dimensions at every order. This one does
not mix them at all: it orders by one dimension and breaks ties by the others. A
time range therefore stays **one contiguous run** whatever the space bits do — which
is the property that makes the shipped key good at Q2 — while inside each time
bucket the spatial Hilbert curve clusters, which is what Q1, Q3 and Q4 want.

Do **not** quote the §6 bound as "no sort key can win on both shapes". D23 wins all
four. The bound applies to interleaving, and this construction is not interleaving.

It is also a different construction from arm C, which put both halves through *one*
curve so that space perturbed the time order.

Its cost is one short run per time bucket for a wide-time query, and the bucket
width governs how many such runs there are.

---

## 8. The rows-per-bucket rule

The measured optimum is **not** "12.5 minutes". It is *the bucket that holds roughly
a dozen pages of entries*.

With the time domain spanning ±100 years, a bucket at `tbits` is
`6.3115e15 µs / 2^tbits`; at 23 bits that is 752 s = 12.54 min, and over 1843774
boxes spread across 10 days it holds `1843774 × 752 / 864000 ≈ 1606` rows ≈ 12
pages.

### 8.1 The confirming experiment

If the real quantity is rows per bucket, then changing the row count must move the
optimum by a predictable amount. Cutting trips at 7200 s instead of 900 s over the
same ten days gives **278909** boxes instead of 1843774 — 6.6× fewer, and
log₂ 6.6 = **2.7 bits**.

Wide corpus, control is shipped master, all arms answering 3079 / 5914 / 14779 / 23
rows, and **all three rounds bit-identical**:

| arm | rows/bucket | Q1 | Q2 | Q3 | Q4 | dominates? |
| --- | --- | --- | --- | --- | --- | --- |
| ctl | — | 395 buf | 139 buf | 1126 buf | 29 buf | — |
| t=19 | 3886 | 0.44 | **1.37** | 0.68 | 0.59 | no — loses Q2 |
| **t=20** | 1943 | 0.48 | 0.96 | 0.76 | 0.55 | **yes** |
| **t=21** | 972 | 0.66 | 0.92 | 0.88 | 0.45 | **yes** |
| t=23 | 243 | 0.81 | 0.91 | **1.00** | 0.59 | no — ties Q3 |

Both boundaries of the winning window slid by the predicted amount:

| boundary | narrow corpus | wide corpus | shift | predicted |
| --- | --- | --- | --- | --- |
| Q2 breaks (bucket too wide) | t=21 (1.19) | t=19 (1.37) | 2 bits | 2.7 |
| degenerates (bucket too narrow) | t=26 (1.09/1.14) | t=23 (1.00) | 3 bits | 2.7 |

12.5 minutes falls outside the wide corpus's window entirely, while the narrow
corpus's winning 1606 rows/bucket sits inside that window's span of 972–1943.

**The quantity is rows per bucket — a low multiple of page capacity — not a
duration.** And rows per bucket derives from `N` and the column's time extent,
both of which a table knows about itself. The constant can in principle be
computed rather than fitted.

### 8.2 What this does *not* establish

Two corpus shapes fix that the window **moves with N**, and roughly by the
predicted amount. They do not fix the exact slope, and both corpora are AIS. A
third shape — or a non-mobility workload — could still surprise.

---

## 9. Why PostgreSQL cannot derive it

To compute the bucket at build time the sort key function would have to see the
table. **It cannot.** Read in PostgreSQL 16.13, 17.9 and 18.3; identical in all
three.

1. `PrepareSortSupportFromGistIndexRel(indexRel, ssup)` holds `indexRel` **in hand**
   and passes only `OidFunctionCall1(fn, PointerGetDatum(ssup))`
   (`src/backend/utils/sort/sortsupport.c`). No relation crosses the boundary.
2. `SortSupportData` (`src/include/utils/sortsupport.h`) declares no relation.
   `ssup_extra` is `palloc0`'d in `tuplesort_begin_index_gist` and, across the whole
   backend, is written **only by opclass functions themselves** — core never primes
   it.
3. **Opclass parameters cannot smuggle it either.** They ride `flinfo->fn_extra`,
   set only by `index_getprocinfo` (`src/backend/access/index/indexam.c`), while
   `OidFunctionCall1Coll` builds a bare `FmgrInfo` via `fmgr_info`
   (`src/backend/utils/fmgr/fmgr.c`). So `PG_HAS_OPCLASS_OPTIONS()` is **false**
   inside a GiST sortsupport function.
4. **The key cannot adapt from the data either.** `tuplesort_begin_index_gist`,
   which fixes the comparator, runs *before* `table_index_build_scan` feeds the
   first tuple (`src/backend/access/gist/gistbuild.c`). `abbrev_abort` can only
   switch abbreviation off; it cannot re-key.

**On PostgreSQL the bucket width can only be a compiled-in constant or a GUC.**
Nothing about where MobilityDB keeps its own statistics changes this: the blocker
is the entry point, not the location of the statistics.

---

## 10. The routes out

### 10.1 Sort-Tile-Recursive packing — needs no exchange rate, but not expressible

Sort by *x*, cut into `ceil(sqrt(N/f))` slabs, sort each slab by *y*, pack. Every
count derives from `N` and page capacity, and **no cross-dimensional comparison
ever happens** — so no exchange rate is ever fixed. It is the one construction that
genuinely dissolves §3.

But `sortsupport` **cannot express it**: which slab an entry lands in depends on
*all* entries, and a `SortSupport` comparator sees two. This route means a build
path of our own, not `FUNCTION 11`.

### 10.2 An upstream PostgreSQL patch

`PrepareSortSupportFromGistIndexRel` already holds the index relation and simply
passes nothing along. Priming `fn_extra` with opclass options there is a few lines
and follows the existing opclass-parameter design, which would make
`USING gist (col opclass(time_bits = 21))` expressible — the same shape as
`gist_trgm_ops(siglen = ...)`. This is upstream's gap, not MobilityDB's, and it is
a slow path.

### 10.3 A GUC

Reachable from anywhere in the backend, so it works. It moves the choice to
somebody who knows their data — but it still needs a default, and most installations
will never set it.

### 10.4 Derive it where the build is ours

MEOS and its in-memory index own their bulk build end to end, so they can take a
pre-pass over the entries and compute `N` and the time extent honestly. There the
constant genuinely becomes derived, and the rule of §8 is directly usable.

Two things would have to move out of `mobilitydb/` for that to pay off, and they
are separable pieces of work:

- The **statistics machinery**. `mobilitydb/src/geo/tspatial_analyze.c` splits
  favourably: its `ND_BOX` / `ND_STATS` core takes live structs only, with no
  PostgreSQL coupling, and lifts essentially as-is, while `mobilitydb/src/temporal/span_analyze.c`
  and `temporal_analyze.c` compute histograms **directly into** `stats->stakind` /
  `stavalues` / `stanumbers` as Datums, so their store/processing split has to be
  manufactured — a MEOS core taking live structs, with a thin shim that fetches
  Datums and packs the slots.
- The **key itself**. `index_sortsupport.c` / `.h` are under `mobilitydb/`, so no
  other engine can share the sort key today. A derivation with no key to feed buys
  nothing.

### 10.5 Characteristic velocity

A proposal worth recording because it will be made again: scale time into a
spatial-equivalent dimension by a reference speed, `t' = v_ref · t`, so that all
dimensions carry metres and the curve becomes physically meaningful.

It is the best *documentation* of the constant we have — far better than a bare bit
shift. But it is **not a derivation**: `v_ref` *is* the exchange rate of §3, given a
physical name. And it is per-dataset by its own logic (walking ≈1.5 m/s, urban
≈10 m/s, highway ≈30 m/s), so it reintroduces exactly the fitted-to-one-workload
property that parked this work — while §9 says the value cannot be derived at build
time anyway. Adopting it would ship a compiled-in guess with a physical-sounding
name.

Note also that it does not address §6: a velocity-scaled interleaved curve is still
an interleaved curve, and Q2 still breaks.

---

## 11. Traps and negative results

### 11.1 The suite cannot adjudicate a sort order

`*_indexes_tbl` proves the index **answers correctly** — that oracle is real — but
every total order returns the right rows, so pg_regress is green whatever the order
is and can see nothing about clustering. Build time **and** query page reads are
both required, or a faster build with a worse index passes as a win.

A related blindness: `tools/scripts/check_opclass_members.py` compares OPERATOR
members and is blind to a missing FUNCTION, so it would never report a class that
forgot `FUNCTION 11`. What keeps a future family born with it are the
`indexes.sql.tmpl` and `gist.sql.tmpl` templates, and `INHERITANCE_MAP.md` §6 names
what those templates declare — so a member added to a template belongs in the same
commit.

### 11.2 The shipped key is a time index in disguise

`stbox_sort_hash` truncates `gbox_get_sortable_hash() >> 32`, keeping 16 of 32 bits
per axis. The PostGIS normalisation puts every coordinate in `[1,2)`, where a
float32 sign and exponent are **9 constant bits** — so only **7 mantissa bits
survive**, a cell roughly 4 degrees across.

Two independent measurements convict it:

| measurement | distinct key values | longitude resolution |
| --- | --- | --- |
| 400×400 grid over the benchmark window, shipped `>> 32` | **6** of 160000 | 4.19° ≈ 267 km |
| same grid, hash undivided | **160000** of 160000 | 4.1e-05° ≈ 2.6 m |
| the real corpus, shipped `>> 32` | **96** of 1843774, one covering **35.3%** | — |

**The rule that generalises: a monotone float→integer map is order-preserving but
not uniformly spaced, so its high bits are not a coarser version of the value —
they are the exponent.** Where a curve is given `b` bits per dimension, rank with a
**linear map from a declared domain onto `[0, 2^b)`**, never with a truncated bit
pattern. The tell, worth recognising on sight, is a `>>` applied to the result of a
float-bit-pattern hash.

The consequence is uncomfortable and should be stated plainly: **the 0.12–0.14
buffer ratios of §1 belong to the time half**, and arm C shows that repairing the
degeneracy inside the composed form *regresses on all four shapes*. The shipped key
performs well **because** it is degenerate. This is a real defect that exists today,
and it is not a defect with a free fix.

A general lesson worth carrying: **a float bit pattern cannot be truncated.** Sign
and exponent are constant over a normalised range, so `hash >> 32` keeps only
mantissa.

### 11.3 A single scalar clustering score cannot pick the exchange rate

The probe scores a sort order by summing per-page bounding-box extent and reporting
`norm volume` = Σ (Δx/ex)·(Δy/ey)·(Δt/et), each dimension divided by the corpus
extent. Over the lexicographic family it falls **monotonically** — 3.839e-05 at
t=17 down to 3.771e-06 at t=26 — so it ranks **t=26 best**. PostgreSQL ranks t=26
**worst** of the three tried (§5).

The cause is structural, not a tuning slip. Dividing Δt by the corpus duration and
Δx by the corpus width **declares how many seconds one degree is worth**. That ratio
is the very constant under test, so the metric answers with whatever rate it was
built from. A scalar volume can compare two orders at a **fixed** bit split; it
cannot choose the split.

This retires any single-number clustering score here — "tightness", "volume",
"average page extent". The oracle has to be query shapes, because **a query is what
fixes the rate**.

What the probe *can* still do is report per-dimension page extents, which need no
normalisation and which corroborate §8 in exchange-rate-free form.

### 11.4 A sorted build buys build time and can spend query time

PostGIS states this in its own installed SQL: sort support stays out of the default
operator class below PostgreSQL 15 because of *"query performance degradation caused
by GiST index page overlap"*, and from 15 the sorting build *"uses picksplit function
to find better partitioning for index records"*.

So an A/B must show query cost did not regress; it is not enough to show the build
got faster. It gates nothing here — CI builds 16 through 19 — but it names the exact
quantity to check.

### 11.5 Operational traps

- **Both build directories must be configured with the PostgreSQL 18 binaries on
  `PATH`.** ctest starts the postgres the CMake cache holds; a PG-17-configured build
  dies on `unrecognized configuration parameter extension_control_path` and the
  runner reports `done` having written nothing.
- **Never `git stash` in one of these worktrees.** The stash stack is shared across
  worktrees of one repository; a `git stash -u` here popped an unrelated session's
  `stash@{0}`.
- **SRIDs outside 4326 / 3857 / 3395 have no declared extent**, so a coordinate falls
  back to its bit pattern. That is PostGIS's own position — `gbox.c` carries a
  `TODO: reconsider when we will have machinery to properly get bounds by SRID`.
  Name the gap; never present it as covered.
- The **775×** figure that circulated in this campaign belongs to the in-memory twin
  and has no PostgreSQL-side reproduction. The numbers in §1 are the ones to quote.

---

## 12. What would reopen this

The work is parked, not abandoned. Any **one** of the following changes the picture:

1. **A construction that needs no exchange rate and fits a pairwise comparator.**
   STR (§10.1) qualifies on the first count and fails on the second. Something that
   passes both settles §3 outright.
2. **A PostgreSQL release that lets support function 11 see the index relation or its
   opclass options** (§10.2). Then §8 becomes a per-table derivation and the
   objection dissolves.
3. **A decision to ship the derivation in MEOS only** (§10.4), accepting a documented
   default on the PostgreSQL side. This needs no upstream change and is the shortest
   path to using §8 for something.
4. **A third corpus shape that breaks the rows-per-bucket rule** (§8.2). That would
   retire the lexicographic family and make the whole question moot.
5. **An A/B in which the composed key wins build time but loses query time.** That
   was the original condition for reopening the composed-vs-curve fork; the
   measurements in §5 show it winning both, so the condition is currently unmet.

What does **not** reopen it: a proposal to build an *n*-dimensional Hilbert curve
over normalised centroids. That is the 3-D row of §5, it is implemented, and §6
explains why its failure is structural rather than a tuning miss.

---

## 13. Reproducing every measurement

```sh
# 1. Build the corpus (both shapes)
EMIT=~/gistbench/boxes_full.csv \
  ~/probes/gistsort_locality 999999999 <ais-dir>/aisdk-2025-01-*.csv
GAP_SEC=1800 MAXDUR_SEC=7200 EMIT=~/gistbench/boxes_wide.csv \
  ~/probes/gistsort_locality_shape 999999999 <ais-dir>/aisdk-2025-01-*.csv

# 2. Build the probe (gbox_get_sortable_hash is in libpostgis.a, not liblwgeom.a)
gcc -O2 -o gistsort_locality_shape gistsort_locality_shape.c \
  -I<wt>/build-ext/postgis/liblwgeom -I<wt>/postgis/liblwgeom -I<wt>/postgis \
  <wt>/build-ext/postgis/libpostgis.a -lgeos_c -lproj -lm
```

Each arm is a separate worktree so that a control and a treatment differ by exactly
one commit, and the runner drives each worktree's own isolated cluster over its own
Unix socket, so parallel sessions never collide. Load the corpus into a
`CREATE TABLE boxes (box stbox)`, `VACUUM ANALYZE`, then per arm: drop the index,
`CHECKPOINT`, `CREATE INDEX ... USING gist (box)`, record `pg_relation_size`, and run
the four queries of §4 under `EXPLAIN (ANALYZE, BUFFERS, COSTS off, TIMING off)` with
`enable_seqscan = off`.

Read the row counts first. Then read total buffers — **hit-versus-read is not a
quality signal**, because each arm builds its own index so cache warmth differs.

---

Related: `tools/codegen/inherited/INHERITANCE_MAP.md` §6 names what the index
templates declare, so an operator-class member added to a template belongs in the
same commit as the template change.
