# Build stage
ARG POSTGRES_VERSION=17
ARG POSTGIS_VERSION=3.5

FROM postgis/postgis:${POSTGRES_VERSION}-${POSTGIS_VERSION} AS builder

# Install build dependencies
RUN apt-get update \
    && apt-get install -y --no-install-recommends \
        build-essential \
        cmake \
        libproj-dev \
        libjson-c-dev \
        libgsl-dev \
        libgeos-dev \
        postgresql-server-dev-${PG_MAJOR} \
    && rm -rf /var/lib/apt/lists/*

# Copy local MobilityDB source instead of downloading
WORKDIR /usr/local/src/MobilityDB
COPY . .

# Build MobilityDB
RUN mkdir -p build \
    && cd build \
    && rm -rf * \
    && cmake -DCMAKE_BUILD_TYPE=Release \
             -DNPOINT=on -DCBUFFER=on -DPOSE=on .. \
    && make -j$(nproc) \
    && make install

# Prepare initialization script
RUN cp docker/initdb-mobilitydb.sh /tmp/11_mobilitydb.sh

# Final stage
FROM postgis/postgis:${POSTGRES_VERSION}-${POSTGIS_VERSION}

# Configuration Parameters
LABEL maintainer="MobilityDB Project - https://github.com/MobilityDB/MobilityDB"
LABEL org.opencontainers.image.description="MobilityDB - An open source geospatial trajectory data management & analysis platform"
LABEL org.opencontainers.image.source="https://github.com/MobilityDB/MobilityDB"

# Install runtime dependencies only
RUN apt-get update \
    && apt-get install -y --no-install-recommends \
        libproj-dev \
        libjson-c-dev \
        libgsl-dev \
        libgeos-dev \
    && rm -rf /var/lib/apt/lists/*

# Copy MobilityDB installed files from builder
COPY --from=builder /usr/share/postgresql/${PG_MAJOR}/extension/mobilitydb* /usr/share/postgresql/${PG_MAJOR}/extension/
COPY --from=builder /usr/lib/postgresql/${PG_MAJOR}/lib/libMobilityDB* /usr/lib/postgresql/${PG_MAJOR}/lib/
COPY --from=builder /tmp/11_mobilitydb.sh /docker-entrypoint-initdb.d/11_mobilitydb.sh
