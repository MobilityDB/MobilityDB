/*****************************************************************************
 *
 * This MobilityDB code is provided under The PostgreSQL License.
 * Copyright (c) 2016-2025, Université libre de Bruxelles and MobilityDB
 * contributors
 *
 * MobilityDB includes portions of PostGIS version 3 source code released
 * under the GNU General Public License (GPLv2 or later).
 * Copyright (c) 2001-2025, PostGIS contributors
 *
 * Permission to use, copy, modify, and distribute this software and its
 * documentation for any purpose, without fee, and without a written
 * agreement is hereby granted, provided that the above copyright notice and
 * this paragraph and the following two paragraphs appear in all copies.
 *
 * IN NO EVENT SHALL UNIVERSITE LIBRE DE BRUXELLES BE LIABLE TO ANY PARTY FOR
 * DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES, INCLUDING
 * LOST PROFITS, ARISING OUT OF THE USE OF THIS SOFTWARE AND ITS DOCUMENTATION,
 * EVEN IF UNIVERSITE LIBRE DE BRUXELLES HAS BEEN ADVISED OF THE POSSIBILITY
 * OF SUCH DAMAGE.
 *
 * UNIVERSITE LIBRE DE BRUXELLES SPECIFICALLY DISCLAIMS ANY WARRANTIES,
 * INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY
 * AND FITNESS FOR A PARTICULAR PURPOSE. THE SOFTWARE PROVIDED HEREUNDER IS ON
 * AN "AS IS" BASIS, AND UNIVERSITE LIBRE DE BRUXELLES HAS NO OBLIGATIONS TO
 * PROVIDE MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS.
 *
 *****************************************************************************/


/**
 * @file
 * @brief Tables stating the schema that a point cloud identifier names
 *
 * A pcpoint and a pcpatch carry a pcid and nothing else about their own
 * layout: the dimensions they hold, how each is stored, and what a stored
 * integer means as a coordinate are stated by the schema that pcid names.
 * The two tables state it in SQL, so registering a schema needs no XML
 * document.
 *
 * The size of a dimension, the offset of a dimension within a point and the
 * width of a point are absent because the engine computes them from the
 * interpretation and from the dimensions before each one, and the X, Y, Z
 * and M dimensions are absent because they resolve from the names. A row
 * stating any of them could only contradict the engine.
 */

CREATE TABLE pointcloud_schemas (
  pcid         integer PRIMARY KEY CHECK (pcid > 0),
  srid         integer NOT NULL,
  compression  text    NOT NULL DEFAULT 'none'
                       CHECK (compression IN ('none','dimensional','laz')),
  description  text
);

/*
 * A dimension is identified two ways, by number and by name, and both are
 * stated rather than derived: the number is the order the dimension is stored
 * in, which the order the rows happen to arrive in must not decide, because a
 * reload, a COPY or a parallel insert would silently relayout a point; the
 * name is what the X, Y, Z and M dimensions resolve from. Stating the number
 * makes a duplicate or a gap a constraint violation rather than a wrong byte
 * offset.
 *
 * No column name needs quoting in any host the tables are materialised in:
 * `offset` is a reserved word in PostgreSQL and in DuckDB, and `position`,
 * while usable as a column, cannot be called as a bare function in DuckDB,
 * which an accessor mirroring the column would have to be.
 */
CREATE TABLE pointcloud_dimensions (
  pcid            integer NOT NULL REFERENCES pointcloud_schemas(pcid)
                          ON DELETE CASCADE,
  dim_no          integer NOT NULL CHECK (dim_no >= 1),
  dim_name        text    NOT NULL,
  interpretation  text    NOT NULL
                          CHECK (interpretation IN ('int8_t','uint8_t',
                                 'int16_t','uint16_t','int32_t','uint32_t',
                                 'int64_t','uint64_t','double','float')),
  dim_scale       double precision NOT NULL DEFAULT 1,
  dim_offset      double precision NOT NULL DEFAULT 0,
  active          boolean NOT NULL DEFAULT true,
  description     text,
  PRIMARY KEY (pcid, dim_no),
  UNIQUE (pcid, dim_name)
);

COMMENT ON TABLE  pointcloud_schemas IS
  'Schema that a point cloud identifier names, stated in SQL. One row per '
  'schema; the dimensions it holds are the rows of pointcloud_dimensions.';
COMMENT ON COLUMN pointcloud_schemas.pcid IS
  'Identifier a pcpoint or a pcpatch carries to name this schema.';
COMMENT ON COLUMN pointcloud_schemas.srid IS
  'Spatial reference system every value of this schema is expressed in.';
COMMENT ON COLUMN pointcloud_schemas.compression IS
  'How a patch of this schema is stored: ''none'', ''dimensional'' or ''laz''.';
COMMENT ON COLUMN pointcloud_schemas.description IS
  'Free-form description for human readers.';

COMMENT ON TABLE  pointcloud_dimensions IS
  'Dimensions of the schema a pcid names, one row each. The size of a '
  'dimension and its offset within a point are absent because they follow '
  'from the interpretation and from the dimensions before it.';
COMMENT ON COLUMN pointcloud_dimensions.pcid IS
  'Schema this dimension belongs to.';
COMMENT ON COLUMN pointcloud_dimensions.dim_no IS
  'Number of this dimension within the schema, from 1, which is the order it '
  'is stored in, stated rather than taken from the order the rows arrive in.';
COMMENT ON COLUMN pointcloud_dimensions.dim_name IS
  'Name of this dimension, unique within the schema. The X, Y, Z and M '
  'dimensions resolve from it.';
COMMENT ON COLUMN pointcloud_dimensions.interpretation IS
  'Name of the numeric type a value of this dimension is stored as.';
COMMENT ON COLUMN pointcloud_dimensions.dim_scale IS
  'Factor a stored value is multiplied by to give the coordinate.';
COMMENT ON COLUMN pointcloud_dimensions.dim_offset IS
  'Value added to a scaled stored value to give the coordinate.';
COMMENT ON COLUMN pointcloud_dimensions.active IS
  'True when the dimension holds values.';
COMMENT ON COLUMN pointcloud_dimensions.description IS
  'Free-form description for human readers.';

/* Helper SQL functions that answer what a pcid names without a value of that
 * schema in hand and without exposing the table layout, as the geopose_frames
 * registry answers for a frame. */

CREATE FUNCTION pointCloudSchemaSRID(pcid integer) RETURNS integer
  LANGUAGE SQL IMMUTABLE STRICT PARALLEL SAFE
  AS $$ SELECT srid FROM pointcloud_schemas WHERE pointcloud_schemas.pcid = $1 $$;

CREATE FUNCTION pointCloudSchemaCompression(pcid integer) RETURNS text
  LANGUAGE SQL IMMUTABLE STRICT PARALLEL SAFE
  AS $$ SELECT compression FROM pointcloud_schemas
        WHERE pointcloud_schemas.pcid = $1 $$;

/* A pcid no schema names answers NULL, as the other two do; a schema every
 * dimension of which is inactive answers 0. A bare count over the dimensions
 * cannot tell those apart, so the count hangs off the schema row. */
CREATE FUNCTION pointCloudSchemaNDims(pcid integer) RETURNS integer
  LANGUAGE SQL IMMUTABLE STRICT PARALLEL SAFE
  AS $$ SELECT count(d.pcid)::integer FROM pointcloud_schemas s
        LEFT JOIN pointcloud_dimensions d
          ON d.pcid = s.pcid AND d.active
        WHERE s.pcid = $1 GROUP BY s.pcid $$;

/* Mark both catalogs as configuration tables so that pg_dump preserves the
 * schemas a user registers, as the geopose_frames registry does. */
SELECT pg_catalog.pg_extension_config_dump('pointcloud_schemas', '');
SELECT pg_catalog.pg_extension_config_dump('pointcloud_dimensions', '');

/*****************************************************************************/
