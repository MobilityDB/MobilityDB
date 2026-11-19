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
| Version | v0.9.0 |
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
| a raster file → value | `rasterFromGDALRaster(bytea, srid = 0)` | `raquetRead(path text, quadbin)`, `raquetRead(bytea, quadbin)` |
| a row of a Raquet file → value | | `raquetFromRow(block bigint, band bytea, metadata text, bandnum = 1)` |
| value → a raster file | `asGDALRaster(raster, format, options = NULL, srid = NULL)`, `asTIFF(raster, options = NULL, srid = NULL)`, `asPNG(raster, options = NULL)`, `asJPEG(raster, options = NULL)` | |

The WKB of a `raster` is PostGIS's raster WKB, so a value written by PostGIS
is read by every engine and the converse. `raquetRead` reads a raster file of
any format GDAL reads, from a path on the server or from the bytes of the
file, into a tile; when no QUADBIN cell is given, the cell is derived from the
geotransform of the file, which must state Web Mercator. The text of a
`raquet` is its hexadecimal WKB.

A `raster` is written in a format of GDAL and read from the bytes of a file
in one as PostGIS writes and reads it: `asGDALRaster`, `asTIFF`, `asPNG` and
`asJPEG` write the bytes `ST_AsGDALRaster`, `ST_AsTIFF`, `ST_AsPNG` and
`ST_AsJPEG` write, and `rasterFromGDALRaster` reads the raster
`ST_FromGDALRaster` reads. `format` is the short name of a GDAL driver that
writes a copy of a raster into memory (`GTiff`, `PNG`, `JPEG` ...), and
`options` its creation options, each `KEY=VALUE`; in MEOS the options are an
array of texts and their number, one option an element and a NULL element
stating none, handed to GDAL as its own list of strings, so that a value holding
a space or a quote is written as it is. A NULL `srid` when writing is
the raster's own, the pixels not being reprojected, and an `srid` of 0 when
reading the one the file states. A format takes the bands its driver takes: a
GeoTIFF holds one pixel type and one nodata value for all its bands, PNG one
to four bands of 8 or 16 unsigned bits, JPEG one or three bands of 8 unsigned
bits.

A Raquet file is a Parquet table whose rows are tiles keyed by their QUADBIN
cell in the column `block`, the row of block 0 holding the metadata of the
file as JSON. An engine that reads Parquet scans the rows itself and hands
each to `raquetFromRow` with the metadata, so that every engine decodes a tile
alike. The metadata states the size of a tile (`tiling.block_width`,
`tiling.block_height`), the type and nodata value of each band (`bands`), the
layout of the bands (`band_layout`: `sequential`, one column per band, or
`interleaved`, the bands of a pixel side by side in one column) and their
compression (`compression`: none, `gzip`, whose zlib form is read too, or
`jpeg` and `webp`, an image of every band of unsigned 8 bits). The pixels are
little-endian, row by row from the top. `bandnum` is the band of the bytes,
or the band taken from an interleaved column. The row of block 0, a band the
metadata does not state, bytes of another size than the tile, and a band
stating a scale or an offset, which a tile does not carry, are refused.

`asCoverageJSON` writes the encoding of OGC API – Coverages and OGC API –
Environmental Data Retrieval, CoverageJSON:

| Function | Coverage |
|---|---|
| `asCoverageJSON(raster)` | a `Grid`: the axes `x` and `y` run from the centre of the first pixel to the centre of the last, each band is a parameter `band1`, `band2` ..., whose values run row by row from the top row, a nodata pixel `null` |
| `asCoverageJSON(raquet)` | a `Grid` of Web Mercator, EPSG:3857, over the bounds of the tile's QUADBIN cell |
| `asCoverageJSON(tfloat, tgeompoint)` | a `Trajectory` of the values a trajectory reads (section 5): a point per instant of the values, at the position of the trajectory at that instant, so the instants a trajectory crosses from one pixel to the next are points of the coverage; values read at one place, the positions being equal doubles, are a `Point` for one instant and a `PointSeries` for several, the instants on its axis `t`, as the position query of OGC API – Environmental Data Retrieval answers them |

EPSG:4326 is stated as OGC CRS84, whose longitude comes first as MEOS states
it; another system is stated by its EPSG identifier, its coordinates listed in
the order the authority gives its axes. A raster with a skew, whose pixels lie
on no axes of a grid, and a value in no reference system are refused. A
coverage holds the values at its points, so a step or a linear `tfloat` is
written as the values at its instants.

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
| `atRasterValue`, `minusRasterValue`, `eRasterValue`, `aRasterValue` `(traj, tile, vspan)` and `(traj, raquet[], vspan)` | as above |

With `v` the values `rasterValue` reads along `traj`, or `rasterTileValue` for a
tile and an array of tiles, the tile of the higher zoom answering where tiles
overlap:

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
value. A trajectory reads such a series through overloads of the names that
read one raster, given the array of its rasters and the array of their
periods, paired one to one:

| Function | Result |
|---|---|
| `rasterValue(traj, rast raster[], validtime tstzspan[], band = 1)` | `tfloat` |
| `rasterTileValue(traj, rast raquet[], validtime tstzspan[])` | `tfloat` |
| `atRasterValue`, `minusRasterValue` `(traj, rast, validtime, vspan[, band])` | `tgeompoint` |
| `eRasterValue`, `aRasterValue` `(traj, rast, validtime, vspan[, band])` | `boolean` |

Each raster is read over the part of `traj` within its period, as section 5
reads one raster, and the reads are merged in the order of the periods,
whatever the order of the arrays. Rows sharing one period are the tiles of
one raster in space, read together as an array of tiles is read; two rows of
one place and one period are one tile given twice, read where they hold one
value and refused where they hold two, as `merge` refuses two temporal values
that differ at one timestamp. A part of
the trajectory within no period reads no value, as a nodata pixel reads none.
Two distinct periods that overlap, which would give two values at one time,
and arrays of different lengths are refused. The restrictions read these
values as section 6 states. A query passes the rows that meet the trajectory,
selected by the indexes on `validtime` and on the raster's extent, gathered
with `array_agg`. In MEOS: `raster_value_series`, `raster_tile_value_series`
and their restrictions, `raster_at_value_series` and the others, each taking
the rasters, a `Span` array of their periods and the count.

## 8. Versioning and stability

The minor number rises with each part of the interface master gains, the
patch number with corrections of wording.

| Version | Content |
|---|---|
| v0.1.0 | The two types, the WKB readers and writers and `raquetRead`, the values along a trajectory over a raster, a raster file, a Raquet tile and an array of tiles, the restrictions over a raster and a raster file, a raster series as rows of a table |
| v0.2.0 | `raster` a type of the MEOS catalog, `T_RASTER` |
| v0.3.0 | The restrictions over a Raquet tile and an array of tiles |
| v0.4.0 | One row of a Raquet file read into a tile, `raquetFromRow` |
| v0.5.0 | A raster, a Raquet tile and the values a trajectory reads written as CoverageJSON |
| v0.6.0 | The writers of a raster to a format of GDAL and its reader from the bytes of a file in one |
| v0.7.0 | The values a trajectory reads from a raster series and their restrictions |
| v0.8.0 | The values read at one place written as a CoverageJSON `Point` or `PointSeries` |
| v0.9.0 | The creation options of the GDAL writers taken in MEOS as an array of texts, as SQL and GDAL take them |

## 9. Out of scope

Future: each of these is added here with its version.

- A Raquet file read as its tiles and written from them, through GDAL's
  Parquet driver.

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
