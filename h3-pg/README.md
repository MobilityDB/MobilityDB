# h3-pg: Uber's H3 Hexagonal Hierarchical Geospatial Indexing System in PostgreSQL

[![test-linux](https://github.com/zachasme/h3-pg/workflows/test-linux/badge.svg)](https://github.com/zachasme/h3-pg/actions)
[![test-macos](https://github.com/zachasme/h3-pg/workflows/test-macos/badge.svg)](https://github.com/zachasme/h3-pg/actions/workflows/test-macos.yml)
[![test-windows](https://github.com/zachasme/h3-pg/workflows/test-windows/badge.svg)](https://github.com/zachasme/h3-pg/actions/workflows/test-windows.yml)
[![License](https://img.shields.io/badge/License-Apache2-blue.svg)](LICENSE)

This library provides PostgreSQL bindings for the [H3 Core Library](https://github.com/uber/h3). For API reference, please see the [H3 Documentation](https://uber.github.io/h3).

Developed in collaboration with [Scandinavian Highlands](http://www.scandinavian-highlands.com).

## Binary distributions
These don't require you have the development headers or cmake installed.

### Debian/Ubuntu (Ubuntu 22.04 LTS (Jammy Jellyfish) +)
Replace 16 with your postgresql version.
Refer to [PGDG Ubuntu](https://www.postgresql.org/download/linux/ubuntu/) and [PGDG Debian](https://www.postgresql.org/download/linux/debian/) for installing PostgreSQL.
More details about the various packages at https://apt.postgresql.org
```
sudo apt install postgresql-16-h3
```

### Redhat Derivatives (Rocky / EL 8+ / Fedora 37+)
Replace 16 with your postgresql version
Refer to [PGDG Redhat Derivatives](https://www.postgresql.org/download/linux/redhat/) for installing PostgreSQL.
More details about the various packages at https://yum.postgresql.org
```
sudo yum install h3-pg_16
```

### Windows

Included as part of PostGIS Bundle 3.3+ for PostgreSQL 11-16
Details: [postgis.net windows](https://postgis.net/documentation/getting_started/install_windows/released_versions/)  
Works with [PostgreSQL Windows](https://www.postgresql.org/download/windows/) and PostGIS bundle is accessible via the included Application Stackbuilder.

## Compiling Prerequisites

- PostgreSQL 11+ (_including server headers_, e.g. `postgresql-server-dev-14`)
- C compiler (e.g., `gcc`)
- [CMake](https://cmake.org/) 3.20+
- GNU Make

## Quick Overview

If the prerequisites are met you can use the [PGXN Client](docs/pgxnclient.md) to download, build, and install, e.g.:

```shell
$ pgxn install h3
$ pgxn load h3
$ psql
=# SELECT h3_latlng_to_cell(POINT('37.3615593,-122.0553238'), 5);
  h3_latlng_to_cell
-----------------
 85e35e73fffffff
(1 row)
```

(You can install a specific version using `pgxn install 'h3=3.7.2'` and `pgxn load 'h3=3.7.2'` for example)

See [Building](#building) for other installation methods.

## Usage

> :tada: **Note:** The following usage docs apply to **H3 v4**, which was released on August 23, 2022.
>
> - For v3 docs, [see the latest v3.x.x release](https://github.com/zachasme/h3-pg/blob/v3.7.2/README.md).
> - For breaking changes in v4, [see the CHANGELOG](./CHANGELOG.md). In particular, most [function names have changed](https://h3geo.org/docs/library/migration-3.x/functions).

Generally, all functions have been renamed from camelCase in H3 to snake\_case in SQL.

See [API reference](https://pgxn.org/dist/h3/docs/api.html) for all provided functions.

## Building

```bash
# Generate native build system
cmake -B build -DCMAKE_BUILD_TYPE=Release

# Build extension(s)
cmake --build build

# Install extensions (might require sudo)
cmake --install build --component h3-pg
```

## Contributing

Pull requests and GitHub issues are welcome. Please include tests for new work. Please note that the purpose of this extension is to expose the API of the H3 Core library, so we will rarely accept new features that are not part of that API. New proposed feature work is more appropriate in the core C library or in a new extension that depends on h3-pg.

See [Development](docs/development.md).

## License

This project is released under the [Apache 2.0 License](LICENSE.md).
