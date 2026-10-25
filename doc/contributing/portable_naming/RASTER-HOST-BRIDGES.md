<!--
  MobilityDB — Portable Naming: The raster in every engine, and its bridges to the host rasters
  Copyright(c) MobilityDB Contributors

  This documentation is licensed under a Creative Commons Attribution-Share
  Alike 3.0 License: https://creativecommons.org/licenses/by-sa/3.0/
-->

# The raster in every engine, and its bridges to the host rasters

[Back to the index](00-INDEX.md) · [Aggregates plan](AGGREGATES-PLAN.md) · [Implementation plan](IMPLEMENTATION-PLAN.md)

Every engine carries MEOS's `raster` with its whole algebra, and converts it to and from the raster
its host offers, where it offers one: DuckDB's raster extension (`duckdb-raster`) and Apache
Sedona's raster on Spark; Flink offers none. This document states what is measured, the decisions,
and the pull requests that deliver it in MEOS, MobilityDB and the bindings in scope (JMEOS,
MobilitySpark, MobilityFlink, MobilityDuck). Measured at MobilityDB master `982eeeafce`, JMEOS main
`a29e4b29c`, MobilityDuck main pinning DuckDB `d8cdaa33fda8` (v1.5.5), `ahuarte47/duckdb-raster`
main (pushed 2026-10-18) and `apache/sedona` master (pushed 2026-10-23), on 2026-10-25.

## The raster on the Spark and Flink surfaces

The catalog states 55 SQL names over `raster` or `summarystats`; 6 are the transition and final
functions of the three raster aggregates, which no binding carries. Of the 49 a query calls, the
generated Spark and Flink surfaces carry the same 44 (JMEOS main's generator over the catalog,
`--engine spark-sql` and `flink-sql`): the scalar raster functions, `rasterUnion`,
`sameAlignmentAgg`, `summaryStatsAgg`, `summaryStats` and its six accessors (JMEOS #186, #187,
#188; MobilitySpark #87, #88; MobilityFlink #88, #90; MobilityDB #3172, #3177). Five are left out,
on both engines, for the reason the generator states:

| SQL name | Reason | Where the gap is |
|---|---|---|
| `dumpAsPolygons` | `setret:column/return:geomval`: a set of rows whose geometry column the generator does not read | JMEOS generator |
| `valueCount` | `setret:column/return:integer`: a set of `(value, count)` rows | JMEOS generator |
| `dumpValues`, `pixelAsPoints` | `arity:jmeos`: the jar's parameter count differs from the visible C parameters | JMEOS generator or the catalog's shape |
| `reclass` | `arity:sql`: the SQL arguments do not pair with the C parameters | the catalog or the MobilityDB wrapper |

## The host rasters

| Engine | Host raster | Its names | A raster value |
|---|---|---|---|
| DuckDB | `duckdb-raster` | `RT_` prefix, types and functions | none: a tile is a row of `RT_Read` (below) |
| Spark | Apache Sedona, `RasterUDT` | `RS_` prefix (`RS_MakeEmptyRaster`, `RS_MakeRaster`, `RS_FromGeoTiff`, `RS_AsGeoTiff`, ...) | yes, read and written as GeoTIFF or built from its band values; no PostGIS WKB form |
| Flink | none: Sedona's Flink module registers geometry, geography and box types alone | | |

In each engine the host's names carry its prefix and MobilityDB's carry none, so the two never
share a name. On Spark the bridge reads and builds a Sedona raster from its band values and
georeferencing (`RS_MakeEmptyRaster`, `RS_MakeRaster`, its band and georeference accessors), which
RD1's functions answer on the MEOS side.

## The raster in DuckDB

## What is measured

| Fact | Measured |
|---|---|
| MobilityDuck's raster surface | `raquet` (a BLOB of MEOS WKB) and the GDAL file sampling; no `raster` type, so none of the 48 functions, the 3 aggregates or the 6 accessors |
| The DuckDB version the extension is published for | v1.5.2 to v1.5.5, which includes the v1.5.5 MobilityDuck pins |
| The extension's types | `RT_DATACUBE` (BLOB), `RT_BBOX` (struct `xmin, ymin, xmax, ymax`), `RT_STATS` (struct `minimum, maximum, sum, mean, stddev, valid_count, nodata_count`), `RT_ARRAY` (struct alias, unregistered); `src/raster/raster_types.cpp` |
| The extension's functions | `RT_Read`, `RT_Stats`, `RT_CubeStats`, `RT_CubeStats_Agg`, `RT_RasterValue`, `RT_CoordValue`, `RT_Envelope`, ..., all prefixed `RT_` |
| A raster value in the extension | none: `RT_Read` answers one row per tile, its pixels in `databand_N` (or `datacube`) BLOB columns, its georeferencing in a `metadata` JSON column with `tile_x`, `tile_y`; the extension's own functions georeference a tile by those four (`RT_Stats`'s zonal form) |
| The `metadata` JSON | the extension's own format: `file_format: "raster"`, `version: "0.1.0"`, `data_format`, `datacube`, `crs` (`EPSG:n` or WKT), `transform` (the six GDAL coefficients), `bounds`, `geometry`, `width`, `height`, `blocksize_x`, `blocksize_y`, `band_count`, and per band `name`, `description`, `type_name`, `data_type` (a GDAL code), `data_size`, `width`, `height`, `colorinterp`, `nodata`, `scale`, `offset`, `unit`; `src/raster/raster_utils.cpp` |
| The datacube BLOB | an N-dimensional array with a small header, its data `RAW`, `SNAPPY`, `GZIP` or `ZSTD` (`docs/functions.md`, BLOB structure) |
| What MEOS states of a raster | `raster_width`, `raster_height`, `raster_srid`, `raster_upper_left_x/y`, `raster_scale_x/y`, `raster_skew_x/y`, `raster_num_bands`, `raster_band_pixel_type`, `raster_band_nodata_value`, `raster_from_wkb`, `raster_as_wkb`, `raster_dump_values` |
| What MEOS lacks for the conversion | the pixels of a band as their bytes (`raster_dump_values` answers doubles, which drop the pixel type and are inexact over 64-bit integers), and a raster built from pixels and a georeferencing (MEOS reads a raster from WKB alone) |
| The build of the extension beside MobilityDuck | blocked on MobilityDuck's GDAL overlay port, 3.8.5 with `geos` and `network` only, where the extension needs `hdf5`, `jpeg`, `lzma`, `openjpeg`, `openssl`, `png`, `zstd` and a `tiff` dependency |

## Decisions

1. **The raster is MEOS's** (the user, 2026-10-25): "MEOS algebra is closed. The canonical way to
   solve this is that everything is done in raster as in MEOS, and then enable the casting between
   MEOS and DuckDB raster." MobilityDuck binds MEOS's `raster` and its algebra from the catalog, as
   every family; the conversion to and from the extension is the binding's boundary layer, which
   converts a platform's type to the MEOS type and back.
2. **The two never share a name** (the user, 2026-10-25: "there must be a SUPER EVIDENT way to
   distinguish between MEOS raster types and DuckDB raster types"). Every name the extension adds
   carries the prefix `RT_`; every name without it is MobilityDB's, the name of the MobilityDB
   manual and of chapter 18, the same in every engine.
3. **The conversions take the form every cast of the ecosystem takes**: a constructor named after
   the type it builds, `type1(type2)`, which is also the cast `type2::type1` where one value
   converts to one value.

   | From | To | SQL |
   |---|---|---|
   | a tile of `RT_Read` | `raster` | `raster(databand_1, tile_x, tile_y, metadata)` |
   | `raster` | `RT_DATACUBE` | `CAST(rast AS RT_DATACUBE)` |
   | `raster` | the `metadata` of a tile | `rasterMetadata(rast)` |
   | `summarystats` | `RT_STATS` | `CAST(stats AS RT_STATS)`, `nodata_count` NULL |
   | `RT_STATS` | `summarystats` | `CAST(stats AS summarystats)` |

   A raster is the tile it gives: `raster(CAST(r AS RT_DATACUBE), 0, 0, rasterMetadata(r)) = r`.
4. **A raster passes to the extension with its georeferencing** (2026-10-25): the pixels by the
   cast and the metadata by `rasterMetadata`, the two columns the extension's functions read a tile
   from, so the round trip loses nothing. `rasterMetadata` takes the class prefix as a generic
   property does in the portable dialect (`rasterWidth`, `raquetWidth`, MobilityDB #3168).
5. **The extension's formats stay in MobilityDuck.** The `metadata` JSON and the datacube codec are
   the extension's private formats; MEOS states the facts host-neutrally (the accessors above and
   the two functions it lacks), and MobilityDuck assembles the extension's JSON and datacube from
   them.
6. **The functions that build and read a raster take PostGIS's names and arguments**, the rule of
   the raster chapter ("A raster function takes the name of its PostGIS raster counterpart with the
   `ST_` prefix removed, and the names and defaults of its arguments"): `makeEmptyRaster`
   (`ST_MakeEmptyRaster`), `addBand` with an initial value and with the band of another raster
   (`ST_AddBand`'s forms), `setValues` from an array of values (`ST_SetValues`). The pixels of a
   band as bytes are `pixels(raster, band)`, the twin of `pixels(raquet)`; a raster built from such
   bytes is the twin of `raquet(pixels, width, height, quadbin, pixtype, nodata)`:
   `raster(pixels, width, height, upperleftx, upperlefty, scalex, scaley, skewx, skewy, srid,
   pixeltype, nodataval)`, one band, further bands through `addBand(torast, fromrast, ...)`. The
   bytes keep every pixel exact, a 64-bit integer included; the arrays of values are the form
   Sedona and `ST_SetValues` exchange.

## The pull requests

| # | Repository | Change | State |
|---|---|---|---|
| RD1 | MobilityDB | the functions of decision 6: `pixels(raster, band)` (MEOS `raster_band_pixels`), `raster(pixels, width, height, ..., pixeltype, nodataval)`, `makeEmptyRaster`, `addBand` (both forms), `setValues`; over rt_core's `rt_band_get_data`, `rt_raster_new` and its geotransform setters, `rt_band_new_inline`, `rt_raster_add_band`, `rt_band_set_pixel`; regression and MEOS tests, the manual in English and Spanish | not started |
| RD2 | JMEOS, MobilitySpark, MobilityFlink | RD1's functions reach both engines through the catalog (`bytea`, `text`, arrays, integers and doubles are carried); a consumer test of the round trip `raster(pixels(r), width(r), ..., bandPixelType(r), bandNoDataValue(r)) = r` for a raster of one band | after RD1 |
| RD3 | MobilityDuck | the `raster` family: its type (a BLOB of MEOS WKB, as `raquet`), the six generator lists, so the generator registers the raster functions, aggregates and accessors from the catalog; to measure, whether the generator answers `summarystats` as a struct (the JVM surfaces needed JMEOS #187 and #188 for it) | not started |
| RD4 | MobilityDuck | a probe loading the extension beside MobilityDuck at v1.5.5: whether a cast reaches `RT_DATACUBE`, a type another extension registers | not started |
| RD5 | MobilityDuck | the GDAL overlay port with the features the extension needs | not started, the build blocker |
| RD6 | MobilityDuck | the boundary layer of decisions 3-5: the datacube decoder and encoder (`RAW`, `SNAPPY`, `GZIP`, `ZSTD`), `raster(datacube, tile_x, tile_y, metadata)` over `raster_make_empty` and `raster_add_band`, `CAST(raster AS RT_DATACUBE)` over `raster_band_pixels`, `rasterMetadata` written from the MEOS accessors, the `summarystats` and `RT_STATS` casts; a test of the round trip | after RD3-RD5 |
| RD8 | JMEOS | the four left out by the generator: `dumpAsPolygons` and `valueCount` (rows of a set whose columns are a geometry and a struct), `dumpValues` and `pixelAsPoints` (`arity:jmeos`); then the consumer tests | not started |
| RD9 | MobilityDB or MEOS-API | `reclass` (`arity:sql`): pair its SQL arguments with the C parameters in the catalog | not started |
| RD10 | MobilitySpark | the bridge to Sedona's raster, its names prefixed `RS_`: a Sedona raster from a MEOS raster and back over the georeference accessors and the band values (`RS_MakeEmptyRaster`, `RS_MakeRaster`, RD1's functions); a test of the round trip | after RD1 |
| RD7 | MobilityDB | the manual: the raster chapter's section on the two rasters (`doc/temporal_cell_index.xml`, branch `estebanzimanyi:doc/raster-in-duckdb`, commit `a0b592dfe4`, no PR yet), decisions 2-4 with `rasterMetadata` and the round trip, the tables of the raster chapter fitted to the page; Spanish before its PR | drafted |

## Feasibility

| Part | Feasibility | Why |
|---|---|---|
| RD1 | high | rt_core, vendored in MEOS, has every primitive; three thin public functions and their SQL |
| RD2 | high | the types are carried; no generator change expected |
| RD3 | medium | `raquet` is the exact sibling; the struct result and the aggregate argument forms are to measure |
| RD8, RD9 | medium | each reason is named by the generator; the set-returning rows need the generator to read a geometry and a struct column, the arities a look at the catalog's shape |
| RD10 | medium | Sedona builds a raster from band values and reads them back; the bridge is the binding's, over RD1 |
| RD4-RD6 | lowest | the code is feasible, the build and test are not yet: the extension's type is another extension's, the datacube brings its compression libraries, and the extension builds beside MobilityDuck only after RD5 |
