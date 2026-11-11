<!--
  MobilityDB — Exactness: The exact arithmetic of the geometry engine
  Copyright(c) MobilityDB Contributors

  This documentation is licensed under a Creative Commons Attribution-Share
  Alike 3.0 License: https://creativecommons.org/licenses/by-sa/3.0/
-->

# The exact arithmetic of the geometry engine

**For discussion by the committers. Nothing here is merged.** Master at the time of writing:
`a4ac8e7b92`. Every function, type and constant is cited by name and file; every count is read on
that master.

## 1. Summary

Every exact answer of MEOS, a distance, a predicate, a relationship, a buffer or a hull, ends in
signs of polynomials of the input doubles. Those signs are computed by one arithmetic made of
tiers: a value in doubles with a bound on its error, a pair of doubles with a bound, an expansion
of doubles, and a wide dyadic number. A sign is read in the cheapest tier whose bound decides it,
and the last tier decides every finite input.

This note states that arithmetic as one layer of the engine, and the rule that keeps it one:

- **A. Three layers.** The user entries, the geometric algorithms, and the exact arithmetic
  (Section 2). The exact arithmetic is spread today over `meos/src/geo/geo_funcs.c` and
  `meos/include/geo/geo_funcs.h`, interleaved with the algorithms that use it.
- **B. One machinery per need.** The engine holds three tier ladders, two pair-of-doubles
  types, three wide-number types and four searches for the nearest double (Section 4). Nothing
  stops a new exact question from adding its own; the rule of Section 5 does, and the plan of
  Section 7 merges what exists.
- **C. A seam at the user entries.** An entry checks its arguments and calls the engine once
  (Section 6). That seam keeps another geometry library possible behind the same entries; no
  such library is planned, and GEOS stays a test witness.

## 2. The three layers

| Layer | What it holds | Where it lives |
|---|---|---|
| User entries | The functions a caller of MEOS reaches, most of them named after their PostGIS counterpart: argument checks, then one call into the engine | `meos/src/geo/postgis_funcs.c` (70 entries carry `@note PostGIS function`), a few in `geo_funcs.c` and `geo_buffer.c` |
| Geometric algorithms | The distance walks, the relate engine (DE-9IM), the arrangement of edges and arcs, the buffer, the hulls, the centroid, the enclosing circle | `meos/src/geo/geo_funcs.c`, `geo_arrange.c`, `geo_buffer.c`, `postgis_funcs.c` |
| Exact arithmetic | The number representations, the tiers, and the exact signs every algorithm reads: orientation, incircle, the offset of a point from a segment or a plane, the meeting of an arc and a segment, the numbers of a tower of square roots | `meos/src/geo/geo_funcs.c` and `meos/include/geo/geo_funcs.h` |

An algorithm never computes a sign itself: it asks the arithmetic layer for it. The arithmetic
layer knows no geometry type: its inputs are doubles and arrays of doubles.

## 3. The tiers

### 3.1 Tier 0: doubles with a bound

The value is evaluated in doubles, and a bound on its error is computed from the magnitudes of
its terms, as Shewchuk bounds a determinant. Where the value stands clear of its bound its sign
is exact. The bound is a filter, never a tolerance: it decides whether the double can be trusted,
never what the answer is. Most calls end here.

- Written per predicate, inline, because it is the hot path: `cross_product_sign`,
  `dot_product_sign` and `triple_product_sign` are `static inline` in `geo_funcs.h`, each
  calling its exact stage, defined in `geo_funcs.c`, only where the bound cannot tell.
- Generic forms: `ArcsegFilter` (a value and its bound) with the `arcseg_f_*` operations, and
  the first pass of `polynomial_sign_filter`.
- Range: every term between 2^-900 and 2^900, or as the predicate states; outside it the tier
  declines and the next one reads the value.

### 3.2 Tier 1: pairs of doubles with a bound

The value is held as the unevaluated sum of two doubles, formed by the error-free transforms
`two_diff`, `two_product` and `two_sum`, with a bound on its distance to the exact value: about
104 bits. An exact zero, a pair of zeros with no error, is an exact zero, which tier 0 cannot
tell.

- `DDBound` with the `ddb_*` operations (`ddb_add`, `ddb_mul`, `ddb_dot3`, `ddb_cross3`,
  `ddb_sign`, `ddb_is_zero`).
- The pair functions `pair_mul`, `pair_add` and `pair_mid_sign`, whose bound the caller carries,
  and `polynomial_sign_pairs` and `incircle_sign_filter`, which carry their own.

### 3.3 Tier 2: expansions

The value is an expansion, a sum of doubles that do not overlap, held in increasing order of
magnitude, whose last component carries the sign of the whole (Shewchuk). Sums and products are
exact: `grow_expansion`, `scale_expansion`, `expansion_sum`, `expansion_product`, and
`compress_expansion` keeps them short.

- Range: exact where no product of components overflows or underflows. A degree-three sign of
  differences each within 2^-200 and 2^200 stays inside it (`vector_triple_sign`); a sign of
  degree six needs a narrower range, as `segment_square_least` states for its scaled values
  (lowest bit at or above 2^-179).
- Memory: `polynomial_sign_stack` forms each term in an arena of `POLY_STACK_ARENA` (2048)
  doubles and the total in two buffers of `POLY_STACK_TOTAL` (1024); `ArcsegArena` holds
  `ARCSEG_ARENA_SIZE` (4096) doubles on the stack and takes blocks past them. An arena records
  when a product may have left the exact range (`unsafe`), and the answer of that evaluation is
  discarded for the next tier's.

### 3.4 Tier 3: wide dyadic numbers

The value is a dyadic rational, a sign, an integer magnitude in 64-bit limbs and the weight of
its lowest bit. Every finite double is one, and sums and products are exact at any magnitude:
this tier decides every finite input.

- `WideDyadic`: a fixed capacity of `WIDE_DYADIC_LIMBS` (320) limbs, so one value takes about
  2.5 KB and a copy moves all of it; `wide_dyadic_add`, `wide_dyadic_mul`,
  `wide_dyadic_product_cmp`, `polynomial_sign_wide`.
- `TowerWide`: the same number with as many limbs as the value takes, allocated in the blocks of
  a `TowerCtx`.
- The two's complement integer of `WIDE_LIMBS` (66) limbs that `square_products_wide` sets, with
  `wide_add`, `wide_product`, `wide_magnitude` and `wide_sign`, for a sum of products of two
  doubles.

### 3.5 Rounding: the double nearest an exact value

A distance is answered as the double nearest its exact value. The double is found by comparing
the exact value with the midpoints between candidate doubles, each comparison an exact sign, so no
rounded value decides it. `nearest_double_exact` does this for any value a side function reads
exactly: a gallop from a candidate brackets the answer, and halving the bracket finds it.

### 3.6 What the tiers cost

Measured by callgrind on the cell "refuters at" of ED24 (the refuting face corpus, 320 rows, each
asked at the double nearest its distance), on the branch of MobilityDB #3484 over master
`9695710930`, with the probe `perf_facedwithin3d`, which times `geom_dwithin3d` against
`lwgeom_mindistance3d_tolerance` in one process:

| Where | Cost |
|---|---|
| The whole cell, native | 3.42M instructions, against 1.67M for `lwgeom_mindistance3d_tolerance` called directly (2.05) |
| Tier 3 (`WideDyadic`), every function together | 0.63M, 19% of the cell |
| One tie of `plane_point_offset_sign` read in tier 3 | about 12K instructions; the 8 of its 32 calls that reach tier 3 take 0.10M |
| One row of lwgeom, for scale | about 5.2K instructions |

A single tie read in tier 3 costs more than lwgeom's whole answer to a row. A tier 2 between
tier 1 and tier 3 is what keeps a tie off tier 3.

## 4. The machinery the engine holds

### 4.1 Three tier ladders

| Ladder | How a question is written | Tiers | Call sites |
|---|---|---|---|
| The expression ladder: `tower_pred_run` over a `TowerCtx`, values `TowerX`, operations `tx_diff`, `tx_add`, `tx_sub`, `tx_scale`, `tx_mul`, `tx_sign`, `tx_zero` | Once, as a function of the `tx_*` operations (`TowerPred`); the same code runs in every tier | 0 (`ArcsegFilter`), 1 (`DDBound`), 2 (expansions in an `ArcsegArena`), 3 (`TowerWide`), each read where the one before cannot tell or leaves its range | 17 calls of `tower_pred_run` |
| The polynomial ladder: `polynomial_sign_exact`, then `polynomial_sign_wide` where the caller says the range is left | As a list of terms (`PolyTerm`: a coefficient times up to `POLY_TERM_MAXDEG` (6) exact sums) over exact sums (`ExactSum`: up to `EXACT_SUM_MAXTERMS` (4) doubles held as an expansion) | 0 (`polynomial_sign_filter`), 1 (`polynomial_sign_pairs`), 2 (`polynomial_sign_stack`, then the heap), 3 (`polynomial_sign_wide`, read by `segment_square_cmp` alone) | 54 calls of `polynomial_sign_exact`, 1 of `polynomial_sign_wide` |
| Hand-written ladders | Each predicate restates its value in each tier it has | as each one has them (Section 4.4) | each predicate its own |

The expression ladder is the most general. It takes any expression, not only an expanded
polynomial, it has all four tiers, and its tier 3 holds a value of any size. The polynomial
ladder is the most used.

### 4.2 Two pair-of-doubles types

`DDBound` carries its bound in the value; `pair_mul`, `pair_add`, `pair_mid_sign`, the pairs of
`polynomial_sign_pairs` and of `incircle_sign_filter` (`incircle_pair`) leave it to the caller.
Both stand for the same tier.

### 4.3 Three wide-number types

`WideDyadic` (fixed capacity), `TowerWide` (the size of the value) and the `WIDE_LIMBS` integer
of `square_products_wide` all hold an exact dyadic number in 64-bit limbs. `TowerWide` is the
general one: `WideDyadic` is the same number at a fixed capacity, and an integer of `WIDE_LIMBS`
limbs is one whose lowest bit has a fixed weight (`WIDE_LOWEST_BIT`).

### 4.4 The exact signs and the tiers each one has

| Sign | Degree | Tier 0 | Tier 1 | Tier 2 | Tier 3 | Calls |
|---|---|---|---|---|---|---|
| `cross_product_sign` (orientation in the plane) | 2 | inline in `geo_funcs.h` | | `cross_product_sign_exact`, scaled | `cross_product_sign_wide` | 102 |
| `dot_product_sign` | 2 | inline | | `dot_product_sign_exact` | none | 1 |
| `triple_product_sign` | 3 | inline | | `triple_product_sign_exact` | none | 6 |
| `incircle_sign` | 4 | in doubles | `incircle_sign_filter` | `incircle_sign_exact` | none | 3 |
| `vector_triple_sign` | 3 | none | `DDBound` | `triple_product_sign_exact` where every difference is exact | `WideDyadic` | 7 |
| `plane_cross_dot_sign` | 4 | none | `DDBound` | none | `WideDyadic` | 3 |
| `plane_point_offset_sign` | 6 | none | `DDBound` | none | `wide_dyadic_product_cmp` | 1 |
| `segment_interior_offset_sign` | 6 | none | none | none | `segment_interior_wide_values` | 1 |
| `point_segment_offset_sign` | 4 | none | none | `square_products_expansion` where `square_products_exact` holds | `square_products_wide` and `wide_product` | 6 |
| `segment_square_cmp` | 6 | `polynomial_sign_exact` | `polynomial_sign_exact` | `polynomial_sign_exact` | `polynomial_sign_wide` | 1 |
| the signs of an arc and a segment, of two arcs, of a tower | up to 8 | the expression ladder | the expression ladder | the expression ladder | the expression ladder | 17 |

The branch of MobilityDB #3484 (draft) adds tier 0 to `vector_triple_sign`,
`plane_point_offset_sign` and `segment_interior_offset_sign`, tier 1 to
`segment_interior_offset_sign` (`segment_interior_offset_filter`) and to
`point_segment_offset_sign` (`point_segment_side_sign_filter`); each is written in the predicate,
by hand, as Section 5 asks no new one to be.

Two gaps follow from the table:

- **No tier 2** in `plane_cross_dot_sign`, `plane_point_offset_sign` and
  `segment_interior_offset_sign`: a tie goes to tier 3, at the cost of Section 3.6.
- **No tier 3** in `dot_product_sign`, `triple_product_sign` and `incircle_sign`: their exact
  stages state they are exact where no product overflows or underflows, and their bodies call
  no stage of wide numbers after their expansions.

### 4.5 Four searches for the nearest double

`nearest_double_exact` (any exact side), `wide_dyadic_nearest_root` (the root of a quotient of
`WideDyadic`), `ddb_nearest_root` (a root placed by `DDBound` residuals) and the candidate moves
of `point_distance_offset_filter` all find the double nearest an exact value by moving a
candidate across the midpoints. `wide_dyadic_nearest_root` and `nearest_double_exact` state the
same gallop and halving.

## 5. The rule

**One machinery per need.** A new exact question is written on what the arithmetic layer
holds:

1. **Its sign is an expression of the input doubles, written once**, and the one ladder reads it
   in every tier. A question restates its value per tier only in its tier 0, where an inline
   filter specific to it is the hot path.
2. **Every sign has every tier.** Tier 0 or 1 decides most calls, tier 2 keeps ties off tier 3,
   and tier 3 makes it exact for every finite input. A sign missing a tier is a gap of this
   layer, closed here and not in the predicate.
3. **One type per tier**: one bounded value in doubles, one pair of doubles, one expansion with
   its arena, one wide number.
4. **One search for the nearest double**, reading any exact side.
5. **A question an algorithm needs and the layer lacks is added to the layer**, as a sign of the
   arithmetic, with its tiers, where every other algorithm reaches it.

Section 7 merges the machinery of Section 4 into this one, a step per pull request.

## 6. The seam at the user entries

A user entry checks its arguments and calls the engine once: `geom_dwithin3d` validates its
operands, dispatches by dimension and type, and asks `geom_dwithin3d_faces` or the distance walk
for the answer. Kept that way, the engine behind the entries can change without changing any
entry or any caller.

- **What the seam allows.** Another geometry library behind the same entries, such as a build
  that answers through GEOS or CGAL, needs only the engine calls replaced. Its answers would
  differ where the libraries differ: MEOS answers the double nearest the exact value and reads
  circular arcs and faces in 3D as they are, which GEOS does not.
- **What is decided.** No such build is planned. Once every entry answers natively, GEOS leaves
  the shipped library and stays the witness of the tests where straight segments are compared;
  on master MEOS links no GEOS, and the stand-in header `meos/include/geos-none/geos_c.h` is
  all that remains of it.
- **What the seam asks.** An entry holds no geometry of its own: no walk, no sign, no loop over
  points. The entries of `postgis_funcs.c` are kept to that.

## 7. Plan

Each step is one pull request, judged as every pull request of these notes is: CGAL on the
head's answers, and the speed cells of every operation it touches, measured against GEOS called
directly.

### EA1. Which ladder is the one ladder

The expression ladder and the polynomial ladder each answer what the other answers; one of them
is kept. The step measures the cost of one sign read both ways, on the same inputs, before
anything moves:

- `segment_square_cmp`, a degree-six polynomial on the polynomial ladder, written as a
  `TowerPred`;
- `plane_point_offset_sign`, written both ways, on the ties of "refuters at".

The expression ladder is the proposal, being the general one, provided its cost per sign is at
most that of the polynomial ladder. Two of its costs the polynomial ladder does not have: every
`tx_*` operation branches on the tier of its context, and `tower_pred_run` evaluates the whole
question again in each tier it reaches, where the polynomial ladder forms its terms once. Whether
they matter on a short sign is what the measurement decides. The ladder kept takes a
name of the arithmetic, not of the towers it first served.

**The measurement.** Every call the speed cells make to `polynomial_sign_exact` and to
`plane_point_offset_sign` on master `a4ac8e7b92` is recorded by a shim preloaded ahead of
`libmeos` (both are reached through its PLT), then answered by each form of its sign in a pass
of its own under callgrind. The forms, added to a measurement build only:

- the polynomial ladder itself (`polynomial_sign_exact`), and the same terms on the expression
  ladder (`tower_pred_run` over the terms);
- for `plane_point_offset_sign`: the function (`DDBound`, then `WideDyadic`), its value on the
  expression ladder with the normal formed once, and its value expanded into 45 terms of degree
  six on the polynomial ladder;
- each question's own first tiers (`polynomial_sign_filter`, or the `DDBound` tier of the
  function) ahead of the expression ladder from its expansions on, the shape EA2 gives a sign.

Calls recorded: buffer of the natural areas at distance 1 (716 polynomials), the enclosing circle
of regular polygons (936), the face corpora of ED24 at the distance (40 polynomials and 11 009
calls of `plane_point_offset_sign`), the refuting rows of ED21 (14). The union, relate and
curve-distance cells make neither call.

| Question | Calls | Polynomial ladder | Expression ladder | Own first tiers, then expression ladder |
|---|---|---|---|---|
| polynomials of the buffer | 716 | 459 | 1 812 | 455 |
| polynomials of the circle | 932 | 4 211 | 8 426 | 4 205 |
| polynomials of the circle, ties | 4 | 9 180 | 18 997 | 15 634 |
| polynomials of the faces, ties | 8 | 31 438 | 62 423 | 54 035 |
| polynomials of ED21, ties | 6 | 10 410 | 6 953 | 19 356 |

Instructions per call. A tie is a call whose exact sign is 0.

| `plane_point_offset_sign` | Calls | The function | Expression ladder, normal formed once | Own `DDBound` tier, then expression ladder | 45 terms on the polynomial ladder |
|---|---|---|---|---|---|
| refuting faces, ties | 104 | 12 877 | 10 212 | 14 626 | 84 524, 50 calls answered 0 for -1 |
| refuting faces, others | 124 | 5 929 | 13 498 | 12 037 | 44 079 |
| random faces | 2 991 | 2 109 | 1 691 | 2 050 | 9 593 |
| real faces | 7 790 | 1 376 | 1 669 | 1 327 | 8 663 |

What it shows:

1. **The expression ladder is exact unaided; the polynomial ladder is not.** Every form on the
   expression ladder agrees with the function on all 11 009 calls and with the polynomial
   ladder on all 1 706 polynomials. The 45-term form answers 0 for -1 on the 50 calls asked at
   the least subnormal distance, where `d^2` underflows in the expansions: the polynomial ladder
   is exact only where its caller keeps the range, as `segment_square_least` does before
   choosing `polynomial_sign_wide`.
2. **How a question is written costs more than which ladder reads it.** Expanded into monomials,
   `plane_point_offset_sign` costs 4 to 7 times the function; written with its normal formed
   once, on the expression ladder, 0.79 to 1.21 times it on the ties, the random and the real
   faces.
3. **On the same terms, the polynomial ladder is the cheaper.** The expression ladder costs 2.0
   to 4.1 times it where the first tiers decide; with the question's own first tiers ahead of
   it, the two cost the same there. On the ties the expression ladder from its expansions on
   costs 1.5 to 1.9 times the polynomial ladder's expansions on the stack.
4. **No tier 2 of the engine brings a tie of degree six near lwgeom's cost of a row** (about
   5.2K instructions on the face corpus): a tie costs 10 to 15K on every form.

The condition of the proposal is not met as it stands: the expression ladder is the one way to
write a question (results 1 and 2), and its tiers past the first cost more than the polynomial
ladder's (result 3). The step that follows, before EA2, rebuilds the expansions of the expression
ladder on the stack arena of `polynomial_sign_stack` and measures again, so that one ladder
carries the cheaper evaluation of each tier.

### EA2. Tier 2 for the signs of degree six

`plane_point_offset_sign` and `segment_interior_offset_sign` read their ties on the ladder EA1
keeps, in place of going from `DDBound` to `WideDyadic`; `plane_cross_dot_sign` follows. This is
what MobilityDB #3484 needs to bring "refuters at" toward parity, and the distance walk of ED20,
ED21 and ED23 reaches `segment_interior_offset_sign` too.

### EA3. The arithmetic in its own file

A pure move: the representations, the tiers, the ladders, the searches and the exact signs leave
`geo_funcs.c` for a file of their own, with a header of their own; the algorithms stay. The
filters of tier 0 and the operations of tiers 0 and 1, `static inline` or
`pg_attribute_always_inline` today (116 such definitions in `geo_funcs.c`), go into the header as
`static inline`, so that every algorithm file inlines them as `geo_funcs.c` does. The step moves
no code but declarations and definitions, and its speed cells show no cell slower than before.

The build of MEOS has no link-time optimisation (no `-flto` or `INTERPROCEDURAL_OPTIMIZATION` in
its CMake files): a function defined outside the file of its callers is not inlined in them, and
no test sees the difference. The step therefore counts, for each moved function, its calls left in the
machine code at every call site, before and after the move (`objdump -d` for the calls inside a
file, `objdump -dr` for the `R_X86_64_PLT32` relocations across files), and no count rises. The
`inline` keyword decides nothing here: the count does.

The move waits for a window with no open pull request on these files: on `a4ac8e7b92`,
MobilityDB #3484 and #3557 change `geo_funcs.c` and `postgis_funcs.c`, and the steps of
[Exact buffer](EXACT-BUFFER.md) from 18 on change `geo_funcs.c`, `geo_arrange.c` and
`geo_buffer.c`. The names of the file and of its header are a decision of the committers
(Section 8).

### How a merge is proven

EA4 to EA6 each fold two or more implementations of one thing into one. Each starts with a
differential: the implementations exposed side by side in a measurement build and answered on
the same inputs, the calls the speed cells make recorded as EA1 records them, together with the
cases where the arithmetic is hard (exact ties, subnormal and huge magnitudes). A pair that
agrees everywhere is the evidence for the merge; a pair that disagrees is a defect, fixed first
in its own pull request. EA1 is such a differential, and it finds one: the polynomial ladder
answers a subnormal distance wrongly where its caller does not keep the range.

### EA4. One pair type

`DDBound` stands for tier 1: `pair_mul`, `pair_add`, `pair_mid_sign`, `incircle_pair` and the
pairs of `polynomial_sign_pairs` become `ddb_*` operations, their bounds carried in the value.

### EA5. One wide number

`TowerWide`, the size of its value, stands for tier 3: `WideDyadic` and the `WIDE_LIMBS` integer
of `square_products_wide` become it. A value of tier 3 then costs the limbs it holds, not 320.

### EA6. One search for the nearest double

`nearest_double_exact` stands for the search: `wide_dyadic_nearest_root`, `ddb_nearest_root` and
the candidate moves of `point_distance_offset_filter` become side functions it reads.

### EA7. Every sign with every tier

The gaps of Section 4.4 close on the one ladder: tier 2 where it is missing (after EA2), tier 3
for `dot_product_sign`, `triple_product_sign` and `incircle_sign`, each with a witness past the
range of its expansions.

## 8. Decisions asked of the committers

1. The one ladder: the expression ladder, provided EA1 measures it at the cost of the polynomial
   ladder or less.
2. The names of the arithmetic's file and header (EA3), and of the ladder (EA1), as the layer
   names them rather than the first question each served.
3. The window of EA3: one in which no pull request on these files is open, MobilityDB #3484
   and #3557 being two such pull requests, and step 18 of [Exact buffer](EXACT-BUFFER.md) not
   started.
4. The seam of Section 6 as the rule of the user entries, with no other library planned.
