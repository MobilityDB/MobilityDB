<!--
  MobilityDB — Rasters in every engine: implementation plan
  Copyright(c) MobilityDB Contributors

  This documentation is licensed under a Creative Commons Attribution-Share
  Alike 3.0 License: https://creativecommons.org/licenses/by-sa/3.0/
-->

# Rasters in every engine: the implementation plan

[Back to the index](00-INDEX.md) · [Assessment](ASSESSMENT.md)

Each gap of [the assessment](ASSESSMENT.md) and the pull request that closes it, in order. A row
names the layer it changes, and its State column names the pull request that closes it. The names a row proposes are proposals until the portable naming notes of
MobilityDB #2861 state them; a format's reader and writer follow the input and output notes of
MobilityDB #3353.

## MEOS and MobilityDB

| Gap | Task | State |
|---|---|---|
| `Raster` has no `MeosType` slot | R1: `T_RASTER = 88`, the next free value after `T_TS2CELL = 87`, and its `MEOS_TYPE_NAMES` entry `{ "raster", "Raster" }`, as `T_GEOMETRY` and `T_PCPATCH` hold the slots of PostGIS's and pgpointcloud's types; the catalog then reads `Raster` as a type of its own. The WKB of a `raster` stays PostGIS's: the slot changes no encoding, and no WKB of another type embeds it | open |
| The restrictions exist over a `Raster` and a raster file, not over a `Raquet` tile | R2: `atRasterValue`, `minusRasterValue` → `tgeompoint` and `eRasterValue`, `aRasterValue` → `boolean` over a `raquet` and a `raquet[]`, in MEOS over the pixel walk of the tiles that `rasterTileValue` already walks (MobilityDB #3743), so the restriction is dated at the crossings the walk dates | open |
| No GDAL reader or writer of a `Raster` | R3: a writer of a `Raster` to any GDAL format and to GeoTIFF, and a reader of a `Raster` from the bytes of a GDAL file, in MEOS through GDAL, as PostGIS spells them (`ST_AsGDALRaster(raster, format, options, srid)`, `ST_AsTIFF`, `ST_FromGDALRaster(bytea, srid)`) without the `ST_` prefix in SQL; the writer of a `Raquet` tile follows from it, a tile being a raster of one band over its cell | open |
| No CoverageJSON writer | R4: `asCoverageJSON` of a `Raster` and a `Raquet` (a `Grid` coverage, one parameter per band, its axes from the geotransform, its reference system from the SRID) and of the value along a trajectory (a `tfloat` over the points of a `tgeompoint`, a `Trajectory` coverage), written by MEOS, so that MobilityAPI serves the EDR and Coverages encodings without encoding a raster itself | open |
| No reader or writer of a Raquet Parquet file | R5: the reader of a Raquet file as the set of its tiles and its metadata row, and the writer of a set of tiles as a Raquet file, in MEOS, for the engines and bindings with no Parquet reader of their own; an engine that reads Parquet (DuckDB, Spark) scans the rows itself and builds each tile through `raquet_make`, the tile still a MEOS value | open |
| A raster that changes in time | R11: a raster series is a relation of rows `(validtime tstzspan, rast raster)`, never a temporal raster type, since a value `[r1@t1, ..., rn@tn]` carries a whole raster per instant and nothing in it interpolates. MEOS reads a trajectory along a series through overloads of the names that read one raster: `rasterValue(tgeompoint, raster[], tstzspan[], band)` and `rasterTileValue(tgeompoint, raquet[], tstzspan[])` → `tfloat`, and the same overloads of `atRasterValue`, `minusRasterValue`, `eRasterValue` and `aRasterValue`. Each raster is read over the part of the trajectory within its span, in one ordered walk, as `rasterTileValue(tgeompoint, raquet[])` walks the tiles side by side in space. A part of the trajectory within no span has no value, as a nodata pixel has none; spans that overlap are refused with an error. A query passes the rows that meet the trip, selected by the indexes on `validtime` and `stbox(rast)`, gathered in `validtime` order; every engine answers it through the same MEOS function. In MobilityAPI the `validtime` column is the temporal axis of a raster collection | DECIDED by the owner 2026-11-17 |

## The catalog and the bindings

| Gap | Task | State |
|---|---|---|
| How much of the raster surface each binding carries is not measured | R6: a matrix of every public raster function (122 in `meos_raster.h` at `247d7da164`) against each binding's generated surface at its pinned commit, built by a tool that reads the catalog and the binding's object layer as `tools/io_matrix.py --repo` does for the readers and writers; `--check` diffs it against the table it writes into this note. A binding's name is matched through the catalog's `altSqlName` as well as its SQL name, so `sameAlignmentAgg`, the name DuckDB, Spark and Flink give the aggregate MobilityDB calls `sameAlignment`, counts as that aggregate | open |
| The gaps R6 finds | One row per binding and gap, numbered from R12, each closed in the binding's generator from the catalog, never by a hand-written function, as the bindings plan of MobilityDB #2861 requires | after R6 |

The catalog needs no change of its own beyond R1: `typeEncodings` already exchanges `Raster` and
`Raquet` as WKB and hexadecimal WKB, and every new MEOS function of R2 to R5 reaches it by the
regeneration that follows its merge.

## MobilityDuck

MobilityDuck adds to the surface of DuckDB's own extensions and replaces none of them. Its raster
functions are those of the catalog that R6 measures, registered by its generators; its row is the
row R6 writes for it, and a gap found there is a row from R12 like any binding's. A Raquet file is
read by DuckDB's Parquet scan, each row built into a tile by `raquet_make` (R5).

## MobilityAPI

Each row adds one conformance class or encoding, its endpoints answering through the MEOS function
the row names, its tests the abstract tests of that class against the fixture of the raster tests.

| Gap | Task | State |
|---|---|---|
| MobilityAPI publishes no raster | R7: a raster collection as a coverage of OGC API – Coverages Part 1 (the draft OGC 19-087, its version recorded in the conformance declaration): `/collections/{collectionId}/coverage` with its domain set and range type, subset by `bbox`, `datetime` and `properties` (the bands, through `clip` and the band accessors), its encoding negotiated among GeoTIFF (R3), CoverageJSON (R4) and Raquet (R5); the items `Raster` or `Raquet` tiles | after R3, R4, R5 |
| No query of a raster at a place or along a path | R8: the queries of OGC API – EDR Part 1, version 1.2, over a raster collection: `/position` (the pixel value at a point and a time), `/area` (the raster clipped to a polygon), `/trajectory` (the `LINESTRING` received, its M ordinate the time, built into a `tgeompoint` and answered by `rasterValue` or `rasterTileValue`), `/corridor` (the raster clipped to the buffer of the path); answered in CoverageJSON (R4) | after R4 |
| No value of a raster along a moving feature | R9: a temporal property of OGC API – Moving Features Part 1 that is the value of a raster collection along the feature's own trajectory, a `tfloat` answered by `rasterValue` on the server, and its restriction to a range of values through `atRasterValue` | after R7 |
| What a raster collection answers is written by hand | R10: the functions a raster collection answers, generated from the catalog as `catalog_gen.go` generates them for the temporal types (MobilityAPI #76, MERGED), a query MobilityDB states no function for refused naming that function | with R7 |

## Order

R1 first, as the slot every generated layer reads; R2 to R5 in MEOS and MobilityDB, each its own
pull request; R6 once R1 is in the catalog, so that it measures `Raster` as a type; the binding
rows R6 finds; then MobilityAPI, R10 with R7, R8, and R9. R11 comes before R7
declares a temporal axis, the axis being the series' `validtime`.
