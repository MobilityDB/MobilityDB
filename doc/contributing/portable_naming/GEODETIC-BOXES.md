<!--
  MobilityDB — Portable Naming: The geodetic boxes
  Copyright(c) MobilityDB Contributors

  This documentation is licensed under a Creative Commons Attribution-Share
  Alike 3.0 License: https://creativecommons.org/licenses/by-sa/3.0/
-->

# The geodetic boxes

[Back to the index](00-INDEX.md)

A geodetic `stbox` states a minimum and a maximum longitude and latitude. The manual
(`doc/box_types.xml`) defines its spatial extent as the longitude/latitude rectangle that covers
the value: "The box of a geodetic temporal point holds the great circles it travels between its
positions". Several operations read the box as something else: a geography polygon whose edges
are great circles, or a planar box in which a distance in metres is added to degrees. This
document states the defects, measured on MobilityDB master `985fdb26b7` with the probe
`999_probe_earth.test.sql` (kept outside the tree), and the pull requests that correct them.

**The campaign is deferred** while the target is the Spark and Flink surfaces. The family 12 earth
model (decision 10 of [Native geometry operations](12-NATIVE-GEOMETRY.md)) keeps the distance
operators over geography on the spheroid and does not wait on it; their index-ordered search does.

## What is measured

| Operation | Input | Answer | The rectangle's answer |
|---|---|---|---|
| `nearestApproachDistance(stbox, geography)` | `GEODSTBOX X((4,51),(6,52))`, `Point(5 50)` | 111713.5 m | at most 111238.7 m, the distance to `Point(5 51)`, a point of the box |
| `ORDER BY trip \|=\| geography 'Point(4.35 50.85)' LIMIT 20` over a GiST index on 2,000 `tgeogpoint` | | `ERROR: index returned tuples in wrong order` | the 20 nearest |
| `stbox(geography)` | `Linestring(0 50, 60 50)` | `GEODSTBOX X((0,50),(60,50))` | latitude up to 54.004979, the great circle's midpoint |
| `stbox(geography)` | `Linestring(170 0, -170 0)` | `GEODSTBOX X((-170,0),(170,0))` | the 20 degrees across the antimeridian |
| `tgeography && stbox` | the line above, `GEODSTBOX X((29,54),(31,55))` | 0 rows | 1 row: the line passes latitude 54.004979 at longitude 30 |
| `area(stbox, false)` | `GEODSTBOX X((0,0),(60,60))` | 37638683639267 m² | 36810833880023 m², R² Δλ (sin φ₂ − sin φ₁) |

The box distance converts the box to a geography polygon (`stbox_geo`): its south edge, a great
circle, rises north of the parallel the box states, so a point on that parallel lies outside the
polygon and nearer the query than the polygon is. The index distance (`Tspatial_gist_distance`,
the SP-GiST leaf sibling) calls the same box distance, which is then no lower bound of what the
operator computes. PostGIS keeps its geography `<->` on the sphere for this reason
(`geography_distance_knn`: "must use sphere, can't get index to harmonize with spheroid").

## What reads a geodetic box (read, not run)

| Function | Reading today | Rectangle |
|---|---|---|
| `stbox_geo`, `stbox_to_geo` | a polygon marked geodetic, great-circle edges | no |
| `stbox_area`, `stbox_perimeter` | `geog_area`, `geog_perimeter` of that polygon | no |
| `stbox_spatial_dist` (`stbox_nad`, `nad_stbox_stbox`), `nad_stbox_geo`, `nad_tgeo_stbox` | the geography distance to that polygon | no |
| `stbox_gist_distance`, the SP-GiST leaf distance | `nad_stbox_stbox` | no |
| `distance_stbox_nodebox` (SP-GiST inner nodes) | `hypot` of the degree gaps | not a bound in metres near the poles |
| `stbox_expand_space_set` (`expandSpace`), `makeExpandExpr` (the `eDwithin`/`aDwithin` index condition) | metres added to degrees | no |
| `geo_set_stbox` for a geography other than a point | `lwgeom_calculate_gbox_cartesian` | no: no great-circle bulge, no antimeridian |
| `tgeoinstarr_set_stbox`, `tgeoinstarr_extend_stbox_geodetic` (`tgeogpoint`) | the great-circle extreme of each segment, the full longitude range across the antimeridian | yes |
| `tpointseq_linear_at_stbox_geodetic` | the arcs entering and leaving the rectangle | yes |
| `tgeoinst_restrict_stbox_iter` (a `tgeography` instant) | `geom_intersection2d` with the polygon of `stbox_geo` | no |
| `inter_stbox_stbox`, the topological and position operators, `stbox_expand`, `stbox_quad_split`, the GiST penalty | the coordinates | yes |
| `stbox_expand`, `stbox_round_set`, `stbox_expand_space_set`, `nd_box_from_stbox` | read Z whenever the box is geodetic | the Z flag is the input's: a 2D geodetic box has none, so `expandSpace` refuses to shrink it |

The callers of `stbox_geo` in the network point, pose, rigid geometry and circular buffer families
(`tnpoint_distance.c`, `tpose_distance.c`, `pose.c`, `trgeo_distance.c`, `tcbuffer_distance.c`,
`cbuffer.c`) take the same polygon for a geodetic box. A geodetic box never has `xmin > xmax`:
`stbox_set` orders the bounds, and a value crossing the antimeridian takes the full longitude
range. The manual sentence "Geodetic boxes always have a Z dimension" (`doc/box_types.xml`)
states what the code does not.

## The plan

Each a pull request against master, with its regression tests and its manual entries in English
and Spanish:

| PR | Topic | What it changes |
|---|---|---|
| GB1 | The box of a geography covers it | `geo_set_stbox` extends the latitude over each great-circle edge and takes the shorter side across the antimeridian, through `dggs_lonlat_segment_extend_box`, as `tgeoinstarr_extend_stbox_geodetic` does for `tgeogpoint`; `&&` finds the line above |
| GB2 | The distance to a geodetic box is the distance to its rectangle | the nearest point of a rectangle lies on the query's meridian when the query's longitude is inside, else on the facing meridian edge at the foot of the perpendicular: on the sphere in closed form, on the spheroid by minimising the geodesic distance along that edge; the index distances take the closed-form lower bound on reduced latitudes scaled by (1 − f) a, which holds because the spheroid is that sphere shortened by (1 − f) along its axis; `distance_stbox_nodebox` in metres for a geodetic box; the index-ordered search answers the 20 nearest and agrees with the sequential scan |
| GB3 | The area and the perimeter of a geodetic box | the zone area of the rectangle, R² Δλ (sin φ₂ − sin φ₁) on the sphere and its authalic form on the spheroid; the perimeter two meridian arcs and two parallel arcs |
| GB4 | Expanding a geodetic box by a distance | `expandSpace` in metres: the latitude by the distance over the smallest meridian radius, the longitude over the parallel nearest the pole, the full longitude range once the expansion reaches a pole, so the `eDwithin` index condition keeps every match; a 2D geodetic box has no Z in `stbox_expand`, `stbox_round_set`, `stbox_expand_space_set`, `nd_box_from_stbox`; the manual sentence on Z |
| GB5 | The geography of a geodetic box | a geography polygon cannot state a parallel, so `geography(stbox)` and its callers (`tgeoinst_restrict_stbox_iter` and the families above) need a decision: a polygon whose parallels are densified to a stated tolerance, or a refusal; open |

GB2 also settles the distance from a geodetic box to a geography other than a point: zero when
the geography meets the rectangle, else the least distance between the geography's great-circle
edges and the rectangle's edges, the meridian edges being geodesics and the parallel edges not;
open.

**State.** Deferred. The branch `fix/geodetic-knn-bound`, on MobilityDB master `985fdb26b7`,
holds no commit.
