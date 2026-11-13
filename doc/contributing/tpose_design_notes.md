<!--
  MobilityDB — Temporal Poses and Pose Chains: Design Notes
  Copyright(c) MobilityDB Contributors

  This documentation is licensed under a Creative Commons Attribution-Share
  Alike 3.0 License: https://creativecommons.org/licenses/by-sa/3.0/
-->

# Temporal Poses and Pose Chains — Design Notes

A pose is a position with an orientation, and a pose chain is an ordered list of poses, each one
placed in the frame of the one before it, as the joints of an arm are. Most operations on a chain
read one place of it: either the pose the whole chain composes to, or every joint along it. The
notes below state which place each operation reads, so that a user can tell what a query answers
before running it.

> **Audience**: a user writing SQL or MEOS code over `tpose` and `tposechain` values, and a
> contributor adding an operation to either type. The user manual (`doc/temporal_pose_types.xml`)
> documents each function; these notes state the choices that run across them.

---

## What a pose and a pose chain hold

A `pose` holds a position and an orientation: in two dimensions `x`, `y` and an angle `theta`, in
three dimensions `x`, `y`, `z` and a unit quaternion `W`, `X`, `Y`, `Z`.

A `posechain` holds at least one pose, its **links** (`PoseChain` in `meos/include/pose/posechain.h`).
The first link is expressed in the outer frame of the chain, the frame its SRID names; every later
link is expressed in the frame the link before it defines. OGC GeoPose 1.0 (clause 4.2.8) forbids an
inner frame from being topocentric, so a chain has one outer frame, hence one SRID and one dimension
for all of its links.

Composing the first `n` links gives the pose of **joint `n`** in the outer frame
(`posechain_prefix_pose`). Composing all of them gives the pose of the innermost frame, the **hand**
of the chain (`posechain_to_pose`). For a chain of one link, the hand is that link.

A `tposechain` holds the same number of links at every instant: a chain that gains a joint during
its motion is a different structure, not a later value of the same one.

## How a value moves between two instants

A `tpose` interpolates linearly in position and by SLERP in rotation (`posesegm_interpolate`).

A `tposechain` interpolates **link by link**, each link as a `tpose` does. Composing a chain
multiplies the rotations of its links, so the joints of a chain of more than one link do not move
along straight lines between two instants: a joint swings around the joints before it.

## Converting a pose chain

`tposechain_to_tpose` (SQL `tpose(tposechain)`) gives the **hand** at every instant. For a chain of
more than one link the result is a **step function**: the hand interpolated between two instants
is not the pose interpolated between the two composed poses, and a linear result would answer a
pose the chain never holds. For a chain of one link the interpolation of the input carries over
unchanged. `tposechain_to_tgeompoint` and `tposechain_to_tgeogpoint` convert through it, so they
give the position of the hand, held constant between two instants for a chain of more than one
link.

## Which place each operation reads

| Operation | Reads | Between two instants |
|---|---|---|
| The box, `stbox(tposechain)`, and the box operators (`&&`, `@>`, ...) | **every joint** | the box of the instants |
| Restriction to a box, `atStbox` and `minusStbox`, and `?&&` and `%&&` with a box | **every joint** | joint 1 moves linearly; the other joints, composed, are held constant |
| Conversion to `tpose`, `tgeompoint`, `tgeogpoint` | the **hand** | held constant for more than one link |
| Distance, nearest approach (`tDistance`, `|=|`, `nearestApproachInstant`, `shortestLine`, ...) | the **hand** | held constant for more than one link |
| Spatial relationships (`eIntersects`, `eDwithin`, `aIntersects`, ...) | the **hand** | held constant for more than one link |

**The box reads every joint.** `posechain_set_stbox` covers the composed position of every prefix
of the chain, not only of the whole of it: every joint is a place, and a query window that meets
the elbow but not the hand still meets the chain. An index on a `tposechain` column therefore
finds a chain by any of its joints.

**The restriction to a box reads every joint, as the box does.** A chain lies in a box at a time
when any of its joints lies in it, so `atStbox(chain, box)` keeps the times at which some joint's
position lies in the box and `minusStbox` keeps the other times; `chain ?&& box` holds when
`atStbox` is not empty and `chain %&& box` when `minusStbox` is empty. Joint 1 is the first link,
which moves linearly, so the times it spends in the box are exact; the joints composed of more than
one link are held constant between two instants, as the conversion to a temporal pose holds the
hand.

**Distance and the spatial relationships read the hand.** `tposechain_distance.c` answers for the
composed value `tposechain_to_tpose` gives, and the spatial relationships of a chain read the
temporal point of that composed value (`tposechain_tpoint`). So `eIntersects(chain, geometry)` asks
whether the **hand** ever meets the geometry, while `chain ?&& stbox(geometry)` asks whether **any
joint** ever lies in its box: a chain whose elbow crosses a region its hand never reaches answers
false to the first and true to the second.

## Choosing between tpose and tposechain

Use `tpose` for one rigid body moving in a frame: a vehicle, a drone, a camera.

Use `tposechain` for a body whose parts move relative to each other, each described in the frame
of the one before it: a robot arm, a crane with a trolley and a hook, a human skeleton read joint
by joint. Query its box, restrict it to a box, or index it to find it by any of its joints; convert
it to a `tpose`, or measure it, to follow its hand.
