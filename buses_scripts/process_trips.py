import psycopg2

# --- CONFIGURATION DE CONNEXION ---
conn = psycopg2.connect(
    host="localhost",
    port="5432",
    dbname="brussels_bus",
    user="postgres",
    password="1234"
)
cur = conn.cursor()

# --- RECUPERER TOUS LES trip_id UNIQUES ---
cur.execute("SELECT DISTINCT trip_id FROM trip_segs")
trip_ids = [row[0] for row in cur.fetchall()]

print(f"Nombre de trip_id à traiter : {len(trip_ids)}")

for trip_id in trip_ids:
    print(f"Traitement du trip_id : {trip_id}")

    insert_query = """
    INSERT INTO trip_points (trip_id, route_id, service_id, stop1_sequence,
        point_sequence, point_geom, point_arrival_time)
    WITH temp1 AS (
        SELECT trip_id, route_id, service_id, stop1_sequence,
            stop2_sequence, no_stops, stop1_arrival_time, stop2_arrival_time, seg_length,
            (dp).path[1] AS point_sequence, no_points, (dp).geom as point_geom
        FROM trip_segs, ST_DumpPoints(seg_geom) AS dp
        WHERE trip_id = %s
    ),
    temp2 AS (
        SELECT trip_id, route_id, service_id, stop1_sequence,
            stop1_arrival_time, stop2_arrival_time, seg_length,  point_sequence,
            no_points, point_geom
        FROM temp1
        WHERE (point_sequence <> no_points OR stop2_sequence = no_stops) AND temp1.seg_length <> 0
    ),
    temp3 AS (
        SELECT trip_id, route_id, service_id, stop1_sequence,
            stop1_arrival_time, stop2_arrival_time, point_sequence, no_points, point_geom,
            ST_Length(ST_Makeline(array_agg(point_geom) OVER w)) / seg_length AS perc
        FROM temp2 WINDOW w AS (PARTITION BY trip_id, service_id, stop1_sequence
            ORDER BY point_sequence)
    )
    SELECT trip_id, route_id, service_id, stop1_sequence,
        point_sequence, point_geom,
        CASE
        WHEN point_sequence = 1 then stop1_arrival_time
        WHEN point_sequence = no_points then stop2_arrival_time
        ELSE stop1_arrival_time + ((stop2_arrival_time - stop1_arrival_time) * perc)
        END AS point_arrival_time
    FROM temp3;
    """

    try:
        cur.execute(insert_query, (trip_id,))
        conn.commit()
        print(f"--> Insertion OK pour trip_id {trip_id}")
    except Exception as e:
        print(f"ERREUR sur trip_id {trip_id} : {e}")
        conn.rollback()

cur.close()
conn.close()

print("Traitement terminé.")
