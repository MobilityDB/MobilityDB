<!--
  MobilityDB — Rasters in every engine: index
  Copyright(c) MobilityDB Contributors

  This documentation is licensed under a Creative Commons Attribution-Share
  Alike 3.0 License: https://creativecommons.org/licenses/by-sa/3.0/
-->

# Rasters in every engine

MEOS holds two raster types. `Raster` is the raster of PostGIS: any number of bands over an
affine grid in any spatial reference system, read and written as the WKB of PostGIS, its
algorithms those of the PostGIS raster core that MEOS vendors (`meos/src/raster/raster_rtcore.c`).
`Raquet` is one tile of the Raquet format of CARTO: one band over the cell of a QUADBIN grid in
Web Mercator, its geotransform derived from the cell, so that a Parquet file of Raquet rows is a
raster that any engine reading Parquet reads without GDAL. Both relate to a moving object in one
way that neither PostGIS nor CARTO states: the value of the raster along a trajectory, a temporal
float whose pieces are the pixels the trajectory walks (`rasterValue`, `rasterTileValue`), and the
restriction of the trajectory to where the raster takes some values (`atRasterValue`,
`minusRasterValue`, `eRasterValue`, `aRasterValue`).

These notes state, for every raster function, where it is in MEOS, in MobilityDB, in the MEOS-API
catalog, in each binding, in MobilityDuck and in MobilityAPI, what is missing at each layer, the
OGC API standards through which MobilityAPI publishes rasters, and the pull requests that close
each gap.

The names follow the portable naming notes of MobilityDB #2861 (`doc/contributing/portable_naming/`);
reading and writing every type in every format follows the input and output notes of MobilityDB
#3353 (`doc/contributing/io/`); the exactness of the pixel walk is the exactness plan of
MobilityDB #3287 (`doc/contributing/exactness/`) and the cell walks of the manual.

## The notes

| Note | What it states |
|---|---|
| [Assessment](ASSESSMENT.md) | The two types and what each answers, the raster surface at each layer (MEOS, the SQL functions, the catalog, the bindings, MobilityDuck, MobilityAPI), the gaps, and the OGC API standards that fit |
| [Implementation plan](IMPLEMENTATION-PLAN.md) | The pull request that closes each gap, in MEOS and MobilityDB, the catalog, each binding, MobilityDuck and MobilityAPI, and its state |
| [Rasters specification](../../specs/meos-rasters.md) | The user document of the campaign: what MEOS and MobilityDB answer over rasters, each pull request of the campaign raising its version with the part of the interface master gains |

## The rules

1. **MEOS is the one implementation.** Every raster algorithm, reader and writer lives in MEOS,
   so that MobilityDB, every binding, MobilityDuck and MobilityAPI answer alike; the SQL
   function is a wrapper over it, and a binding or a server holds no raster code of its own.
2. **Each type has its catalog slot.** As PostGIS's `geometry` has `T_GEOMETRY` and
   pgpointcloud's `pcpatch` has `T_PCPATCH`, PostGIS's `raster` has `T_RASTER`, the next free
   value of `MeosType`, and its names in `MEOS_TYPE_NAMES`: the SQL type `raster`, the binding
   class `Raster`. A slot changes no encoding: the WKB of a `raster` stays PostGIS's.
3. **A value travels as its WKB.** `Raster` as PostGIS's raster WKB, `Raquet` as the WKB of
   `T_RAQUET`, through `asBinary` and `<type>FromBinary` and their hexadecimal forms, as rule 6
   of the portable dialect states for every type.
4. **Formats are readers and writers of MEOS.** GeoTIFF and the other GDAL formats, Raquet as
   Parquet rows, and CoverageJSON are each a `<type>From<Format>` and an `as<Format>` of MEOS,
   GDAL being the library MEOS reads and writes them through, as the input and output notes
   require of every format; no server or binding encodes a raster itself.
5. **MobilityAPI publishes rasters through OGC API standards.** OGC API – Coverages Part 1, a
   draft (OGC 19-087) whose version MobilityAPI records, for the raster as a resource, its
   encodings negotiated (GeoTIFF, CoverageJSON, Raquet); OGC API – Environmental Data Retrieval
   (EDR) Part 1, version 1.2, for the queries along a path (`/position`, `/area`, `/trajectory`,
   `/corridor`); and the temporal properties of OGC API – Moving Features Part 1 for the value
   of a raster along a moving feature's own trajectory, which only MobilityAPI can answer, the
   moving feature being on the server.
6. **What MobilityAPI answers about a raster is what MobilityDB declares over it.** As for the
   temporal types (`catalog_gen.go`), the functions per raster type are generated from the
   catalog, and a query MobilityDB states no function for is refused naming the function.

## The user document and the close of the campaign

`doc/specs/meos-rasters.md` (MobilityDB #3777) is what a user reads: it states only what master
answers, and its Future list the parts this plan has decided. Each pull request of the campaign
changes it in the same pull request, moving its part from the Future list into the body and
raising the minor version. When every row of the plan is merged, the specification gains a
section stating why the interface is what it is, the insights of these notes a user needs, and
this plan pull request closes unmerged, its last comment naming the version of the specification
that carries them.

## Acceptance

The raster tests of MobilityDB (`mobilitydb/test/raster`) and MEOS (`meos/test/raster*_test.c`)
are the oracle of every binding and engine; the pixel walk is judged by the CGAL judge of the
cell walks. A binding or engine supports a raster function when it answers the oracle's cases
alike; MobilityAPI supports an endpoint when its conformance tests answer the abstract tests of
the standard's conformance classes against the fixture.

## Where things stand

- MEOS and MobilityDB hold both types and their functions (122 public MEOS functions in
  `meos_raster.h`, 139 SQL functions in `mobilitydb/sql/raster/500_raster.in.sql`), the value
  along a trajectory among them, its pixel walk judged exact.
- The catalog exchanges `Raster` and `Raquet` as WKB, `Raster` in its slot `T_RASTER` (R1,
  MobilityDB #3780).
- No raster writer to GeoTIFF or CoverageJSON exists in MEOS (R3, R4).
- The restrictions read the values of a `Raquet` tile and an array of tiles (R2, MobilityDB
  #3781); one row of a Raquet file is read into a tile (R5, #3782), and a whole Raquet
  Parquet file is read and written by no MEOS function.
- `tools/raster_matrix.py` measures the raster surface each binding and MobilityDuck carries
  (R6, MobilityDB #3776).
- MobilityAPI publishes no raster (R7 to R10).
