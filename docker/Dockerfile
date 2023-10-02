ARG POSTGRES_VERSION
ARG POSTGIS_VERSION

FROM postgis/postgis:$POSTGRES_VERSION-$POSTGIS_VERSION

# Configuration Parameters
LABEL maintainer="MobilityDB Project - https://github.com/MobilityDB/MobilityDB"
ENV POSTGRES_DB=mobilitydb
ENV POSTGRES_USER=docker
ENV POSTGRES_PASSWORD=docker
ENV MOBILITYDB_VERSION $MOBDB_VERSION

# Fix the Release file expired problem
RUN echo "Acquire::Check-Valid-Until \"false\";\nAcquire::Check-Date \"false\";" | cat > /etc/apt/apt.conf.d/10no--check-valid-until


# Install Prerequisites
RUN apt-get update \
 && apt-get install -y \
    build-essential \
    cmake \
    git \
    libproj-dev \
    g++ \
    wget \
    autoconf \
    autotools-dev \
    libgeos-dev \
    libpq-dev \
    libproj-dev \
    libjson-c-dev \
    protobuf-c-compiler \
    xsltproc \
    libgsl-dev \
    libgslcblas0 \
    postgresql-server-dev-${PG_MAJOR} \
  && rm -rf /var/lib/apt/lists/*

# Install MobilityDB
RUN wget -O MobilityDB.tar.gz "https://github.com/MobilityDB/MobilityDB/archive/v1.0.tar.gz" \
 && mkdir -p /usr/local/src/MobilityDB \
 && tar \
        --extract \
        --file MobilityDB.tar.gz \
        --directory /usr/local/src/MobilityDB \
        --strip-components 1 \
 && rm MobilityDB.tar.gz
RUN mkdir /usr/local/src/MobilityDB/build
RUN cd /usr/local/src/MobilityDB/build && \
    cmake .. && \
    make -j$(nproc) && \
    make install

RUN rm /docker-entrypoint-initdb.d/10_postgis.sh
COPY ./docker/initdb-mobilitydb.sh /docker-entrypoint-initdb.d/mobilitydb.sh
RUN chmod +x /docker-entrypoint-initdb.d/mobilitydb.sh
