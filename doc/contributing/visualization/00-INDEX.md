<!--
  MobilityDB — Visualization: index
  Copyright(c) MobilityDB Contributors

  This documentation is licensed under a Creative Commons Attribution-Share
  Alike 3.0 License: https://creativecommons.org/licenses/by-sa/3.0/
-->

# Every viewer over every engine

A viewer of the ecosystem shows the moving objects of any of its engines and holds no code of its
own for an engine. Display, like indexing with MEST, is a concern orthogonal to the engines: its
primitives live once in MEOS and reach every engine through the MEOS catalog. A viewer reads its
trips through MobilityAPI or from a file, holds them as MEOS values in MEOS.js (the browser) or
PyMEOS (QGIS and notebooks), and draws what MEOS answers: the position of a trip at the time of
the clock is `valueAtTimestamp`, never an interpolation the viewer computes.

These notes state the stack, what each viewer of the organization holds, the display primitives
MEOS carries and the ones it lacks, and the pull requests that bring every viewer onto the stack.
The release of the bindings the viewers install (MEOS.js, PyMEOS) from one tag of MobilityDB is
rows X5 to X8 of the generation plan of MobilityDB #3928 (`doc/contributing/generation/`); the
binary interchange is the input and output plan of MobilityDB #3788 (`doc/contributing/io/`); the
names of the MEOS functions are the portable naming notes of MobilityDB #2861.

## The notes

| Note | What it states |
|---|---|
| [Assessment](ASSESSMENT.md) | The stack, the display primitives of MEOS, what each viewer of the organization holds and the environment it serves, and the measurements of the ways to animate a trip |
| [Implementation plan](IMPLEMENTATION-PLAN.md) | The steps VS1 to VS9, one pull request each, the prerequisites V1 to V9 outside the viewers, their order, and what is left out |

## The stack

| Layer | Repository | Role |
|---|---|---|
| Model and wire format | MEOS | One data model and one serialization, the same bytes in every engine |
| Engines | MobilityDB, MobilityDuck, MobilitySpark, MobilityFlink, MobilityKafka, MobilityNebula | Compute; each is a backend of MobilityAPI |
| Gateway | MobilityAPI | The one contract a client reads: OGC API Moving Features, collections (Part 1) and streams (Part 4) |
| Browser library | MEOS.js: `meos.js/viewer`, `meos.js/deckgl` | Reads the trips of a file or a collection, decodes MEOS values, computes in the browser, builds the deck.gl layers |
| Desktop library | PyMEOS (`pymeos_cffi`) | Decodes MEOS values and computes in the Python of QGIS and of a notebook |
| Viewers | MobilityDeck, MOVE, MobilityOpenLayers, MobilityLeaflet, MobilityGeoServer, MEOS.js-examples, the notebooks of MobilityDeck | One viewer for each environment its users work in; the engine they show is a setting of MobilityAPI, not code |

## The rules

1. A viewer computes no temporal value: a position, a restriction, a speed is a MEOS function
   called through MEOS.js or PyMEOS.
2. A viewer reads its trips through MobilityAPI or from a file of MEOS values (hex WKB, one value
   per line); an engine reaches every viewer once it is a backend of MobilityAPI.
3. The code every browser viewer shares lives in MEOS.js (`meos.js/viewer`, `meos.js/deckgl`),
   never copied into a viewer.
4. Every viewer draws the same data for the same query, whichever engine answers it: the check of
   VS3 compares that data, not pixels.
5. A viewer stays available as the viewer of its environment; a viewer another subsumes in the same
   environment is archived, its README pointing to the one that subsumes it.

## Acceptance

Every viewer of the stack table reads its trips through MobilityAPI or a file, computes no temporal
value of its own, and shows the trips of VS3's check from every engine MobilityAPI serves; the demo
of VS9 plays any viewer over any engine.
