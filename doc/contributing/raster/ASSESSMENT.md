<!--
  MobilityDB — Rasters in every engine: assessment
  Copyright(c) MobilityDB Contributors

  This documentation is licensed under a Creative Commons Attribution-Share
  Alike 3.0 License: https://creativecommons.org/licenses/by-sa/3.0/
-->

# Rasters in every engine: the assessment

[Back to the index](00-INDEX.md) · [Implementation plan](IMPLEMENTATION-PLAN.md)

## The two types

| | `Raster` | `Raquet` |
|---|---|---|
| What it is | PostGIS's raster: bands over an affine grid | one tile of CARTO's Raquet format: one band over a QUADBIN cell |
| Reference system | any, its SRID in the value | Web Mercator, fixed by the QUADBIN grid |
| Geotransform | in the value (upper left, scale, skew) | derived from the QUADBIN cell |
| Storage beyond one value | a table of tiles, as PostGIS stores rasters | a Parquet file of Raquet rows, read by any engine that reads Parquet |
| Algorithms | the PostGIS raster core, vendored in MEOS | the QUADBIN arithmetic of MEOS |
| GDAL | for formats other than WKB | to read a file into a tile (`raquetRead`) only |
| Catalog slot | none | `T_RAQUET` |
| SQL type | PostGIS's `raster` (the extension requires `postgis_raster`) | MobilityDB's `raquet` |

The two are not alternatives: a `Raster` is the raster a PostGIS user holds, in its own system and
grid; a `Raquet` is the raster a cloud warehouse holds, tiled for the scan. MEOS answers the same
questions over both.

## MEOS and MobilityDB

`meos/include/meos_raster.h` declares 122 functions, and `mobilitydb/sql/raster/500_raster.in.sql`
139 SQL functions over them and 7 aggregates:

- **Input and output.** `asBinary`, `asHexWKB`, `rasterFromBinary`, `rasterFromHexWKB`,
  `raquetFromBinary`, `raquetFromHexWKB`, the `raquet` text form, and `raquetRead`, which reads
  any GDAL format into a tile, deriving its QUADBIN cell from the geotransform.
- **Accessors.** `width`, `height`, `numBands`, `SRID`, `upperLeftX`, `upperLeftY`, `scaleX`,
  `scaleY`, `skewX`, `skewY`, `bandPixelType`, `bandHasNoDataValue`, `bandNoDataValue`,
  `pixels`, `quadbin`, `stbox`.
- **Processing, as PostGIS names it.** `clip`, `transform`, `transformPipeline`, `rescale`,
  `reclass`, `setValues`, `mapAlgebra`, `slope`, `tpi`, `tri`, `dumpAsPolygons`, `dumpValues`,
  `pixelAsPoints`, `summaryStats`, `valueCount`, `valuePercent`, the arithmetic operators, and the
  aggregates `rasterUnion`, `summaryStatsAgg`, `sameAlignment`.
- **Along a trajectory.** `rasterValue(tgeompoint, raster, band)` and
  `rasterTileValue(tgeompoint, raquet)` → `tfloat`, the value of the pixel the trajectory is in,
  its crossings of pixel bounds dated exactly (the pixel walk, judged by CGAL on 1507 trips); and
  `atRasterValue`, `minusRasterValue` → `tgeompoint`, `eRasterValue`, `aRasterValue` → `boolean`.
  Each of the five has an overload over a raster file on the server, named by its path
  (`raster_value_gdal` and its restrictions in MEOS), and `rasterTileValue` one over an array of
  tiles.

**Gaps.**

| Gap | Row |
|---|---|
| `Raster` has no `MeosType` slot and no `MEOS_TYPE_NAMES` entry, so no binding or server reads its names from the catalog of types | R1 |
| The restrictions `atRasterValue`, `minusRasterValue`, `eRasterValue` and `aRasterValue` exist over a `Raster` and a raster file, not over a `Raquet` tile or an array of tiles | R2 |
| No writer of a `Raster` to GeoTIFF or another GDAL format (`ST_AsGDALRaster`, `ST_AsTIFF` in PostGIS), and no reader of a `Raster` from the bytes of such a file (`ST_FromGDALRaster`): GDAL reads a file into a `Raquet` tile only (`raquetRead`) | R3 |
| No writer to CoverageJSON, the encoding of OGC API – Coverages and EDR | R4 |
| No reader and writer of a Raquet Parquet file as a set of tiles | R5 |

## The catalog and the bindings

The MEOS-API catalog states `Raster` and `Raquet` in `typeEncodings`, each exchanged as WKB and
hexadecimal WKB (`raster_from_wkb`, `raster_as_wkb`, `raquet_from_wkb`, `raquet_as_wkb`), and
the 122 public functions of `meos_raster.h` in the family `RASTER`, 113 of them carrying the SQL
name they answer to (the nine others are the copy, size, free and pixel type helpers) and 82 of
them exposable over the network. A binding generated from the catalog therefore can carry the
raster surface with no raster code of its own; how much of it each carries is not measured. R6
measures it.

## MobilityAPI

MobilityAPI publishes moving features through OGC API – Moving Features, their temporal geometry
in any spatial temporal type of MEOS. It publishes no raster. The OGC API standards that fit:

| Standard | Status | What it serves | Fit |
|---|---|---|---|
| OGC API – Coverages, Part 1: Core | a draft of the Coverages Standards Working Group (OGC 19-087), not yet an approved standard | a coverage as a resource, its domain set and range type, subset by `bbox`, `datetime` and `properties` (bands), its encoding negotiated | the raster itself: a collection whose items are `Raster` or `Raquet` tiles |
| OGC API – Environmental Data Retrieval (EDR), Part 1 | approved standard, version 1.2 (OGC 19-086r9) | queries of a coverage at a position, in an area, a cube, along a trajectory or a corridor, answered in CoverageJSON among others | the queries along a path: `/trajectory` is the value along a path that `rasterValue` computes |
| OGC API – Moving Features, Part 1: Core | version 1.0 (OGC 22-003r3), the standard MobilityAPI implements | temporal properties of a moving feature | the value of a raster along a moving feature's own trajectory, a `tfloat` temporal property |
| OGC API – Tiles, Part 1 | not checked for this plan | a raster cut in tiles of a tile matrix set | a later choice; a Raquet file is already tiled in the QUADBIN grid, which is Web Mercator's |

Raquet is CARTO's specification, not an OGC standard: MobilityAPI offers it as an encoding of a
coverage (a Parquet response, as it already reads GeoParquet in its bulk ingest), not as a
conformance class. CoverageJSON is an OGC community standard, version 1.0 (OGC 21-069r2).

Coverages Part 1 being a draft, MobilityAPI implements the draft its working group publishes
(`opengeospatial/ogcapi-coverages`) and records the draft it conforms to; the queries along a
path, which EDR states in an approved standard, do not wait for it.

The trajectory of EDR is a static path, a `LINESTRING` whose M ordinate is the time; a moving
feature on the server is a richer path, and its temporal property is the query only MobilityAPI
answers. The two compose: EDR's `/trajectory` builds a `tgeompoint` from the coordinates it
receives and answers `rasterValue` over it, and the temporal property answers it over the
feature's own trajectory.

**Questions the plan settles.**

- A raster that changes in time (a series of rasters, one per instant or interval) is a coverage
  with a temporal axis; MobilityDB holds it as a table of rasters with a time column, and the
  value along a trajectory then reads, at each instant, the raster of that instant. MEOS states
  it as overloads over the rows of the series, an array of rasters and an array of their
  validity spans, and no temporal raster type holds it (R11).
- CoverageJSON is written by MEOS (R4) rather than by the server, by rule 4 of the index.
