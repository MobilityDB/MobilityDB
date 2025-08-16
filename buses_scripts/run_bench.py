import os
import numpy as np
import psycopg2
import time
from datetime import datetime

# Config
DB_CONFIG = {
    'dbname': 'brussels_bus',
    'user': 'postgres',
    'password': '1234',
    'host': 'localhost',
    'port': 5432
}

METRICS = [
    {
        'name': 'lcss',
        'sql': """
            SELECT
                lcssDistance(a.trip, b.trip, 500.0) AS Dist
            FROM tips_1000 a, tips_1000 b  ;
        """
    },
    {
        'name': 'frechet',
        'sql': """
            SELECT
                frechetDistance(a.trip, b.trip) AS Dist
            FROM tips_1000 a, tips_1000 b  ;
        """
    },
    {
        'name': 'dtw',
        'sql': """
            SELECT
                dynTimeWarpDistance(a.trip, b.trip) AS Dist
            FROM tips_1000 a, tips_1000 b  ;
        """
    },
    {
        'name': 'hausdorff',
        'sql': """
            SELECT
                hausdorffDistance(a.trip, b.trip) AS Dist
            FROM tips_1000 a, tips_1000 b  ;
        """
    },
    
    {
        'name': 'avg',
        'sql': """
            SELECT
                averageHausdorffDistance(a.trip, b.trip) AS Dist
            FROM tips_1000 a, tips_1000 b;
        """
    }
]

RUNS_PER_METRIC = 10

def run_explain_analyze(metric_name, select_sql, runs=10):
    # Génère le nom du dossier
    folder_name = datetime.now().strftime('%Y%m%d_tips_1000')
    # Crée le dossier s'il n'existe pas
    os.makedirs(folder_name, exist_ok=True)
    # Chemin complet du fichier de log
    log_file = os.path.join(folder_name, f"explain_analyze_{metric_name}.log")

    average = []
    with psycopg2.connect(**DB_CONFIG) as conn, open(log_file, "w") as log:
        cur = conn.cursor()
        log.write(f"===== EXPLAIN ANALYZE log for {metric_name} =====\n")
        print(f"EXPLAIN ANALYZE log for {metric_name}")
        for i in range(1, runs + 1):
            log.write(f"\n--- Run {i} ---\n")
            t0 = time.time()
            explain_sql = f"EXPLAIN ANALYZE {select_sql.strip()}"
            
            cur.execute(explain_sql)
            plan = cur.fetchall()
            elapsed = time.time() - t0
            average.append(elapsed)
            for row in plan:
                log.write(row[0] + '\n')
            log.write(f"\nRun {i} duration: {elapsed:.2f} seconds\n")
        print(f"Done: {log_file}")
        # Write summary statistics to log file
        log.write(f"\nmin duration: {min(average):.2f} seconds\n")
        log.write(f"max duration: {max(average):.2f} seconds\n")
        log.write(f"average duration: {sum(average)/runs:.2f} seconds\n")
        log.write(f"std deviation: {np.std(average):.2f} seconds\n")

if __name__ == "__main__":
    for metric in METRICS:
        run_explain_analyze(metric['name'], metric['sql'], runs=RUNS_PER_METRIC)
