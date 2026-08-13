#!/usr/bin/env python3
"""Report the index terms an entry documents but does not index.

An entry indexes every function and operator its signatures declare, in both
languages. The signatures are the authority: a name that appears in a signature
line belongs in the index, and the two languages document the same signatures,
so their index terms agree.

``--report`` prints, per entry, the terms the signatures call for against the
terms each language carries.
"""
import io
import os
import re
import sys

ROOT = os.path.dirname(os.path.dirname(os.path.dirname(os.path.dirname(
    os.path.abspath(__file__)))))

# an operator group -- "set {+, -, *} set" -- and a bare operator between two
# operands, as the signature lines spell them
GROUP = re.compile(r"\{([^{}]*?)\}")
FN = re.compile(r"^([A-Za-z][A-Za-z0-9_]*)\(")


def sql_names():
    """Every function and aggregate the SQL declares."""
    names = set()
    for base, _dirs, fs in os.walk(os.path.join(ROOT, "mobilitydb/sql")):
        for f in fs:
            if f.endswith(".in.sql"):
                s = io.open(os.path.join(base, f), encoding="utf-8").read()
                names |= set(re.findall(
                    r"CREATE (?:OR REPLACE )?(?:FUNCTION|AGGREGATE) "
                    r"([A-Za-z][A-Za-z0-9_]*)\(", s))
    return names


def entries(path):
    """id -> (indexed terms, signature lines) for every entry of a chapter."""
    s = io.open(path, encoding="utf-8").read()
    out = {}
    for p in re.split(r'(?=<listitem xml:id=")', s)[1:]:
        m = re.match(r'<listitem xml:id="([^"]+)"', p)
        if not m:
            continue
        blk = re.split(r"</listitem>", p, 1)[0]
        terms = re.findall(r"<primary><varname>([^<]+)</varname>", blk)
        sigs = re.findall(r"<para><varname>(.+?)</varname></para>", blk, re.S)
        out[m.group(1)] = (terms, sigs)
    return out


def wanted(sigs, declared):
    """The terms the signature lines call for, in order of appearance."""
    out = []
    for sig in sigs:
        fn = FN.match(sig.strip())
        if fn and fn.group(1) in declared:
            if fn.group(1) not in out:
                out.append(fn.group(1))
            continue
        for grp in GROUP.findall(sig):
            for op in (o.strip() for o in grp.split(",")):
                # an operand placeholder is a word; an operator is punctuation
                if op and not re.match(r"^[A-Za-z]", op) and op not in out:
                    out.append(op)
    return out


def main():
    declared = sql_names()
    docs = os.path.join(ROOT, "doc")
    for f in sorted(os.listdir(docs)):
        if not f.endswith(".xml"):
            continue
        es = os.path.join(docs, "es", f)
        if not os.path.exists(es):
            continue
        en_e, es_e = entries(os.path.join(docs, f)), entries(es)
        for i, (terms, sigs) in en_e.items():
            if i not in es_e:
                continue
            es_terms = es_e[i][0]
            if set(terms) == set(es_terms):
                continue
            want = wanted(sigs, declared)
            print("== %-30s %s" % (f, i))
            print("   signatures call for : %s" % want)
            print("   English indexes     : %s" % terms)
            print("   Spanish indexes     : %s" % es_terms)
    return 0


if __name__ == "__main__":
    sys.exit(main())
