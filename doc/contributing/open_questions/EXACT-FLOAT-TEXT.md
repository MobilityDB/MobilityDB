<!--
  MobilityDB — Open questions for the committers: Exact text output of floating-point values
  Copyright(c) MobilityDB Contributors

  This documentation is licensed under a Creative Commons Attribution-Share
  Alike 3.0 License: https://creativecommons.org/licenses/by-sa/3.0/
-->

# Exact text output of floating-point values

**For discussion by the committers. Nothing here is merged or pushed.**
Master at the time of writing: `16dd7eac4c`.

## 1. Summary

MobilityDB writes a floating-point value as text with **at most 15 digits after the decimal
point**. A double needs up to 17 significant digits to be read back as the same double, so
the text of a value below 10 is often a different number than the value it writes:

- **A. The text form of a value is not that value.** Of the 96 non-null rows of the regression
  fixture `tbl_tfloat`, 19 read back from their own text as a different `tfloat`; of the 100
  rows of `tbl_floatspan`, 2 do. Anything that stores, exchanges or compares values through
  their text (a client, a CSV export, a binding, a test) works on neighbouring doubles.
- **B. MF-JSON has the same loss, and an explicit precision cannot cure it.** `asMFJSON` caps
  its precision at 15 without saying so; the same 19 rows of `tbl_tfloat` read back from
  their MF-JSON as a different value.
- **C. A test hides it.** `023_temporal_inout_tbl` checks the MF-JSON round trip by comparing
  `asText` of both sides. Both sides are rounded the same way, so the check reads 0 rows
  while 19 values differ.

PostgreSQL itself writes a `float8` in the shortest form that reads back exactly (since
PostgreSQL 12), and MobilityDB already does so in one place: the dimensions of a point cloud
value in MF-JSON. Writing the number types the same way fixes all three. It needs two
decisions:

- **Decision 1 (section 7):** whether the default text of a floating-point value becomes the
  shortest exact form, or stays 15 decimal digits.
- **Decision 2 (section 7):** if it changes, which notation: PostgreSQL's (`1.2e-05`, `-0`), or
  the shortest digits laid out as today (`0.000012`, `0`).

## 2. A simple case

```sql
SELECT tfloat '0.30000000000000004@2001-01-01';
-- master:    0.3@2001-01-01
-- prototype: 0.30000000000000004@2001-01-01

SELECT 0.1::float8 + 0.2::float8;
-- PostgreSQL: 0.30000000000000004
```

The value `0.30000000000000004` is `0.1 + 0.2` in double precision. PostgreSQL writes it
exactly; a `tfloat` holding the same double writes `0.3`, which reads back as another double.

```sql
SELECT floatspan '[1.2460633656010034, 3.5)'::text::floatspan
     = floatspan '[1.2460633656010034, 3.5)';
-- master: f        prototype: t
```

## 3. Where the digits are fixed

- `OUT_DEFAULT_DECIMAL_DIGITS 15` in `meos/include/temporal/type_inout.h` is the default of
  every output function (`Set_out`, `Span_out`, `Temporal_out`, `Tbox_out`, `Stbox_out`,
  `Pose_out`, …) and of the parameter `maxdecimaldigits integer DEFAULT 15` of the 95 SQL
  declarations of `asText`, `asEWKT` and `asMFJSON`.
- The value reaches `float8_out(num, maxdd)` in `pgtypes/utils/float.c`, which calls the
  PostGIS function `lwprint_double(d, maxdd, buf)`. That function prints with Ryu's fixed-point
  routine, `d2sfixed_buffered_n(d, maxdd, …)`: **`maxdd` counts digits after the decimal
  point**, and values below `1e-8` or at least `1e15` switch to exponent notation.
- `temporal_as_mfjson` in `meos/src/temporal/type_out.c` caps its precision at
  `OUT_DEFAULT_DECIMAL_DIGITS`, so `asMFJSON(v, 17)` writes 15.
- `asText(v, 17)` passes 17 to `lwprint_double` and is exact on master: the loss is the
  default, not a limit of the text writer.
- The shortest exact printer is already in the tree: `pgtypes/common/d2s.c` vendors
  PostgreSQL's `double_to_shortest_decimal_buf`, and `mfjson_double_exact_sb` in `type_out.c`
  uses it for point cloud dimensions, "so that reading them back recovers the value exactly".

**How much survives.** Fifteen digits after the point is a different number of significant
digits for each magnitude. Writing 100,000 random doubles per range with 15 decimals and
reading them back (a simulation of the same rule, not a MobilityDB run):

| Value range | Significant digits written | Doubles that do not read back |
|---|---|---|
| [0.001, 0.01) | about 13 | 99.91% |
| [0.1, 1) | 15 | 91.79% |
| [1, 10) | 16 | 26.01% |
| [10, 100) and above | 17 or more | 0.00% |

The convention comes from PostGIS, whose `ST_AsText` writes coordinates with
`maxdecimaldigits` decimal places. For a coordinate in degrees or metres the lost digit is far
below any physical meaning; for a `tfloat`, a `floatspan` or a `tbox` the value is the data.

## 4. The constraint: one value, several text forms

A floating-point value is written by the output function (`::text`, the literal a client
sees), by `asText` and `asEWKT`, and by `asMFJSON`. Over `tbl_tfloat`:

| Form | master, rows that read back as another value | prototype |
|---|---|---|
| output function / `asText` | 19 of 96 | 0 |
| `asMFJSON` | 19 of 96 | 19 of 96 (the cap of section 3, untouched by the prototype) |
| `asBinary` (WKB) | 0 | 0 |

A fix of the text writer alone leaves MF-JSON lossy and makes the two text forms disagree,
which is exactly what moves in `023_temporal_inout_tbl` (section 8). **The fix must cover
every text form at once**, through `float8_out` and the MF-JSON writer, so that every text
form of a value reads back as that value.

## 5. The proposed code

The prototype changes one function:

```c
char *
float8_out(double num, int maxdd)
{
  assert(maxdd >= 0);
  char *str = palloc(OUT_DOUBLE_BUFFER_SIZE);
  if (maxdd >= OUT_DEFAULT_DECIMAL_DIGITS)
    double_to_shortest_decimal_buf(num, str);   /* exact, PostgreSQL's float8 form */
  else
    lwprint_double(num, maxdd, str);            /* the precision the user asked for */
  return str;
}
```

A precision below 15 keeps its meaning: `asText(v, 3)` still writes 3 decimals. The default
and any precision of 15 or more write the shortest form that reads back as the same double.
The MF-JSON writer needs the same change, and its cap removed.

Because the SQL default is the literal `15`, the prototype cannot tell `asText(v)` from
`asText(v, 15)`. A final version would rather mark "no precision asked" explicitly (for
instance a default of `-1` or `NULL` in the 95 declarations) so that an explicit 15 keeps
meaning 15 decimals; this changes the SQL signatures' default, not their names or arguments.

## 6. Proof

The prototype is the change of section 5 on master `16dd7eac4c`, built with every family on
PostgreSQL 18, against a master build of the same commit.

| Probe | master | prototype |
|---|---|---|
| `tfloat '0.30000000000000004@2001-01-01'` | `0.3@…` | `0.30000000000000004@…` |
| `floatspan '[1.2460633656010034, 3.5)'` | `[1.246063365601003, 3.5)` | `[1.2460633656010034, 3.5)` |
| its text read back equals it | `f` | `t` |
| `asText(…, 17)` | `[1.2460633656010034, 3.5)` | same |
| `asText(…, 3)` | `[1.246, 3.5)` | same |
| `tfloat '123456.78901234567@…'` | exact | exact |
| `tbl_tfloat` rows lossy through text | 19 of 96 | 0 |
| `tbl_floatspan` rows lossy through text | 2 of 100 | 0 |
| `tbl_floatset`, `tbl_floatspanset`, `tbl_tboxfloat` rows lossy | 0 | 0 |
| `tbl_tfloat` rows lossy through MF-JSON | 19 | 19 |

`FromText` and the typed literal always agree (0 rows differ over every fixture): the loss is
in the writer, never in the reader.

### Performance

Writing all of `tbl_tfloat_big` as text, timed in the same session as a control that writes
the time of the same values (no floating-point output), 7 alternations per session, two
sessions per build, builds interleaved; only the ratio to the control is meaningful:

| Build | Median ratio to the control (14 runs) | Range |
|---|---|---|
| master | 1.68 | 1.46–2.01 |
| prototype | 1.85 | 1.61–1.95 |

About 10% more time to write a `tfloat` column as text, with overlapping ranges. The text is
0.06% longer (4,232,387 characters against 4,230,026).

## 7. The decisions

**Decision 1: the default.**

| Option | Text of a value | Cost |
|---|---|---|
| keep 15 decimal digits | lossy below 10 (section 3) | none; MF-JSON stays lossy, the test of section 8 stays blind |
| **shortest exact form (proposed)** | reads back as the same double, as PostgreSQL's `float8` | the expected outputs of section 8; about 10% on text output |

**Decision 2: the notation**, if the default changes.

| Option | `0.000012` | `-0.0` | Outputs that move |
|---|---|---|---|
| PostgreSQL's `float8` form (the prototype) | `1.2e-05` | `-0` | 10 tests (section 8) |
| shortest digits in today's layout | `0.000012` | `0` | only the tests whose values gain digits (2 of the 10), plus the MF-JSON test |

The first is what a PostgreSQL user already reads for a `float8`. The second writes every
value whose 15-decimal text already reads back exactly with the same characters, at the cost
of laying out Ryu's shortest digits in MEOS's own fixed notation.

**Out of scope, stated for completeness:** geometry coordinates are written by PostGIS
(`lwgeom_to_wkt`) and follow `ST_AsText`; nothing here changes them.

## 8. Blast radius

**Code.** `float8_out` in `pgtypes/utils/float.c`, the MF-JSON writer and its cap in
`meos/src/temporal/type_out.c`, and, for the explicit default of section 5, the 95 SQL
declarations with `maxdecimaldigits integer DEFAULT 15` and the wrappers that read it.

**Expected outputs.** On the prototype, the full regression suite (320 tests, every family,
PostgreSQL 18) moves ten tests:

| Kind | Tests | Example |
|---|---|---|
| more digits, the intended change | `001_set`, `164_trgeo_spatialfuncs` | `0.785398163397448` → `0.7853981633974483` |
| exponent notation below `1e-4` (decision 2) | `026_tnumber_mathfuncs`, `056_tpoint_spatialfuncs`, `306_tnpoint_spatialfuncs`, `164_trgeo_spatialfuncs` | `0.000012` → `1.2e-05` |
| negative zero written (decision 2) | `100_pose`, `103_pose_geopose`, `105_tpose_spatialfuncs`, `550_posechain` | `Pose(POINT Z (8 47 0),1,0,0,0)` → `…,1,0,0,-0)` |
| a check that hid the loss (defect C) | `023_temporal_inout_tbl` | MF-JSON round trip: `0` rows differ → `19` |

Few outputs move because the suite already rounds most floating-point results with
`round(…, 6)` to keep them stable across platforms. Users see every value they read as text.

## 9. Plan

**One rule for every text form:** without an explicit precision, a floating-point value is
written in the shortest form that reads back as the same double; an explicit precision keeps
its meaning.

**Steps**, each its own PR with the regression suite and the moved outputs classified:

1. **Independent of the decisions, first:** the check of `023_temporal_inout_tbl` compares
   the values, not their text, so that it sees the MF-JSON loss.
2. **The writer:** `float8_out` and the MF-JSON writer, with the notation chosen in decision 2,
   and the MF-JSON cap removed.
3. **The default:** the SQL default and the output functions state "no precision asked"
   explicitly, so that an explicit 15 keeps its meaning.

If the committers keep 15 digits, step 1 still applies, and the manual states in the entries
of `asText` and `asMFJSON` that the default text of a value below 10 may read back as a
neighbouring double, and that `asText(v, 17)` is exact.
