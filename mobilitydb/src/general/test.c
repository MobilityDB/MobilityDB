/**
 * @brief This is the final function for GeomUnion
 *       aggregate. Will have as input an array of Geometries.
 *       Will iteratively call GEOSUnion on the GEOS-converted
 *       versions of them and return PGIS-converted version back.
 *       Changing combination order *might* speed up performance.
 * @note PostGIS function: Datum pgis_union_geometry_array(PG_FUNCTION_ARGS)
 */
GSERIALIZED *
PGIS_union_geometry_array(GSERIALIZED **gsarr, int nelems)
{
  assert(nelems > 0);
  /* One geom geom? Return it */
  if (nelems == 1)
    return gsarr[0];
  
  Datum value;

  bool is3d = false, gotsrid = false;
  int curgeom = 0, count = 0;

  GSERIALIZED *gser_out = NULL;

  GEOSGeometry *g = NULL;
  GEOSGeometry *g_union = NULL;
  GEOSGeometry **geoms = NULL;

  int32_t srid = SRID_UNKNOWN;
  int empty_type = 0;

  initGEOS(lwpgnotice, lwgeom_geos_error);

  /*
  ** Collect the non-empty inputs and stuff them into a GEOS collection
  */
  geoms = palloc(sizeof(GEOSGeometry *) * nelems);

  /*
  ** We need to convert the array of GSERIALIZED into a GEOS collection.
  ** First make an array of GEOS geometries.
  */
  for (int i = 0; i < nelems; i++)
  {
    /* Check for SRID mismatch in array elements */
    if (gotsrid)
      ensure_same_srid(gserialized_get_srid(gsarr[i]), srid);
    else
    {
      /* Initialize SRID/dimensions info */
      srid = gserialized_get_srid(gsarr[i]);
      is3d = gserialized_has_z(gsarr[i]);
      gotsrid = 1;
    }

    /* Don't include empties in the union */
    if (gserialized_is_empty(gsarr[i]))
    {
      int gser_type = gserialized_get_type(gsarr[i]);
      if (gser_type > empty_type)
        empty_type = gser_type;
    }
    else
    {
      g = POSTGIS2GEOS(gsarr[i]);

      /* Uh oh! Exception thrown at construction... */
      if ( ! g )
        elog(ERROR, "One of the geometries in the set could not be converted to GEOS");

      geoms[curgeom++] = g;
    }
  }

  /*
  ** Take our GEOS geometries and turn them into a GEOS collection,
  ** then pass that into cascaded union.
  */
  if (curgeom > 0)
  {
    g = GEOSGeom_createCollection(GEOS_GEOMETRYCOLLECTION, geoms, curgeom);
    if (!g) elog(ERROR, "Could not create GEOS COLLECTION from geometry array");

    g_union = GEOSUnaryUnion(g);
    GEOSGeom_destroy(g);
    if (!g_union) elog(ERROR, "GEOSUnaryUnion");

    GEOSSetSRID(g_union, srid);
    gser_out = GEOS2POSTGIS(g_union, is3d);
    GEOSGeom_destroy(g_union);
  }
  /* No real geometries in our array, any empties? */
  else
  {
    /* If it was only empties, we'll return the largest type number */
    if ( empty_type > 0 )
      return geometry_serialize(lwgeom_construct_empty(empty_type, srid, is3d, 0)));
    /* Nothing but NULL, returns NULL */
    else
      return NULL;
  }

  if (! gser_out)
  {
    /* Union returned a NULL geometry */
    return NULL;
  }

  return gser_out;
}
