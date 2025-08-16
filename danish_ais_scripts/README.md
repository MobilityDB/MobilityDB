
Simple README
===================

A tiny guide for where to put the data (from the ZIP) and how to run the scripts.

1) Put the data in the right place
----------------------------------
Unzip your archive and place the two files exactly like this:

MobilityDB/
├─danish_ais_scripts/
    ├─ aisdk-2024-12-04.csv
    ├─ denmark-latest.osm.pbf
    ├─ script_compil.sh
    ├─ script_compil2.sh
    ├─ generate_trips.sql
    ├─ generate_harbors.sql
    ├─ generate_clusters_ais.sql
    ├─ generate_clusters_ais.py
    ├─ run_bench.py
    └─ memory_check.sh


2) One small edit (CSV path)
----------------------------
Open generate_trips.sql and point the COPY path to your CSV in data/, e.g.:

COPY "AISInput"(...)
FROM '/absolute/path/to/your/project/danish_ais_scripts/data/aisdk-2024-12-04.csv'
WITH (FORMAT csv, HEADER true);

Tip: you can also use a relative path like ./data/aisdk-2024-12-04.csv if you run psql from the same folder.


3) Requirements (quick)
-----------------------
- PostgreSQL + PostGIS
- MobilityDB (the scripts try to build/install it)
- osm2pgsql (for the .osm.pbf import)
- Python 3 with: psycopg2 (or psycopg2-binary), pandas, numpy, scikit-learn

Default DB creds used by the scripts:
- DB: danish_ais
- host: localhost
- port: 5432
- user: postgres
- password: 1234

(You can change these inside the scripts if needed.)


4) How to run
-------------
From inside danish_ais_scripts/:

# 1) Build MobilityDB, create DB, import OSM, and load initial SQL
bash script_compil.sh

# 2) psql -h localhost -U postgres -d danish_ais -f generate_clusters_ais.sql

# 3) Cluster trips (writes labels back into the DB):
python3 -m pip install psycopg2-binary pandas numpy scikit-learn
python3 generate_clusters_ais.py

# Optional benchmarking:
python3 run_bench.py        # execution-time logs
bash memory_check.sh        # memory usage (Linux)


5) Common issues
----------------
- COPY failed (CSV path): Fix the path in generate_trips.sql. Make sure the server can read it.
- "osm2pgsql" not found: Install it and re-run script_compil.sh / script_compil2.sh.
- Password prompts: Set PGPASSWORD=1234 in your shell or edit/remove -W in script_compil2.sh.
- Empty clusters/trips: Adjust filters in generate_harbors.sql (the "clusterId IN (...)" list) and re-run.
- DBSCAN all noise (-1): Tweak eps/min_samples in generate_clusters_ais.py or revisit normalization in generate_clusters_ais.sql.

That’s it — drop the two files into data/, fix the CSV path once, and run the scripts above.
