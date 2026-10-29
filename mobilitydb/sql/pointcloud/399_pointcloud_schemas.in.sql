/*****************************************************************************
 *
 * This MobilityDB code is provided under The PostgreSQL License.
 * Copyright (c) 2016-2026, Université libre de Bruxelles and MobilityDB
 * contributors
 *
 * MobilityDB includes portions of PostGIS version 3 source code released
 * under the GNU General Public License (GPLv2 or later).
 * Copyright (c) 2001-2026, PostGIS contributors
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
 * @details A pcpoint and a pcpatch carry a pcid and nothing else about their
 * own
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

/* The pcpatch and pcpoint types and the PC_ functions are pgPointCloud's, and
 * pgPointCloud reads a schema from its own catalog pointcloud_formats alone,
 * as the XML document the column schema holds. The trigger below writes into
 * that catalog the document of every schema the two tables state, so a schema
 * registered once in SQL is read alike by MobilityDB and by pgPointCloud.
 *
 * The document is the one pc_schema_from_xml in pointcloud-pg/lib/pc_schema.c
 * reads: one pc:dimension per row in the order of dim_no, its size the width
 * of its interpretation, and pc:active stated as 1 or 0, the two spellings
 * that both the upstream parser, which reads the field with atoi, and the one
 * MEOS builds read, since a document omitting pc:active declares every
 * dimension inactive. The names and the descriptions are escaped, and the
 * scale and the offset are written with extra_float_digits = 3, which is
 * exact whatever the session sets, as pg_dump writes a double.
 *
 * The row is written only while pointcloud_formats accepts it, that is, while
 * the schema has an X and a Y dimension and a dimension at every position, as
 * PC_SchemaIsValid checks, and while the pcid is one pgPointCloud can name.
 * Otherwise, for instance after the schema row and before its dimensions are
 * inserted, the pcid has no row there, as it has none once the schema goes.
 * A row trigger fires after the statement has changed every row, so a schema
 * whose dimensions one statement inserts is written once it is complete.
 *
 * pointcloud_formats lives in the schema of pgPointCloud, which the search
 * path of CREATE EXTENSION holds, since pgPointCloud is required by this
 * build, so the functions below keep that search path. */

CREATE FUNCTION pointCloudFormatsSync() RETURNS trigger AS $$
DECLARE
  id integer;
  doc text;
BEGIN
  IF TG_OP = 'TRUNCATE' THEN
    /* Emptying either table leaves no schema with a dimension */
    DELETE FROM pointcloud_formats f USING pointcloud_schemas s
      WHERE f.pcid = s.pcid;
    RETURN NULL;
  END IF;
  FOREACH id IN ARRAY ARRAY(SELECT DISTINCT p FROM unnest(ARRAY[
      CASE WHEN TG_OP <> 'DELETE' THEN NEW.pcid END,
      CASE WHEN TG_OP <> 'INSERT' THEN OLD.pcid END]) p
      WHERE p IS NOT NULL) LOOP
    SELECT '<?xml version="1.0" encoding="UTF-8"?>' || E'\n' ||
      '<pc:PointCloudSchema xmlns:pc="http://pointcloud.org/schemas/PC/1.1" ' ||
      'xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance">' || E'\n' ||
      string_agg(
        '  <pc:dimension>' || E'\n' ||
        '    <pc:position>' || d.dim_no || '</pc:position>' || E'\n' ||
        '    <pc:size>' || CASE d.interpretation
          WHEN 'int8_t' THEN 1 WHEN 'uint8_t' THEN 1
          WHEN 'int16_t' THEN 2 WHEN 'uint16_t' THEN 2
          WHEN 'int32_t' THEN 4 WHEN 'uint32_t' THEN 4 WHEN 'float' THEN 4
          WHEN 'int64_t' THEN 8 WHEN 'uint64_t' THEN 8 WHEN 'double' THEN 8
          END || '</pc:size>' || E'\n' ||
        CASE WHEN d.description IS NULL THEN '' ELSE
          '    <pc:description>' || replace(replace(replace(d.description,
          '&', '&amp;'), '<', '&lt;'), '>', '&gt;') || '</pc:description>' ||
          E'\n' END ||
        '    <pc:name>' || replace(replace(replace(d.dim_name,
          '&', '&amp;'), '<', '&lt;'), '>', '&gt;') || '</pc:name>' || E'\n' ||
        '    <pc:interpretation>' || d.interpretation ||
          '</pc:interpretation>' || E'\n' ||
        '    <pc:scale>' || d.dim_scale || '</pc:scale>' || E'\n' ||
        '    <pc:offset>' || d.dim_offset || '</pc:offset>' || E'\n' ||
        '    <pc:active>' || CASE WHEN d.active THEN 1 ELSE 0 END ||
          '</pc:active>' || E'\n' ||
        '  </pc:dimension>' || E'\n', '' ORDER BY d.dim_no) ||
      '  <pc:metadata>' || E'\n' ||
      '    <Metadata name="compression">' || s.compression || '</Metadata>' ||
        E'\n' ||
      '  </pc:metadata>' || E'\n' ||
      '</pc:PointCloudSchema>'
    INTO doc
    FROM pointcloud_schemas s JOIN pointcloud_dimensions d ON d.pcid = s.pcid
    WHERE s.pcid = id
    GROUP BY s.pcid, s.compression;
    IF doc IS NULL THEN
      DELETE FROM pointcloud_formats WHERE pcid = id;
    ELSE
      BEGIN
        UPDATE pointcloud_formats f SET srid = s.srid, schema = doc
          FROM pointcloud_schemas s WHERE f.pcid = id AND s.pcid = id;
        IF NOT FOUND THEN
          INSERT INTO pointcloud_formats (pcid, srid, schema)
            SELECT id, s.srid, doc FROM pointcloud_schemas s WHERE s.pcid = id;
        END IF;
      EXCEPTION WHEN check_violation THEN
        DELETE FROM pointcloud_formats WHERE pcid = id;
      END;
    END IF;
  END LOOP;
  RETURN NULL;
END;
$$ LANGUAGE plpgsql
  SET search_path FROM CURRENT
  SET extra_float_digits = 3;

CREATE TRIGGER pointcloud_schemas_formats
  AFTER INSERT OR UPDATE OR DELETE ON pointcloud_schemas
  FOR EACH ROW EXECUTE FUNCTION pointCloudFormatsSync();
CREATE TRIGGER pointcloud_dimensions_formats
  AFTER INSERT OR UPDATE OR DELETE ON pointcloud_dimensions
  FOR EACH ROW EXECUTE FUNCTION pointCloudFormatsSync();
CREATE TRIGGER pointcloud_schemas_formats_truncate
  BEFORE TRUNCATE ON pointcloud_schemas
  FOR EACH STATEMENT EXECUTE FUNCTION pointCloudFormatsSync();
CREATE TRIGGER pointcloud_dimensions_formats_truncate
  BEFORE TRUNCATE ON pointcloud_dimensions
  FOR EACH STATEMENT EXECUTE FUNCTION pointCloudFormatsSync();

/* pointcloud_formats is a configuration table of pgPointCloud, so pg_dump
 * keeps the rows the trigger above writes, as it keeps the rows of the two
 * tables. A restore loads the two tables and pointcloud_formats in an order
 * pg_dump chooses: when the two tables come first, the trigger has written
 * the row of each schema they state before the dumped copy of that row
 * arrives. A row inserted for a pcid the two tables state, while a row of
 * that pcid exists, is therefore not inserted: the document of that pcid is
 * the one its rows write. The trigger is on a table of pgPointCloud, so it is
 * declared to depend on this extension, which drops it with the extension. */

CREATE FUNCTION pointCloudFormatsInsert() RETURNS trigger AS $$
BEGIN
  IF EXISTS (SELECT 1 FROM pointcloud_schemas WHERE pcid = NEW.pcid) AND
     EXISTS (SELECT 1 FROM pointcloud_formats WHERE pcid = NEW.pcid) THEN
    RAISE NOTICE 'The schema of pcid % is the one pointcloud_schemas and '
      'pointcloud_dimensions state', NEW.pcid;
    RETURN NULL;
  END IF;
  RETURN NEW;
END;
$$ LANGUAGE plpgsql
  SET search_path FROM CURRENT;

CREATE TRIGGER pointcloud_formats_stated
  BEFORE INSERT ON pointcloud_formats
  FOR EACH ROW EXECUTE FUNCTION pointCloudFormatsInsert();
ALTER TRIGGER pointcloud_formats_stated ON pointcloud_formats
  DEPENDS ON EXTENSION mobilitydb;

/* Answer what a pcid names without a value of that schema in hand and without
 * exposing the table layout. The schema is resolved as a value of it is, from
 * these rows and else from pgPointCloud's pointcloud_formats, and a statement
 * reads it as its catalogs state it when it starts, so the functions are
 * STABLE. A pcid no schema names answers NULL. */

CREATE FUNCTION pointCloudSchemaSRID(pcid integer) RETURNS integer
  AS 'MODULE_PATHNAME', 'Pointcloud_schema_srid'
  LANGUAGE C STABLE STRICT PARALLEL SAFE;

CREATE FUNCTION pointCloudSchemaCompression(pcid integer) RETURNS text
  AS 'MODULE_PATHNAME', 'Pointcloud_schema_compression'
  LANGUAGE C STABLE STRICT PARALLEL SAFE;

/* A schema every dimension of which is inactive answers 0 */
CREATE FUNCTION pointCloudSchemaNDims(pcid integer) RETURNS integer
  AS 'MODULE_PATHNAME', 'Pointcloud_schema_ndims'
  LANGUAGE C STABLE STRICT PARALLEL SAFE;

/* Mark both catalogs as configuration tables so that pg_dump preserves the
 * schemas a user registers, as the geopose_frames registry does. */
SELECT pg_catalog.pg_extension_config_dump('pointcloud_schemas', '');
SELECT pg_catalog.pg_extension_config_dump('pointcloud_dimensions', '');

/*****************************************************************************/
