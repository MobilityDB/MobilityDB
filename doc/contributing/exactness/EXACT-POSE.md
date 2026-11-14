<!--
  MobilityDB — Exactness: Temporal poses and pose chains
  Copyright(c) MobilityDB Contributors

  This documentation is licensed under a Creative Commons Attribution-Share
  Alike 3.0 License: https://creativecommons.org/licenses/by-sa/3.0/
-->

# Temporal poses and pose chains

**For discussion by the committers.**
Master at the time of writing: `990787c4fc`.

## 1. Summary

A pose is a position with an orientation, and a pose chain composes poses link by link, each in
the frame of the one before it. The position of a `tpose` moves linearly between two instants, as
a `tgeompoint` does, so every operation that reads only that position inherits the exactness of
the temporal points. What is not exact is everything the orientation enters: the angle a 2D pose
turns through, the rotation a 3D pose interpolates, and above all the place of a joint of a chain,
which composes positions rotated by orientations. This note lists where MEOS answers these
approximately today, what the exact answer is, and in which order the work comes. The semantics
each operation follows (which place of a chain it reads) are stated for users in
`doc/contributing/tpose_design_notes.md`, written with the branch `feat/tposechain-restrict-stbox`
(not merged).

The pose kernels this note asks for are new mechanisms, so they come after the consolidation of
the exact arithmetic ([The exact arithmetic of the geometry engine](EXACT-ARITHMETIC.md)), as every
new mechanism does.

## 2. How a pose and a pose chain move

`posesegm_interpolate` interpolates a pose between two instants:

- **Position**: linearly, `start * (1 - ratio) + end * ratio`, in two and three dimensions.
- **2D orientation**: the angle `theta` linearly along the shorter turn, wrapped into `(-pi, pi]`.
- **3D orientation**: the unit quaternion by SLERP, the second negated where the dot product of
  the two is negative, so the rotation takes the shorter arc.

A pose chain interpolates link by link, each link as a pose does. Joint `n` of a chain is the
composition of its first `n` links (`posechain_prefix_pose`), computed by `pose_compose_values`:
in two dimensions the child's translation rotated by the parent's angle, `sin` and `cos` taken in
doubles, and the angles added; in three dimensions the quaternion product; for a geographic chain
on the WGS-84 ellipsoid through Earth-centred coordinates.

## 3. Where MEOS answers approximately

### P1. Whether a 2D pose turns is decided by a tolerance

`posesegm_interpolate` keeps the start angle when `fabs(theta_delta) < MEOS_EPSILON`, and
interpolates it otherwise. A tolerance there decides an answer: two poses whose angles differ by
less than `MEOS_EPSILON` are read as not turning, so the angle between them answers the start angle
where the interpolation answers another double. The exact question is whether the two angles are
equal, `theta_delta == 0`, and its answer is a comparison of two doubles. This is one of the
tolerances beside an exact sign that the consolidation removes, and it belongs with its C steps.

### P2. A composed joint is a constructed point

`pose_compose_values` takes `sin` and `cos` of the parent's angle in doubles, so the position of
every joint past the first is a rounded point MEOS constructs. The exact rule applies its rule 4:
the first move is to stop constructing the point, deciding a question about a joint from the doubles
of its links where a sign can. Here no sign can in general: `cos` and `sin` of a double are not
rational, so whether a composed joint lies on one side of a line has no exact rational answer, and
a question about a composed joint is decided within the band of a constructed point,
`MEOS_GEOM_TOLERANCE + 4 * DBL_EPSILON * max|coord|`, as `geogpoint_meeting_band` reads it for
the points of two great circles. Where the angles are multiples of a
quarter turn, `sin` and `cos` are exact only up to the rounding of `M_PI / 2` (its cosine is
`6.1e-17`, not 0), so even those chains need the band.

No question here needs a machinery of its own; each has a survivor of the consolidation:

- **The rounding of the joint.** `sin` and `cos` are read once each, as the doubles nearest them,
  through the one search of EA6, `nearest_double_exact` with a `NearestSide`, as EA8 reads the
  sine and cosine of degrees (`sind_nearest`, `cosd_nearest`, MobilityDB #3637, merged). The
  composed joint is then the same double on every platform, where a call to the platform's `sin`
  gives the doubles of its libm.
- **The signs of a joint once it is constructed.** Its side of a line, its order along a segment
  and its distance to another point are signs on doubles, and they reduce to the orientation and
  dot signs already merged: `cross_product_sign`, `vector_triple_sign`,
  `point_segment_dot_sign_inputs`, `point_segment_offset_sign` and `point_within_distance_sign`,
  this last one deciding the band itself.
- **A filter it adds** follows the convention of the consolidation's third phase: a sign spells
  "undecided" as 2 inside the layer, and an enumeration keeps a value no answer takes.

### P3. A composed joint is held constant between two instants

The chain interpolates link by link, so between two instants a joint composed of more than one link
swings around the joints before it. MEOS holds it instead at its value of the start instant:

- `tposechain_to_tpose`, and through it `tposechain_to_tgeompoint` and
  `tposechain_to_tgeogpoint`, give a step function for a chain of more than one link, because the
  composed pose interpolated between two instants is not the pose interpolated between the two
  composed poses;
- distance and nearest approach (`tposechain_distance.c`) answer for that step function;
- the spatial relationships read the temporal point of that step function (`tposechain_tpoint`);
- the restriction of a chain to a box (`tposechain_restrict_stbox`, branch
  `feat/tposechain-restrict-stbox`, not merged) holds every joint past the first in the same way.

So each answers for a chain that jumps at its instants and rests between them, where the chain
itself moves. The exact answer reads the joint as the chain moves it. In two dimensions joint `n`
is `p1(t) + R(theta1(t)) p2(t) + R(theta1(t) + theta2(t)) p3(t) + ...`, each `p` and `theta`
linear in `t` between two instants, so each coordinate is a sum of terms `(a + b t) cos(c + d t)`
and `(a + b t) sin(c + d t)`. A joint entering a box is a root of such a sum against a border; the
nearest approach of a joint to a geometry is a root of its derivative. The roots are isolated in
doubles with a bound on their error (the value/error pairs of the filter tier) and dated to the
microsecond as [Exact crossings](EXACT-CROSSINGS.md) dates every crossing; a segment may hold more
than one, which is what [Any number of turning points](N-TURNING-POINTS.md) lets a lift carry. In
three dimensions the rotations are SLERP arcs, and the same holds with quaternion terms.

### P4. A 3D orientation between two instants is a rounded quaternion

SLERP evaluates `acos`, `sin` and a division in doubles, so a 3D pose between two instants holds a
rounded quaternion. The position is unaffected, so a `tpose` in three dimensions stays exact for
everything that reads its position; the rounding reaches only the composed joints of a 3D chain,
under P2 and P3.

### P5. A geographic chain composes on the ellipsoid

A geographic chain places each offset in the tangent plane of its parent on the WGS-84 ellipsoid,
through Earth-centred coordinates. Its joints are geodetic constructed points, and the rules for
them are those of [Geodetic operations](GEODETIC-OPERATIONS.md).

## 4. Order

1. **P1** with the consolidation's removal of the tolerances that stand beside an exact sign: the
   comparison of two doubles replaces `MEOS_EPSILON`, with a ledger of pose pairs whose angles
   differ by less than it.
2. **P2** rounds `sin` and `cos` of a composition once each through `nearest_double_exact`, and
   states the band of a composed joint once, in one function the band gate admits as a constructed
   point, which every reader of a composed joint calls and decides by `point_within_distance_sign`.
3. **P3** after the consolidation. The root isolation of a sum of terms `(a + b t) cos(c + d t)`
   is a capability, not a pose kernel: it enters the shared layer as a step of
   [The exact arithmetic of the geometry engine](EXACT-ARITHMETIC.md), on its value/error pairs,
   and the joints of a chain are its first reader, in two dimensions first, then in three, each
   judged by an oracle that composes the links on rationals for the positions and on a certified
   enclosure for `sin` and `cos`. The operations of
   P3 then read the joint as the chain moves it, each in its own pull request, and the step
   functions of `tposechain_to_tpose` stay what that conversion answers, a composed pose the chain
   holds at its instants.
4. **P4** and **P5** follow their geometry: P4 with P3 in three dimensions, P5 with the geodetic
   operations.
