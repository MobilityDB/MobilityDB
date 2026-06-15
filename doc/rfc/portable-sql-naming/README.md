# RFC — Portable SQL naming & polymorphic-function overload resolution

Status: **Draft for discussion** (docs-only). Captures findings from running the
canonical BerlinMOD suite on MobilitySpark (MEOS-backed UDFs) against
`ecosystem-pin-2026-06-15f`. No code changes — this is a design note to align the
ecosystem and seed the portable-SQL RFC (cf. RFC #920 / #1173, `doc/portable_sql.xml`).

---

## 0. Motivation — what the bench exposed

The BerlinMOD portable suite runs **end-to-end on Spark**, multi-threaded, after the
15f thread-safety restore (q02 `local[*]` 15.5 s, no crash). 9 of 18 queries pass —
including the th3 cell-set prefilter (q02/q04) and the NxN array queries (q05/q06/q10).
The 7 failures trace to **two** root causes, both at the SQL-naming / generation layer,
not the engine:

1. **Polymorphic functions with arity-varying overloads** (`stbox`, `round`) that
   engines without native overloading (Spark, Nebula) cannot register under one name
   (§1).
2. **Hand-maintained `@sqlfn` tags carry several classes of irregularity** — the SQL
   surface name is the single key every binding generates from, so any drift there
   breaks portability (§2).

Both belong in the portable-SQL RFC because they are **contracts every engine must
reproduce identically**, not per-binding patches (North Star: one SoT, all bindings
generated, zero hand special-cases).

---

## 1. Polymorphic-function overload resolution

### 1.1 The problem
MEOS exposes polymorphic SQL names whose overloads vary in **both arity and argument
type**. From the live 15f catalog:

| Name | Overloads (by arity) |
|------|----------------------|
| `stbox` | 14 @ arity 1 (`geom→`, `tstzspan→`, `npoint→`, …), 8 @ arity 2 (`geom+timestamptz`, `geom+tstzspan`, …), 1 @ arity 11 (full numeric constructor) |
| `round` | 9 @ arity 2 (`tfloat+int`, `geom+int`, `tbox+int`, …), 3 @ arity 3 (array round) |

- **PostgreSQL / MobilityDB** resolve these natively by SQL overload resolution.
- **DuckDB / MobilityDuck** resolve via `ScalarFunctionSet` (multiple typed signatures
  per name).
- **PyMEOS / .NET** resolve via language overloading / optional args.
- **Spark / Nebula** UDF registries **cannot overload a name** → the generator emits one
  registration per signature-group, they collide, last-wins silently drops the rest.
  Result: `stbox(geom, ts)` lost; `round`'s digit arg over-hidden to 1-arg → q09 + q11–q16
  fail.

### 1.2 This is the dispatch we already ship — one dimension wider
The generator already dispatches **same-arity** polymorphism by argument type:

| Family | Mechanism |
|--------|-----------|
| Operators (`eEq`, `eIntersects`, `overlaps`) | parse each arg per candidate; first all-parse wins (WKB/WKT mutually discriminating) |
| `atTime` / `minusTime` | classify the time arg (timestamp / period / set / spanset) → route to the matching overload |
| Comparison fold (`eEq(h3indexset, th3index)`) | non-temporal overloads folded into one parse-distinct dispatcher |

`atTime` is the **direct parallel** to `stbox(geom, <time>)`. The only new dimension
`stbox`/`round` add is **varying arity**.

### 1.3 Proposed contract (engine-agnostic)
> A polymorphic **camelCase** name resolves a call `name(a₀…aₙ)` to the unique MEOS
> backing whose signature matches **(arity, ordered argument type-kinds)** — identical to
> PostgreSQL overload resolution. Native-overloading engines register each overload's
> signature directly; UDF-registry engines emit **one** function that branches first on
> argument count, then on argument types (reusing the existing per-arg parse-dispatch
> within each arity bucket). The dispatcher registers the **camelCase** name and never
> leaks a C underscore.

Plus a sub-rule the `round` case forces:
> **Native-built-in deferral** — where a portable name coincides with a host built-in of
> identical semantics (`round(double, int)` ≡ MobilityDB float-round ≡ Spark/DuckDB native
> double-round), the binding must **not** shadow the built-in; the generated dispatcher
> claims only the MEOS-typed first args and lets a numeric first arg fall through to native.

### 1.4 Worked examples (camelCase SQL surface; C backings are internal)
```
stbox(geom)            arity 1 → geo→stbox
stbox(geom, ts)        arity 2 → dispatch 2nd-arg type (timestamptz / tstzspan)   ← like atTime
stbox(b,b,b,i,…,span)  arity 11 → full numeric constructor

round(tfloat, 3)       → tnumber round
round(geom, 3)         → geo round
round(2.7183, 3)       → native round(double,int)                                 ← deferral
```

### 1.5 Parity at a glance
| Call | PG | DuckDB | Spark today | Spark proposed |
|------|----|--------|-------------|----------------|
| `stbox(geom)` | overload | signature | `UDF1` ✓ | arity-1 bucket |
| `stbox(geom, ts)` | overload | signature | **dropped** ✗ | arity-2 dispatch |
| `round(tfloat, 3)` | overload | signature | hidden→`UDF1` ✗ | arity-2, digit kept |
| `round(2.7, 3)` | overload | signature | **shadows native** ✗ | defer to native |

---

## 2. camelCase `@sqlfn` convention & irregularity audit

The user-facing SQL interface is **camelCase, no underscores**. Underscored names are
either **C backings** (`geo_timestamptz_to_stbox`, `float_round`) or **internal**
SQL functions (aggregate trans/final/combine, selectivity estimators, debug helpers,
and type/operator machinery: in/out/recv/send/cmp/hash/eq/lt/…) which legitimately keep
underscores and **should not be on the user-facing binding surface at all**.

Because the `@sqlfn` tags were maintained by hand, the live 15f catalog
(556 distinct `@sqlfn`, 338 clean camelCase, **218 dirty**) carries **several distinct
classes** of irregularity. They need **per-category** review — not a blind sweep.

### Category table (for discussion)

| Cat | Class | Distinct | Severity | Disposition (to confirm per item) |
|-----|-------|----------|----------|-----------------------------------|
| **B1** | **Semantic mistag — source** | ~16 | **Bug** | `stbox.c` repeats `@sqlfn temporal_below()` across the whole positional block (`Left`/`Overleft`/`Right`/`Overright`/`Below`/`Overbelow`/`Above`/`Overabove`/`Front`/`Back`/`Before`/`After`/… lines 1260–1495) — a copy-paste; only `Below_stbox_stbox` should be `temporal_below`. Fix the tags at source. |
| **B2** | **Mistag — catalog extractor** | n/a | **Parser bug** | e.g. catalog shows `cbuffer_intersects ← disjoint_cbuffer_cbuffer`, but source correctly tags `cbuffer_disjoint()` (cbuffer.c:571) and `cbuffer_intersects()` (cbuffer.c:589). The MEOS-API `@sqlfn` extractor mis-attributes when two tags sit close. Fix the extractor, not the source. |
| **C** | **Stale pre-dialect names** | 4 | Consistency | `ever_eq` / `ever_ne` / `always_eq` / `always_ne` on **tjsonb** comparison fns — missed the eat-prefix rename to `eEq`/`eNe`/`aEq`/`aNe` (canonical since pin 15a). |
| **A** | **PascalCase** | 13 | Casing | `Tmax`/`Tmin`/`Xmax`/`Xmin`/`Ymax`/`Ymin`/`Zmax`/`Zmin`/`SRID`/`Stbox_expand_time`. Decide: `xmax` vs `xMax`; keep `SRID` (acronym) or lowercase. |
| **D** | **Internal leaked onto surface** | 111 | Policy | type I/O + operator support (in/out/recv/send/cmp/hash/eq/lt/…). Decide: exclude from the generated user surface entirely. |
| **E** | **Underscored user-facing** | 94 | camelCase | by family below — but **some are intentional** dimension-encoding aliases (e.g. `set_left` ≡ "before" on a 1-D ordered domain, per the positional bare-name policy), so review per family. |

**E by family (distinct `@sqlfn`):** h3 27 · temporal 19 · tbox 8 · cbuffer 6 ·
tjsonb 6 · jsonbset 5 · tpcbox 5 · set 4 · span 4 · time 2 · stbox 2 · misc 4
(`hash_extended`, `lower_inc`, `tcbuffer_constructor`, `overlaps_bbox`).

### Why RFC, not a per-binding fix
The camelCase `@sqlfn` is the **single dispatch key every binding generates from**. An
underscore leak, a stale dialect name, or a copy-paste mistag there propagates to *every*
engine identically. Both §1 (overload resolution) and §2 (naming) belong in one RFC
section: *"User-facing SQL names are camelCase; polymorphic names resolve by (arity,
type); internal functions are off-surface."*

### Open questions for the maintainers
1. **A** — `Xmax`/`Tmax` → `xmax` or `xMax`? Keep `SRID` uppercase (acronym)?
2. **D** — confirm the 111 machinery tags are excluded from the generated surface.
3. **E** — per family: which are genuine camelCase fixes vs intentional positional/
   dimension-encoding aliases (the `set_left ≡ before` class).
4. **B1/B2** — confirm the `stbox.c` `temporal_below` block is a copy-paste to correct at
   source, and prioritize the MEOS-API extractor fix for the close-tag mis-attribution.

---

*Evidence: MobilitySpark `feat/generated-dispatch` / MobilityDB-BerlinMOD
`feat/berlinmod-benchmark` at `ecosystem-pin-2026-06-15f`; full-bench log 9/18 pass,
0 crashes.*
