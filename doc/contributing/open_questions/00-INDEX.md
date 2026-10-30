<!--
  MobilityDB — Open questions for the committers: index
  Copyright(c) MobilityDB Contributors

  This documentation is licensed under a Creative Commons Attribution-Share
  Alike 3.0 License: https://creativecommons.org/licenses/by-sa/3.0/
-->

# Open questions for the committers

Each note states a defect or an inconsistency measured on master, the change that resolves it,
its evidence and its blast radius, and the decision the committers are asked to take. Nothing
in these notes is merged; each change follows the committers' answer, in its own pull request.

| Note | The question |
|---|---|
| [Exact crossings in the lifting infrastructure](EXACT-CROSSINGS.md) | Which microsecond dates a crossing that falls between two microseconds, and whether ever/always and the temporal result read the crossing in the same time |
| [Any number of turning points in a binary lift](N-TURNING-POINTS.md) | Whether the lifting of a function of two arguments accepts any number of turning points per segment, as the unary lift already does, so that the temporal rigid geometries and the geodetic points stop keeping walks of their own |
| [The distance of non-convex and holed geometries](NON-CONVEX-DISTANCE.md) | How the distance of a temporal rigid geometry answers a body or a static polygon that is not convex or has holes, which master refuses, and whether the minimum over parts carries its corners at their true instants |
| [No NaN in MEOS and MobilityDB](NAN-POLICY.md) | The rule (NaN refused at every entry point, infinity a float value, a NaN result an error, -0 equal to 0), the census of where master does not apply it, and the fixes in one PR |
| [Feasibility of the distance of non-convex and holed geometries](NON-CONVEX-DISTANCE-FEASIBILITY.md) | What master already provides for that plan, what is new, how the convex cover is made exact, where the difficulty and the risk lie, and in which order the work is delivered |
| [Exact text output of floating-point values](EXACT-FLOAT-TEXT.md) | Whether the default text of a floating-point value becomes the shortest form that reads back as the same double, and in which notation |
