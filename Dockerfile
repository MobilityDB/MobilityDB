FROM kartoza/postgis:11.0-2.5
ENV POSTGRES_DBNAME=mobilitydb
ENV POSTGRES_MULTIPLE_EXTENSIONS=postgis,hstore,postgis_topology,mobilitydb
ENV EXTRA_CONF="shared_preload_libraries = 'pg_cron,postgis-2.5'\nmax_locks_per_transaction = 150\n"
WORKDIR /usr/local/src
ADD . MobilityDB
RUN apt-get update
RUN apt-get remove -y postgresql-11-postgis-3 postgresql-11-postgis-3-scripts
RUN apt-get install -y cmake build-essential libpq-dev liblwgeom-dev libproj-dev libjson-c-dev
RUN rm -f /etc/apt/sources.list.d/pgdg.list
RUN apt-get update
RUN apt-get install -y postgresql-server-dev-11
RUN mkdir /usr/local/src/MobilityDB/build
RUN cd /usr/local/src/MobilityDB/build && \
	cmake .. && \
	make && \
	make install

