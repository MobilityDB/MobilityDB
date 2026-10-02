<!--
  MobilityDB — Portable Naming: Family 4 — Spatial transformations: `transform`, `translate` and the affine family
  Copyright(c) MobilityDB Contributors

  This documentation is licensed under a Creative Commons Attribution-Share
  Alike 3.0 License: https://creativecommons.org/licenses/by-sa/3.0/
-->

# Family 4 — Spatial transformations: `transform`, `translate` and the affine family

[Back to the index](00-INDEX.md)

`transform` and `translate` are built-in functions of Spark 3.5.1 (`transform` is its
higher-order array function); Flink 2.0.0 defines neither. MobilityDB declares `transform` 21
times at master `2495c4cc36`. `transformPipeline` (17 signatures) conflicts with nothing and forms
a group with `transform` (rule 5). `translate` is one of the affine transformations PostGIS names
`ST_Affine`, `ST_Rotate`, `ST_RotateX`, `ST_RotateY`, `ST_RotateZ`, `ST_Scale`, `ST_Transscale`
and `ST_Translate`; they conflict with nothing, and rule 5 moves them with `translate`.

## The `transform` signatures

| Operand types | `transform` wrapper | MEOS function | `transformPipeline` over the same types |
|---|---|---|---|
| `geometry`, `geography` | SQL over PostGIS `ST_Transform` | `geo_transform` | not declared (MEOS has `geo_transform_pipeline`) |
| `geomset`, `geogset`, `cbufferset`, `poseset`, `posechainset` | `Spatialset_transform` | `spatialset_transform` | yes |
| `tgeompoint`, `tgeogpoint`, `tgeometry`, `tgeography`, `tcbuffer`, `tpose`, `tposechain`, `trgeometry` | `Tspatial_transform` | `tspatial_transform` | yes |
| `cbuffer` | `Cbuffer_transform` | `cbuffer_transform` | yes |
| `pose` | `Pose_transform` | `pose_transform` | yes |
| `posechain` | `Posechain_transform` | `posechain_transform` | yes |
| `stbox` | `Stbox_transform` | `stbox_transform` | yes |
| `raster` (2 forms: to an SRID, to the grid of another raster) | `Raster_transform`, `Raster_transform_raster` | `raster_transform` | no (G7 adds it) |

## The affine signatures

At master `0be1b51060` (`076_tgeo_analytics.in.sql`, `076_tpoint_analytics.in.sql`), each
declared over `tgeometry` and `tgeompoint`:

| SQL name | Arguments after the temporal value | Backing | MEOS function |
|---|---|---|---|
| `affine` | 12 coefficients (3D) | `Tgeo_affine` | `tgeo_affine` |
| `affine` | 6 coefficients (2D) | `Tgeo_affine_2d` | `tgeo_affine_2d` |
| `rotate` | `angle` | `Tgeo_rotate_z` | `tgeo_rotate_z` |
| `rotate` | `angle, x0, y0` | `Tgeo_rotate` | `tgeo_rotate` |
| `rotate` | `angle, geometry` | SQL: `rotate($1, $2, ST_X($3), ST_Y($3))` | none |
| `rotateX`, `rotateY`, `rotateZ` | `angle` | `Tgeo_rotate_x`, `_y`, `_z` | `tgeo_rotate_x`, `_y`, `_z` |
| `scale` | `geometry` factors, optional `origin geometry` | `Tgeo_scale` | `tgeo_scale` |
| `scale` | `xfactor, yfactor, zfactor` | `Tgeo_scale_xyz` | `tgeo_scale_xyz` |
| `scale` | `xfactor, yfactor` | SQL: `scale($1, $2, $3, 1)` | none |
| `transscale` | `deltax, deltay, xfactor, yfactor` | `Tgeo_transscale` | `tgeo_transscale` |
| `translate` | `deltax, deltay, deltaz` | `Tgeo_translate` | `tgeo_translate` |
| `translate` | `deltax, deltay` | SQL: `translate($1, $2, $3, 0)` | none |

30 signatures: `affine` 4, `rotate` 6, `rotateX`, `rotateY`, `rotateZ` 2 each, `scale` 8,
`transscale` 2, `translate` 4. `rotate(t, angle)` and `rotateZ(t, angle)` call the one wrapper
`Tgeo_rotate_z`, as PostGIS `ST_Rotate` and `ST_RotateZ` are one rotation. No affine function is
declared over `tgeography`, `tcbuffer`, `tpose`, `tposechain` or `trgeometry`, and MEOS has none
over a base geometry: PostgreSQL users reach `geometry` through PostGIS `ST_Affine` and its
siblings, which Spark and Flink do not have.

`scale` also names 15 declarations over the sets, spans and span sets of numbers, dates and
timestamps (`Numset_scale`, `Numspan_scale`, `Numspanset_scale`, `Tstzset_scale`,
`Tstzspan_scale`, `Tstzspanset_scale`), which scale an extent rather than a geometry.

## Over `geometry` and `geography`

PostGIS declares the whole family over `geometry`, 15 signatures (`ST_Affine` 2, `ST_Rotate` 3,
`ST_RotateX`, `ST_RotateY`, `ST_RotateZ`, `ST_Translate` 2, `ST_Scale` 4, `ST_Transscale`), and
none over `geography` (0 of the 111 functions of `geography.sql.in`). An affine map of longitude
and latitude is not a transformation of the sphere: it keeps neither geodesics nor distances, and
it breaks across the antimeridian and at the poles.

In the source of master `0be1b51060`, the MEOS functions compute each instant as
`tgeoinst_affine_iter` does: a copy of the value, then liblwgeom's `lwgeom_affine`, a planar map.
The only check on that path is `VALIDATE_TGEO`, `tgeo_type_all` admits `T_TGEOGPOINT` and
`T_TGEOGRAPHY`, and no `ensure_not_geodetic` appears in `tgeo_affine`, `tgeo_affine_coefs` or
`tgeoinst_affine_iter`. Read, not run: whether a `tgeogpoint` passed through the MEOS API is
transformed or refused is what the test of G11 establishes. SQL declares the family over
`tgeometry` and `tgeompoint` only, so PostgreSQL does not reach that path.

## Final names by the decided rules

| Flink and Spark name | Operand types | `transform` | `transformPipeline` name | its signatures |
|---|---|---|---|---|
| `geoTransform` | `geometry`, `geography`, `geomset`, `geogset`, `tgeompoint`, `tgeogpoint`, `tgeometry`, `tgeography`, `trgeometry` | 9 | `geoTransformPipeline` | 7 |
| `cbufferTransform` | `cbuffer`, `cbufferset`, `tcbuffer` | 3 | `cbufferTransformPipeline` | 3 |
| `poseTransform` | `pose`, `poseset`, `tpose` | 3 | `poseTransformPipeline` | 3 |
| `posechainTransform` | `posechain`, `posechainset`, `tposechain` | 3 | `posechainTransformPipeline` | 3 |
| `stboxTransform` | `stbox` | 1 | `stboxTransformPipeline` | 1 |
| `rasterTransform` | `raster` | 2 | `rasterTransformPipeline` | 1 (G7) |
| | **Total** | **21** | | **18** |

| Flink and Spark name | PostgreSQL name | Operand types | Signatures |
|---|---|---|---|
| `geoTranslate` | `translate` | `geometry`, `tgeompoint`, `tgeometry`, `trgeometry` | 7 |
| `geoAffine` | `affine` | `geometry`, `tgeompoint`, `tgeometry` | 6 |
| `geoRotate` | `rotate` | `geometry`, `tgeompoint`, `tgeometry`, `trgeometry` | 11 |
| `geoRotateX`, `geoRotateY`, `geoRotateZ` | `rotateX`, `rotateY`, `rotateZ` | `geometry`, `tgeompoint`, `tgeometry`, `trgeometry` | 4 each |
| `geoScale` | `scale` | `geometry`, `tgeompoint`, `tgeometry` | 12 |
| `geoTransscale` | `transscale` | `geometry`, `tgeompoint`, `tgeometry` | 3 |
| | | **Total** | **51** |

Each prefix is the base type as in family 2 (`geoRound`, boxes their own name; `trgeometry` the
`geo` of the geometry it answers at an instant) and the stem of the MEOS function (`geo_`, `tgeo_`, `cbuffer_`, `pose_`, `posechain_`,
`stbox_`, `raster_`). `geoTransscale` spells the PostGIS `ST_Transscale` and MobilityDB's `transscale`.

## Checks

Neither engine defines, and neither parser refuses, `geoTransform`, ..., `rasterTransform`, the
six `...TransformPipeline`, `geoTranslate`, `geoAffine`, `geoRotate`, `geoScale` or
`geoTransscale`; none is a MobilityDB SQL name today (`f4-spark.tsv`, `f4-flink.tsv`, the names
lower-cased, as both engines match them). `geoRotateX`, `geoRotateY`, `geoRotateZ` and
`rasterTransformPipeline` are not in those runs and are measured the same way before R9.

## Decisions

1. **`translate` and the whole affine family move under `geo`** in Flink and Spark:
   `geoTranslate`, `geoAffine`, `geoRotate`, `geoRotateX`, `geoRotateY`, `geoRotateZ`,
   `geoScale`, `geoTransscale`: the 30 temporal signatures and 15 over `geometry` (decision 4),
   45 in all (rule 5). PostgreSQL keeps `translate`, `affine`,
   `rotate`, `rotateX`, `rotateY`, `rotateZ`, `scale` and `transscale` (rule 2), and the names
   reach the engines as `@altsqlfn` on the wrappers (R9).
2. **`scale` over sets, spans and span sets keeps its name**: it is another operation and
   conflicts with nothing (rule 7).
3. **The three SQL compositions become calls of the MEOS functions**: `translate(t, dx, dy)` and
   `scale(t, xf, yf)` are the 3D forms with `deltaz` 0 and `zfactor` 1, which the catalog states
   as defaults of the C-backed form; `rotate(t, angle, geometry)` reads its centre inside MEOS
   rather than through PostGIS `ST_X`/`ST_Y` (G10).
4. **The family reaches `geometry` and refuses `geography`.** MEOS gains `geo_affine`,
   `geo_affine_2d`, `geo_rotate`, `geo_rotate_x`, `geo_rotate_y`, `geo_rotate_z`, `geo_scale`,
   `geo_scale_xyz`, `geo_translate` and `geo_transscale`, each `lwgeom_affine` over a copy as
   `tgeoinst_affine_iter` computes it, and SQL declares the 15 PostGIS signatures over
   `geometry` under the names the family carries over the temporal types, `translate`, `affine`,
   `rotate`, `rotateX`, `rotateY`, `rotateZ`, `scale` and `transscale`. The
   `tgeo_*` affine functions and the new `geo_*` ones refuse a geodetic value, as the planar
   `geom_*` entries do.
5. **PostgreSQL declares `transformPipeline` over `geometry` and `geography`**, the name the
   operation carries over the temporal types, beside PostGIS's
   `ST_TransformPipeline(geom, 'urn:ogc:def:coordinateOperation:EPSG::16031')`
   ([decision 0.5](IMPLEMENTATION-PLAN.md)). Spark and Flink, which have no PostGIS, call
   `geoTransformPipeline(geom, ...)`, the `@altsqlfn` of the same wrapper over the MEOS
   `geo_transform_pipeline`. The raster gains `transformPipeline` (G7): 20 `transformPipeline`
   signatures in PostgreSQL.
6. **`translate` and `rotate` reach `tcbuffer`, `tpose`, `tposechain` and `trgeometry`;
   `scale`, `affine` and `transscale` do not.** A translation or a rotation keeps each value what
   its type states: a `Cbuffer` is one centre and one radius (`cbuffer/cbuffer.h`), a pose a
   position and an orientation (`pose/pose.h`), a rigid geometry a reference geometry moved by a
   pose (`trgeometryinst_make(geom, pose, t)`). A scale with two factors, a shear or a rotation
   out of the plane turns a circle into an ellipse, which one radius cannot state, and deforms a
   rigid body. `rotateX` and `rotateY` reach the pose types and `trgeometry`, not `tcbuffer`,
   whose value has no Z; over a 2D value they lift it to 3D exactly, the position at z = 0 and
   the orientation the rotation composed with the planar one. The Flink and Spark names take the
   base type (`cbufferTranslate`, `cbufferRotate`, `poseTranslate`, `poseRotate`, …), as
   `cbufferTransform` does; `trgeometry` takes `geoTranslate` and `geoRotate`, as in family 2;
   and `INHERITANCE_MAP.md` states the family in its Transformations row (G4).
