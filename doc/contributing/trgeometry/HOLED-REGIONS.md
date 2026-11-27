<!--
  MobilityDB — The temporal rigid geometries: Holed regions
  Copyright(c) MobilityDB Contributors

  This documentation is licensed under a Creative Commons Attribution-Share
  Alike 3.0 License: https://creativecommons.org/licenses/by-sa/3.0/
-->

# Holed regions

**The rule, decided by the owner: a region with holes is supported.** A `trgeometry`
whose reference geometry is a polygon with holes, and a geometry with holes it is related to,
restricted by or measured against, are answered by every operation as the region they are: a point
inside a hole is outside the region. Until a kernel reads the holes of an operand, the operation
refuses it with an error naming the holes; it never answers for the region its exterior ring bounds. This is
decision 6 of [the implementation plan](IMPLEMENTATION-PLAN.md).

## What master answers

Measured at `7dc42788bb` on four operand pairs, each in linear and in step interpolation: a body,
the square [s, s+4] × [0, 4] with the hole (s+1, s+3) × (1, 3), moving from s = 0 to s = 10 in a day,
against the small square [2, 2.5]²; and a plain body against the region [-1, 12] × [-1, 5] with the
hole [4, 6] × [-0.5, 1.5]. A cell reads **answers**, **refuses** or **wrong**.

| Function | Holed body, linear | Holed body, step | Holed region, linear | Holed region, step |
|---|---|---|---|---|
| `eIntersects`, `aDisjoint`, `tIntersects` | refuses (the distance) | answers | refuses (the distance) | answers |
| `eContains`, `eTouches` | refuses (linear) | answers | refuses (linear) | answers |
| `eDwithin`, `tDwithin`, `tDistance`, `|=|` | refuses | refuses | refuses | refuses |
| `atGeometry`, `minusGeometry` | **wrong**: the hole is read as filled | answers | refuses | refuses |
| `traversedArea(x, true)` | **wrong**: the two placements, not the sweep | answers | not applicable | not applicable |

The two wrong answers:

- `atGeometry` of the holed body against [2, 2.5]² answers `[2001-01-01 00:00, 06:00]`, as the same
  square without its hole does. The small square lies in the hole while s ≤ 1, so the body meets it
  from 02:24 to 06:00. The clip of `trgeo_geom_clip.c` reads the exterior ring alone.
- `traversedArea(x, true)` of the holed body answers 24, the area of its two end placements, holes
  included. The swept region is [0, 14] × [0, 4], of area 56: each point of the hole of a placement
  is crossed by a wall of another. The same square without its hole answers 56.

The step answers read each placement as the static geometry it is, holes included, through the
relationships of the temporal geometry it traces. The distance refuses a holed operand by name in
both interpolations: its walk reads a convex polygon (`trgeo_distwalk.c`). The restriction refuses a
holed region before it reads the interpolation (`trgeometry_restrict_geom`), so a body that holds
each pose is refused where its placements would answer.

## What closes it

| Row | Task | Route |
|---|---|---|
| TG29 | `atGeometry` and `minusGeometry` of a holed body in linear motion | a refusal, as the restriction by a holed region gives, then the answer: the times the region meets the exterior ring and does not lie in a hole of the body |
| TG30 | `traversedArea(x, true)` of a holed body | a refusal, then the answer: the sweep of the exterior ring minus what no wall crosses |
| TG15 | `atGeometry`, `minusGeometry` against a holed region | step and discrete by the placements; linear by the containment of TG13 |
| TG12 | The distance of a holed body and to a holed region | [its note](NON-CONVEX-DISTANCE.md): the distance to a polygon with holes is the distance to its rings where the point lies outside, and 0 inside |
| TG13 | `eContains`, `eCovers`, `eTouches` in linear motion | [its note](RIGID-SWEPT-RELATIONSHIPS.md), which reads a holed operand by its rings |

The first step of TG29 and TG30 turns each wrong answer into a refusal, as the rule asks; the second
answers it. Each row is witnessed by a CGAL judge of the head's answers on a corpus of holed bodies
and regions, and measured against GEOS or the function of liblwgeom it replaces.
