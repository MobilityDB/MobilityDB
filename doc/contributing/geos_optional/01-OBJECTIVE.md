<!--
  MobilityDB — Making GEOS optional: why, and what finished means
  Copyright(c) MobilityDB Contributors

  This documentation is licensed under a Creative Commons Attribution-Share
  Alike 3.0 License: https://creativecommons.org/licenses/by-sa/3.0/
-->

# 1. Why GEOS is being made optional

**Measured at** MobilityDB master `d9b9c11985`.

## 1.1 What GEOS is, and where it sits in MobilityDB

GEOS is a C++ library of planar geometry algorithms: it decides whether two shapes intersect,
computes their intersection, the union of many shapes, the region within a distance of a shape
(the *buffer*), and so on. PostGIS answers most of its geometry functions with it.

MobilityDB has three paths to GEOS, and since MobilityDB #3179 and #3185, merged as
`9bb3bb7a2b` and `1a7a1a0e11`, each ends in MobilityDB's own code:

1. **Through the PostGIS geometry library.** MobilityDB carries a copy of PostGIS's C geometry
   library, *liblwgeom*, in `postgis/liblwgeom/`. Some functions of that library answer by
   calling GEOS underneath. The script `tools/scripts/check_liblwgeom_geos.py` knows which: it
   reads the library and finds **81** such *entry points* — public functions of liblwgeom that a
   caller can call and that end up in GEOS. The files holding them are left out of the build,
   and `meos/src/geo/geo_lwgeom_none.c` answers what the rest of the library calls of them.
2. **Directly.** MEOS converts no geometry to GEOS's format and holds no GEOS context:
   `meos/src/geo/postgis_funcs.c` answers through the native routes alone, and
   `meos/src/temporal/meos.c` creates no per-thread GEOS context.
3. **Through the raster code.** The copy of PostGIS's raster core under `postgis/rt_core/`
   calls GEOS entry points when it turns a raster into polygons, and
   `meos/src/geo/geo_geos_none.c` answers them natively (§1.3).

## 1.2 Why it has to become optional

Three reasons. The first is the one the campaign is named after.

**A real-time operating system cannot carry GEOS.** MEOS, MobilityDB's core library, is meant to
run outside a database as well: in a stream processor, on a vehicle, on a sensor gateway. Many of
those devices run a *real-time operating system* (RTOS) — FreeRTOS, Zephyr, VxWorks and the like —
whose purpose is that every task finishes within a known time, on a processor with little memory.
GEOS is built for the opposite setting, a desktop or server, and it shows in four measurable ways:

- **It is a large C++ library.** GEOS 3.14.1 is about 167,000 lines of C++. On the x86-64 Linux
  system these notes are measured on, `libgeos.so` is 2.5 MB and its C interface `libgeos_c.so`
  another 0.3 MB.
- **It needs the C++ runtime.** `ldd` on `libgeos.so` lists `libstdc++.so.6` (2.7 MB on that
  system) and `libgcc_s.so.1`. Many RTOS toolchains provide no C++ standard library, or provide a
  reduced one, and the processors they target often have a few hundred kilobytes of memory in
  all.
- **It reports errors by throwing C++ exceptions**: `throw` appears at 370 places in 149 of its
  source files, and GEOS has no build option that turns exceptions off. Exception support is one
  of the first things an RTOS build disables, because unwinding the stack takes a time nobody can
  bound in advance, which is exactly what a real-time system must be able to do.
- **It allocates memory freely and invisibly.** Its algorithms build C++ containers and objects
  on the heap at well over a thousand places (`new`, `make_unique`, `std::vector`, `std::map`).
  An RTOS application wants every allocation to go through an allocator it chooses and can
  bound; inside GEOS that is not under the caller's control.

None of this is a defect of GEOS. It means that a MEOS for those targets needs every geometry
answer from code it carries itself.

**A temporal query asks the same spatial question many times.** It asks it once per instant of
a trajectory, and each call to GEOS first converts the geometry to GEOS's own format and the
answer back. On short questions asked millions of times that conversion outweighs the question,
so the project keeps those questions on native code that reads MEOS's geometries as they are.

**GEOS cannot represent a circular arc.** PostGIS geometry types include curves:
`CIRCULARSTRING`, `COMPOUNDCURVE`, `CURVEPOLYGON`. GEOS has none of them. Before it computes
anything, it replaces each arc by a chain of short straight segments — it *polygonizes* the arc —
and it answers about that polygon. MobilityDB cannot accept this, because circles are part of its
own data model: its *temporal circular buffer* type (`tcbuffer`) is a disc whose centre and
radius change over time, and the rounded corners of every buffer are arcs. [Note 2](02-JUDGING-AN-ANSWER.md) measures what the chords cost on a case whose exact answer is
a formula.

## 1.3 How a build without GEOS works

Every build is a build without GEOS: MobilityDB #3185, merged as `1a7a1a0e11`, deletes the
`GEOS` build option, `cmake/FindGEOS.cmake` and the GEOS context with `geos_get_context()`, and a
configure line passing `-DGEOS` is ignored.

- **No GEOS header is read.** The build puts `meos/include/geos-none/geos_c.h` on the include
  path in place of GEOS's own header. That stand-in declares only the few GEOS types and entry
  points MobilityDB still names, so the vendored PostGIS sources compile unmodified, and any
  other GEOS call left behind fails to compile with its name in the error, rather than linking
  quietly and failing at run time.
- **The liblwgeom files that call GEOS are left out**, and the few of their functions the rest of
  liblwgeom still names are defined in `meos/src/geo/geo_lwgeom_none.c`. Its file comment states
  which of them MEOS actually reaches: only `lwgeom_centroid`, which k-means clustering calls,
  and which is answered there by the native `meos_centroid`. The others (`lwgeom_make_valid`,
  `lwgeom_intersection_prec`, `lwgeom_unaryunion`, `lwgeom_split`, `lwgeom_offsetcurve`) are
  reached only from liblwgeom functions MEOS never calls; they exist so the library links, and
  they raise the error saying the operation needs GEOS. In the extension, the archive of the
  vendored geometry library links the MEOS archive in turn, so the linker reads the MEOS archive
  again after it and those calls resolve in a build with no family.
- **The raster core's GEOS calls are answered natively** by `meos/src/geo/geo_geos_none.c`: a
  GEOS geometry *is* a liblwgeom geometry there, so conversion is a copy, and the union the
  polygonization asks for is MEOS's own. Those definitions carry the names of the GEOS C API
  (`GEOSUnaryUnion`, `GEOSisValid`, `initGEOS`, ...), which PostGIS calls in the same backend, and
  PostgreSQL loads every extension library with global symbol resolution. The file is therefore
  an object library of its own whose symbols are hidden from the dynamic table of the shared
  library: exported, they answer the GEOS calls of a PostGIS loaded after MobilityDB, and a
  session calling `ST_IsValid` and `ST_UnaryUnion` aborts the backend; hidden, it answers.
- **`meos_full_version()` and `mobilitydb_full_version()` name no GEOS version**, only those of
  PostgreSQL, PostGIS, PROJ and JSON-C.

One caveat is outside MobilityDB's control: when the raster family is built (`-DRASTER=ON`),
MobilityDB links GDAL, and a system GDAL may itself link GEOS. The GEOS-free `libmeos.so` measured
for these notes links GDAL 3.8.4 (`libgdal.so.34`), and `ldd` shows that library loading
`libgeos_c.so.1` and `libgeos.so.3.12.1`
([`tools/results/libmeos_nogeos_links.txt`](tools/results/libmeos_nogeos_links.txt)). A build that
loads no GEOS at all therefore needs `RASTER=OFF`, or a GDAL built without GEOS.

## 1.4 How much is left, and how that is counted

`python3 tools/scripts/check_liblwgeom_geos.py` prints the count. On master `1a7a1a0e11` it
reads **1 baselined of 81 entry points**, where *baselined* means "still called, and known",
listed in `tools/scripts/liblwgeom_geos_baseline.txt`:

| Entry point still called | Called from |
|---|---|
| `rt_raster_gdal_polygonize` | `meos/src/raster/raster_rtcore.c` |

At master `52b0c9e4d5` it read 4: `lwgeom_intersection_prec`, `lwgeom_difference_prec` and
`lwgeom_unaryunion_prec`, called from `meos/src/geo/postgis_funcs.c`, beside the raster entry.

The script is also a guard in continuous integration: the list may only shrink, and a new call
to any of the 81 entry points fails the check. The count is read from the script, never from a
document — this one included.

The checker counts *places in the code* that call GEOS. It does not say which MEOS functions lead
to those places, nor which inputs get there; [note 5](05-WHAT-STILL-NEEDS-GEOS.md) answers both,
by reading the code and by running it in a build without GEOS.

The three entries of `postgis_funcs.c` are three of the four *fall-backs* of
[note 5](05-WHAT-STILL-NEEDS-GEOS.md): code that runs only when no native code answered. The
fourth fall-back, in the union of an array of geometries, calls GEOS's own C interface rather
than liblwgeom, so this checker does not count it. MobilityDB #3179 deletes all four (note 5
§5.9). The raster entry is not a fall-back: it is answered natively, by the file named above.

## 1.5 What "finished" means

An operation leaves GEOS only when its native answer meets two obligations, both explained in
[note 2](02-JUDGING-AN-ANSWER.md):

1. **It is correct**, as judged by an exact reference on the answers of the proposed change
   itself.
2. **It is at least as fast as GEOS**, measured against GEOS called directly in the same process.

The campaign is finished when every operation meets both, and then one more step is taken, which
the committers decided on 2026-09-22: **GEOS leaves the shipped library entirely.** The `#if GEOS`
fall-backs, the `GEOS` build option and the GEOS context are deleted, by MobilityDB #3179 and
#3185, merged as `9bb3bb7a2b` and `1a7a1a0e11`. The stand-in header and the
two files that answer GEOS's entry points stay, compiled in every build, because the vendored
PostGIS code, its raster core included, calls those entry points. There is then one
implementation, so an answer that differs from PostGIS is a failing test, never
a run-time branch. GEOS stays in the test tools only: as the speed reference, and as one of the
correctness references for straight-edged shapes (note 2 §2.1 says exactly where).

Two related decisions settle the scope:

- **No reduced "profile" of MEOS is built for small devices.** MobilityDB is already built in
  *families*: groups of types switched on or off at configure time (`ALL`, `CBUFFER`, `H3`,
  `JSON`, `NPOINT`, `POINTCLOUD`, `POSE`, `RGEO`, `QUADBIN`, `RASTER`, `S2CELL`), and those
  switches are the subset mechanism. A finer profile is built only when a user asks for one with
  a concrete device and function list.
- **A user-facing chapter of the manual is owed at the end**: an extensive explanation of the
  native engine for a user deciding whether to trust it in place of GEOS, stating where they
  differ — the exact arcs above all — and where MEOS is slower or narrower.
