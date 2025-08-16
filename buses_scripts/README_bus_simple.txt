
Simple README (TXT) — GTFS (Brussels buses)
===========================================

Start here (GTFS data)
----------------------
This folder already contains a README about importing **GTFS Static** into MobilityDB (see README.MD).
Follow that guide to unzip your GTFS archive and import it into PostgreSQL/PostGIS + MobilityDB.
If the SQL import needs fixing (format issues) or shapes are missing, refer to the tips in README.MD.


Where to put the GTFS data (from your ZIP)
------------------------------------------
Unzip your GTFS archive into a local folder under this project, for example:

brussels_bus_scripts/
├─ gtfs/
│  ├─ agency.txt
│  ├─ stops.txt
│  ├─ routes.txt
│  ├─ trips.txt
│  ├─ stop_times.txt
│  ├─ shapes.txt        (generate with pfaedle if missing)
│  └─ calendar*.txt
├─ README.MD            (GTFS Static guide)
├─ script_compil2.sh
├─ generate_benchmark_bus.sql
├─ generate_clusters_bus.py
├─ run_bench.py
└─ memory_check.sh

Notes:
- In the SQL files referenced by README.MD (e.g., import_sql.sql), point paths to this ./gtfs/ folder.
- script_compil2.sh calls the GTFS SQL files in order, so keep them in the same directory as the script.


How to use the provided scripts (quick)
--------------------------------------
From inside the folder that contains the scripts above:

1) Build DB and import GTFS
   -------------------------
   # Creates database "brussels_bus", enables extensions, and runs the GTFS SQL files
   bash script_compil2.sh

   Defaults inside the script:
   - host: localhost, port: 5432, user: postgres, password: 1234, db: brussels_bus
   - Edit the script if your credentials differ.
   - Make sure the GTFS SQL (create_tables.sql, import_sql.sql, preprocess_target.sql, transform_mdb.sql)
     can find your unzipped GTFS folder (./gtfs/).

2) Create a sample and compute pairwise distances
   ----------------------------------------------
   # Builds a small Brussels sample (EPSG:25831 in meters), clusters midpoints (K-means),
   # then computes pairwise distances and per-cluster min–max normalization
   psql -h localhost -U postgres -d brussels_bus -f generate_benchmark_bus.sql

   This creates:
   - smallsample_bxl        : sample of trips in meters (tgeompoint 25831)
   - trip_clusters_bxl      : quick K-means groups by midpoint
   - alldistances_bxl       : Frechet, DTW (per-step), Hausdorff, Avg-Hausdorff, and LCSS-based distance (ε=50 m)

3) Cluster trips with DBSCAN (precomputed distance matrices)
   ---------------------------------------------------------
   # Writes cluster labels back to the sample table for each metric
   python3 -m pip install psycopg2-binary pandas numpy scikit-learn
   python3 generate_clusters_bus.py

   Defaults inside the script:
   - DB: brussels_bus (change in DB_CONFIG if needed)
   - Reads from alldistances_bxl and writes cluster columns to smallsample_bxl
   - Metrics: frechet_norm, hausdorff_norm, avg_hausdorff_norm, lcss_norm, dtw_norm
   - DBSCAN params: eps=0.25, min_samples=2

4) (Optional) Execution-time benchmarks
   ------------------------------------
   # Runs EXPLAIN ANALYZE multiple times and logs durations
   python3 run_bench.py

   Notes:
   - Edit the SQL in run_bench.py to point to your sample table (e.g., smallsample_bxl)
     instead of any placeholder (tips_1000).

5) (Optional) Memory usage measurement (Linux)
   -------------------------------------------
   # Measures peak RSS deltas for each function, single backend per run
   bash memory_check.sh

   Defaults inside the script:
   - DBNAME="brussels_bus"
   - TABLE="trip_300k_point", COL="temp" (adjust to a real table/column in your DB)
   - Functions: hausdorffDistance, averageHausdorffDistance, lcssDistance(ε), frechetDistance, dynTimeWarpDistance
   - Outputs: mem_runs.csv (details), mem_summary.csv (aggregates)


Common issues
-------------
- GTFS import paths:
  Edit paths in the GTFS SQL files (as described in README.MD) so they point to your ./gtfs/ folder.
- Credentials:
  Change DB/user/password in script_compil2.sh, generate_clusters_bus.py (DB_CONFIG),
  run_bench.py (DB_CONFIG), and memory_check.sh (DBNAME) if needed.
- Empty/odd clustering results:
  Relax DBSCAN eps or min_samples in generate_clusters_bus.py, or review normalization in alldistances_bxl.
- LCSS/CRS:
  Distances in generate_benchmark_bus.sql are computed on EPSG:25831 (meters). The LCSS epsilon there is 50.0 (=50 m).
