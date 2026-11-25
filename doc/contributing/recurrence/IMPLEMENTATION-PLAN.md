<!--
  MobilityDB — Recurring temporal values: implementation plan
  Copyright(c) MobilityDB Contributors

  This documentation is licensed under a Creative Commons Attribution-Share
  Alike 3.0 License: https://creativecommons.org/licenses/by-sa/3.0/
-->

# Recurring temporal values: the implementation plan

[Back to the index](00-INDEX.md)

Part 0 drafts the article in full and defines in it the implementation that fulfills its
objective; Parts I to IV build that implementation, each task one pull request; Part V measures it
and completes the article.

**THE NAMES.** Each function takes its three names as its sibling `temporal_shift_time` does: the
MEOS function, its MobilityDB wrapper with the leading capital (`@csqlfn #Temporal_shift_time()`),
and one bare SQL name in camel case overloaded over every temporal type (`shiftTime`). Hence
`temporal_occurrence`, `Temporal_occurrence`, `occurrence`; `temporal_occurrence_origins`,
`Temporal_occurrence_origins`, `occurrenceOrigins`; `temporal_occurrences`,
`Temporal_occurrences`, `occurrences`; and, on the date set as `dateset_shift` is,
`dateset_origins`, `Dateset_origins`, `origins`.

## The timeline

**THE HORIZON (the owner, 2026-11-26).** The article is finished, measurements included, by
2026-12-14. Each pull request is merged as soon as its receipts are earned, so the timeline is set
by the work and its receipts, never by review.

| Milestone | Tasks | Target |
|---|---|---|
| M1, the bibliography checked against the publishers' records | RC2 | 2026-11-28 |
| M2, MEOS: origins, one occurrence, the selected origins, the occurrences | RC3, RC4, RC5 | 2026-12-01 |
| M3, MobilityDB: the SQL functions, the table function with its planner support, the test against materialized occurrences | RC6, RC7, RC8 | 2026-12-03 |
| M4, the GTFS adapter and the table function in MobilityDuck and MobilitySpark | RC9, RC10 | 2026-12-05 |
| M5, the benchmark harness: loaders of the three representations, the workloads Q1 to Q8, the row-for-row check, the synthetic feed generator | RC11 | 2026-12-07 |
| M6, the benchmark run: every workload, the window sweep, ten runs each, the answers checked | RC11 | 2026-12-10 |
| M7, the article complete: results against the cost analysis, figures, both authors' review | RC12 | 2026-12-14 |

M3 and M5 overlap: the harness is written against the SQL functions named above while RC6 to RC8
earn their receipts. A milestone that slips moves every later one by the same amount; the slip and
its cause are recorded in the row of the task.

## Part 0 — the article and its blueprint

| Task | Change | State |
|---|---|---|
| RC0 | The article drafted in full, from the thesis of S. Swirydowicz (its survey of periodicity in databases and of periodic pattern mining, its GTFS setting, its grouping of trips) and the design sketch it led to: model, time-zone semantics, algebra, query processing, cost analysis, implementation in MEOS and MobilityDB, GTFS case study, evaluation design. The article states that it derives from the thesis and reports none of the thesis's intermediate measurements | drafted: *Recurring Spatiotemporal Data as a Table Function: A Template-and-Origins Model for MobilityDB*, 13 pages |
| RC1 | The blueprint: the sections of the article that define the implementation (model, algebra, query processing, implementation), summarized in [the index](00-INDEX.md#the-blueprint); every later task cites the element it builds | drafted |
| RC2 | The bibliography checked against the publishers' records; the owner's review of the draft | open |

## Part I — MEOS

| Task | Change | State |
|---|---|---|
| RC3 | `dateset_origins(const Set *dates, const char *zone)`: the origins of a set of service dates, local noon minus 12 hours in the zone, the one function that reads a time zone. Tests on both daylight-saving changes of a year and on a zone without them | open |
| RC4 | `temporal_occurrence(const Temporal *temp, TimestampTz ref, TimestampTz origin)` and `temporal_occurrence_origins(const Temporal *temp, TimestampTz ref, const Set *origins, const Span *window)`: one occurrence, and the origins whose occurrences meet a window by two binary searches, with the inclusivity of the window and of the support carried over. Tests on occurrences that start before the window and end in it | open |
| RC5 | `temporal_occurrences(const Temporal *temp, TimestampTz ref, const Set *origins, const Span *window, int *count)`: the occurrences at those origins, built in order | open |

Each function goes into the MEOS catalog and is generated into every binding; none needs code
written by hand in a binding.

## Part II — MobilityDB

| Task | Change | State |
|---|---|---|
| RC6 | `origins(dateset, text)`, `occurrence(T, timestamptz, timestamptz)` and `occurrenceOrigins(T, timestamptz, tstzset, tstzspan)` for every temporal type, from the catalog | open |
| RC7 | `occurrences(T, timestamptz, tstzset, tstzspan) RETURNS SETOF (origin timestamptz, occurrence T)`, value per call, with a planner support function returning the number of selected origins | open |
| RC8 | The regression test that compares `occurrences` with fully materialized occurrences row for row, on dates on both sides of a daylight-saving change, stop times beyond 24:00, occurrences crossing the window bounds, and service dates added and removed | open |

## Part III — the GTFS adapter

| Task | Change | State |
|---|---|---|
| RC9 | The GTFS chapter of the MobilityDB workshop loading trips as templates with origins: service dates from `calendar.txt` and `calendar_dates.txt`, templates at the origin of a reference date, trips grouped by path and relative timing, origins from `origins(dates, agency_timezone)` | open |

## Part IV — the hosts beyond PostgreSQL

| Task | Change | State |
|---|---|---|
| RC10 | `occurrences` as a table function in the bindings whose host has them (MobilityDuck, MobilitySpark), from the generated functions | open |

## Part V — the evaluation and the article

| Task | Change | State |
|---|---|---|
| RC11 | The evaluation of the article: materialized occurrences, templates with the source calendar resolved at query time, and templates with origins in both indexing forms, on the STIB-MIVB feeds, a large feed with long service periods, and synthetic feeds; the workloads Q1 to Q8; every answer of the recurring form checked against the materialized one | open |
| RC12 | The article completed with the measurements of RC11 against the cost analysis it states | open |

**THE DATA AND THE MACHINE (the owner, 2026-11-26).** The large feed is the Swiss national GTFS
feed of opentransportdata.swiss: one integrated timetable, services spanning the whole timetable
year and therefore both daylight-saving changes, and every mode. The timetable year that starts
in December 2026 is downloaded once and pinned by its version and date; the article reports the
notices of the MobilityData GTFS validator on it and on the STIB-MIVB feed. The benchmark runs on
the machine that holds the book databases, whose disk holds the materialized representation of a
whole year and on which no other work runs during a measurement, so that absolute times are valid;
the code and its receipts are earned on the development machine, and the harness reaches the
benchmark machine through the `MobilityDB/claude-kits` repository.
