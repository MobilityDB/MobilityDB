<!--
  MobilityDB — Recurring temporal values: index
  Copyright(c) MobilityDB Contributors

  This documentation is licensed under a Creative Commons Attribution-Share
  Alike 3.0 License: https://creativecommons.org/licenses/by-sa/3.0/
-->

# Recurring temporal values: a template, its origins, and a table function

[The implementation plan](IMPLEMENTATION-PLAN.md)

A recurring temporal value is an ordinary temporal value, the *template*, together with the finite
sorted set of instants at which it starts again, its *origins*. No type represents it: the
template keeps its own type (`tgeompoint`, `tint`, any temporal type), the origins are a
`tstzset`, and the two are stored as columns of a row or as a template relation joined to a
relation of origins. Occurrences are produced on demand by a set-returning function, the
counterpart for temporal values of `generate_series()`, which emits only the occurrences that meet
a query window, as ordinary rows that every existing function and operator accepts.

The work derives from the master's thesis of S. Swirydowicz, *Manipulation of GTFS Periodic
Trajectories in MobilityDB* (Université libre de Bruxelles, 2024,
<https://github.com/sswirydo/Master-Thesis>), which extended MobilityDB with the periodic types
`pint` and `pgeompoint` and a `pmode` type (period, repetitions, anchor), and named the copy of
the temporal hierarchy they require as the obstacle to carrying them further.

**THE RULING (the owner, 2026-11-26).** The periodic types of the thesis require a duplicate type
hierarchy that cannot be afforded; recurrence is served by a table function parallel to
`generate_series()`. No periodic type, no recurrence flag inside `Temporal`, no wrapper type with
an input prefix, and no recurrence-aware overload of the existing functions.

## The blueprint

The plan works towards the implementation the article (Part 0 of the plan) defines as the one
that fulfills its objective. Its elements:

| Element | Statement |
|---|---|
| Recurring value | `(T, r, S)`: template `T` of any temporal type, reference origin `r`, origins `S` a `tstzset`; occurrence at `s` is `T` shifted by the elapsed duration `s - r` |
| Service dates | A `dateset` `D` and a time zone `z`; origin of a date `d` is local noon of `d` in `z` minus 12 hours (the GTFS definition), which is correct across daylight-saving changes and for stop times beyond 24:00 |
| Normalization | Outside MEOS: an adapter resolves a source (GTFS, NeTEx, iCalendar) into `D = (B ∪ A) − X` with the existing `dateset` operators `+`, `-` and the `setUnion` aggregate |
| Template-level operations | Shift-invariant operations (trajectory, length, value set, ever and always predicates against a static value) give one answer for all occurrences; shift-equivariant ones (speed, distance to a static geometry, restriction to a geometry) give a new template over the same origins |
| Shared origins | Two recurring values with the same reference origin, whose occurrences at distinct origins do not overlap, interact through their templates: `g(P1, P2) = (g(T1, T2), r, S1 ∩ S2)` |
| Candidate selection | The origins whose occurrence meets `[wl, wu]` are exactly `S ∩ [wl - umax, wu - umin]`, with `[umin, umax]` the support of `T` relative to `r`: two binary searches |
| Indexing | Templates by their `stbox` (GiST, SP-GiST); origins by the GiST class of `tstzset` or by a relation `(template_id, origin)` with a B-tree |
| Planner | A planner support function giving the row count of `occurrences`, as `generate_series` has |

## Where each part lives

| Part | Home |
|---|---|
| MEOS functions | `meos/src/temporal/`, declared in `meos/include/meos.h`, in the MEOS catalog, so every binding generates them |
| SQL surface | `mobilitydb/sql/temporal/`, `mobilitydb/src/temporal/` |
| GTFS adapter | The GTFS chapter of the MobilityDB workshop, as SQL over the loaded feed |
| Article | Outside this repository; the PDF built from it is the reference the plan follows |
