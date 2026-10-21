<!--
  MobilityDB — Making GEOS optional: appendix, the MEOS functions with a path to GEOS
  Copyright(c) MobilityDB Contributors

  This documentation is licensed under a Creative Commons Attribution-Share
  Alike 3.0 License: https://creativecommons.org/licenses/by-sa/3.0/
-->

# Appendix A. The MEOS functions with a path to GEOS

**Measured at** MobilityDB master `d9b9c11985`.

This appendix lists every public MEOS function whose code can reach GEOS, and says for each one
what a run in a build without GEOS shows. [Note 5](05-WHAT-STILL-NEEDS-GEOS.md) explains how the
list is obtained and what it means; this page is the list itself.

## How to read the table

- **Function** — the public C function of MEOS. Its SQL name is the one its manual entry gives.
- **Group** — the documentation group of the function (`@ingroup`), which names its family: for
  example `meos_geo_rel_ever` holds the "ever" spatial relationships of temporal geometries,
  `meos_cbuffer_rel_temp` the temporal relationships of temporal circular buffers.
- **Fall-backs on its path** — which of the four GEOS fall-backs of `meos/src/geo/postgis_funcs.c`
  its chain of calls can reach: the intersection, the difference, the union of one geometry
  (*unary union*) and the union of an array of geometries (*array union*).
- **Gateways** — the functions on its paths that call a fall-back-holding function directly
  (note 5 §5.4). A function usually has several. Only certain inputs make a gateway reach GEOS,
  so functions sharing a gateway are exposed in the same way; they can still differ, since each
  hands the gateway its own arguments.
- **In a build without GEOS** — the result of running the function, in a MEOS built with
  `-DGEOS=OFF` from the master above, through the probe named in parentheses (all in
  [`tools/`](tools/), their output in [`tools/results/`](tools/results/)):
  - **reaches GEOS**: some input makes it raise the error of a fall-back ("… is answered by the
    GEOS library, which this build excludes"); note 5 gives the inputs;
  - **answered every tested input**: it answered every input of the probe, which includes the
    inputs that make other functions reach GEOS;
  - **not run**: no probe calls it directly. Its gateway column says which tested functions share
    its path.

The list is the output of [`tools/geos_reach.py`](tools/geos_reach.py), in
[`tools/results/geos_reach.txt`](tools/results/geos_reach.txt), which also gives each function's
file, its shortest chain of calls and its gateways. It lists **134 of the 3051 public functions** of `meos/src` (both counts in
[`tools/results/geos_reach_counts.txt`](tools/results/geos_reach_counts.txt));
the other 2917 have no path to GEOS at all.

## The table

| Function | Group | Fall-backs on its path | Gateways | In a build without GEOS |
|---|---|---|---|---|
| `tcbuffer_convex_hull` | meos_cbuffer_accessor | array union, unary union | `tcbufferseq_traversed_area`, `tcbufferseqset_traversed_area` | not run |
| `nad_tcbuffer_geo` | meos_cbuffer_dist | array union, unary union | `tcbufferseq_traversed_area`, `tcbufferseqset_traversed_area` | not run |
| `nad_tcbuffer_stbox` | meos_cbuffer_dist | array union, unary union | `tcbufferseq_traversed_area`, `tcbufferseqset_traversed_area` | not run |
| `shortestline_tcbuffer_geo` | meos_cbuffer_dist | array union, unary union | `tcbufferseq_traversed_area`, `tcbufferseqset_traversed_area` | not run |
| `adisjoint_tcbuffer_geo` | meos_cbuffer_rel_ever | array union, unary union | `tcbufferseq_traversed_area`, `tcbufferseqset_traversed_area` | not run |
| `adwithin_tcbuffer_geo` | meos_cbuffer_rel_ever | array union, unary union | `tcbufferseq_traversed_area`, `tcbufferseqset_traversed_area` | not run |
| `aintersects_tcbuffer_geo` | meos_cbuffer_rel_ever | array union, unary union | `tcbufferseq_traversed_area`, `tcbufferseqset_traversed_area` | not run |
| `atouches_tcbuffer_geo` | meos_cbuffer_rel_ever | array union, difference, intersection, unary union | `geo_values_collect`, `geom_array_union`, `geom_difference2d`, `geom_intersection2d`, `tcbufferseq_traversed_area`, `tcbufferseqset_traversed_area`, `tpointseq_linear_trajectory`, `tpointseqset_linear_trajectory` | not run |
| `edisjoint_tcbuffer_geo` | meos_cbuffer_rel_ever | array union, unary union | `tcbufferseq_traversed_area`, `tcbufferseqset_traversed_area` | not run |
| `edwithin_tcbuffer_geo` | meos_cbuffer_rel_ever | array union, unary union | `tcbufferseq_traversed_area`, `tcbufferseqset_traversed_area` | not run |
| `eintersects_tcbuffer_geo` | meos_cbuffer_rel_ever | array union, unary union | `tcbufferseq_traversed_area`, `tcbufferseqset_traversed_area` | answered every tested input (gateway_reach) |
| `etouches_tcbuffer_geo` | meos_cbuffer_rel_ever | array union, difference, intersection, unary union | `geo_values_collect`, `geom_array_union`, `geom_difference2d`, `geom_intersection2d`, `tcbufferseq_traversed_area`, `tcbufferseqset_traversed_area`, `tpointseq_linear_trajectory`, `tpointseqset_linear_trajectory` | not run |
| `tcontains_cbuffer_tcbuffer` | meos_cbuffer_rel_temp | array union | `geom_array_union` | answered every tested input (gateway_reach) |
| `tcontains_geo_tcbuffer` | meos_cbuffer_rel_temp | array union, difference, intersection, unary union | `geom_array_union`, `geom_difference2d`, `geom_intersection2d`, `tpointseq_linear_trajectory` | not run |
| `tcontains_tcbuffer_cbuffer` | meos_cbuffer_rel_temp | array union | `geom_array_union` | not run |
| `tcovers_cbuffer_tcbuffer` | meos_cbuffer_rel_temp | array union | `geom_array_union` | not run |
| `tcovers_geo_tcbuffer` | meos_cbuffer_rel_temp | array union, difference, intersection, unary union | `geom_array_union`, `geom_difference2d`, `geom_intersection2d`, `tpointseq_linear_trajectory` | not run |
| `tcovers_tcbuffer_cbuffer` | meos_cbuffer_rel_temp | array union | `geom_array_union` | not run |
| `tdisjoint_cbuffer_tcbuffer` | meos_cbuffer_rel_temp | array union, difference, intersection, unary union | `geom_array_union`, `geom_difference2d`, `geom_intersection2d`, `tcbufferseq_traversed_area`, `tpointseq_linear_trajectory` | not run |
| `tdisjoint_geo_tcbuffer` | meos_cbuffer_rel_temp | array union, difference, intersection, unary union | `geom_array_union`, `geom_difference2d`, `geom_intersection2d`, `tcbufferseq_traversed_area`, `tpointseq_linear_trajectory` | not run |
| `tdisjoint_tcbuffer_cbuffer` | meos_cbuffer_rel_temp | array union, difference, intersection, unary union | `geom_array_union`, `geom_difference2d`, `geom_intersection2d`, `tcbufferseq_traversed_area`, `tpointseq_linear_trajectory` | not run |
| `tdisjoint_tcbuffer_geo` | meos_cbuffer_rel_temp | array union, difference, intersection, unary union | `geom_array_union`, `geom_difference2d`, `geom_intersection2d`, `tcbufferseq_traversed_area`, `tpointseq_linear_trajectory` | not run |
| `tdwithin_geo_tcbuffer` | meos_cbuffer_rel_temp | array union, difference, intersection, unary union | `geom_array_union`, `geom_difference2d`, `geom_intersection2d`, `tcbufferseq_traversed_area`, `tpointseq_linear_trajectory` | not run |
| `tdwithin_tcbuffer_geo` | meos_cbuffer_rel_temp | array union, difference, intersection, unary union | `geom_array_union`, `geom_difference2d`, `geom_intersection2d`, `tcbufferseq_traversed_area`, `tpointseq_linear_trajectory` | answered every tested input (gateway_reach) |
| `tintersects_cbuffer_tcbuffer` | meos_cbuffer_rel_temp | array union, difference, intersection, unary union | `geom_array_union`, `geom_difference2d`, `geom_intersection2d`, `tcbufferseq_traversed_area`, `tpointseq_linear_trajectory` | not run |
| `tintersects_geo_tcbuffer` | meos_cbuffer_rel_temp | array union, difference, intersection, unary union | `geom_array_union`, `geom_difference2d`, `geom_intersection2d`, `tcbufferseq_traversed_area`, `tpointseq_linear_trajectory` | not run |
| `tintersects_tcbuffer_cbuffer` | meos_cbuffer_rel_temp | array union, difference, intersection, unary union | `geom_array_union`, `geom_difference2d`, `geom_intersection2d`, `tcbufferseq_traversed_area`, `tpointseq_linear_trajectory` | not run |
| `tintersects_tcbuffer_geo` | meos_cbuffer_rel_temp | array union, difference, intersection, unary union | `geom_array_union`, `geom_difference2d`, `geom_intersection2d`, `tcbufferseq_traversed_area`, `tpointseq_linear_trajectory` | answered every tested input (gateway_reach) |
| `ttouches_geo_tcbuffer` | meos_cbuffer_rel_temp | array union, difference, intersection, unary union | `geom_array_union`, `geom_difference2d`, `geom_intersection2d`, `tpointseq_linear_trajectory` | not run |
| `ttouches_tcbuffer_geo` | meos_cbuffer_rel_temp | array union, difference, intersection, unary union | `geom_array_union`, `geom_difference2d`, `geom_intersection2d`, `tpointseq_linear_trajectory` | not run |
| `tcbuffer_at_geom` | meos_cbuffer_restrict | array union, difference, intersection, unary union | `geom_array_union`, `geom_difference2d`, `geom_intersection2d`, `tcbufferseq_traversed_area`, `tpointseq_linear_trajectory` | answered every tested input (gateway_reach) |
| `tcbuffer_at_stbox` | meos_cbuffer_restrict | array union, difference, intersection, unary union | `geom_array_union`, `geom_difference2d`, `geom_intersection2d`, `tcbufferseq_traversed_area`, `tpointseq_linear_trajectory` | not run |
| `tcbuffer_minus_geom` | meos_cbuffer_restrict | array union, difference, intersection, unary union | `geom_array_union`, `geom_difference2d`, `geom_intersection2d`, `tcbufferseq_traversed_area`, `tpointseq_linear_trajectory` | not run |
| `tcbuffer_minus_stbox` | meos_cbuffer_restrict | array union, difference, intersection, unary union | `geom_array_union`, `geom_difference2d`, `geom_intersection2d`, `tcbufferseq_traversed_area`, `tpointseq_linear_trajectory` | not run |
| `tcbuffer_traversed_area` | meos_cbuffer_spatial_accessor | array union, unary union | `tcbufferseq_traversed_area`, `tcbufferseqset_traversed_area` | answered every tested input (gateway_reach) |
| `tgeo_convex_hull` | meos_geo_accessor | array union, unary union | `geo_values_collect`, `tpointseq_linear_trajectory`, `tpointseqset_linear_trajectory` | answered every tested input (gateway_reach) |
| `tgeo_traversed_area` | meos_geo_accessor | array union, unary union | `geo_values_collect` | **reaches GEOS** (temporal_reach, gateway_reach) |
| `geom_array_union` | meos_geo_base_spatial | array union | `geom_array_union` | **reaches GEOS** (arrunion_min, sweep) |
| `geom_difference2d` | meos_geo_base_spatial | array union, difference, intersection | `geom_array_union`, `geom_difference2d`, `geom_intersection2d` | **reaches GEOS** (coll_detail, sweep) |
| `geom_intersection2d` | meos_geo_base_spatial | array union, difference, intersection | `geom_array_union`, `geom_difference2d`, `geom_intersection2d` | **reaches GEOS** (coll_multi, coll_detail, sweep) |
| `geom_intersection2d_coll` | meos_geo_base_spatial | array union, difference, intersection | `geom_array_union`, `geom_difference2d`, `geom_intersection2d` | not run |
| `geom_unary_union` | meos_geo_base_spatial | array union, unary union | `(itself)` | **reaches GEOS** (coll_multi, sweep) |
| `tgeo_space_time_boxes` | meos_geo_bbox_split | array union, difference, intersection | `geom_array_union`, `geom_difference2d`, `geom_intersection2d` | not run |
| `acontains_geo_tgeo` | meos_geo_rel_ever | array union, unary union | `geo_values_collect`, `tpointseq_linear_trajectory`, `tpointseqset_linear_trajectory` | not run |
| `acontains_tgeo_geo` | meos_geo_rel_ever | array union, unary union | `geo_values_collect`, `tpointseq_linear_trajectory`, `tpointseqset_linear_trajectory` | not run |
| `acontains_tgeo_tgeo` | meos_geo_rel_ever | array union, unary union | `geo_values_collect`, `tpointseq_linear_trajectory`, `tpointseqset_linear_trajectory` | not run |
| `acovers_geo_tgeo` | meos_geo_rel_ever | array union, unary union | `geo_values_collect`, `geom_array_union`, `tpointseq_linear_trajectory`, `tpointseqset_linear_trajectory` | not run |
| `acovers_tgeo_geo` | meos_geo_rel_ever | array union, unary union | `geo_values_collect`, `geom_array_union`, `tpointseq_linear_trajectory`, `tpointseqset_linear_trajectory` | not run |
| `acovers_tgeo_tgeo` | meos_geo_rel_ever | array union, unary union | `geo_values_collect`, `tpointseq_linear_trajectory`, `tpointseqset_linear_trajectory` | not run |
| `adisjoint_geo_tgeo` | meos_geo_rel_ever | array union, unary union | `geo_values_collect`, `geom_array_union`, `tpointseq_linear_trajectory`, `tpointseqset_linear_trajectory` | not run |
| `adisjoint_tgeo_geo` | meos_geo_rel_ever | array union, unary union | `geo_values_collect`, `geom_array_union`, `tpointseq_linear_trajectory`, `tpointseqset_linear_trajectory` | not run |
| `adwithin_geo_tgeo` | meos_geo_rel_ever | array union, unary union | `geo_values_collect`, `geom_array_union`, `tpointseq_linear_trajectory`, `tpointseqset_linear_trajectory` | not run |
| `adwithin_tgeo_geo` | meos_geo_rel_ever | array union, unary union | `geo_values_collect`, `geom_array_union`, `tpointseq_linear_trajectory`, `tpointseqset_linear_trajectory` | not run |
| `aintersects_geo_tgeo` | meos_geo_rel_ever | array union, unary union | `geo_values_collect`, `geom_array_union`, `tpointseq_linear_trajectory`, `tpointseqset_linear_trajectory` | not run |
| `aintersects_tgeo_geo` | meos_geo_rel_ever | array union, unary union | `geo_values_collect`, `geom_array_union`, `tpointseq_linear_trajectory`, `tpointseqset_linear_trajectory` | not run |
| `atouches_geo_tpoint` | meos_geo_rel_ever | array union, difference, intersection, unary union | `geo_values_collect`, `geom_array_union`, `geom_difference2d`, `geom_intersection2d`, `tpointseq_linear_trajectory`, `tpointseqset_linear_trajectory` | not run |
| `econtains_geo_tgeo` | meos_geo_rel_ever | array union, unary union | `geo_values_collect`, `tpointseq_linear_trajectory`, `tpointseqset_linear_trajectory` | answered every tested input (gateway_reach) |
| `econtains_tgeo_geo` | meos_geo_rel_ever | array union, unary union | `geo_values_collect`, `tpointseq_linear_trajectory`, `tpointseqset_linear_trajectory` | not run |
| `econtains_tgeo_tgeo` | meos_geo_rel_ever | array union, unary union | `geo_values_collect`, `tpointseq_linear_trajectory`, `tpointseqset_linear_trajectory` | not run |
| `ecovers_geo_tgeo` | meos_geo_rel_ever | array union, unary union | `geo_values_collect`, `geom_array_union`, `tpointseq_linear_trajectory`, `tpointseqset_linear_trajectory` | not run |
| `ecovers_tgeo_geo` | meos_geo_rel_ever | array union, unary union | `geo_values_collect`, `geom_array_union`, `tpointseq_linear_trajectory`, `tpointseqset_linear_trajectory` | not run |
| `ecovers_tgeo_tgeo` | meos_geo_rel_ever | array union, unary union | `geo_values_collect`, `tpointseq_linear_trajectory`, `tpointseqset_linear_trajectory` | not run |
| `edisjoint_geo_tgeo` | meos_geo_rel_ever | array union, unary union | `geo_values_collect`, `geom_array_union`, `tpointseq_linear_trajectory`, `tpointseqset_linear_trajectory` | not run |
| `edisjoint_tgeo_geo` | meos_geo_rel_ever | array union, unary union | `geo_values_collect`, `geom_array_union`, `tpointseq_linear_trajectory`, `tpointseqset_linear_trajectory` | not run |
| `edwithin_geo_tgeo` | meos_geo_rel_ever | array union, unary union | `geo_values_collect`, `geom_array_union`, `tpointseq_linear_trajectory`, `tpointseqset_linear_trajectory` | not run |
| `edwithin_tgeo_geo` | meos_geo_rel_ever | array union, unary union | `geo_values_collect`, `geom_array_union`, `tpointseq_linear_trajectory`, `tpointseqset_linear_trajectory` | not run |
| `eintersects_geo_tgeo` | meos_geo_rel_ever | array union, unary union | `geo_values_collect`, `geom_array_union`, `tpointseq_linear_trajectory`, `tpointseqset_linear_trajectory` | not run |
| `eintersects_tgeo_geo` | meos_geo_rel_ever | array union, unary union | `geo_values_collect`, `geom_array_union`, `tpointseq_linear_trajectory`, `tpointseqset_linear_trajectory` | answered every tested input (gateway_reach) |
| `etouches_geo_tpoint` | meos_geo_rel_ever | array union, difference, intersection, unary union | `geo_values_collect`, `geom_array_union`, `geom_difference2d`, `geom_intersection2d`, `tpointseq_linear_trajectory`, `tpointseqset_linear_trajectory` | not run |
| `tcontains_geo_tgeo` | meos_geo_rel_temp | array union, difference, intersection, unary union | `geom_array_union`, `geom_difference2d`, `geom_intersection2d`, `tpointseq_linear_trajectory` | answered every tested input (gateway_reach) |
| `tcovers_geo_tgeo` | meos_geo_rel_temp | array union, difference, intersection, unary union | `geom_array_union`, `geom_difference2d`, `geom_intersection2d`, `tpointseq_linear_trajectory` | not run |
| `tdisjoint_geo_tgeo` | meos_geo_rel_temp | array union, difference, intersection, unary union | `geom_array_union`, `geom_difference2d`, `geom_intersection2d`, `tpointseq_linear_trajectory` | not run |
| `tdisjoint_tgeo_geo` | meos_geo_rel_temp | array union, difference, intersection, unary union | `geom_array_union`, `geom_difference2d`, `geom_intersection2d`, `tpointseq_linear_trajectory` | not run |
| `tdwithin_geo_tgeo` | meos_geo_rel_temp | array union, difference, intersection, unary union | `geom_array_union`, `geom_difference2d`, `geom_intersection2d`, `tpointseq_linear_trajectory` | not run |
| `tdwithin_tgeo_geo` | meos_geo_rel_temp | array union, difference, intersection, unary union | `geom_array_union`, `geom_difference2d`, `geom_intersection2d`, `tpointseq_linear_trajectory` | not run |
| `tintersects_geo_tgeo` | meos_geo_rel_temp | array union, difference, intersection, unary union | `geom_array_union`, `geom_difference2d`, `geom_intersection2d`, `tpointseq_linear_trajectory` | not run |
| `tintersects_tgeo_geo` | meos_geo_rel_temp | array union, difference, intersection, unary union | `geom_array_union`, `geom_difference2d`, `geom_intersection2d`, `tpointseq_linear_trajectory` | answered every tested input (gateway_reach) |
| `ttouches_geo_tgeo` | meos_geo_rel_temp | array union, difference, intersection, unary union | `geom_array_union`, `geom_difference2d`, `geom_intersection2d`, `tpointseq_linear_trajectory` | not run |
| `ttouches_tgeo_geo` | meos_geo_rel_temp | array union, difference, intersection, unary union | `geom_array_union`, `geom_difference2d`, `geom_intersection2d`, `tpointseq_linear_trajectory` | not run |
| `tgeo_at_geom` | meos_geo_restrict | array union, difference, intersection, unary union | `geom_array_union`, `geom_difference2d`, `geom_intersection2d`, `tpointseq_linear_trajectory` | **reaches GEOS** (gateway_reach) |
| `tgeo_at_stbox` | meos_geo_restrict | array union, difference, intersection | `geom_array_union`, `geom_difference2d`, `geom_intersection2d` | answered every tested input (gateway_reach) |
| `tgeo_minus_geom` | meos_geo_restrict | array union, difference, intersection, unary union | `geom_array_union`, `geom_difference2d`, `geom_intersection2d`, `tpointseq_linear_trajectory` | answered every tested input but two, where it returns no value without an error (gateway_reach) |
| `tgeo_minus_stbox` | meos_geo_restrict | array union, difference, intersection | `geom_array_union`, `geom_difference2d`, `geom_intersection2d` | not run |
| `tpoint_at_geom` | meos_geo_restrict | array union, difference, intersection, unary union | `geom_array_union`, `geom_difference2d`, `geom_intersection2d`, `tpointseq_linear_trajectory` | answered every tested input (gateway_reach) |
| `tpoint_minus_geom` | meos_geo_restrict | array union, difference, intersection, unary union | `geom_array_union`, `geom_difference2d`, `geom_intersection2d`, `tpointseq_linear_trajectory` | not run |
| `tgeo_space_boxes` | meos_geo_tile | array union, difference, intersection | `geom_array_union`, `geom_difference2d`, `geom_intersection2d` | not run |
| `tgeo_space_split` | meos_geo_tile | array union, difference, intersection | `geom_array_union`, `geom_difference2d`, `geom_intersection2d` | not run |
| `tgeo_space_time_split` | meos_geo_tile | array union, difference, intersection | `geom_array_union`, `geom_difference2d`, `geom_intersection2d` | not run |
| `tnpoint_trajectory` | meos_npoint_accessor | array union | `geom_array_union` | not run |
| `nad_tnpoint_geo` | meos_npoint_dist | array union | `geom_array_union` | not run |
| `nad_tnpoint_npoint` | meos_npoint_dist | array union | `geom_array_union` | not run |
| `nad_tnpoint_stbox` | meos_npoint_dist | array union | `geom_array_union` | not run |
| `shortestline_tnpoint_geo` | meos_npoint_dist | array union | `geom_array_union` | not run |
| `shortestline_tnpoint_npoint` | meos_npoint_dist | array union | `geom_array_union` | not run |
| `tnpoint_at_geom` | meos_npoint_restrict | array union, difference, intersection, unary union | `geom_array_union`, `geom_difference2d`, `geom_intersection2d`, `tpointseq_linear_trajectory` | not run |
| `tnpoint_at_stbox` | meos_npoint_restrict | array union, difference, intersection | `geom_array_union`, `geom_difference2d`, `geom_intersection2d` | not run |
| `tnpoint_minus_geom` | meos_npoint_restrict | array union, difference, intersection, unary union | `geom_array_union`, `geom_difference2d`, `geom_intersection2d`, `tpointseq_linear_trajectory` | not run |
| `tnpoint_minus_stbox` | meos_npoint_restrict | array union, difference, intersection | `geom_array_union`, `geom_difference2d`, `geom_intersection2d` | not run |
| `tpose_trajectory` | meos_pose_accessor | array union, unary union | `geo_values_collect`, `tpointseq_linear_trajectory`, `tpointseqset_linear_trajectory` | not run |
| `nad_tpose_geo` | meos_pose_dist | array union, unary union | `geo_values_collect`, `tpointseq_linear_trajectory`, `tpointseqset_linear_trajectory` | not run |
| `nad_tpose_pose` | meos_pose_dist | array union, unary union | `geo_values_collect`, `tpointseq_linear_trajectory`, `tpointseqset_linear_trajectory` | not run |
| `nad_tpose_stbox` | meos_pose_dist | array union, unary union | `geo_values_collect`, `tpointseq_linear_trajectory`, `tpointseqset_linear_trajectory` | not run |
| `shortestline_tpose_geo` | meos_pose_dist | array union, unary union | `geo_values_collect`, `tpointseq_linear_trajectory`, `tpointseqset_linear_trajectory` | not run |
| `shortestline_tpose_pose` | meos_pose_dist | array union, unary union | `geo_values_collect`, `tpointseq_linear_trajectory`, `tpointseqset_linear_trajectory` | not run |
| `tpose_at_geom` | meos_pose_restrict | array union, difference, intersection, unary union | `geom_array_union`, `geom_difference2d`, `geom_intersection2d`, `tpointseq_linear_trajectory` | not run |
| `tpose_at_stbox` | meos_pose_restrict | array union, difference, intersection | `geom_array_union`, `geom_difference2d`, `geom_intersection2d` | not run |
| `tpose_minus_geom` | meos_pose_restrict | array union, difference, intersection, unary union | `geom_array_union`, `geom_difference2d`, `geom_intersection2d`, `tpointseq_linear_trajectory` | not run |
| `tpose_minus_stbox` | meos_pose_restrict | array union, difference, intersection | `geom_array_union`, `geom_difference2d`, `geom_intersection2d` | not run |
| `raster_tile_value_array` | meos_raster | array union | `geom_array_union` | not run |
| `trgeometry_convex_hull` | meos_rgeo_accessor | array union, unary union | `trgeo_geoms_merge` | answered every tested input (trgeo_reach) |
| `trgeometry_length` | meos_rgeo_accessor | array union, unary union | `tpointseq_linear_trajectory` | not run |
| `trgeometry_traversed_area` | meos_rgeo_accessor | array union, unary union | `trgeo_geoms_merge` | **reaches GEOS** (trgeo_reach) |
| `trgeometry_append_tsequence` | meos_rgeo_modif | array union | `geom_array_union` | not run |
| `trgeometry_merge` | meos_rgeo_modif | array union | `geom_array_union` | not run |
| `trgeometry_merge_array` | meos_rgeo_modif | array union | `geom_array_union` | not run |
| `acontains_geo_trgeometry` | meos_rgeo_rel_ever | array union, unary union | `trgeo_geoms_merge` | not run |
| `acovers_geo_trgeometry` | meos_rgeo_rel_ever | array union, unary union | `trgeo_geoms_merge` | not run |
| `acovers_trgeometry_geo` | meos_rgeo_rel_ever | array union, unary union | `trgeo_geoms_merge` | not run |
| `adisjoint_trgeometry_geo` | meos_rgeo_rel_ever | array union, unary union | `trgeo_geoms_merge` | not run |
| `aintersects_trgeometry_geo` | meos_rgeo_rel_ever | array union, unary union | `trgeo_geoms_merge` | not run |
| `atouches_trgeometry_geo` | meos_rgeo_rel_ever | array union, unary union | `trgeo_geoms_merge` | not run |
| `econtains_geo_trgeometry` | meos_rgeo_rel_ever | array union, unary union | `trgeo_geoms_merge` | not run |
| `ecovers_geo_trgeometry` | meos_rgeo_rel_ever | array union, unary union | `trgeo_geoms_merge` | not run |
| `ecovers_trgeometry_geo` | meos_rgeo_rel_ever | array union, unary union | `trgeo_geoms_merge` | not run |
| `edisjoint_trgeometry_geo` | meos_rgeo_rel_ever | array union, unary union | `trgeo_geoms_merge` | not run |
| `eintersects_trgeometry_geo` | meos_rgeo_rel_ever | array union, unary union | `trgeo_geoms_merge` | not run |
| `etouches_trgeometry_geo` | meos_rgeo_rel_ever | array union, unary union | `trgeo_geoms_merge` | not run |
| `temporal_append_tsequence` | meos_temporal_modif | array union | `geom_array_union` | not run |
| `temporal_insert` | meos_temporal_modif | array union | `geom_array_union` | not run |
| `temporal_merge` | meos_temporal_modif | array union | `geom_array_union` | **reaches GEOS**, and reports the refusal as an error (tmerge_min, merge_sweep) |
| `temporal_merge_array` | meos_temporal_modif | array union | `geom_array_union` | not run |
| `temporal_update` | meos_temporal_modif | array union | `geom_array_union` | not run |
| `atouches_tpoint_geo` | meos_temporal_spatial_rel_ever | array union, difference, intersection, unary union | `geo_values_collect`, `geom_array_union`, `geom_difference2d`, `geom_intersection2d`, `tpointseq_linear_trajectory`, `tpointseqset_linear_trajectory` | not run |
| `etouches_tpoint_geo` | meos_temporal_spatial_rel_ever | array union, difference, intersection, unary union | `geo_values_collect`, `geom_array_union`, `geom_difference2d`, `geom_intersection2d`, `tpointseq_linear_trajectory`, `tpointseqset_linear_trajectory` | not run |
