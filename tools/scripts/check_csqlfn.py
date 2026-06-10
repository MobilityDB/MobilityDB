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


def delegate(body):
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
                pg, doc, body = mt.group(1), mt.group(2) or '', mt.group(3) or ''
                sf = re.search(r'@sqlfn\s+(.+)', doc)
                so = re.search(r'@sqlop\s+(.+)', doc)
                dg, how = delegate(body)
                rows.append((pg, sf.group(1).strip() if sf else '',
                             so.group(1).strip() if so else '', dg, how,
                             os.path.basename(path)))
    return rows


def ingroup_block(lines, i):
    """Scan one @ingroup block; return (name, has_csqlfn, is_binding_api, end)."""
    has = False
    j = i
    while j < len(lines) and lines[j].strip() != '*/':
        if '@csqlfn' in lines[j]:
            has = True
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
    return name, has, is_api, j


# Datum-signature functions are the generic Datum workers (one C function
# polymorphic over every base type); they are intentionally NOT tagged -- the
# @csqlfn link lives on their typed instantiations in *_meos.c, so they are
# excluded here and the guard does not flag them.
def meos_public(dirs):
    """Map meos_fn -> (file, has_csqlfn) for every binding-API MEOS function."""
    out = {}
    for d in dirs:
        for path in sorted(glob.glob(f'{ROOT}/meos/src/{d}/**/*.c', recursive=True)):
            lines = read_text(path).split('\n')
            i = 0
            while i < len(lines):
                if '@ingroup meos_' in lines[i]:
                    name, has, is_api, end = ingroup_block(lines, i)
                    if name and is_api:
                        out[name] = (os.path.basename(path), has)
                    i = end
                i += 1
    return out


def cap(fn):
    """Capitalize the first letter (the PG wrapper own-stem convention)."""
    return fn[0].upper() + fn[1:]


def resolve(dirs):
    """Return (rows, auto, review): auto = safely-resolvable missing @csqlfn links."""
    rows = pg_wrappers(dirs)
    pgnames = {r[0] for r in rows}
    ptr_deleg = {}      # meos_fn -> wrappers binding it via &pointer (authoritative)
    for pg, _sf, _so, dg, how, _f in rows:
        if dg and how == 'ptr':
            ptr_deleg.setdefault(dg, set()).add(pg)
    auto, review = [], []
    for fn, (f, has) in meos_public(dirs).items():
        if has:
            continue
        if cap(fn) in pgnames:
            auto.append((fn, cap(fn), f, 'ownstem'))
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


def main():
    """Dispatch on the mode argument and report the @csqlfn coverage."""
    mode = sys.argv[1] if len(sys.argv) > 1 else '--gaps'
    dirs = sys.argv[2:] or ['temporal', 'geo', 'cbuffer', 'npoint', 'pose', 'rgeo',
                            'h3', 'pointcloud']
    rows, auto, review = resolve(dirs)
    if mode == '--table':
        for pg, sf, so, dg, _how, _f in sorted(rows):
            print(f'{pg:42s} sqlfn={sf:24s} sqlop={so:10s} -> meos:{dg or "-"}')
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
