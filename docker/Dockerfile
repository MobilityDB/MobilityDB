ARG POSTGRES_VERSION
ARG POSTGIS_VERSION

FROM postgis/postgis:$POSTGRES_VERSION-$POSTGIS_VERSION

# Configuration Parameters
LABEL maintainer="MobilityDB Project - https://github.com/MobilityDB/MobilityDB"

# Fix the Release file expired problem
RUN printf "Acquire::Check-Valid-Until \"false\";\nAcquire::Check-Date \"false\";" | cat > /etc/apt/apt.conf.d/10no--check-valid-until

ARG MOBILITYDB_TAG

# 1. Install Prerequisites
# 2. Get, build and install MobilityDB
# 3. Uninstall prerequisites
RUN apt-get update \
  && apt-get install -y --no-install-recommends \
    build-essential \
    cmake \
    wget \
    libproj-dev \
    libjson-c-dev \
    libgsl-dev \
    libgeos-dev \
    postgresql-server-dev-"${PG_MAJOR}" \
  && rm -rf /var/lib/apt/lists/* \
  && wget -O MobilityDB.tar.gz "https://github.com/MobilityDB/MobilityDB/archive/$MOBILITYDB_TAG.tar.gz" \
  && mkdir -p /usr/local/src/MobilityDB/build \
  && tar \
    --extract \
    --file MobilityDB.tar.gz \
    --directory /usr/local/src/MobilityDB \
    --strip-components 1 \
  && rm MobilityDB.tar.gz \
  && cd /usr/local/src/MobilityDB/build \
  && cmake .. \
  && make -j"$(nproc)" \
  && make install \
  && cp ../docker/initdb-mobilitydb.sh /docker-entrypoint-initdb.d/11_mobilitydb.sh \
  && cd / \
  && rm -rf /usr/local/src/MobilityDB \
  && apt-get purge -y --auto-remove \
    build-essential \
    cmake \
    wget \
    postgresql-server-dev-"${PG_MAJOR}"
