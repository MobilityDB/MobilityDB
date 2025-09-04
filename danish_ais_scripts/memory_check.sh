#!/bin/sh
# Sequential Postgres memory benchmark (single backend per run, PID fixed).

set -e

DBNAME="danish_ais"
TABLE="max_100kpoints"    # single row
COL="temp"
NUM_RUNS=3
SLEEP_BEFORE=1        # seconds to sleep in backend before running the function
SAMPLE_INT=0.05       # seconds between RSS polls
WORK_MEM="64MB"

# function,epsilon (epsilon empty if unused)
FUNCS='
hausdorffDistance,
averageHausdorffDistance,
lcssDistance,500.0
frechetDistance,
dynTimeWarpDistance,
'

DETAILS="mem_runs.csv"
SUMMARY="mem_summary.csv"
echo "function,run,baseline_kb,peak_kb,delta_kb,elapsed_s" > "$DETAILS"
echo "function,min_delta_kb,max_delta_kb,avg_delta_kb,min_s,max_s,avg_s" > "$SUMMARY"

rss_kb() {
  awk '/^VmRSS:/{print $2; exit}' "/proc/$1/status" 2>/dev/null || echo 0
}

sql_for() {
  func="$1"; eps="$2"
  if [ "$func" = "lcssDistance" ] && [ -n "$eps" ]; then
    echo "select ${func}(${COL}, ${COL}, ${eps}) from ${TABLE};"
  else
    echo "select ${func}(${COL}, ${COL}) from ${TABLE};"
  fi
}

for line in $FUNCS; do
  FUNC=$(echo "$line" | cut -d, -f1)
  EPS=$(echo "$line" | cut -d, -f2)
  [ -n "$FUNC" ] || continue

  echo "=== $FUNC ==="
  deltas_tmp=$(mktemp)
  secs_tmp=$(mktemp)

  run=1
  while [ "$run" -le "$NUM_RUNS" ]; do
    pid_file=$(mktemp)
    peak_file=$(mktemp)

    # Start the query in background: first print PID, then sleep, then run query
    {
      psql -qAt -d "$DBNAME" <<EOF
        select pg_backend_pid();
        select pg_sleep(${SLEEP_BEFORE});
        set work_mem='${WORK_MEM}';
        $(sql_for "$FUNC" "$EPS")
EOF
    } > "$pid_file" &
    job_pid=$!

    # Read the first line from psql output as the backend PID
    while [ ! -s "$pid_file" ]; do sleep 0.01; done
    PG_PID=$(head -n1 "$pid_file")
    echo "Run $run PID=$PG_PID"

    # === baseline sample while backend is sleeping ===
    BASELINE=$(rss_kb "$PG_PID")

    # Start monitoring that PID's RSS (peak)
    peak=0
    (
      while [ -d "/proc/$PG_PID" ]; do
        r=$(rss_kb "$PG_PID")
        [ "$r" -gt "$peak" ] && peak="$r"
        sleep "$SAMPLE_INT"
      done
      echo "$peak" > "$peak_file"
    ) &
    mon_pid=$!

    # Wait for psql to finish (query + backend exit)
    t_start=$(date +%s%N)
    wait "$job_pid" 2>/dev/null || true
    t_end=$(date +%s%N)
    wait "$mon_pid" 2>/dev/null || true

    elapsed=$(awk -v a="$t_start" -v b="$t_end" 'BEGIN{printf "%.3f",(b-a)/1e9}')
    PEAK=$(cat "$peak_file")
    DELTA=$(awk -v p="$PEAK" -v b="$BASELINE" 'BEGIN{d=p-b; if(d<0)d=0; print d}')

    echo "$FUNC,$run,$BASELINE,$PEAK,$DELTA,$elapsed" >> "$DETAILS"
    echo "$DELTA" >> "$deltas_tmp"
    echo "$elapsed" >> "$secs_tmp"

    rm -f "$pid_file" "$peak_file"
    run=$((run+1))
  done

  mind=$(sort -n "$deltas_tmp" | head -1)
  maxd=$(sort -n "$deltas_tmp" | tail -1)
  avgd=$(awk '{s+=$1}END{if(NR){printf "%.1f",s/NR}else{print "0"}}' "$deltas_tmp")

  mins=$(sort -n "$secs_tmp" | head -1)
  maxs=$(sort -n "$secs_tmp" | tail -1)
  avgs=$(awk '{s+=$1}END{if(NR){printf "%.3f",s/NR}else{print "0"}}' "$secs_tmp")

  echo "$FUNC,$mind,$maxd,$avgd,$mins,$maxs,$avgs" >> "$SUMMARY"
  rm -f "$deltas_tmp" "$secs_tmp"
done

echo "Done."
echo "Details: $DETAILS"
echo "Summary: $SUMMARY"
