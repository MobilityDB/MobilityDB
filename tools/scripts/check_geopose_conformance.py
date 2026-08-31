#!/usr/bin/env python3
#
# This MobilityDB code is provided under The PostgreSQL License.
# Copyright (c) 2016-2026, Université libre de Bruxelles and MobilityDB
# contributors
#
# MobilityDB includes portions of PostGIS version 3 source code released
# under the GNU General Public License (GPLv2 or later).
# Copyright (c) 2001-2025, PostGIS contributors
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

"""Validate the emitted GeoPose documents against the normative OGC schemas.

MobilityDB implements the eight conformance classes of OGC GeoPose v1.0
(OGC 21-056r11): Basic-YPR, Basic-Quaternion, Advanced, the Regular and
Irregular Composite Sequence Series, the Stream, the Chain and the Graph.
Each class has a normative JSON schema, published under

    https://schemas.opengis.net/geopose/1.0/schemata/

A round-trip test proves that MobilityDB reads back what it writes, which
holds just as well for a document no other implementation accepts.  Agreement
with the schema is what a conformance claim rests on, so this check reads the
documents the tests emit and validates each one against the schema of its
class.

The documents come from the expected output of the regression tests, so no
database is needed: the file holds the exact bytes the implementation
produces, and a change to the encoding that breaks conformance changes that
file and fails here.

The schemas use eight keywords -- `$ref`, `definitions`, `description`,
`items`, `minItems`, `properties`, `required` and `type` -- with every `$ref`
local to `#/definitions`.  Validating that subset takes little code and keeps
the check free of any third-party dependency, which is why it is spelled out
below rather than delegated to a JSON Schema library.

Usage:
  check_geopose_conformance.py           validate every document (CI guard)
  check_geopose_conformance.py --list    print the documents and their class
  check_geopose_conformance.py --emit D  write one sample document per class
                                         into the directory D

A conformance submission carries a sample document per class it claims, named
by the key the OGC test suite reads it under.  `--emit` writes that set from
the documents this check already validates, so the submission carries the
exact bytes the implementation produces, beside a manifest naming the class,
the schema and the expected output each one comes from.  A document is written
only once it satisfies its schema, so the set can never claim more than the
check proves.

The two incremental stream documents have no SQL surface of their own, and
need none: a whole stream is `{header, streamElements[]}` written with the
functions that write them, against the outer frame anchored at the same first
instant, so each member IS the incremental document.  They are validated and
sampled from the whole stream that carries them.

Exit status is non-zero when a document does not satisfy its schema, or when
a conformance class emits no document at all.
"""

import json
import os
import sys

ROOT = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))

# The normative schemas, retrieved from schemas.opengis.net.
SCHEMA_DIR = os.path.join(ROOT, 'mobilitydb', 'test', 'pose', 'schemas')

# The expected outputs holding the emitted documents, one per family that
# writes a conformance class.
EXPECTED = [
    os.path.join(ROOT, 'mobilitydb', 'test', 'pose', 'expected',
        '103_pose_geopose.test.out'),
    os.path.join(ROOT, 'mobilitydb', 'test', 'posechain', 'expected',
        '555_tposechain_geopose.test.out'),
]

# A document is recognised by the member that only its class carries, tried in
# this order so that a Series is never mistaken for the Basic document of its
# inner frames.
CLASSES = [
    # A whole stream holds a header and its elements, so its own member names
    # it before either of theirs is looked for.
    ('streamElements', 'Stream',
        'GeoPose.Composite.Sequence.Stream.Schema.json'),
    ('innerFrameSeries', 'Regular Series',
        'GeoPose.Composite.Sequence.Series.Regular.Schema.json'),
    ('innerFrameAndTimeSeries', 'Irregular Series',
        'GeoPose.Composite.Sequence.Series.Irregular.Schema.json'),
    # A stream is written as two documents: the header once, then one element
    # per pose. `outerFrame` comes after the two Series members because a
    # Series carries an outer frame as well, and its own member names it more
    # precisely; a stream header is what is left holding only an outer frame.
    # A Chain names an outer frame as well, so the member holding its own
    # sequence of transformations comes before the one a stream header is
    # left holding alone.
    ('frameChain', 'Chain',
        'GeoPose.Composite.Chain.Schema.json'),
    # A graph names its frames in a list and its edges in another, and no other
    # class carries either member, so `frameList` identifies it on its own.
    ('frameList', 'Graph',
        'GeoPose.Composite.Graph.Schema.json'),
    ('streamElement', 'Stream element',
        'GeoPose.Composite.Sequence.StreamElement.Schema.json'),
    ('outerFrame', 'Stream header',
        'GeoPose.Composite.Sequence.StreamHeader.Schema.json'),
    # An Advanced document carries its position inside the frame it names, so
    # `frameSpecification` is what distinguishes it from a Basic one; both
    # carry a quaternion, so this comes first.
    ('frameSpecification', 'Advanced',
        'GeoPose.Advanced.Schema.json'),
    ('quaternion', 'Basic-Quaternion',
        'GeoPose.Basic.Quaternion.Schema.json'),
    ('angles', 'Basic-YPR',
        'GeoPose.Basic.YPR.Schema.json'),
]

# A Basic-Quaternion document that carries only what a static pose has to say
# satisfies the standard's stricter schema for the class as well, which admits
# `position` and `quaternion` and nothing else. A temporal instant additionally
# carries `validTime` and satisfies the permissive schema alone, so the strict
# one is demanded of exactly those documents that hold no time.
STRICT = ('Basic-Quaternion', 'validTime',
    'GeoPose.Basic.Strict_Quaternion.Schema.json')

# The classes the expected output has to contain for this check to mean
# anything. A whole stream is among them: a value a query already holds is
# written in one piece. The two INCREMENTAL stream documents have no SQL
# surface of their own -- a stream is emitted a piece at a time by a producer,
# which a query is not -- but a whole stream is built FROM them, member for
# member, so every one of them travels inside it and is demanded here too.
REQUIRED = ('Regular Series', 'Irregular Series', 'Basic-Quaternion',
    'Basic-YPR', 'Advanced', 'Stream', 'Stream header', 'Stream element',
    'Chain', 'Graph')

# The name the OGC test suite reads a class's sample document under, which is
# what `--emit` names the file. The suite tests seven classes and Irregular
# Series is not among them -- the standard defines it, the suite has no section
# for it -- so it carries no key and its sample is named after the class.
SUITE_KEYS = {
    'Basic-YPR': 'basicypr',
    'Basic-Quaternion': 'basicquaternion',
    'Advanced': 'advanced',
    'Chain': 'chain',
    'Graph': 'graph',
    'Regular Series': 'seriesregular',
    'Stream': 'streamrecord',
    'Stream element': 'streamelement',
    'Stream header': 'streamheader',
    'Irregular Series': None,
}

# Irregular Series carries no suite key, so its sample is named the way the
# suite names its sibling and the set reads uniformly.
UNKEYED_NAMES = {'Irregular Series': 'seriesirregular'}


TYPES = {
    'object': dict,
    'array': list,
    'string': str,
    'boolean': bool,
}


def one_type_ok(value, expected):
    """Return whether @p value has the single JSON Schema type @p expected."""
    if expected == 'null':
        return value is None
    if expected in ('number', 'integer'):
        # A JSON boolean is not a number, while Python bool derives from int.
        if isinstance(value, bool):
            return False
        if expected == 'integer':
            return isinstance(value, int)
        return isinstance(value, (int, float))
    return isinstance(value, TYPES[expected])


def type_ok(value, expected):
    """Return whether @p value has the JSON Schema type @p expected.
    @details The type of a member that the schemas leave optional is a list
    holding `null` beside the type the member has when it is present.
    """
    if isinstance(expected, list):
        return any(one_type_ok(value, e) for e in expected)
    return one_type_ok(value, expected)


def validate(value, schema, root, path, errors):
    """Collect into @p errors the ways @p value departs from @p schema."""
    ref = schema.get('$ref')
    if ref is not None:
        # Every reference in these schemas is local to `#/definitions`.
        target = root
        for step in ref.lstrip('#/').split('/'):
            target = target[step]
        validate(value, target, root, path, errors)
        return

    expected = schema.get('type')
    if expected is not None and not type_ok(value, expected):
        errors.append('%s: expected %s, found %s' %
            (path or '(document)', expected, type(value).__name__))
        return

    # A member the schema leaves optional is absent as JSON `null`, and then
    # carries neither the members nor the constraints of its object type.
    if isinstance(value, dict):
        for name in schema.get('required', []):
            if name not in value:
                errors.append('%s: missing required member `%s`' %
                    (path or '(document)', name))

        for name, sub in schema.get('properties', {}).items():
            if name in value:
                validate(value[name], sub, root, '%s.%s' % (path, name),
                    errors)

        # `additionalProperties: false` is what makes a schema strict: the
        # document may carry the members the schema names and no others.
        if schema.get('additionalProperties') is False:
            for name in value:
                if name not in schema.get('properties', {}):
                    errors.append('%s: carries `%s`, which the schema does '
                        'not allow' % (path or '(document)', name))

    items = schema.get('items')
    if items is not None and isinstance(value, list):
        for i, elem in enumerate(value):
            validate(elem, items, root, '%s[%d]' % (path, i), errors)

    least = schema.get('minItems')
    if least is not None and isinstance(value, list) and len(value) < least:
        errors.append('%s: holds %d items, %d required' %
            (path, len(value), least))


def classify(doc):
    """Return the conformance class of @p doc, or None when it has none."""
    for member, name, schema in CLASSES:
        if member in doc:
            return name, schema
    return None, None


def stream_parts(doc):
    """Yield the incremental documents a whole stream is built from.

    A whole stream is `{header, streamElements[]}`, and the encoder writes
    both members with the functions that write the standalone documents,
    against the outer frame anchored at the same first instant. Each member
    is therefore the incremental document itself, so a stream carries one
    header and one element per pose, and the two classes that have no SQL
    surface of their own are validated and sampled from it.
    """
    header = doc.get('header')
    if isinstance(header, dict):
        yield 'Stream header', header
    for elem in doc.get('streamElements') or []:
        if isinstance(elem, dict):
            yield 'Stream element', elem


def documents(path):
    """Yield every JSON document emitted as a result row of the tests."""
    with open(path, encoding='utf-8') as f:
        for line in f:
            # A result row is indented by one space; a query echoed back
            # carries the document inside a quoted literal.
            if not line.startswith(' {'):
                continue
            try:
                doc = json.loads(line.strip())
            except ValueError:
                continue
            if isinstance(doc, dict):
                yield doc


def emit(directory, samples, strict_class, strict_file):
    """Write one sample document per class into @p directory.

    Each file is named by the key the OGC test suite reads its class under,
    beside a manifest naming the class, the schema it satisfies and the
    expected output the bytes come from.
    """
    try:
        os.makedirs(directory, exist_ok=True)
    except OSError as e:
        print('check_geopose_conformance: cannot write to %s: %s' %
            (directory, e))
        return 1

    entries = []
    for name in sorted(samples):
        doc, schema_file, source, is_strict = samples[name]
        key = (SUITE_KEYS.get(name) or UNKEYED_NAMES.get(name) or
            name.lower().replace(' ', ''))
        filename = '%s.json' % key
        with open(os.path.join(directory, filename), 'w',
                encoding='utf-8') as f:
            json.dump(doc, f, indent=2)
            f.write('\n')
        entry = {
            'file': filename,
            'class': name,
            'suite_key': SUITE_KEYS.get(name),
            'schema': schema_file,
            'source': os.path.relpath(source, ROOT),
        }
        # The standard gives Basic-Quaternion a permissive and a strict schema
        # and the suite tests the permissive one, so a submission has to name
        # which form its sample takes.
        if name == strict_class:
            entry['strict_schema'] = strict_file if is_strict else None
        entries.append(entry)

    manifest = {'documents': entries}
    with open(os.path.join(directory, 'manifest.json'), 'w',
            encoding='utf-8') as f:
        json.dump(manifest, f, indent=2)
        f.write('\n')

    print('check_geopose_conformance: wrote %d sample documents and a '
        'manifest to %s' % (len(entries), directory))
    for entry in entries:
        print('  %-18s %s' % (entry['class'], entry['file']))
    return 0


def main():
    argv = sys.argv[1:]
    listing = '--list' in argv
    directory = None
    if '--emit' in argv:
        i = argv.index('--emit')
        if i + 1 >= len(argv) or argv[i + 1].startswith('--'):
            print('check_geopose_conformance: --emit needs a directory')
            return 1
        directory = argv[i + 1]

    for path in EXPECTED:
        if not os.path.isfile(path):
            print('check_geopose_conformance: no expected output at %s' %
                os.path.relpath(path, ROOT))
            return 1

    schemas = {}
    for _, name, filename in CLASSES:
        path = os.path.join(SCHEMA_DIR, filename)
        if not os.path.isfile(path):
            print('check_geopose_conformance: no schema at %s' %
                os.path.relpath(path, ROOT))
            return 1
        with open(path, encoding='utf-8') as f:
            schemas[name] = json.load(f)

    strict_class, strict_unless, strict_file = STRICT
    strict_path = os.path.join(SCHEMA_DIR, strict_file)
    if not os.path.isfile(strict_path):
        print('check_geopose_conformance: no schema at %s' %
            os.path.relpath(strict_path, ROOT))
        return 1
    with open(strict_path, encoding='utf-8') as f:
        strict_schema = json.load(f)

    total = 0
    failed = 0
    strict = 0
    seen = {}
    # The first conformant document of each class is the one `--emit` writes,
    # which makes the sample set a function of the committed bytes alone.
    samples = {}
    schema_of = dict((name, filename) for _, name, filename in CLASSES)
    for path, doc in [(p, d) for p in EXPECTED for d in documents(p)]:
        name, _ = classify(doc)
        if name is None:
            continue
        total += 1
        seen[name] = seen.get(name, 0) + 1
        errors = []
        validate(doc, schemas[name], schemas[name], '', errors)
        is_strict = False
        if name == strict_class and strict_unless not in doc:
            validate(doc, strict_schema, strict_schema, '', errors)
            strict += 1
            is_strict = not errors
        if not errors and name not in samples:
            samples[name] = (doc, schema_of[name], path, is_strict)
        # A whole stream is the two incremental documents composed, so each
        # one is validated in its own right rather than only as a member.
        if name == 'Stream':
            for sub, part in stream_parts(doc):
                total += 1
                seen[sub] = seen.get(sub, 0) + 1
                sub_errors = []
                validate(part, schemas[sub], schemas[sub], '', sub_errors)
                if sub_errors:
                    failed += 1
                    print('check_geopose_conformance: a %s document is not '
                        'conformant' % sub)
                    for e in sub_errors:
                        print('    %s' % e)
                    print('    %s' % json.dumps(part)[:200])
                elif sub not in samples:
                    samples[sub] = (part, schema_of[sub], path, False)
                if listing:
                    print('  %-18s %s' %
                        (sub, 'ok' if not sub_errors else 'INVALID'))
        if listing:
            print('  %-18s %s' % (name, 'ok' if not errors else 'INVALID'))
        if errors:
            failed += 1
            print('check_geopose_conformance: a %s document is not '
                'conformant' % name)
            for e in errors:
                print('    %s' % e)
            print('    %s' % json.dumps(doc)[:200])

    if not total:
        print('check_geopose_conformance: no GeoPose document found in %s' %
            ', '.join(os.path.relpath(p, ROOT) for p in EXPECTED))
        return 1

    missing = [name for name in REQUIRED if name not in seen]
    if missing:
        print('check_geopose_conformance: no document emitted for %s' %
            ', '.join(missing))
        return 1

    if failed:
        print('check_geopose_conformance: %d of %d documents are not '
            'conformant.' % (failed, total))
        return 1

    print('check_geopose_conformance: %d documents conform to the OGC '
        'GeoPose 1.0 schemas (%s); %d of them satisfy the strict %s schema '
        'as well.' % (total,
        ', '.join('%s %d' % (n, c) for n, c in sorted(seen.items())),
        strict, strict_class))

    if directory is not None:
        return emit(directory, samples, strict_class, strict_file)
    return 0


if __name__ == '__main__':
    sys.exit(main())
