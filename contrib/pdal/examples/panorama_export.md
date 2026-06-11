# MobilityDB → PANORAMA / LISA heritage Potree integration

Sibling document to [`potree_export.md`](potree_export.md) targeted at the LISA / CReA-Patrimoine / ALICe / ATM heritage workflow that publishes site scans through PANORAMA's Potree gallery (<https://panorama.ulb.ac.be/potree/>). Where `potree_export.md` covers the FARI CAVE real-time traffic case, this one covers the **longitudinal heritage** case: a single site rescanned across multiple campaigns, exported through the same PDAL → PotreeConverter pipeline.

## When this applies

The PANORAMA workflow's *current* default is one site × one campaign × one Potree URL. The MobilityDB layer becomes useful precisely when that breaks: the same physical site is scanned again — after restoration, after damage, on an annual monitoring cadence, after vegetation regrowth, before/after settlement studies, etc. A `tpcpatch` row indexes those campaigns under one identifier, and the export pipeline below produces three Potree URLs for the price of one storage unit:

* the 2018 campaign,
* the 2024 campaign,
* a *change* scene that contains only the points that moved more than ε between campaigns.

PANORAMA's Potree front-end stays unchanged; what we add is the indexed source of truth and the SQL-driven export pipeline.

## The data model

```sql
CREATE TABLE heritage_sites (
  id           int PRIMARY KEY,
  name         text,
  description  text
);

CREATE TABLE heritage_scans (
  site_id      int REFERENCES heritage_sites(id),
  campaigns    tpcpatch,
  PRIMARY KEY (site_id)
);
```

One row per site; `campaigns` is a `tpcpatch` whose instants are the per-campaign cleaned + registered scans. Each instant's pcpatch carries the full point payload for that campaign.

Indexed lookups: `tpcbox` GiST / SP-GiST + temporal predicates make queries like "all sites scanned in 2019 *and* 2024 within 50 km of Brussels" a one-line SQL.

## Per-campaign export pipeline

```json
{
  "_comment": "Export campaign N of site M to a Potree-ready LAS.",
  "pipeline": [
    {
      "type": "readers.tpcpatch",
      "connection": "dbname=panorama",
      "query": "SELECT (EXTRACT(EPOCH FROM timestampN(s.campaigns, n)) * 1000000)::bigint AS t, valueN(s.campaigns, n) AS pcp, PC_PCId(valueN(s.campaigns, n)) AS pcid FROM heritage_scans s, generate_series(1, numInstants(s.campaigns)) n WHERE s.site_id = 42 AND timestampN(s.campaigns, n) = '2024-04-01'::timestamptz",
      "time_column": "t",
      "patch_column": "pcp",
      "pcid_column": "pcid"
    },
    {
      "type": "writers.las",
      "filename": "/tmp/site42_2024.las",
      "extra_dims": "all",
      "minor_version": "4",
      "dataformat_id": "6"
    }
  ]
}
```

Then:

```bash
PotreeConverter /tmp/site42_2024.las -o /var/www/potree/site42/2024 --overwrite
```

Replace `2024` and `2024-04-01` for each campaign. The `WHERE timestampN(...) = ...` clause picks one campaign by date; switch to `BETWEEN` for a range scan.

## Change-detection scene

The interesting query for heritage is "what changed between two campaigns". Two approaches:

### A. Naive bbox-coarse change

Drop into PostgreSQL's set difference at the bbox level — fast, useful as a preview:

```sql
-- "Points whose containing 1m grid cell is in the 2024 scan but not the
-- 2018 scan." Approximate; the cell size is the worst-case error.
WITH cells_2024 AS (
  SELECT DISTINCT
    floor(x) AS gx, floor(y) AS gy, floor(z) AS gz
  FROM (SELECT (points(valueN(campaigns,
                  (SELECT n FROM generate_series(1, numInstants(campaigns)) n
                   WHERE timestampN(campaigns, n) = '2024-04-01')))).pcp p
        FROM heritage_scans WHERE site_id = 42) sub,
       LATERAL (SELECT PC_Get(p, 'X') AS x, PC_Get(p, 'Y') AS y, PC_Get(p, 'Z') AS z) c
),
cells_2018 AS (...)  -- same shape with 2018-04-01
SELECT * FROM cells_2024 EXCEPT SELECT * FROM cells_2018;
```

### B. Exact ICP change via `contrib/pcl`

Once the PCL ICP filter lands (`pcpatch_icp` is on the roadmap), the contract becomes:

```sql
-- Align 2024 scan onto 2018 scan's reference frame, then keep only
-- points whose ICP-aligned position differs from any 2018 point by
-- more than 5 cm.
SELECT
  pcpatch_change_detect(
    /* ref       */ valueN(s.campaigns, n2018),
    /* incoming  */ valueN(s.campaigns, n2024),
    /* threshold */ 0.05)
FROM heritage_scans s,
     generate_series(1, numInstants(s.campaigns)) n2018,
     generate_series(1, numInstants(s.campaigns)) n2024
WHERE s.site_id = 42
  AND timestampN(s.campaigns, n2018) = '2018-09-01'
  AND timestampN(s.campaigns, n2024) = '2024-04-01';
```

The result is a `pcpatch` containing only the points that moved. Pipe through PDAL → PotreeConverter → a third Potree URL that lights up only the changed regions.

## What PANORAMA gets that pure Potree doesn't give them

1. **Single source of truth.** All campaigns of a site live under one row, indexed by site geometry and time. No directory naming convention or manual file management.
2. **Time-bounded queries.** "All scans of UNESCO-listed buildings between 2010 and 2020." One SQL line, indexed.
3. **Change-detection materialised as a Potree URL.** The change scene composes through the same export pipeline as the campaign scenes, so the viewer code stays identical.
4. **Dump/restore portability.** A site's full multi-campaign history travels as one `pg_dump`, ready to drop into another instance.

## What stays unchanged on the PANORAMA side

* PotreeConverter, the front-end JS viewer, the static-file hosting model — all unchanged.
* CloudCompare and bespoke registration scripts — still upstream of `tpcpatch` ingestion.
* Acquisition expertise (laser, drone, photogrammetry) — entirely owned by LISA / CReA / ALICe.

## First-meeting offering

A 5-minute synthetic demo:

1. Two timestamped scans of the same fixture site (same X/Y/Z, slight Z displacement to simulate settling).
2. `INSERT INTO heritage_scans VALUES (1, tpcpatchSeq(ARRAY[tpcpatch(p1, t1), tpcpatch(p2, t2)]))`.
3. Three PDAL pipelines exporting `?campaign=2018`, `?campaign=2024`, and `?diff=2018-vs-2024`.
4. Three PotreeConverter invocations.
5. Three Potree URLs, all pointing at the same `heritage_scans.id = 1`.

Driving question for the meeting: *"do you have, or expect to soon have, the same site scanned in multiple campaigns?"* If yes, the demo above is the proposal. If no, file under "future work" and continue the conversation when the workflow grows.
