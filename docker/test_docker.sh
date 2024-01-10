#!/usr/bin/env bash
set -Eeo pipefail
# shellcheck disable=SC2043

# mobilitydb local build matrix
for mobilitydb_ver in develop ; do
  for pg_major in 16 15 14 13 12 ; do
    for postgis_ver in 3.4 ; do
        docker build --pull --progress=plain  \
            --build-arg POSTGRES_VERSION=$pg_major \
            --build-arg POSTGIS_VERSION=$postgis_ver \
            --build-arg MOBILITYDB_TAG=$mobilitydb_ver \
            -t testmobilitydb:${pg_major}-${postgis_ver}-${mobilitydb_ver} \
            -f docker/Dockerfile\
            .
    done
  done
done
