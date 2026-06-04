<!--
Copyright(c) MobilityDB Contributors

This documentation is licensed under a
Creative Commons Attribution-Share Alike 3.0 License
https://creativecommons.org/licenses/by-sa/3.0/
-->

# Mobility data platform interoperability: data and computation

An edge-to-cloud mobility data platform is interoperable only if both the data and the computation move freely across the ecosystem. These are two distinct properties:

- Portable data: mobility values produced by one engine are read, losslessly and without that engine's runtime, by any other.
- Portable computation: the same mobility analytics run, comparably, across the ecosystem's engines.

Each is necessary and neither is sufficient. Portable data without portable computation is a faithful lake nothing can query; portable computation without portable data is engines that cannot read each other's mobility data. Together they are what makes the platform edge-to-cloud.

Each property has a reproducible companion instantiation:

| Property | Companion | Instantiates |
| --- | --- | --- |
| Portable data | [Arrow interchange instantiation](./arrow-interchange-instantiation.md) | the optional Arrow interchange layer of the Temporal Data Lake architecture |
| Portable computation | [BerlinMOD cross-platform benchmark](https://github.com/MobilityDB/MobilityDB-BerlinMOD) | the cross-platform mobility-analytics methodology |

The data-portability instantiation is the optional Arrow interchange over the MEOS-WKB and native-scalar sidecar substrate. Each companion is self-contained, independently reproducible, and states its own arguments and measured results.
