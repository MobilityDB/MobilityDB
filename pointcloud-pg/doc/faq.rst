.. _faq:

******************************************************************************
FAQ
******************************************************************************

**What can be done with pgPointcloud?**

  - pgPointcloud offers a way to efficiently store points in a postgres
    database.  In fact, pgpPointCloud stores groups of points (``pcPoints``)
    called ``pcPatch`` storing groups of point allows efficient compression of
    data.  This patch have a bounding box which can be used by PostGIS spatial
    features to greatly accelerate queries. pgPointcloud allows then to
    efficiently query very big point cloud. Querying can be done using spatial
    criteria (which points are in this area), as well as point attributes
    criteria (which points have a value of this attribute between .. and ...
    ?).

  - The point cloud being in a server, they can be used for processing or
    visualization, or streamed.

|

**Why use pgPointcloud to store Lidar points into postgreSQL and not in point cloud files?**

  The traditional way of storing point cloud is using several files containing
  each a part of the point cloud. This has some severe limitations:

    - Not efficient data query (to get a few points, you need to read the whole file)
    - No concurrency (only one user can modify points at a time/read points at
      a time)
    - Files tends to get duplicated a lot (each worker has it's own private
      version) no security of data ( file could be corrupted by a processing,
      hard to manage who access what)
    - Hard to use several different point cloud at the same time
    - Hard to use point cloud with other spatial data (vector, raster, images)

  pgPointcloud solves all of this problem, at a very low cost: you have to use a
  DBMS.

|

**Does pgPointcloud scale?**

  pgPointcloud is a young project, yet it has been proven to work fast (1ms query
  time) with a 5 billions points cloud.

  Currently pgPointcloud is fast for:

    - Load data into DB
    - Automatically compress data
    - Query patches based on spatial or other attributes

  It is slow for:

    - Data output (100k pts/sec)
    - In base conversion (no functions)
