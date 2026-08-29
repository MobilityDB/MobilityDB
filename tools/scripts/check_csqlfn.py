#!/usr/bin/env python3
#
# This MobilityDB code is provided under The PostgreSQL License.
# Copyright (c) 2016-2025, Universite libre de Bruxelles and MobilityDB
# contributors
#
"""Doxygen @csqlfn completeness checker / table builder for the MEOS API."""
#
# The PG wrappers (mobilitydb/src/**.c) are the authoritative source: each
# PG_FUNCTION_INFO_V1 wrapper carries @sqlfn + @sqlop and delegates to exactly
# one MEOS function (via an &fn function pointer, or its returned call). The
# MEOS function that a wrapper binds MUST carry the reverse link @csqlfn #Wrapper
# so the binding code generators (PyMEOS / JMEOS / MEOS.NET / ...) can derive the
# SQL function and operator for each MEOS API function.
#
# Usage:
#   check_csqlfn.py --table [dir...]   print the (meos_fn, @csqlfn, @sqlfn, @sqlop) table
#   check_csqlfn.py --gaps  [dir...]   list MEOS API functions missing the @csqlfn link
#   check_csqlfn.py --fix   [dir...]   insert the auto-resolvable @csqlfn tags
#
# Exit status is non-zero when --gaps finds an auto-resolvable gap (CI guard).

import collections
import glob
import os
import re
import sys

ROOT = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
KW = {'if', 'for', 'while', 'return', 'sizeof', 'switch', 'else', 'do', 'assert',
      'case'}


def read_text(path):
    """Read a source file as UTF-8 text."""
    with open(path, encoding='utf-8') as fp:
        return fp.read()


def meos_defined():
    """Set of every function name defined in the MEOS sources."""
    names = set()
    for path in glob.glob(f'{ROOT}/meos/src/**/*.c', recursive=True):
        names |= set(re.findall(r'^([a-z][A-Za-z0-9_]+)\(', read_text(path), re.M))
    return names - KW


MEOS = meos_defined()


# A wrapper body ends at the first line that is a lone closing brace. Reading
# on to the next PG_FUNCTION_INFO_V1 swallows any static helper defined in
# between, and the helper's own call then answers for the wrapper: the last
# wrapper before Jsonbset_path_match_common resolves to jsonbset_path_match,
# one operation off, for every group of jsonb wrappers that ends in a helper.
BODY_END = re.compile(r'^\}$', re.M)

# A wrapper that shares its work with its siblings holds no call of its own:
# it passes fcinfo and the flag that tells it apart to a static helper of the
# family, and the helper makes the call. The delegate is the helper's.
COMMON_CALL = re.compile(r'\b([A-Z][A-Za-z0-9_]*_common)\s*\(\s*fcinfo\b')


def function_body(src, name):
    """The body of the function @p name defined in @p src, or the empty string."""
    m = re.search(r'^%s\(' % re.escape(name), src, re.M)
    return wrapper_body(src[m.start():]) if m else ''


def wrapper_body(text):
    """The text up to the lone closing brace that ends the first function."""
    m = BODY_END.search(text)
    return text[:m.start()] if m else text


def delegate(body, src=''):
    """Return the single MEOS function a PG wrapper binds (&fn pointer first)."""
    for fn in re.findall(r'&([a-z][A-Za-z0-9_]+)\b', body):
        if fn in MEOS:
            return fn, 'ptr'
    m = re.search(r'\b(?:result|res)\s*=\s*([a-z][A-Za-z0-9_]+)\s*\(', body)
    if m and m.group(1) in MEOS:
        return m.group(1), 'ret'
    m = re.search(r'PG_RETURN_\w+\(\s*([a-z][A-Za-z0-9_]+)\s*\(', body)
    if m and m.group(1) in MEOS:
        return m.group(1), 'ret'
    m = COMMON_CALL.search(body)
    if m and src:
        helper = function_body(src, m.group(1))
        if helper:
            fn, _how = delegate(helper)
            if fn:
                return fn, 'common'
    return None, None


def pg_wrappers(dirs):
    """List of (PgName, sqlfn, sqlop, meos_fn, how, file)."""
    rows = []
    for d in dirs:
        for path in glob.glob(f'{ROOT}/mobilitydb/src/{d}/**/*.c', recursive=True):
            src = read_text(path)
            for mt in re.finditer(
                    r'PG_FUNCTION_INFO_V1\((\w+)\);\s*\n(/\*\*.*?\*/)?\s*'
                    r'(.*?)(?=\nPGDLLEXPORT|\nPG_FUNCTION_INFO_V1|\Z)', src, re.S):
                pg, doc = mt.group(1), mt.group(2) or ''
                body = wrapper_body(mt.group(3) or '')
                sf = re.search(r'@sqlfn\s+(.+)', doc)
                so = re.search(r'@sqlop\s+(.+)', doc)
                dg, how = delegate(body, src)
                rows.append((pg, sf.group(1).strip() if sf else '',
                             so.group(1).strip() if so else '', dg, how,
                             os.path.basename(path)))
    return rows


# The catalog reads a tag reference as '#(\w+)\s*\(\)', so '#Name ()' with a
# stray space parses there and must parse here too: a checker stricter than the
# generator reports a gap the generator does not have.
CSQLFN_REF = re.compile(r'#(\w+)\s*\(\)')


def ingroup_block(lines, i):
    """Scan one @ingroup block; return (name, refs, is_binding_api, internal, end)."""
    refs = set()
    internal = False
    j = i
    tag = False
    while j < len(lines) and lines[j].strip() != '*/':
        if '@ingroup meos_internal' in lines[j]:
            internal = True
        if '@csqlfn' in lines[j]:
            tag = True
        elif re.search(r'@\w', lines[j]):
            tag = False
        if tag:
            refs |= set(CSQLFN_REF.findall(lines[j]))
        j += 1
    k = j + 1
    name = None
    while k < len(lines) and k < j + 6:
        mm = re.match(r'^([a-z][A-Za-z0-9_]+)\s*\(', lines[k])
        if mm:
            name = mm.group(1)
            break
        k += 1
    is_api = False
    if name:
        sig = ' '.join(lines[k:k + 4])
        sig = sig[:sig.find(')') + 1] if ')' in sig else sig
        is_api = 'Datum' not in sig  # skip generic Datum workers (not binding API)
    return name, refs, is_api, internal, j


# Datum-signature functions are the generic Datum workers (one C function
# polymorphic over every base type); they are intentionally NOT tagged -- the
# @csqlfn link lives on their typed instantiations in *_meos.c, so they are
# excluded here and the guard does not flag them.
def meos_public(dirs):
    """Map meos_fn -> (file, refs, internal) for every binding-API MEOS function."""
    out = {}
    for d in dirs:
        for path in sorted(glob.glob(f'{ROOT}/meos/src/{d}/**/*.c', recursive=True)):
            lines = read_text(path).split('\n')
            i = 0
            while i < len(lines):
                if '@ingroup meos_' in lines[i]:
                    name, refs, is_api, internal, end = ingroup_block(lines, i)
                    if name and is_api:
                        out[name] = (os.path.basename(path), refs, internal)
                    i = end
                i += 1
    return out


def sql_bound():
    """Wrappers a CREATE FUNCTION binds, and the count of name-bound ones skipped.

    `AS 'MODULE_PATHNAME', 'Wrapper'` names its symbol; the bare
    `AS 'MODULE_PATHNAME'` form takes the SQL name as the symbol, which no
    wrapper of this convention carries, so those are counted and reported rather
    than silently dropped.
    """
    names, bare = set(), 0
    for path in glob.glob(f'{ROOT}/mobilitydb/sql/**/*.in.sql', recursive=True):
        text = read_text(path)
        names |= set(re.findall(r"MODULE_PATHNAME'\s*,\s*'(\w+)'", text))
        bare += len(re.findall(r"AS 'MODULE_PATHNAME'\s*$", text, re.M))
    return names, bare


def cap(fn):
    """Capitalize the first letter (the PG wrapper own-stem convention)."""
    return fn[0].upper() + fn[1:]


def resolve(dirs):
    """Return (rows, auto, review): auto = safely-resolvable missing @csqlfn links."""
    rows = pg_wrappers(dirs)
    pgnames = {r[0] for r in rows}
    ptr_deleg = {}      # meos_fn -> wrappers binding it via &pointer (authoritative)
    deleg_of = {}       # wrapper -> the MEOS function it binds, when resolved
    for pg, _sf, _so, dg, how, _f in rows:
        if dg and how == 'ptr':
            ptr_deleg.setdefault(dg, set()).add(pg)
        if dg:
            deleg_of[pg] = dg
    auto, review = [], []
    for fn, (f, refs, _internal) in meos_public(dirs).items():
        if refs:
            continue
        # A wrapper whose name is the function's own stem binds it, unless the
        # wrapper resolves to a different MEOS function: raquet_read and
        # raquet_read_bytes share the stem Raquet_read, which binds the bytes
        # form, so the name alone would tag the path form that nothing binds
        stem = cap(fn)
        if stem in pgnames and deleg_of.get(stem, fn) == fn:
            auto.append((fn, stem, f, 'ownstem'))
        elif fn in ptr_deleg and len(ptr_deleg[fn]) == 1:
            auto.append((fn, next(iter(ptr_deleg[fn])), f, 'ptr'))
        else:
            review.append((fn, f))
    return rows, auto, review


def add_tag(meos_dir_glob, fn, pg):
    """Insert a '@csqlfn #pg()' line into fn's doc comment. Return True if added."""
    for path in glob.glob(meos_dir_glob, recursive=True):
        with open(path, encoding='utf-8') as fp:
            lines = fp.readlines()
        for i, ln in enumerate(lines):
            if re.match(rf'^{re.escape(fn)}\s*\(', ln):
                j = i - 1
                while j >= 0 and lines[j].strip() != '*/':
                    j -= 1
                if j < 0:
                    return False
                k = j
                while k >= 0 and '/**' not in lines[k]:
                    k -= 1
                if '@csqlfn' in ''.join(lines[k:j + 1]):
                    return False
                lines.insert(j, f' * @csqlfn #{pg}()\n')
                with open(path, 'w', encoding='utf-8') as fp:
                    fp.writelines(lines)
                return True
    return False


def families():
    """Every family directory under mobilitydb/src, discovered not listed.

    A hardcoded subset silently drops a family: json, quadbin and raster were
    absent, so the gaps in them went unreported however often the check ran.
    Reading the directory keeps a family added later covered on its first run.
    """
    src = f'{ROOT}/mobilitydb/src'
    return sorted(d for d in os.listdir(src) if os.path.isdir(f'{src}/{d}'))


def tokens(wrapper):
    """The lowercase name parts of a wrapper, for the commuted-order test."""
    return sorted(wrapper.lower().split('_'))


def incomplete(dirs):
    """Return (commuted, mistagged, dispatch, bare) for the tagged MEOS functions.

    A tag must name EVERY wrapper a CREATE FUNCTION binds to the function, since
    the catalog gives a function the SQL surface of the wrappers its tag names
    and drops the rest. The three groups differ in what closing them costs:

    commuted  the omitted wrapper is the tagged one with its arguments the other
              way round -- one operation, one kernel, and the tag simply misses
              a name. This is the group the check FAILS on.
    mistagged the tag names a wrapper that binds a DIFFERENT function. Each one
              is read on its own: a body-resolved delegate cannot see the
              restrict-at/minus flag dispatch, so this group holds both real
              mistags and functions whose tag the resolver cannot follow.
    dispatch  the omitted wrapper reaches the function through a generic
              dispatcher or from another family. Naming it states that the
              function serves that SQL surface, which changes what the bindings
              generate, so it is a decision rather than a correction.
    """
    rows = pg_wrappers(dirs)
    bound, bare = sql_bound()
    deleg = {pg: dg for pg, _sf, _so, dg, _how, _f in rows if dg}
    binds = {}
    for pg, dg in deleg.items():
        if pg in bound:
            binds.setdefault(dg, set()).add(pg)
    public = sorted(meos_public(dirs).items())
    # A wrapper another function already claims is nobody else's omission. The
    # commuted npoint comparisons are the case: Ever_eq_npoint_tnpoint passes
    # &ever_eq_tnpoint_npoint, so the body resolver reads it as the temporal-first
    # function's, while the npoint-first function of that argument order exists and
    # claims it. Naming it twice gives two MEOS functions one SQL signature, which
    # is the per-wrapper attachment the type scope exists to prevent.
    claimed = {}
    for fn, (_f, refs, _internal) in public:
        for pg in refs:
            claimed.setdefault(pg, set()).add(fn)
    commuted, mistagged, dispatch = [], [], []
    for fn, (f, refs, internal) in public:
        if not refs or internal:
            continue
        missing = sorted(pg for pg in binds.get(fn, set()) - refs
                         if not claimed.get(pg, set()) - {fn})
        if not missing:
            continue
        wrong = sorted(r for r in refs if deleg.get(r, fn) != fn)
        if wrong:
            mistagged.append((fn, f, sorted(refs), missing, wrong))
            continue
        for pg in missing:
            if any(tokens(pg) == tokens(r) for r in refs):
                commuted.append((fn, f, pg))
            else:
                dispatch.append((fn, f, pg))
    return commuted, mistagged, dispatch, bare


# A wrapper is bound by one CREATE FUNCTION per SQL overload, and each overload
# is answered by at most one MEOS function, so a wrapper carrying MORE @csqlfn
# tags than it has SQL declarations has tags naming no callable surface. That is
# not cosmetic: a binding generating from the catalog registers one function per
# tag, and DuckDB, which cannot overload on return type, answers
# `Could not choose a best candidate function for the function call "Xmax(tbox)"`
# once the int and bigint accessors register beside the float one -- a function
# that works today becomes uncallable, and the surface diff reports it as a
# clean addition.
#
# The invariant only names CANDIDATES. A wrapper that dispatches on subtype
# reaches several MEOS functions through one declaration legitimately, so each
# row is read before its tag is touched, exactly as the gap side is.
def overtagged():
    """Return (rows, excess): wrappers carrying more @csqlfn tags than SQL declarations."""
    tags = collections.Counter()
    for path in glob.glob(f'{ROOT}/meos/src/**/*.c', recursive=True):
        for m in re.finditer(r'@csqlfn\s+([^\n*]*)', read_text(path)):
            for w in re.findall(r'#(\w+)\(\)', m.group(1)):
                tags[w] += 1
    decl = collections.Counter()
    for path in glob.glob(f'{ROOT}/mobilitydb/sql/**/*.in.sql', recursive=True):
        for w in re.findall(r"MODULE_PATHNAME'\s*,\s*'(\w+)'", read_text(path)):
            decl[w] += 1
    rows = [(w, t, decl.get(w, 0)) for w, t in tags.items() if t > decl.get(w, 0)]
    rows.sort(key=lambda r: (-(r[1] - r[2]), r[0]))
    return rows, sum(t - d for _, t, d in rows)


def main():
    """Dispatch on the mode argument and report the @csqlfn coverage."""
    mode = sys.argv[1] if len(sys.argv) > 1 else '--gaps'
    dirs = sys.argv[2:] or families()
    rows, auto, review = resolve(dirs)
    if mode == '--table':
        for pg, sf, so, dg, _how, _f in sorted(rows):
            print(f'{pg:42s} sqlfn={sf:24s} sqlop={so:10s} -> meos:{dg or "-"}')
    elif mode == '--complete':
        commuted, mistagged, dispatch, bare = incomplete(dirs)
        print(f'@csqlfn tags omitting a COMMUTED wrapper: {len(commuted)}')
        for fn, f, pg in commuted:
            print(f'  {f:26s} {fn:40s} += #{pg}()')
        print(f'\ntag names a wrapper of another function (read each): '
              f'{len(mistagged)}')
        for fn, f, refs, missing, wrong in mistagged:
            print(f'  {f:26s} {fn:40s} tag={refs} of={wrong} omits={missing}')
        print(f'\nreached through a dispatcher or another family (a decision): '
              f'{len(dispatch)} over {len({fn for fn, _f, _pg in dispatch})} '
              f'functions')
        print(f'\n{bare} bare "AS \'MODULE_PATHNAME\'" bindings carry no wrapper '
              f'symbol and are outside this check')
        sys.exit(1 if commuted else 0)
    elif mode == '--overtagged':
        rows, excess = overtagged()
        print(f'wrappers carrying more @csqlfn tags than SQL declarations: '
              f'{len(rows)} ({excess} excess tags)')
        for w, t, d in rows:
            print(f'  {w:38s} tags={t:<3} declarations={d}')
        sys.exit(0)
    elif mode == '--fix':
        n = 0
        for fn, pg, f, _how in auto:
            if add_tag(f'{ROOT}/meos/src/**/{f}', fn, pg):
                n += 1
        print(f'inserted {n} @csqlfn tags')
    else:
        print(f'auto-resolvable @csqlfn GAPS: {len(auto)}')
        for fn, pg, f, _how in sorted(auto):
            print(f'  {f:26s} {fn:40s} -> #{pg}')
        print(f'\nneeds-review (helpers/workers/divergent): {len(review)}')
        sys.exit(1 if auto else 0)


if __name__ == '__main__':
    main()
