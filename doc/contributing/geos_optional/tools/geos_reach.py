#!/usr/bin/env python3
#
# This MobilityDB code is provided under The PostgreSQL License.
# Copyright (c) 2016-2026, Université libre de Bruxelles and MobilityDB
# contributors
#
# MobilityDB includes portions of PostGIS version 3 source code released
# under the GNU General Public License (GPLv2 or later).
# Copyright (c) 2001-2026, PostGIS contributors
#
# Permission to use, copy, modify, and distribute this software and its
# documentation for any purpose, without fee, and without a written
# agreement is hereby granted, provided that the above copyright notice and
# this paragraph and the following two paragraphs appear in all copies.
#
# IN NO EVENT SHALL UNIVERSITE LIBRE DE BRUXELLES BE LIABLE TO ANY PARTY FOR
# DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES, INCLUDING
# LOST PROFITS, ARISING OUT OF THE USE OF THIS SOFTWARE AND ITS DOCUMENTATION,
# EVEN IF UNIVERSITE LIBRE DE BRUXELLES HAS BEEN ADVISED OF THE POSSIBILITY
# OF SUCH DAMAGE.
#
# UNIVERSITE LIBRE DE BRUXELLES SPECIFICALLY DISCLAIMS ANY WARRANTIES,
# INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY
# AND FITNESS FOR A PARTICULAR PURPOSE. THE SOFTWARE PROVIDED HEREUNDER IS ON
# AN "AS IS" BASIS, AND UNIVERSITE LIBRE DE BRUXELLES HAS NO OBLIGATIONS TO
# PROVIDE MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS.
#

"""Public MEOS functions that can reach a GEOS fall-back.

It reads every function definition of meos/src (static ones included), its
body by brace counting (#body_from) and its @ingroup, with two choices that
matter: #strip_comments blanks comments before bodies are read, and
an edge is ANY reference to a known function name in a body, so a function
passed as a pointer (geo_overlay_lifted(.., geom_intersection2d_route)) counts.
The walk goes up through public functions too: a public function calling a
public function that needs GEOS needs it as well.

usage: geos_reach.py <meos/src> TARGET [TARGET...]
prints: public function, its @ingroup, its file, the targets it reaches, the
shortest chain of calls down to one (ties broken by name), and every gateway
(a function referring directly to a target) it can reach a target through
"""
import collections, os, re, sys

src, targets = sys.argv[1], sys.argv[2:]
KEYWORDS = {"if", "for", "while", "switch", "return", "else", "do", "sizeof", "case"}
HEAD = re.compile(r"^(?:[A-Za-z_][\w \t\*]*?[\s\*])?([a-z_][a-z0-9_]*)\s*\(")
DOC = re.compile(r"/\*\*(.*?)\*/", re.S)

def strip_comments(t):
    t = re.sub(r"/\*.*?\*/", lambda m: re.sub(r"[^\n]", " ", m.group(0)), t, flags=re.S)
    return re.sub(r"//[^\n]*", "", t)

def body_from(text, i):
    depth = 0
    for j in range(i, len(text)):
        if text[j] == "{":
            depth += 1
        elif text[j] == "}":
            depth -= 1
            if depth == 0:
                return text[i:j + 1], j + 1
    return text[i:], len(text)

funcs = {}
dups = collections.defaultdict(list)
for root, _, files in os.walk(src):
    for fn in files:
        if not fn.endswith(".c"):
            continue
        path = os.path.join(root, fn)
        raw = open(path, encoding="utf-8", errors="replace").read()
        docs = [(m.start(), m.end(), m.group(1)) for m in DOC.finditer(raw)]
        text = strip_comments(raw)          # same length and line layout as raw
        lines = text.split("\n")
        offs, o = [], 0
        for ln in lines:
            offs.append(o)
            o += len(ln) + 1
        k = 0
        while k < len(lines):
            ln = lines[k]
            m = HEAD.match(ln)
            if not m or m.group(1) in KEYWORDS or ln.startswith((" ", "\t", "#", "/", "*")):
                k += 1
                continue
            start = offs[k]
            p = text.find("(", start)
            depth, q = 0, p
            while q < len(text):
                if text[q] == "(":
                    depth += 1
                elif text[q] == ")":
                    depth -= 1
                    if depth == 0:
                        break
                q += 1
            after = text[q + 1:q + 200].lstrip()
            if not after.startswith("{"):
                k += 1
                continue
            brace = text.find("{", q)
            body, end = body_from(text, brace)
            name = m.group(1)
            doc = None
            for ds, de, dt in docs:
                if de <= start and start - de < 40:
                    doc = dt
            grp = re.search(r"@ingroup\s+(\w+)", doc or "")
            rel = os.path.relpath(path, src)
            dups[name].append(rel)
            funcs.setdefault(name, dict(
                file=rel, group=grp.group(1) if grp else None,
                external=bool(grp) and "internal" not in grp.group(1),
                idents=set(re.findall(r"\b([a-z_][a-z0-9_]*)\b", body)) - {name}))
            k = text.count("\n", 0, end) + 1

callers = collections.defaultdict(set)
for n, f in funcs.items():
    for c in f["idents"]:
        if c in funcs:
            callers[c].add(n)

reach = collections.defaultdict(set)        # function -> targets it reaches
chain = {}                                  # function -> shortest chain down to a target
for t in targets:
    seen, queue = {t}, collections.deque([t])
    chain.setdefault(t, [t])
    parent = {t: None}
    while queue:
        x = queue.popleft()
        reach[x].add(t)
        if x not in chain or len(chain[x]) > 0:
            path, y = [], x
            while y is not None:
                path.append(y)
                y = parent[y]
            if x not in chain or len(path) < len(chain[x]):
                chain[x] = path
        for c in sorted(callers[x]):     # sorted: the chain chosen among ties is stable
            if c not in seen:
                seen.add(c)
                parent[c] = x
                queue.append(c)

# A gateway is a function referring directly to a target. A public function may
# reach a target through several gateways; all of them are listed, since the
# one on the shortest chain is only one of possibly several equally short ones
gateways = sorted({c for t in targets for c in callers[t]} - set(targets))
ancestors = {}
for g in gateways:
    seen, stack = {g}, [g]
    while stack:
        x = stack.pop()
        for c in callers[x]:
            if c not in seen:
                seen.add(c)
                stack.append(c)
    ancestors[g] = seen

pub = sorted((n for n in reach if funcs.get(n, {}).get("external")),
             key=lambda n: (funcs[n]["group"], n))
for n in pub:
    f = funcs[n]
    via = [g for g in gateways if n in ancestors[g]] or ["(itself)"]
    print("%s\t%s\t%s\t%s\t%s\t%s" % (n, f["group"], f["file"],
                                      ",".join(sorted(reach[n])),
                                      " -> ".join(chain[n]), ",".join(via)))
print("# public functions: %d" % sum(1 for f in funcs.values() if f["external"]),
      file=sys.stderr)
print("# public functions reaching a target: %d" % len(pub), file=sys.stderr)
amb = [n for n in reach if len(dups[n]) > 1]
if amb:
    print("# names defined more than once (first definition used): %s"
          % " ".join(sorted(amb)), file=sys.stderr)
