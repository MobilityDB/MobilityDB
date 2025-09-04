#!/usr/bin/env python3
import psycopg2
import pandas as pd
import numpy as np
from sklearn.cluster import DBSCAN

# === CONFIG ===
DB_CONFIG = {
    'dbname': 'brussels_bus',   # <-- change if needed
    'user': 'postgres',
    'password': '1234',
    'host': 'localhost',
    'port': 5432
}
DISTANCE_TABLE = 'alldistances_bxl'
TRIP_TABLE = 'smallsample_bxl'        # table that has trip_id

# Metrics available in alldistances_bxl:
#   frechet_norm, hausdorff_norm, avg_hausdorff_norm, lcss_dist, dtw_norm
EPS = 0.25
MIN_SAMPLES = 2

def get_connection():
    return psycopg2.connect(**DB_CONFIG)

def fetch_distances(metric_col: str) -> pd.DataFrame:
    # Alias id columns to tripid1/tripid2 so downstream code stays simple
    query = f"""
        SELECT trip_id1 AS tripid1, trip_id2 AS tripid2, {metric_col} AS dist
        FROM {DISTANCE_TABLE};
    """
    with get_connection() as conn:
        df = pd.read_sql(query, conn)
    df.columns = [c.lower() for c in df.columns]
    return df

def build_distance_matrix(df: pd.DataFrame):
    unique_ids = sorted(set(df['tripid1']).union(df['tripid2']))
    id_map = {tid: idx for idx, tid in enumerate(unique_ids)}
    n = len(unique_ids)
    mat = np.full((n, n), np.nan, dtype=float)

    for _, row in df.iterrows():
        i = id_map[row['tripid1']]
        j = id_map[row['tripid2']]
        d = float(row['dist']) if row['dist'] is not None else np.nan
        mat[i, j] = d
        mat[j, i] = d

    np.fill_diagonal(mat, 0.0)

    # Replace missing/invalid with a large finite distance
    offdiag = ~np.eye(n, dtype=bool)
    missing = np.isnan(mat[offdiag]).sum()
    finite_vals = mat[np.isfinite(mat) & (mat > 0)]
    if missing > 0 or not np.isfinite(mat).all():
        large = float(np.nanmax(finite_vals)) * 10.0 if finite_vals.size else 1.0
        if not np.isfinite(large) or large <= 0:
            large = 1.0
        mat[~np.isfinite(mat)] = large
        print(f"[INFO] Replaced {int(missing)} missing/invalid off-diagonal cells with {large:.6g}.")
    return mat, unique_ids

def cluster_dbscan(distance_matrix, eps, min_samples):
    return DBSCAN(eps=eps, min_samples=min_samples, metric='precomputed').fit_predict(distance_matrix)

def write_clusters(trip_ids, clusters, cluster_col):
    with get_connection() as conn:
        with conn.cursor() as cur:
            cur.execute(f"ALTER TABLE {TRIP_TABLE} ADD COLUMN IF NOT EXISTS {cluster_col} integer;")
            for tid, cl in zip(trip_ids, clusters):
                cur.execute(f"UPDATE {TRIP_TABLE} SET {cluster_col} = %s WHERE trip_id = %s;", (int(cl), tid))
        conn.commit()

if __name__ == '__main__':
    metrics = [
        ('frechet_norm',         'frechet_cluster'),
        ('hausdorff_norm',       'hausdorff_cluster'),
        ('avg_hausdorff_norm',   'avg_hausdorff_cluster'),
        ('lcss_norm',            'lcss_cluster'),
        ('dtw_norm',             'dtw_cluster'),
    ]

    for metric_col, cluster_col in metrics:
        print(f"\n--- Processing clustering for metric: {metric_col} ---")
        df = fetch_distances(metric_col)
        print(f"Building distance matrix for {metric_col}...")
        mat, trip_ids = build_distance_matrix(df)
        print(f"Clustering (eps={EPS}, min_samples={MIN_SAMPLES}) for {metric_col}...")
        clusters = cluster_dbscan(mat, EPS, MIN_SAMPLES)
        print(f"Writing clusters for {metric_col} back to DB in column: {cluster_col} ...")
        write_clusters(trip_ids, clusters, cluster_col)
        print(f"Done clustering for {metric_col}. Example assignments (first 10):")
        for tid, cl in list(zip(trip_ids, clusters)):
            print(f"  trip_id: {tid}, cluster: {cl}")
