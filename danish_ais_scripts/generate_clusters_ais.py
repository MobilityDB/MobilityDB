#!/usr/bin/env python3
import psycopg2
import pandas as pd
import numpy as np
from sklearn.cluster import DBSCAN

# === CONFIG ===
DB_CONFIG = {
    'dbname': 'danish_ais',
    'user': 'postgres',
    'password': '1234',
    'host': 'localhost',
    'port': 5432
}
DISTANCE_TABLE = 'allDistances'
TRIP_TABLE = 'smallsample'  # original table with TripId

# Choose the metric to use for clustering:
# 'frechet_norm_big', 'hausdorff_norm_big', 'avg_hausdorff_norm_big', 'lcss_dist', 'dtw_norm_big'
EPS = 0.25    # epsilon for normalized distances
MIN_SAMPLES = 2

# === DB CONNECTION ===
def get_connection():
    return psycopg2.connect(**DB_CONFIG)

# === FETCH DATA ===
def fetch_distances(metric):
    query = f"SELECT TripId1, TripId2, {metric} as dist FROM {DISTANCE_TABLE};"
    with get_connection() as conn:
        df = pd.read_sql(query, conn)
    # normalize column names to lower for consistency
    df.columns = [c.lower() for c in df.columns]
    return df

# === BUILD DISTANCE MATRIX ===
def build_distance_matrix(df):
    unique_ids = sorted(set(df['tripid1']).union(df['tripid2']))
    id_map = {tid: idx for idx, tid in enumerate(unique_ids)}
    n = len(unique_ids)
    mat = np.full((n, n), np.nan, dtype=float)

    for _, row in df.iterrows():
        i = id_map[row['tripid1']]
        j = id_map[row['tripid2']]
        dist = float(row['dist']) if row['dist'] is not None else np.nan
        mat[i, j] = dist
        mat[j, i] = dist

    # zero diagonal
    np.fill_diagonal(mat, 0.0)

    # Handle missing / non-finite cells by replacing with a large finite distance (treated as "far apart").
    offdiag_mask = ~np.eye(n, dtype=bool)
    missing = np.isnan(mat[offdiag_mask]).sum()
    finite_vals = mat[np.isfinite(mat) & (mat > 0)]
    if missing > 0 or not np.isfinite(mat).all():
        if finite_vals.size == 0:
            large = 1.0
        else:
            large = float(np.nanmax(finite_vals)) * 10.0
            if not np.isfinite(large) or large <= 0:
                large = 1.0
        mat[~np.isfinite(mat)] = large
        print(f"[INFO] Replaced {int(missing)} missing/invalid off-diagonal cells with {large:.6g}.")

    return mat, unique_ids

# === CLUSTERING ===
def cluster_dbscan(distance_matrix, eps, min_samples):
    db = DBSCAN(eps=eps, min_samples=min_samples, metric='precomputed')
    return db.fit_predict(distance_matrix)

# === WRITE RESULTS ===
def write_clusters(trip_ids, clusters, cluster_col):
    with get_connection() as conn:
        with conn.cursor() as cursor:
            cursor.execute(f"ALTER TABLE {TRIP_TABLE} ADD COLUMN IF NOT EXISTS {cluster_col} integer;")
            for tid, cl in zip(trip_ids, clusters):
                cursor.execute(f"UPDATE {TRIP_TABLE} SET {cluster_col} = %s WHERE TripId = %s;", (int(cl), int(tid)))
        conn.commit()

if __name__ == '__main__':
    metrics = [
        ('frechet_norm_big', 'frechet_cluster'),
        ('hausdorff_norm_big', 'hausdorff_cluster'),
        ('avg_hausdorff_norm_big', 'avg_hausdorff_cluster'),
        ('lcss_dist', 'lcss_cluster'),
        ('dtw_norm_big', 'dtw_cluster'),
    ]

    for metric, cluster_col in metrics:
        print(f"\n--- Processing clustering for metric: {metric} ---")
        df = fetch_distances(metric)
        print(f"Building distance matrix for {metric}...")
        mat, trip_ids = build_distance_matrix(df)
        print(f"Clustering (eps={EPS}, min_samples={MIN_SAMPLES}) for {metric}...")
        clusters = cluster_dbscan(mat, EPS, MIN_SAMPLES)
        print(f"Writing clusters for {metric} back to DB in column: {cluster_col} ...")
        write_clusters(trip_ids, clusters, cluster_col)
        print(f"Done clustering for {metric}. Example assignments (first 10):")
        for tid, cl in list(zip(trip_ids, clusters)):
            print(f"  TripId: {tid}, Cluster: {cl}")
