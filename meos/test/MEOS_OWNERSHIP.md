# MEOS smoke-test memory ownership rules

## Public API: copy-returning functions

`temporal_start_instant(temp)` → returns an owned copy via `tinstant_copy`.
Caller must `free()` the result.

All `trgeo_start_instant`, `trgeo_end_instant`, `trgeo_instant_n` similarly
return fresh copies.  Callers must free them.

## Internal API: view-returning functions (`meos_internal.h`)

Functions ending in `_inst` or `_inst_p` return **VIEW** pointers directly
into the temporal struct.  **Do not free them.**

| Function | Returns | Free? |
|---|---|---|
| `temporal_start_inst(temp)` | VIEW into temp | NO |
| `temporal_insts_p(temp, &n)` | newly-allocated array of VIEW pointers | free the ARRAY, NOT the elements |
| `tsequence_insts_p(seq)` | newly-allocated array of VIEW pointers | free the ARRAY, NOT the elements |
| `temporal_min_inst_p(temp)` | VIEW into temp | NO |

## Array-returning functions: element ownership depends on type

| Function | Element ownership | How to free |
|---|---|---|
| `trgeo_instants(temp, &n)` | COPIES (`geo_tposeinst_to_trgeo`) | free each element AND the array |
| `trgeo_sequences(temp, &n)` | COPIES (`geo_tposeseq_to_trgeo`) | free each element AND the array |
| `trgeo_segments(temp, &n)` | COPIES | free each element AND the array |
| `cbufferset_values(s)` | COPIES (`datum_copy`) | free each element AND the array |
| `temporal_instants(temp, &n)` | COPIES (`tinstant_copy`) | free each element AND the array |

## `temporal_append_tinstant` consume semantics

When called with `expand=true` and the input sequence has no free slot,
`temporal_append_tinstant` frees the input sequence and returns a new one.
The canonical usage:
```c
temp = temporal_append_tinstant(temp, inst, interp, maxdist, maxt, true);
```
After this call, the original `temp` pointer must NOT be freed again.
If the return value is NULL the function failed WITHOUT consuming `temp`;
any internal allocations made before the failure ARE the callee's
responsibility to free before returning NULL.

## Harness variables: what to free vs. what not to free

| Variable | Created by | Free? |
|---|---|---|
| `trgeo_inst1` | `trgeoinst_make(...)` | YES |
| `trgeo_inst2` | `trgeoinst_make(...)` | YES |
| `trgeo_seq1` | `trgeo_append_tinstant(inst1, ...)` | YES (returned sequence owns storage) |
| `tcbuffer_inst1` | `temporal_start_inst(tcbuffer1)` (view) | NO |
| `tcbuffer_tseq1` | cast of `tcbuffer1` | NO (alias) |
| `tnpoint_inst1` | `temporal_start_inst(tnpoint1)` (view) | NO |
| `tpose_inst1` | `temporal_start_instant(tpose1)` (copy) | YES |
