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
| [Exact text output of floating-point values](EXACT-FLOAT-TEXT.md) | Whether the default text of a floating-point value becomes the shortest form that reads back as the same double, and in which notation |
