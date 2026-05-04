# Changelog

All notable changes to this project will be documented in this file.

Critical bugfixes or breaking changes are marked using a warning symbol: ⚠️

_The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/)._

## Versioning

The H3 core library adheres to [Semantic Versioning](http://semver.org/).
H3-pg has a `major.minor.patch` version scheme. The major and minor version
numbers of H3-pg are the major and minor version of the bound core library,
respectively. The patch version is incremented independently of the core
library.

Because H3-pg is versioned in lockstep with the H3 core library, please
avoid adding features or APIs which do not map onto the
[H3 core API](https://uber.github.io/h3/#/documentation/api-reference/).

## [Unreleased]

<details>
  <summary>
    Changes that have landed in master but are not yet released.
    Click to see more.
  </summary>

</details>

## [4.2.3] - 2025-06-24
- Add `h3_get_resolution_from_tile_zoom` (see [#176], thanks [@sleeping-h])
- Alter function names containing `lat_lng` to `latlng` in order to align with other SQL bindings (see [#177])
- Add support for PostgreSQL 18 ([#179], see thanks [@devrimgunduz])

## [4.2.2] - 2025-02-10
- More upstream copy/paste in support of Debian package (see [#169], thanks [@df7cb])

## [4.2.1] - 2025-02-04

- Copy `h3Index.h` from upstream in support of Debian package (see [#169], thanks [@df7cb])

## [4.2.0] - 2025-01-17

- Bump `h3` to `v4.2.0`
- Add `h3_polygon_to_cells_experimental` (see [#159], thanks [@jmealo])
- Add experimental SP-GIST operator class (see [#43], thanks [@BielStela])
- Fix for MacOS in nixpkgs / NixOS (see [#141], thanks [@wolfgangwalther])

## [4.1.4] - 2024-11-06

- Fix library extension on macOS for PostgreSQL >= 16 (see [#140], thanks [@bayandin])
- Add support for binary I/O (see [#160], thanks [@robertozimek])

## [4.1.3] - 2023-07-26

- Add `geometry @ int` and `geography @ int` operators, as shortcuts for `h3_lat_lng_to_cell`.
- Explain PostGIS SRID expectations (see [#131], thanks [@rustprooflabs])

## [4.1.2] - 2023-02-08

- *Actually* fix `h3_postgis` upgrade path (see [#117], thanks [@mngr777])

## [4.1.1] - 2023-02-07

- Add `postgis_raster` integration (see [#112], thanks [@mngr777])
- Fix `h3_postgis` upgrade path (see [#117], thanks [@mngr777])

## [4.1.0] - 2023-01-18

- Bump `h3` to `v4.1.0`
- Add bindings for `h3_cell_to_child_pos` and `h3_child_pos_to_cell`.
- Use CMake for entire build (see [#70])
- Add helper to fix pass-by-value migration (see [#111])
- Allow distance operator `<->` to work for cells at different resolutions (using center child).

## [4.0.3] - 2022-11-04

- Add BRIN operator class (see [#97], thanks [@mngr777])
- Split PostGIS multipolygons by 180th meridian (see [#90], thanks [@mngr777])
- Add aggregate functions for `h3_postgis` (see [#91], thanks [@mngr777])
- Add recursive `h3_grid_path_cells` (see [#93], thanks [@mngr777])

## [4.0.2] - 2022-09-19

- Fix broken function renames (see [#87], thanks [@Kompza], [@mngr777])

## [4.0.1] - 2022-09-19

- Bump `h3` to `v4.0.1` (was locked on `rc5` for the previous release)
- Add compile flag support check (see [#78], thanks [@mngr777])
- Split polygons by 180th meridian (see [#76], thanks [@mngr777])

## [4.0.0] - 2022-08-24

- ⚠️ Update `h3` core library to `v4.0.0`. Almost all functions have been renamed to align with `h3` core `v4`. Please take care when updating. You can see a [list of changed function names](https://h3geo.org/docs/library/migration-3.x/functions) in the core library documentation.
- ⚠️ The PostGIS functions has been extracted into a separate extension `h3_postgis`. Install using `CREATE EXTENSION h3_postgis;`.
- Enable link time optimization (see [#75], thanks [@mngr777])
- Handle non-polygons for `h3_polyfill` (see [#55], thanks [@Lokks])
- Take advantage of the new v4 error codes (fixes [#71], thanks [@kalenikaliaksandr])

## [3.7.2] - 2022-04-13

- Allow `NULL` in `holes` array for `h3_polyfill` (see [#64], thanks [@mngr777])
- Allow >1Gb memory allocations for `h3_polyfill` (see [#65], thanks [@mngr777])

## [3.7.1] - 2021-06-23

- Update `h3` core library to `v3.7.1`

## [3.7.0] - 2020-09-30

- ⚠️ Default unit for `h3_hex_area` and `h3_edge_length` changed to kilometers
- Update `h3` core library to `v3.7.0`
- Add `h3_point_dist` and `h3_exact_edge_length` bindings
- Add distance operator `<->`
- Fix `h3_to_geography` and `h3_to_geometry` refering to removed functions if extension was upgraded from pre-1.0
- Add optional input validation in geoToH3 (see [#41], thanks [@trylinka])
- Support unit as string argument in `h3_hex_area` and `h3_edge_length` (and flag previous version for deprecation)

## [3.6.5] - 2020-08-14

- Add support for partitioning by hash (see [#37], thanks [@abelvm])
- Fix difference in function flags between fresh install and upgrades (see [#38], thanks [@abelvm])

## [3.6.4] - 2020-06-29

- Update `h3` core library to `v3.6.4`

## [3.6.3] - 2020-04-08

- Build `h3` core using release flag for 2x/3x performance (see [#23], thanks [@komzpa])

## [3.6.2] - 2020-04-07

**This update will corrupt your h3indexes unless your are using a 32-bit build of PostgreSQL (see [#31]). If you upgrade to `4.0.4` you can use the function `h3_pg_migrate_pass_by_reference(h3index)` to retrieve your old h3 cells. See [#111].**

- Add parallel safety flags to PostGIS functions (see [#19], thanks [@komzpa])
- Add B-Tree sort support (see [#24], thanks [@komzpa])
- ⚠️ Make type `h3index` pass-by-value on supported systems (see [#22], [#26], thanks [@komzpa])
- Update `h3` core library to `v3.6.3`

## [3.6.1] - 2019-12-09

- Add `&&`, `@>` and `<@` operators for overlaps, contains and contained by respectively
- Fix PostgreSQL 12 build (see [#18], thanks [@komzpa])
- Update `h3` core library to `v3.6.1`

## [3.6.0] - 2019-10-07

- Add support for `bigint` cast (see [#9], thanks [@kmacdough])
- Add `h3_to_center_child` binding
- Add `h3_get_pentagon_indexes` binding
- Update `h3` core library to `v3.6.0`

## [3.5.0] - 2019-08-01

- Add `h3_get_faces` binding
- ⚠️ Replace `h3_hex_area_m2` and `h3_hex_area_km2` with `h3_hex_area`
- ⚠️ Replace `h3_edge_length_m` and `h3_edge_length_km` with `h3_edge_length`
- ⚠️ Remove `hex_range`, `hex_ranges` and `hex_range_distances`
- Remove `h3` core library version check, since we know which version we are linking
- Fix PostgreSQL 12 build (see [#4], thanks [@komzpa])
- Update `h3` core library to `v3.5.0`

## [3.4.1] - 2019-06-14

- Fix `abs` warning

## [3.4.0] - 2019-06-13

- ⚠️ Remove degree/radian conversion helpers (in favor of built-in RADIANS/DEGREES)

## [1.0.6] - 2019-06-03

- Update `h3` core library to `v3.4.4`

## [1.0.5] - 2019-02-15

- Fix update path

## [1.0.4] - 2019-02-15

- Fix `polyfill` for polygon with multiple holes

## [1.0.3] - 2019-01-27

- Fix update path

## [1.0.2] - 2019-01-27

- Remove git tag check in `distribute` makefile target, since it causes error on pgxn install

## [1.0.1] - 2019-01-27

- Remove usage of `FALSE` instead of 0 in conditional in `ASSERT` macro

## [1.0.0] - 2019-01-27

- Add `h3_get_extension_version()`
- Add hash operator class, now `WHERE IN` works
- ⚠️ Replace `h3_basecells` with `h3_get_res_0_indexes`
- ⚠️ Rename all functions with double `h3_h3_` prefix to use single `h3_` prefix
- ⚠️ Remove `h3_haversine_distance` function
- ⚠️ Change Makefile such that the `h3` core library is cloned, built and statically linked
- Test that upgrade path has same result as fresh install

## [0.4.0] - 2019-01-12

- Add `h3_line` binding
- Fix `h3_h3_to_children_slow`

## [0.3.2] - 2019-01-08

- ⚠️ Fix `btree` operator class indexing

## [0.3.1] - 2018-12-17

- Add `extend` flag to `h3_h3_to_geo_boundary` such that polygons are not wrapped at antimeridian

## 0.3.0 - 2018-12-11

- Initial public release

[unreleased]: https://github.com/zachasme/h3-pg/compare/v4.2.3...HEAD
[4.2.3]: https://github.com/zachasme/h3-pg/compare/v4.2.2...v4.2.3
[4.2.2]: https://github.com/zachasme/h3-pg/compare/v4.2.1...v4.2.2
[4.2.1]: https://github.com/zachasme/h3-pg/compare/v4.2.0...v4.2.1
[4.2.0]: https://github.com/zachasme/h3-pg/compare/v4.1.4...v4.2.0
[4.1.4]: https://github.com/zachasme/h3-pg/compare/v4.1.3...v4.1.4
[4.1.3]: https://github.com/zachasme/h3-pg/compare/v4.1.2...v4.1.3
[4.1.2]: https://github.com/zachasme/h3-pg/compare/v4.1.1...v4.1.2
[4.1.1]: https://github.com/zachasme/h3-pg/compare/v4.1.0...v4.1.1
[4.1.0]: https://github.com/zachasme/h3-pg/compare/v4.0.3...v4.1.0
[4.0.3]: https://github.com/zachasme/h3-pg/compare/v4.0.2...v4.0.3
[4.0.2]: https://github.com/zachasme/h3-pg/compare/v4.0.1...v4.0.2
[4.0.1]: https://github.com/zachasme/h3-pg/compare/v4.0.0...v4.0.1
[4.0.0]: https://github.com/zachasme/h3-pg/compare/v3.7.2...v4.0.0
[3.7.2]: https://github.com/zachasme/h3-pg/compare/v3.7.1...v3.7.2
[3.7.1]: https://github.com/zachasme/h3-pg/compare/v3.7.0...v3.7.1
[3.7.0]: https://github.com/zachasme/h3-pg/compare/v3.6.5...v3.7.0
[3.6.5]: https://github.com/zachasme/h3-pg/compare/v3.6.4...v3.6.5
[3.6.4]: https://github.com/zachasme/h3-pg/compare/v3.6.3...v3.6.4
[3.6.3]: https://github.com/zachasme/h3-pg/compare/v3.6.2...v3.6.3
[3.6.2]: https://github.com/zachasme/h3-pg/compare/v3.6.1...v3.6.2
[3.6.1]: https://github.com/zachasme/h3-pg/compare/v3.6.0...v3.6.1
[3.6.0]: https://github.com/zachasme/h3-pg/compare/v3.5.0...v3.6.0
[3.5.0]: https://github.com/zachasme/h3-pg/compare/v3.4.1...v3.5.0
[3.4.1]: https://github.com/zachasme/h3-pg/compare/v3.4.0...v3.4.1
[3.4.0]: https://github.com/zachasme/h3-pg/compare/v1.0.6...v3.4.0
[1.0.6]: https://github.com/zachasme/h3-pg/compare/v1.0.5...v1.0.6
[1.0.5]: https://github.com/zachasme/h3-pg/compare/v1.0.4...v1.0.5
[1.0.4]: https://github.com/zachasme/h3-pg/compare/v1.0.3...v1.0.4
[1.0.3]: https://github.com/zachasme/h3-pg/compare/v1.0.2...v1.0.3
[1.0.2]: https://github.com/zachasme/h3-pg/compare/v1.0.1...v1.0.2
[1.0.1]: https://github.com/zachasme/h3-pg/compare/v1.0.0...v1.0.1
[1.0.0]: https://github.com/zachasme/h3-pg/compare/v0.4.0...v1.0.0
[0.4.0]: https://github.com/zachasme/h3-pg/compare/v0.3.2...v0.4.0
[0.3.2]: https://github.com/zachasme/h3-pg/compare/v0.3.1...v0.3.2
[0.3.1]: https://github.com/zachasme/h3-pg/compare/v0.3.0...v0.3.1
[#4]: https://github.com/zachasme/h3-pg/pull/4
[#9]: https://github.com/zachasme/h3-pg/pull/9
[#18]: https://github.com/zachasme/h3-pg/pull/18
[#19]: https://github.com/zachasme/h3-pg/pull/19
[#22]: https://github.com/zachasme/h3-pg/pull/22
[#23]: https://github.com/zachasme/h3-pg/issues/23
[#24]: https://github.com/zachasme/h3-pg/pull/24
[#26]: https://github.com/zachasme/h3-pg/pull/26
[#31]: https://github.com/zachasme/h3-pg/pull/31
[#37]: https://github.com/zachasme/h3-pg/issues/37
[#38]: https://github.com/zachasme/h3-pg/issues/38
[#41]: https://github.com/zachasme/h3-pg/issues/41
[#43]: https://github.com/zachasme/h3-pg/issues/43
[#55]: https://github.com/zachasme/h3-pg/issues/55
[#64]: https://github.com/zachasme/h3-pg/issues/64
[#65]: https://github.com/zachasme/h3-pg/pull/65
[#70]: https://github.com/zachasme/h3-pg/pull/70
[#71]: https://github.com/zachasme/h3-pg/issues/71
[#75]: https://github.com/zachasme/h3-pg/pull/75
[#76]: https://github.com/zachasme/h3-pg/pull/76
[#78]: https://github.com/zachasme/h3-pg/pull/78
[#87]: https://github.com/zachasme/h3-pg/pull/87
[#90]: https://github.com/zachasme/h3-pg/pull/90
[#91]: https://github.com/zachasme/h3-pg/pull/91
[#93]: https://github.com/zachasme/h3-pg/pull/93
[#97]: https://github.com/zachasme/h3-pg/pull/97
[#111]: https://github.com/zachasme/h3-pg/pull/111
[#112]: https://github.com/zachasme/h3-pg/pull/112
[#117]: https://github.com/zachasme/h3-pg/issues/117
[#131]: https://github.com/zachasme/h3-pg/pull/131
[#140]: https://github.com/zachasme/h3-pg/pull/140
[#141]: https://github.com/zachasme/h3-pg/pull/141
[#159]: https://github.com/zachasme/h3-pg/pull/159
[#160]: https://github.com/zachasme/h3-pg/pull/160
[#169]: https://github.com/zachasme/h3-pg/issues/169
[#176]: https://github.com/zachasme/h3-pg/pull/176
[#177]: https://github.com/zachasme/h3-pg/pull/177
[#179]: https://github.com/zachasme/h3-pg/issues/179
[@abelvm]: https://github.com/AbelVM
[@bayandin]: https://github.com/bayandin
[@BielStela]: https://github.com/BielStela
[@devrimgunduz]: https://github.com/devrimgunduz
[@df7cb]: https://github.com/df7cb
[@jmealo]: https://github.com/jmealo
[@kalenikaliaksandr]: https://github.com/kalenikaliaksandr
[@kmacdough]: https://github.com/kmacdough
[@komzpa]: https://github.com/Komzpa
[@lokks]: https://github.com/Lokks
[@mngr777]: https://github.com/mngr777
[@robertozimek]: https://github.com/robertozimek
[@rustprooflabs]: https://github.com/rustprooflabs
[@sleeping-h]: https://github.com/sleeping-h
[@trylinka]: https://github.com/trylinka
[@wolfgangwalther]: https://github.com/wolfgangwalther
