<!--
   ****************************************************************************
    MobilityDB / MEOS Specification — Spatial Reference Systems
    Copyright(c) MobilityDB Contributors

    This documentation is licensed under a Creative Commons Attribution-Share
    Alike 3.0 License: https://creativecommons.org/licenses/by-sa/3.0/
   ****************************************************************************
-->

# Spatial Reference Systems — Specification

| | |
|---|---|
| Scope | Where every spatial type of MEOS and MobilityDB takes its SRID from, what its readers accept and refuse, what its writers write, and what `SRID`, `setSRID`, `transform` and `transformPipeline` do for it |
| Reference | `meos/src/geo/tspatial_srid.c`, `meos/src/geo/tspatial_parser.c`, `meos/src/temporal/type_in.c`, `meos/src/temporal/type_out.c`, `meos/src/pointcloud/schema_hook.c`, `meos/src/npoint/ways.c` |

## 1. Scope and intent

A spatial value of MEOS states its coordinates in a spatial reference system
named by an SRID. Most types hold their SRID in the value, as a PostGIS
geometry does. Three kinds of type take it from elsewhere and hold none of
their own: a network point takes the SRID of the road network it lies on,
a point cloud value the SRID of the schema its pcid names, and a cell of a
discrete global grid the SRID of WGS84. This document states, for every type,
where its SRID comes from and what each function does with it. It describes
the code; a statement here that the code contradicts is a defect of this
document.

## 2. Notation

*SRID 0* is the unknown reference system. *Known* means a positive SRID.
A function is named by its SQL name; the MEOS function that answers it is
given with its source file.

## 3. The source of the SRID

| Types | SRID | Held in |
|---|---|---|
| `geometry`, `geography`, `geomset`, `geogset`, `tgeompoint`, `tgeogpoint`, `tgeometry`, `tgeography` | the value's | the serialized geometry; a set or temporal value also keeps it in its bounding box |
| `stbox` | the box's | the `srid` field of `STBox` |
| `cbuffer`, `cbufferset`, `tcbuffer` | the value's | the `srid` field of `Cbuffer` |
| `pose`, `poseset`, `tpose`, `posechain`, `posechainset`, `tposechain` | the value's | the `srid` field of `Pose` and `PoseChain` |
| `trgeometry` | its poses' | the poses; the reference geometry is placed in their system |
| `raster` | the raster's | the raster header |
| `npoint`, `nsegment`, `npointset`, `nsegmentset`, `tnpoint` | the network's | nowhere: `get_srid_ways` reads the `ways` table in PostgreSQL and the ways CSV in MEOS (`meos_set_ways_csv`) |
| `pcpoint`, `pcpatch`, `pcpointset`, `pcpatchset`, `tpcpoint`, `tpcpatch` | the schema's | nowhere: `meos_pc_schema_srid` reads the schema the pcid names |
| `tpcbox` | the schema's | the box keeps it beside its pcid, and its readers resolve it from the schema |
| `h3index`, `th3index`, `quadbin`, `tquadbin`, `s2cell`, `ts2cell` | 4326 | nowhere: every cell is in WGS84 |
| `tbox`, `raquet` | none / 4326 | a `tbox` has no spatial dimension; a `raquet` tile states its extent in 4326 |

### 3.1 The schema of a pcid

pgPointCloud keeps the SRID of a schema in the `srid` column of
`pointcloud_formats` and offers no function to change it. MobilityDB reads a
schema from its own `pointcloud_schemas` and `pointcloud_dimensions` tables
first, then from `pointcloud_formats` (`mobilitydb_pc_schema`); standalone
MEOS reads the schemas registered with `meos_pc_schema_register*` or the
document set with `meos_set_pointcloud_schemas_xml`. The schemas are read
again for each statement, so a change to a schema's row reaches the next
statement.

### 3.2 The road network

In PostgreSQL the SRID of the network is the SRID of the geometries of
`public.ways`; in MEOS it is the SRID of the routes of the ways CSV. A
network whose SRID is 0 accepts any SRID a value states.

## 4. The SQL functions

| Types | `SRID` | `setSRID` | `transform(·, srid integer)` | `transform(·, pcid integer)` | `transformPipeline` |
|---|---|---|---|---|---|
| geo, stbox, cbuffer, pose, posechain, trgeometry and their sets and temporals | ✓ | ✓ | ✓ | | ✓ |
| raster | ✓ | | ✓, and `transform(rast, alignto raster)` | | ✓ |
| npoint, nsegment, tnpoint | ✓ | | | | |
| pcpoint, pcpatch, tpcpoint, tpcpatch, tpcbox | ✓ | | | ✓ | |
| pcpointset, pcpatchset | | | | ✓ | |
| cell types | | | | | |

A type whose SRID is not its own has no `setSRID`: the SRID of a network
point is changed by its network, and that of a point cloud value by moving it
into another schema. `setSRID` reinterprets the coordinates and never moves
them; `transform` moves them.

Every `setSRID` reads its SRID as `setSRID` of a geometry does, which reads it
as PostGIS's `ST_SetSRID`: the invalid SRID is refused, a negative SRID is the
unknown SRID 0, which is set, and an SRID above the maximum folds into the
reserved range. A value and its bounding box state the SRID set. In MEOS, where
every spatial type has a setter, a network point, a cell and a point cloud value
refuse any SRID other than the one of their network, of WGS84 or of their
schema.

### 4.1 Transforming a point cloud value

`transform(<type>, pcid integer)` moves a point cloud value into the schema of
another pcid, which states the target SRID and how the coordinates are
stored. X, Y and Z are reprojected from the SRID of the source schema to that
of the target schema; the other dimensions are matched by name, and a
dimension the source lacks takes 0, as pgPointCloud's `PC_Transform` fills it.
A coordinate that does not fit its target dimension at its scale and offset
is refused rather than clamped. A `tpcbox` transforms its extent and takes the
target pcid. No `transform(<type>, srid integer)` is declared for these types,
and the transformation of MEOS to an SRID refuses them (`datum_transf_pj`).

## 5. Readers and writers

### 5.1 What a reader accepts

| Types | A stated SRID |
|---|---|
| SRID held in the value | is stored; within one temporal text, two known SRIDs that differ are refused and an unknown one takes the other's |
| geodetic types (`geography`, `tgeogpoint`, …) | none stated means 4326 |
| network points | must equal the SRID of the network |
| point cloud values | must equal the SRID of the schema; a `tpcbox` keeps the stated SRID when its pcid is 0, names no schema, or names a schema stating SRID 0 |
| cell types | must be 4326 |

The binary readers check the SRID of the WKB header the same way, and the
MF-JSON reader the SRID its `crs` member states: a `crs` stating another SRID
than the one a network point, a cell or a point cloud value takes is refused,
and a `crs` stating the unknown SRID states nothing.

### 5.2 What a writer writes

`asText` and `asBinary` write no SRID. `asEWKT`, `asEWKB` and `asHexEWKB`
write a known SRID, for every type that has these functions, the types whose
SRID is derived included, so that a value written by one database is read by
another whose network or schema states the same SRID. `asMFJSON` writes a
known SRID as its `crs`.

## 6. Values of different SRIDs

An operation on two values of different SRIDs is refused, SRID 0 counting as
different from any known SRID: their coordinates are not in one system, as
metres and yards are not one unit. Two `tpcbox` values must also share their
pcid.

## 7. Related specifications

- [Input and output](meos-io.md), the readers and writers of every type and
  the SRID their extended forms carry.

## 8. References to the implementation

The getters dispatch through `spatial_srid` and `tspatial_srid`, the setters
through `spatial_set_srid`, and the transformations through
`datum_transf_pj`, all in `meos/src/geo/tspatial_srid.c`; the comparison of
two SRIDs is `ensure_same_srid`. A correction to this document cites the
function that contradicts it.
