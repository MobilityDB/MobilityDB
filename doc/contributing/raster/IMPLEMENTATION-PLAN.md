<!--
  MobilityDB — Rasters in every engine: implementation plan
  Copyright(c) MobilityDB Contributors

  This documentation is licensed under a Creative Commons Attribution-Share
  Alike 3.0 License: https://creativecommons.org/licenses/by-sa/3.0/
-->

# Rasters in every engine: the implementation plan

[Back to the index](00-INDEX.md) · [Assessment](ASSESSMENT.md)

Each gap of [the assessment](ASSESSMENT.md) and the pull request that closes it, in order. A row
names the layer it changes, and its State column names the pull request that closes it. The names
a row proposes are proposals until the portable naming notes of MobilityDB #2861 state them; a
format's reader and writer follow the input and output notes of MobilityDB #3353.

## MEOS and MobilityDB

| Gap | Task | State |
|---|---|---|
| `Raster` has no `MeosType` slot | R1: `T_RASTER = 88`, the next free value after `T_TS2CELL = 87`, and its `MEOS_TYPE_NAMES` entry `{ "raster", "Raster" }`, as `T_GEOMETRY` and `T_PCPATCH` hold the slots of PostGIS's and pgpointcloud's types; the catalog then reads `Raster` as a type of its own. The WKB of a `raster` stays PostGIS's: the slot changes no encoding, and no WKB of another type embeds it | MERGED as MobilityDB #3780 |
| The restrictions exist over a `Raster` and a raster file, not over a `Raquet` tile | R2: `atRasterValue`, `minusRasterValue` → `tgeompoint` and `eRasterValue`, `aRasterValue` → `boolean` over a `raquet` and a `raquet[]`, in MEOS over the pixel walk of the tiles that `rasterTileValue` already walks (MobilityDB #3743), so the restriction is dated at the crossings the walk dates | MERGED as MobilityDB #3781 |
| No GDAL reader or writer of a `Raster` | R3: the writers `asGDALRaster(raster, format, options, srid)`, `asTIFF`, `asPNG` and `asJPEG`, as PostGIS's `ST_AsGDALRaster`, `ST_AsTIFF`, `ST_AsPNG` and `ST_AsJPEG` write, and the reader `rasterFromGDALRaster(bytea, srid)`, as PostGIS's `ST_FromGDALRaster` reads, named `<type>From<Format>` as `rasterFromBinary` names PostGIS's `ST_RastFromWKB`; each a MEOS function through GDAL (`raster_as_gdal`, `raster_as_tiff`, `raster_as_png`, `raster_as_jpeg`, `raster_from_gdal`). MEOS takes GDAL's creation options as one string of `KEY=VALUE` pairs, split by GDAL's own tokenizer, so no binding marshals an array of strings; the SQL keeps PostGIS's `options text[]`, which the wrapper joins | DECIDED by the owner 2026-11-17; MERGED as MobilityDB #3783 |
| No CoverageJSON writer | R4: `asCoverageJSON` of a `Raster` and a `Raquet`, a CoverageJSON `Grid` coverage, one parameter per band, its axes from the geotransform, its reference system from the SRID, a nodata pixel written `null` (`raster_as_coveragejson`, `raquet_as_coveragejson`); and of the values along a trajectory, `asCoverageJSON(tfloat, tgeompoint)` (`tfloat_as_coveragejson(const Temporal *temp, const Temporal *traj)`), a `Trajectory` coverage whose position at each instant of the `tfloat` is the trajectory's at that instant, so that MobilityAPI serves the EDR and Coverages encodings without encoding a raster itself | DECIDED by the owner 2026-11-17; MERGED as MobilityDB #3786 |
| No reader or writer of a Raquet Parquet file | R5: a MEOS decoder of one Raquet row (its cell, its band bytes, the file's metadata with the band's compression) into a tile, `raquetFromRow`, for every engine scanning the Parquet file itself (DuckDB, Spark, NebulaStream), so that every engine decodes a tile alike. The file itself is read and written by the engine's own Parquet reader, never by MEOS: GDAL's Parquet driver is Apache Arrow C++, GDAL built without Arrow carries no such driver (Debian's and Ubuntu's GDAL among them), and MEOS links no Arrow. In PostgreSQL a file is loaded as any Parquet file is, then decoded row by row | DECIDED by the owner 2026-11-17; the reader and writer of a whole file in MEOS RULED OUT by the owner 2026-11-18, and its item leaves the Future list of `doc/specs/meos-rasters.md` in the closing pull request; MERGED as MobilityDB #3782 |
| A raster that changes in time | R11: a raster series is a relation of rows `(validtime tstzspan, rast raster)`, never a temporal raster type, since a value `[r1@t1, ..., rn@tn]` carries a whole raster per instant and nothing in it interpolates. MEOS reads a trajectory along a series through overloads of the names that read one raster: `rasterValue(tgeompoint, raster[], tstzspan[], band)` and `rasterTileValue(tgeompoint, raquet[], tstzspan[])` → `tfloat`, and the same overloads of `atRasterValue`, `minusRasterValue`, `eRasterValue` and `aRasterValue`. Each raster is read over the part of the trajectory within its span, in one ordered walk, as `rasterTileValue(tgeompoint, raquet[])` walks the tiles side by side in space. A part of the trajectory within no span has no value, as a nodata pixel has none; spans that overlap are refused with an error. A query passes the rows that meet the trip, selected by the indexes on `validtime` and `stbox(rast)`, gathered in `validtime` order; every engine answers it through the same MEOS function. In MobilityAPI the `validtime` column is the temporal axis of a raster collection | DECIDED by the owner 2026-11-17; rows sharing one period read together as tiles, a tile given twice refused where its values differ (owner 2026-11-17); MERGED as MobilityDB #3792 |

## The catalog and the bindings

| Gap | Task | State |
|---|---|---|
| How much of the raster surface each binding carries is not measured | R6: `tools/raster_matrix.py`, which reads the raster functions a binding calls from the catalog (the public functions of the family `RASTER` carrying a SQL name, less the support functions of a type and of an aggregate, an aggregate counted once under its name) and each binding's object layer as `tools/io_matrix.py --repo` reads it, and credits a function when the layer names it (its SQL name, its `altSqlName` or its C name) and carries every raster type its signatures name; `--check` compares its rows with the table below | MERGED as MobilityDB #3776 |
| The gaps R6 finds | One row per binding and gap, numbered from R12, each closed in the binding's generator from the catalog, never by a hand-written function, as the bindings plan of MobilityDB #2861 requires | after R6 |

### The raster surface of each binding

Built by `tools/raster_matrix.py` (R6) over the MEOS-API catalog at MobilityDB `247d7da164`, each
binding at the commit its row names, its default branch; 71 functions, of which 7 name no raster
type (`rasterTileValueQuadbin` and the `summarystats*` accessors):

| Binding | Commit | raster | raquet | Functions | Missing |
|---|---|---|---|---|---|
| JMEOS | `f456f24e44` | **no** | **no** | 0 of 71 | `SRID`, `aRasterValue`, `addBand`, `asBinary`, `asHexWKB`, ... |
| GoMEOS | `9551b85318` | yes | yes | 57 of 71 | `aRasterValue`, `eRasterValue`, `rasterAdd`, `rasterDiv`, `rasterMul`, ... |
| MEOS.NET | `359675c41e` | yes | yes | 46 of 71 | `aRasterValue`, `asBinary`, `atRasterValue`, `eRasterValue`, `makeEmptyRaster`, ... |
| meos-rs | `2b49f27223` | **no** | **no** | 0 of 71 | `SRID`, `aRasterValue`, `addBand`, `asBinary`, `asHexWKB`, ... |
| MEOS.js | `bbda83df38` | yes | yes | 48 of 71 | `aRasterValue`, `atRasterValue`, `eRasterValue`, `makeEmptyRaster`, `minusRasterValue`, ... |
| MobilityDuck | `e47d67c413` | yes | yes | 70 of 71 | `setValues` |
| MobilityNebula | `5c0974e0a0` | **no** | **no** | 1 of 71 | `SRID`, `aRasterValue`, `addBand`, `asBinary`, `asHexWKB`, ... |
| PyMEOS | `33717764b7` | **no** | **no** | 0 of 71 | `SRID`, `aRasterValue`, `addBand`, `asBinary`, `asHexWKB`, ... |
| PyMEOS-CFFI | `7d41acfbd7` | **no** | **no** | 7 of 71 | `SRID`, `aRasterValue`, `addBand`, `asBinary`, `asHexWKB`, ... |
| MobilityAPI | `3305f77395` | **no** | **no** | 0 of 71 | `SRID`, `aRasterValue`, `addBand`, `asBinary`, `asHexWKB`, ... |

JMEOS, MobilitySpark, MobilityFlink and MobilityKafka generate their surface at build time
(`jmeos-core/target/generated-sources`), so their rows read the committed layer only until they
are rebuilt from a built checkout. The roots are those of the bindings' table of the input and
output notes:

```
raster_matrix.py --catalog meos-idl.json \
  --repo JMEOS=<JMEOS>:jmeos-core/src/main/java/types --repo GoMEOS=<GoMEOS>:types \
  --repo MEOS.NET=<MEOS.NET>:MEOS.NET/Types --repo meos-rs=<meos-rs>:src \
  --repo MEOS.js=<MEOS.js>:core/types,core/objects --repo MobilityDuck=<MobilityDuck>:src \
  --repo MobilityNebula=<MobilityNebula>:nes-plugins/MEOS --repo PyMEOS=<PyMEOS>:pymeos \
  --repo PyMEOS-CFFI=<PyMEOS-CFFI>:pymeos_cffi --repo MobilityAPI=<MobilityAPI>:. \
  [--check IMPLEMENTATION-PLAN.md]
```

The catalog needs no change of its own beyond R1: `typeEncodings` already exchanges `Raster` and
`Raquet` as WKB and hexadecimal WKB, and every new MEOS function of R2 to R5 reaches it by the
regeneration that follows its merge.

## MobilityDuck

MobilityDuck adds to the surface of DuckDB's own extensions and replaces none of them. Its raster
functions are those of the catalog that R6 measures, registered by its generators; its row is the
row R6 writes for it, and a gap found there is a row from R12 like any binding's. A Raquet file is
read by DuckDB's Parquet scan, each row decoded into a tile by `raquet_from_row` (R5).

## MobilityAPI

Each row adds one conformance class or encoding, its endpoints answering through the MEOS function
the row names, its tests the abstract tests of that class against the fixture of the raster tests.

| Gap | Task | State |
|---|---|---|
| MobilityAPI publishes no raster | R7: a raster collection as a coverage of OGC API – Coverages Part 1 (the draft OGC 19-087, its version recorded in the conformance declaration): `/collections/{collectionId}/coverage` with its domain set and range type, subset by `bbox`, `datetime` and `properties` (the bands, through `clip` and the band accessors), its encoding negotiated among GeoTIFF (R3), CoverageJSON (R4) and Raquet (R5); the items `Raster` or `Raquet` tiles | a coverage is one period: a `datetime` selecting one answers it, none answers the latest, one meeting several is refused naming them; encoded in GeoTIFF and CoverageJSON alone, the draft naming no Raquet media type and MEOS writing no Raquet file (R5) (owner 2026-11-18); MERGED as MobilityAPI #79 |
| No query of a raster at a place or along a path | R8: the queries of OGC API – EDR Part 1, version 1.1 (OGC 19-086r6), over a raster collection: `/position` (the pixel value at a point, at an instant a CoverageJSON `Point`, over a span a `PointSeries`), `/area` (the raster clipped to a polygon), `/trajectory` (the `LINESTRING` received, its M ordinate the time, built into a `tgeompoint` and answered by `rasterValue` or `rasterTileValue`), `/corridor` (the raster clipped to the buffer of the path); answered in CoverageJSON (R4) | EDR 1.1, the published version; `/position` written as a `Point` or `PointSeries` by MEOS (MobilityDB #3797, MERGED) and read across the periods of a series once a step trajectory keeps its bounds (MobilityDB #3798, MERGED); a corridor applies its width and states that its height has no axis (owner 2026-11-18); MERGED as MobilityAPI #80 |
| No value of a raster along a moving feature | R9: a TemporalGeometryQuery `/collections/{cid}/items/{fid}/tgsequence/{tgid}/raster?collection=..&band=..` beside distance and velocity, a MobilityAPI query type: the values of a raster collection along the feature's own trajectory, a `TReal` answered by the series reader (`rasterValue` or `rasterTileValue`) over the periods the trajectory meets, derived when asked and never stored, and its restriction to `valueRange=lo,hi` through `atSpan` | the owner (2026-11-18): a derived measure, so a query beside velocity, `/tproperties` staying the store of written values |
| What a raster collection answers is written by hand | R10: the functions a raster collection answers, generated from the catalog as `catalog_gen.go` generates them for the temporal types (MobilityAPI #76, MERGED), a query MobilityDB states no function for refused naming that function | a raster collection is registered: it names an existing table, its `validtime tstzspan` column and its `raster` or `raquet` column, read only (owner 2026-11-18); MERGED as MobilityAPI #77 |

## Order

R1 first, as the slot every generated layer reads; R2 to R5 in MEOS and MobilityDB, each its own
pull request; R6 once R1 is in the catalog, so that it measures `Raster` as a type; the binding
rows R6 finds; then MobilityAPI, R10 with R7, R8, and R9. R11 comes before R7
declares a temporal axis, the axis being the series' `validtime`.
