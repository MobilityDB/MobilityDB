<!--
   ****************************************************************************
    MobilityDB / MEOS Specification — Rasters
    Copyright(c) MobilityDB Contributors

    This documentation is licensed under a Creative Commons Attribution-Share
    Alike 3.0 License: https://creativecommons.org/licenses/by-sa/3.0/
   ****************************************************************************
-->

# Rasters — Interface Specification

| | |
|---|---|
| Version | v0.2.0 |
| Scope | The two raster types of MEOS, how a raster is read and written, the values a trajectory reads from a raster, a Raquet tile or an array of tiles, and the restriction of a trajectory to where those values lie in a range, in SQL, in MEOS and in every binding |
| Reference | `meos/include/meos_raster.h`, `meos/src/raster/raster_rtcore.c` (the PostGIS raster), `meos/src/raster/raquet.c` and `meos/src/raster/raquet_gdal.c` (the Raquet tile), `meos/src/raster/raster_quadbin.c` (the pixel walk), `mobilitydb/sql/raster/500_raster.in.sql`, `mobilitydb/test/raster/` |

## 1. Scope and intent

A raster is a field over a fixed footprint: a value in every pixel of a grid.
What moves is the trajectory, and what MEOS adds to a raster is what a moving
object reads from it, a temporal float whose pieces are the pixels the
trajectory walks. This document states the two raster types of MEOS, how a
raster is read and written, the values a trajectory reads, and the
restrictions of a trajectory to where those values lie, under the same names
in PostgreSQL, in MEOS and in every binding generated from MEOS. It is
prescriptive: the raster tests of MobilityDB and MEOS are its oracle.

A raster that changes in time is a series of rasters, each valid over a
period, held as rows of a table; it is not a temporal type (section 7).

## 2. Notation

`traj` is a `tgeompoint`, `rast` a `raster`, `tile` a `raquet`, `vspan` a
`floatspan`, `band` a band number counted from 1. A trajectory reads a
`raster` in the reference system of the raster, and a trajectory in another
SRID is refused, as every operation of MEOS refuses mixed SRIDs. A trajectory
reads a `raquet` tile by its longitude and latitude, the pixels of the tile
being QUADBIN tiles of a finer zoom.

## 3. The two types

| | `raster` | `raquet` |
|---|---|---|
| What it is | PostGIS's raster: any number of bands over an affine grid | one tile of CARTO's Raquet format: one band over a QUADBIN cell |
| Reference system | any, its SRID in the value | Web Mercator, fixed by the QUADBIN grid |
| Georeferencing | the geotransform the value holds (upper left, scale, skew) | derived from the QUADBIN cell |
| A raster larger than one value | a table of tiles, as PostGIS stores rasters | a Parquet file of Raquet rows, which any engine reading Parquet scans |
| SQL type | PostGIS's `raster`, from `postgis_raster` | MobilityDB's `raquet` |
| Type of the MEOS catalog | `T_RASTER`, named `raster` in SQL and `Raster` in the bindings | `T_RAQUET`, named `raquet` in SQL and `RaQuet` in the bindings |

The algorithms over a `raster` are those of the PostGIS raster core, which
MEOS carries, so a raster is processed in every engine as PostGIS processes
it: `clip`, `transform`, `rescale`, `reclass`, `setValues`, `mapAlgebra`,
`slope`, `tpi`, `tri`, `summaryStats`, `valueCount`, `dumpAsPolygons` and the
others of `500_raster.in.sql`, each the PostGIS function of the same name
without its `ST_` prefix.

## 4. Reading and writing a raster

| Direction | `raster` | `raquet` |
|---|---|---|
| WKB → value | `rasterFromBinary(bytea)`, `rasterFromHexWKB(text)` | `raquetFromBinary(bytea)`, `raquetFromHexWKB(text)` |
| value → WKB | `asBinary(raster)`, `asHexWKB(raster)` | `asBinary(raquet)`, `asHexWKB(raquet)` |
| a raster file → value | | `raquetRead(path text, quadbin)`, `raquetRead(bytea, quadbin)` |

The WKB of a `raster` is PostGIS's raster WKB, so a value written by PostGIS
is read by every engine and the converse. `raquetRead` reads a raster file of
any format GDAL reads, from a path on the server or from the bytes of the
file, into a tile; when no QUADBIN cell is given, the cell is derived from the
geotransform of the file, which must state Web Mercator. The text of a
`raquet` is its hexadecimal WKB.

## 5. The values along a trajectory

| Function | Returns |
|---|---|
| `rasterValue(traj, rast, band = 1, exclude_nodata_value = true, resample = 'nearest')` | `tfloat` |
| `rasterValue(traj, path text, band = 1)` | `tfloat`, read from a raster file on the server |
| `rasterTileValue(traj, tile)` | `tfloat` |
| `rasterTileValue(traj, raquet[])` | `tfloat` |

- **Which pixel.** A position reads the pixel the cell walks specification
  assigns it ([Cells and trajectories](meos-cell-walks.md), section 5,
  Raster): the column and row its exact grid coordinates floor to, a position
  on a pixel boundary belonging to one pixel only.
- **When a trajectory changes pixel.** The instant a trajectory crosses from
  one pixel into the next is computed exactly from the trajectory's segment
  and the grid line, never rounded to a sample; it is dated to the
  microsecond nearest that exact instant (section 6 of the cell walks).
- **Nearest.** Read with `resample = 'nearest'`, the value is the value of
  the pixel the position lies in, so a continuous trajectory reads a step
  `tfloat` whose steps are its pixel crossings, and a discrete trajectory
  reads a discrete `tfloat`.
- **Bilinear.** Read with `resample = 'bilinear'` (the name read without
  regard to case), a position reads the value `ST_Value` reads with that
  resampling, the four pixel centres around it weighted by their distance.
  Along a segment the value is a polynomial of degree two in time over each
  half-pixel square the segment crosses, so the `tfloat` is linear, broken at
  the crossings of the half-pixel lines and at the turning points of each
  polynomial.
- **Nodata and the edge of the raster.** A position over a nodata pixel, when
  `exclude_nodata_value` is true, and a position outside the raster read no
  value: the `tfloat` has no value at those times. A trajectory reading no
  value answers `NULL`. With `exclude_nodata_value` false, a nodata pixel
  reads its nodata value.
- **An array of tiles.** `rasterTileValue(traj, raquet[])` reads every tile
  the trajectory walks. Tiles of one zoom level partition the plane, so they
  answer disjoint times; where tiles of different zoom levels overlap, the
  tile of the higher zoom answers, being the finer, and two tiles of the same
  zoom answer in the order of the array.

## 6. The restrictions to a range of values

| Function | Returns |
|---|---|
| `atRasterValue(traj, rast, vspan, band = 1)`, `atRasterValue(traj, path text, vspan, band = 1)` | `tgeompoint` |
| `minusRasterValue(traj, rast, vspan, band = 1)`, `minusRasterValue(traj, path text, vspan, band = 1)` | `tgeompoint` |
| `eRasterValue(traj, rast, vspan, band = 1)`, `eRasterValue(traj, path text, vspan, band = 1)` | `boolean` |
| `aRasterValue(traj, rast, vspan, band = 1)`, `aRasterValue(traj, path text, vspan, band = 1)` | `boolean` |

With `v` the values `rasterValue` reads along `traj`:

- `atRasterValue` is the trajectory at the times `v` lies in `vspan`,
  `atTime(traj, getTime(atSpan(v, vspan)))`, and `minusRasterValue` the
  trajectory at the times `v` lies outside it,
  `atTime(traj, getTime(minusSpan(v, vspan)))`. A time at which the
  trajectory reads no value belongs to neither. Either answers `NULL` when no
  time qualifies.
- `eRasterValue` is true when `v` ever lies in `vspan`, and `aRasterValue`
  when every value of `v` does. A trajectory reading no value answers false
  to both.

The restrictions read the nearest value, nodata pixels excluded.

## 7. A raster that changes in time

A raster that changes in time, a satellite archive or a forecast, is a table
of rows `(validtime tstzspan, rast raster)`. No temporal type holds it: a
raster does not move, nothing interpolates between two acquisitions, and one
raster per instant would carry the pixels of every acquisition in a single
value. The value a trajectory reads from such a table is read today by
cutting the trajectory at each row's `validtime` (`atTime`) and merging the
reads of the rows (`merge`), the rows selected by the indexes on `validtime`
and on the raster's extent.

## 8. Versioning and stability

The minor number rises with each part of the interface master gains, the
patch number with corrections of wording.

| Version | Content |
|---|---|
| v0.1.0 | The two types, the WKB readers and writers and `raquetRead`, the values along a trajectory over a raster, a raster file, a Raquet tile and an array of tiles, the restrictions over a raster and a raster file, a raster series as rows of a table |
| v0.2.0 | `raster` a type of the MEOS catalog, `T_RASTER` |

## 9. Out of scope

Future: each of these is added here with its version.

- The restrictions over a Raquet tile and an array of tiles, reading the
  values `rasterTileValue` reads.
- `asGDALRaster`, `asTIFF`, `asPNG`, `asJPEG` and `rasterFromGDALRaster`,
  the writers of a raster to any GDAL format and its reader from the bytes of
  a GDAL file, as PostGIS's `ST_AsGDALRaster`, `ST_AsTIFF`, `ST_AsPNG`,
  `ST_AsJPEG` and `ST_FromGDALRaster` write and read; the creation options
  are one string of `KEY=VALUE` pairs, as GDAL tokenizes them, or in SQL the
  `text[]` of PostGIS.
- `asCoverageJSON`: a raster and a tile as a CoverageJSON `Grid` coverage,
  and the values a trajectory reads, `asCoverageJSON(tfloat, tgeompoint)`, as
  a `Trajectory` coverage.
- A Raquet file read as its tiles and written from them, through GDAL's
  Parquet driver, and one Raquet row decoded into a tile, for the engines
  that scan Parquet themselves.
- The values a trajectory reads from a raster series and its restrictions,
  `rasterValue(traj, raster[], tstzspan[], band)` and
  `rasterTileValue(traj, raquet[], tstzspan[])`: each raster read over the
  part of the trajectory within its span, a part within no span reading no
  value, spans that overlap refused.

- MobilityAPI publishing a raster collection through OGC API – Coverages,
  its queries along a path through OGC API – Environmental Data Retrieval,
  and the values a moving feature reads as a temporal property of OGC API –
  Moving Features.

Not specified: the processing functions of section 3 beyond their PostGIS
definitions, which PostGIS's documentation states.

## 10. References

- [Cells and trajectories](meos-cell-walks.md), the pixel of a position and
  the dating of a crossing.
- [Input and output](meos-io.md), the readers and writers of every type.
- [Exact geometry](meos-exact-geometry.md), the exact arithmetic the pixel
  walk reads.
- [PostGIS raster reference](https://postgis.net/docs/RT_reference.html), the
  functions the processing of section 3 follows.
- [Raquet](https://github.com/CartoDB/raquet), CARTO's specification of a
  raster as Parquet rows over QUADBIN cells.

## 11. Consumers

Every binding generated from the MEOS catalog inherits the raster functions
without code of its own: a `raster` and a `raquet` travel as their WKB, and
the values a trajectory reads are answered by MEOS.

## 12. References to the implementation

The source files of the reference line are authoritative. A disagreement
between this document and the code is a defect of one of them, reported as an
issue of MobilityDB.
