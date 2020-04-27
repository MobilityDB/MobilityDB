                                        /*****************************************************************************
 *
 * geography_functions.c
 *	  Spatial functions for PostGIS geography.
 *
 * Portions Copyright (c) 2020, Esteban Zimanyi, Arthur Lesuisse,
 *		Universite Libre de Bruxelles
 * Portions Copyright (c) 1996-2020, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 *****************************************************************************/

#define ACCEPT_USE_OF_DEPRECATED_PROJ_API_H 1

#include <postgres.h>
#include <float.h>
#include <utils/builtins.h>
#include <liblwgeom.h>

#include "postgis.h"
#include "tpoint_spatialfuncs.h"

extern void lwerror(const char *fmt, ...);

/***********************************************************************
 * Definitions and functions copied from PostGIS since they are not exported
 ***********************************************************************/

#define CIRC_NODE_SIZE 8

#define PG_GETARG_GSERIALIZED_P_COPY(varno) ((GSERIALIZED *)PG_DETOAST_DATUM_COPY(PG_GETARG_DATUM(varno)))

extern int ptarray_has_z(const POINTARRAY *pa);
extern int ptarray_has_m(const POINTARRAY *pa);

extern int circ_tree_get_point(const CIRC_NODE* node, POINT2D* pt);
extern int circ_tree_contains_point(const CIRC_NODE* node, const POINT2D* pt, const POINT2D* pt_outside, int* on_boundary);
extern uint32_t edge_intersects(const POINT3D *A1, const POINT3D *A2, const POINT3D *B1, const POINT3D *B2);
extern int edge_intersection(const GEOGRAPHIC_EDGE *e1, const GEOGRAPHIC_EDGE *e2, GEOGRAPHIC_POINT *g);
extern double edge_distance_to_edge(const GEOGRAPHIC_EDGE *e1, const GEOGRAPHIC_EDGE *e2, GEOGRAPHIC_POINT *closest1, GEOGRAPHIC_POINT *closest2);
extern CIRC_NODE* lwgeom_calculate_circ_tree(const LWGEOM* lwgeom);

static inline int
circ_node_is_leaf(const CIRC_NODE* node)
{
	return (node->num_nodes == 0);
}

static double
circ_node_min_distance(const CIRC_NODE* n1, const CIRC_NODE* n2)
{
	double d = sphere_distance(&(n1->center), &(n2->center));
	double r1 = n1->radius;
	double r2 = n2->radius;

	if ( d < r1 + r2 )
		return 0.0;

	return d - r1 - r2;
}

static double
circ_node_max_distance(const CIRC_NODE *n1, const CIRC_NODE *n2)
{
	return sphere_distance(&(n1->center), &(n2->center)) + n1->radius + n2->radius;
}

struct sort_node {
	CIRC_NODE *node;
	double d;
};

static int
circ_nodes_sort_cmp(const void *a, const void *b)
{
	struct sort_node *node_a = (struct sort_node *)(a);
	struct sort_node *node_b = (struct sort_node *)(b);
	if (node_a->d < node_b->d) return -1;
	else if (node_a->d > node_b->d) return 1;
	else return 0;
}

static void
circ_internal_nodes_sort(CIRC_NODE **nodes, uint32_t num_nodes, const CIRC_NODE *target_node)
{
	uint32_t i;
	struct sort_node sort_nodes[CIRC_NODE_SIZE];

	/* Copy incoming nodes into sorting array and calculate */
	/* distance to the target node */
	for (i = 0; i < num_nodes; i++)
	{
		sort_nodes[i].node = nodes[i];
		sort_nodes[i].d = sphere_distance(&(nodes[i]->center), &(target_node->center));
	}

	/* Sort the nodes and copy the result back into the input array */
	qsort(sort_nodes, num_nodes, sizeof(struct sort_node), circ_nodes_sort_cmp);
	for (i = 0; i < num_nodes; i++)
	{
		nodes[i] = sort_nodes[i].node;
	}
}

double
circ_tree_distance_tree_internal(const CIRC_NODE* n1, const CIRC_NODE* n2, double threshold,
		double* min_dist, double* max_dist, GEOGRAPHIC_POINT* closest1, GEOGRAPHIC_POINT* closest2)
{
	double max;
	double d, d_min;
	uint32_t i;

	/* Short circuit if we've already hit the minimum */
	if( *min_dist < threshold || *min_dist == 0.0 )
		return *min_dist;

	/* If your minimum is greater than anyone's maximum, you can't hold the winner */
	if( circ_node_min_distance(n1, n2) > *max_dist )
	{
		return FLT_MAX;
	}

	/* If your maximum is a new low, we'll use that as our new global tolerance */
	max = circ_node_max_distance(n1, n2);
	if( max < *max_dist )
		*max_dist = max;

	/* Polygon on one side, primitive type on the other. Check for point-in-polygon */
	/* short circuit. */
	if ( n1->geom_type == POLYGONTYPE && n2->geom_type && ! lwtype_is_collection((uint8_t) (n2->geom_type)) )
	{
		POINT2D pt;
		circ_tree_get_point(n2, &pt);
		if ( circ_tree_contains_point(n1, &pt, &(n1->pt_outside), NULL) )
		{
			*min_dist = 0.0;
			geographic_point_init(pt.x, pt.y, closest1);
			geographic_point_init(pt.x, pt.y, closest2);
			return *min_dist;
		}
	}
	/* Polygon on one side, primitive type on the other. Check for point-in-polygon */
	/* short circuit. */
	if ( n2->geom_type == POLYGONTYPE && n1->geom_type && ! lwtype_is_collection((uint8_t) (n1->geom_type)) )
	{
		POINT2D pt;
		circ_tree_get_point(n1, &pt);
		if ( circ_tree_contains_point(n2, &pt, &(n2->pt_outside), NULL) )
		{
			geographic_point_init(pt.x, pt.y, closest1);
			geographic_point_init(pt.x, pt.y, closest2);
			*min_dist = 0.0;
			return *min_dist;
		}
	}

	/* Both leaf nodes, do a real distance calculation */
	if( circ_node_is_leaf(n1) && circ_node_is_leaf(n2) )
	{
		double d;
		GEOGRAPHIC_POINT close1, close2;
		/* One of the nodes is a point */
		if ( n1->p1 == n1->p2 || n2->p1 == n2->p2 )
		{
			GEOGRAPHIC_EDGE e;
			GEOGRAPHIC_POINT gp1, gp2;

			/* Both nodes are points! */
			if ( n1->p1 == n1->p2 && n2->p1 == n2->p2 )
			{
				geographic_point_init(n1->p1->x, n1->p1->y, &gp1);
				geographic_point_init(n2->p1->x, n2->p1->y, &gp2);
				close1 = gp1; close2 = gp2;
				d = sphere_distance(&gp1, &gp2);
			}
			/* Node 1 is a point */
			else if ( n1->p1 == n1->p2 )
			{
				geographic_point_init(n1->p1->x, n1->p1->y, &gp1);
				geographic_point_init(n2->p1->x, n2->p1->y, &(e.start));
				geographic_point_init(n2->p2->x, n2->p2->y, &(e.end));
				close1 = gp1;
				d = edge_distance_to_point(&e, &gp1, &close2);
			}
			/* Node 2 is a point */
			else
			{
				/* FIX
				geographic_point_init(n2->p1->x, n2->p1->y, &gp1); */
				geographic_point_init(n2->p1->x, n2->p1->y, &gp2);
				geographic_point_init(n1->p1->x, n1->p1->y, &(e.start));
				geographic_point_init(n1->p2->x, n1->p2->y, &(e.end));
				/* FIX
				close1 = gp1;
				d = edge_distance_to_point(&e, &gp1, &close2); */
				close2 = gp2;
				d = edge_distance_to_point(&e, &gp2, &close1);
			}
		}
		/* Both nodes are edges */
		else
		{
			GEOGRAPHIC_EDGE e1, e2;
			GEOGRAPHIC_POINT g;
			POINT3D A1, A2, B1, B2;
			geographic_point_init(n1->p1->x, n1->p1->y, &(e1.start));
			geographic_point_init(n1->p2->x, n1->p2->y, &(e1.end));
			geographic_point_init(n2->p1->x, n2->p1->y, &(e2.start));
			geographic_point_init(n2->p2->x, n2->p2->y, &(e2.end));
			geog2cart(&(e1.start), &A1);
			geog2cart(&(e1.end), &A2);
			geog2cart(&(e2.start), &B1);
			geog2cart(&(e2.end), &B2);
			if ( edge_intersects(&A1, &A2, &B1, &B2) )
			{
				d = 0.0;
				edge_intersection(&e1, &e2, &g);
				close1 = close2 = g;
			}
			else
			{
				d = edge_distance_to_edge(&e1, &e2, &close1, &close2);
			}
		}
		if ( d < *min_dist )
		{
			*min_dist = d;
			*closest1 = close1;
			*closest2 = close2;
		}
		return d;
	}
	else
	{
		d_min = FLT_MAX;
		/* Drive the recursion into the COLLECTION types first so we end up with */
		/* pairings of primitive geometries that can be forced into the point-in-polygon */
		/* tests above. */
		if ( n1->geom_type && lwtype_is_collection((uint8_t) (n1->geom_type)) )
		{
			circ_internal_nodes_sort(n1->nodes, n1->num_nodes, n2);
			for ( i = 0; i < n1->num_nodes; i++ )
			{
				d = circ_tree_distance_tree_internal(n1->nodes[i], n2, threshold, min_dist, max_dist, closest1, closest2);
				d_min = FP_MIN(d_min, d);
			}
		}
		else if ( n2->geom_type && lwtype_is_collection((uint8_t) (n2->geom_type)) )
		{
			circ_internal_nodes_sort(n2->nodes, n2->num_nodes, n1);
			for ( i = 0; i < n2->num_nodes; i++ )
			{
				d = circ_tree_distance_tree_internal(n1, n2->nodes[i], threshold, min_dist, max_dist, closest1, closest2);
				d_min = FP_MIN(d_min, d);
			}
		}
		else if ( ! circ_node_is_leaf(n1) )
		{
			circ_internal_nodes_sort(n1->nodes, n1->num_nodes, n2);
			for ( i = 0; i < n1->num_nodes; i++ )
			{
				d = circ_tree_distance_tree_internal(n1->nodes[i], n2, threshold, min_dist, max_dist, closest1, closest2);
				d_min = FP_MIN(d_min, d);
			}
		}
		else if ( ! circ_node_is_leaf(n2) )
		{
			circ_internal_nodes_sort(n2->nodes, n2->num_nodes, n1);
			for ( i = 0; i < n2->num_nodes; i++ )
			{
				d = circ_tree_distance_tree_internal(n1, n2->nodes[i], threshold, min_dist, max_dist, closest1, closest2);
				d_min = FP_MIN(d_min, d);
			}
		}
		else
		{
			/* Never get here */
		}

		return d_min;
	}
}

/***********************************************************************
 * ST_MakeLine and ST_LineFromMultiPoint for geographies
 ***********************************************************************/

/**
 * @brief makeline_garray ( GEOGRAPHY[] ) returns a LINE formed by
 * 		all the point geographies in given array.
 * 		array elements that are NOT points are discarded.
 */
PG_FUNCTION_INFO_V1(geography_makeline_garray);
Datum geography_makeline_garray(PG_FUNCTION_ARGS)
{
	ArrayType *array;
	int nelems;
	GSERIALIZED *result = NULL;
	LWGEOM **geoms;
	LWGEOM *outlwg;
	uint32 ngeoms;
	int srid = SRID_UNKNOWN;

	ArrayIterator iterator;
	Datum value;
	bool isnull;

	/* Return null on null input */
	if ( PG_ARGISNULL(0) )
		PG_RETURN_NULL();

	/* Get actual ArrayType */
	array = PG_GETARG_ARRAYTYPE_P(0);

	/* Get number of geographies in array */
	nelems = ArrayGetNItems(ARR_NDIM(array), ARR_DIMS(array));

	/* Return null on 0-elements input array */
	if ( nelems == 0 )
		PG_RETURN_NULL();

	/*
	 * Deserialize all point geometries in array into the
	 * geoms pointers array.
	 * Count actual number of points.
	 */

	/* possibly more then required */
	geoms = palloc(sizeof(LWGEOM *) * nelems);
	ngeoms = 0;

#if MOBDB_PGSQL_VERSION >= 100000
 	iterator = array_create_iterator(array, 0, NULL);
#else
 	iterator = array_create_iterator(array, 0);
#endif

	while( array_iterate(iterator, &value, &isnull) )
	{
		GSERIALIZED *geom;

		if ( isnull )
			continue;

		geom = (GSERIALIZED *)DatumGetPointer(value);

		if ( gserialized_get_type(geom) != POINTTYPE &&
		     gserialized_get_type(geom) != LINETYPE &&
		     gserialized_get_type(geom) != MULTIPOINTTYPE)
		{
			continue;
		}

		geoms[ngeoms++] = lwgeom_from_gserialized(geom);

		/* Check SRID homogeneity */
		if ( ngeoms == 1 )
		{
			/* Get first geometry SRID */
			srid = geoms[ngeoms-1]->srid;
			/* TODO: also get ZMflags */
		}
		else
		{
			error_if_srid_mismatch(geoms[ngeoms-1]->srid, srid);
		}
	}
	array_free_iterator(iterator);

	/* Return null on 0-points input array */
	if ( ngeoms == 0 )
	{
		/* TODO: should we return LINESTRING EMPTY here ? */
		elog(NOTICE, "No points or linestrings in input array");
		PG_RETURN_NULL();
	}

	outlwg = (LWGEOM *)lwline_from_lwgeom_array(srid, ngeoms, geoms);

	result = geography_serialize(outlwg);

	PG_RETURN_POINTER(result);
}

/**
 * makeline ( GEOGRAPHY, GEOGRAPHY ) returns a LINESTRING segment
 * formed by the given point geographies.
 */
PG_FUNCTION_INFO_V1(geography_makeline);
Datum geography_makeline(PG_FUNCTION_ARGS)
{
	GSERIALIZED *pglwg1, *pglwg2;
	GSERIALIZED *result=NULL;
	LWGEOM *lwgeoms[2];
	LWLINE *outline;

	/* Get input datum */
	pglwg1 = PG_GETARG_GSERIALIZED_P(0);
	pglwg2 = PG_GETARG_GSERIALIZED_P(1);

	if ( (gserialized_get_type(pglwg1) != POINTTYPE && gserialized_get_type(pglwg1) != LINETYPE) ||
	     (gserialized_get_type(pglwg2) != POINTTYPE && gserialized_get_type(pglwg2) != LINETYPE) )
	{
		elog(ERROR, "Input geometries must be points or lines");
		PG_RETURN_NULL();
	}

	error_if_srid_mismatch(gserialized_get_srid(pglwg1), gserialized_get_srid(pglwg2));

	lwgeoms[0] = lwgeom_from_gserialized(pglwg1);
	lwgeoms[1] = lwgeom_from_gserialized(pglwg2);

	outline = lwline_from_lwgeom_array(lwgeoms[0]->srid, 2, lwgeoms);

	result = geography_serialize((LWGEOM *)outline);

	PG_FREE_IF_COPY(pglwg1, 0);
	PG_FREE_IF_COPY(pglwg2, 1);
	lwgeom_free(lwgeoms[0]);
	lwgeom_free(lwgeoms[1]);

	PG_RETURN_POINTER(result);
}

/**
 * LineFromMultiPoint ( GEOMETRY ) returns a LINE formed by
 * 		all the points in the in given multipoint.
 */
PG_FUNCTION_INFO_V1(geography_line_from_mpoint);
Datum geography_line_from_mpoint(PG_FUNCTION_ARGS)
{
	GSERIALIZED *ingeom, *result;
	LWLINE *lwline;
	LWMPOINT *mpoint;

	/* Get input GSERIALIZED and deserialize it */
	ingeom = PG_GETARG_GSERIALIZED_P(0);

	if ( gserialized_get_type(ingeom) != MULTIPOINTTYPE )
	{
		elog(ERROR, "makeline: input must be a multipoint");
		PG_RETURN_NULL(); /* input is not a multipoint */
	}

	mpoint = lwgeom_as_lwmpoint(lwgeom_from_gserialized(ingeom));
	lwline = lwline_from_lwmpoint(mpoint->srid, mpoint);
	if ( ! lwline )
	{
		PG_FREE_IF_COPY(ingeom, 0);
		elog(ERROR, "makeline: lwline_from_lwmpoint returned NULL");
		PG_RETURN_NULL();
	}

	result = geography_serialize(lwline_as_lwgeom(lwline));

	PG_FREE_IF_COPY(ingeom, 0);
	lwline_free(lwline);

	PG_RETURN_POINTER(result);
}

/***********************************************************************
 * ST_AddPoint, ST_RemovePoint, ST_SetPoint for geographies
 ***********************************************************************/

PG_FUNCTION_INFO_V1(geography_addpoint);
Datum geography_addpoint(PG_FUNCTION_ARGS)
{
	GSERIALIZED *pglwg1, *pglwg2, *result;
	LWPOINT *point;
	LWLINE *line, *linecopy;
	int32 where;

	pglwg1 = PG_GETARG_GSERIALIZED_P(0);
	pglwg2 = PG_GETARG_GSERIALIZED_P(1);

	if ( gserialized_get_type(pglwg1) != LINETYPE )
	{
		elog(ERROR, "First argument must be a LINESTRING");
		PG_RETURN_NULL();
	}

	if ( gserialized_get_type(pglwg2) != POINTTYPE )
	{
		elog(ERROR, "Second argument must be a POINT");
		PG_RETURN_NULL();
	}

	line = lwgeom_as_lwline(lwgeom_from_gserialized(pglwg1));

	if ( PG_NARGS() > 2 )
	{
		where = PG_GETARG_INT32(2);
	}
	else
	{
		where = line->points->npoints;
	}

	if ( where < 0 || where > (int32) line->points->npoints )
	{
		elog(ERROR, "Invalid offset");
		PG_RETURN_NULL();
	}

	point = lwgeom_as_lwpoint(lwgeom_from_gserialized(pglwg2));
	linecopy = lwgeom_as_lwline(lwgeom_clone_deep(lwline_as_lwgeom(line)));
	lwline_free(line);

	if ( lwline_add_lwpoint(linecopy, point, (uint32_t) where) == LW_FAILURE )
	{
		elog(ERROR, "Point insert failed");
		PG_RETURN_NULL();
	}

	result = geography_serialize(lwline_as_lwgeom(linecopy));

	/* Release memory */
	PG_FREE_IF_COPY(pglwg1, 0);
	PG_FREE_IF_COPY(pglwg2, 1);
	lwpoint_free(point);

	PG_RETURN_POINTER(result);

}

PG_FUNCTION_INFO_V1(geography_removepoint);
Datum geography_removepoint(PG_FUNCTION_ARGS)
{
	GSERIALIZED *pglwg1, *result;
	LWLINE *line, *outline;
	uint32 which;

	pglwg1 = PG_GETARG_GSERIALIZED_P(0);
	which = PG_GETARG_INT32(1);

	if ( gserialized_get_type(pglwg1) != LINETYPE )
	{
		elog(ERROR, "First argument must be a LINESTRING");
		PG_RETURN_NULL();
	}

	line = lwgeom_as_lwline(lwgeom_from_gserialized(pglwg1));

	if ( which > line->points->npoints-1 )
	{
		elog(ERROR, "Point index out of range (%d..%d)", 0, line->points->npoints-1);
		PG_RETURN_NULL();
	}

	if ( line->points->npoints < 3 )
	{
		elog(ERROR, "Can't remove points from a single segment line");
		PG_RETURN_NULL();
	}

	outline = lwline_removepoint(line, which);
	/* Release memory */
	lwline_free(line);

	result = geography_serialize((LWGEOM *)outline);
	lwline_free(outline);

	PG_FREE_IF_COPY(pglwg1, 0);
	PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(geography_setpoint_linestring);
Datum geography_setpoint_linestring(PG_FUNCTION_ARGS)
{
	GSERIALIZED *pglwg1, *pglwg2, *result;
	LWGEOM *lwg;
	LWLINE *line;
	LWPOINT *lwpoint;
	POINT4D newpoint;
	int32 which;

	/* we copy input as we're going to modify it */
	pglwg1 = PG_GETARG_GSERIALIZED_P_COPY(0);

	which = PG_GETARG_INT32(1);
	pglwg2 = PG_GETARG_GSERIALIZED_P(2);


	/* Extract a POINT4D from the point */
	lwg = lwgeom_from_gserialized(pglwg2);
	lwpoint = lwgeom_as_lwpoint(lwg);
	if ( ! lwpoint )
	{
		elog(ERROR, "Third argument must be a POINT");
		PG_RETURN_NULL();
	}
	getPoint4d_p(lwpoint->point, 0, &newpoint);
	lwpoint_free(lwpoint);
	PG_FREE_IF_COPY(pglwg2, 2);

	lwg = lwgeom_from_gserialized(pglwg1);
	line = lwgeom_as_lwline(lwg);
	if ( ! line )
	{
		elog(ERROR, "First argument must be a LINESTRING");
		PG_RETURN_NULL();
	}
	if(which < 0){
		/* Use backward indexing for negative values */
		which = which + line->points->npoints ;
	}
	if ( (uint32_t)which + 1 > line->points->npoints )
	{
		elog(ERROR, "abs(Point index) out of range (-)(%d..%d)", 0, line->points->npoints-1);
		PG_RETURN_NULL();
	}

	/*
	 * This will change pointarray of the serialized pglwg1,
	 */
	lwline_setPoint4d(line, (uint32_t)which, &newpoint);
	result = geography_serialize((LWGEOM *)line);

	/* Release memory */
	lwline_free(line);
	pfree(pglwg1); /* we forced copy, POINARRAY is released now */

	PG_RETURN_POINTER(result);
}

/***********************************************************************
 * ST_GeographyN and ST_NumGeographies for geographies
 ***********************************************************************/

PG_FUNCTION_INFO_V1(geography_numgeographies_collection);
Datum geography_numgeographies_collection(PG_FUNCTION_ARGS)
{
	GSERIALIZED *geom = PG_GETARG_GSERIALIZED_P(0);
	LWGEOM *lwgeom;
	int32 ret = 1;

	lwgeom = lwgeom_from_gserialized(geom);
	if ( lwgeom_is_empty(lwgeom) )
	{
		ret = 0;
	}
	else if ( lwgeom_is_collection(lwgeom) )
	{
		LWCOLLECTION *col = lwgeom_as_lwcollection(lwgeom);
		ret = col->ngeoms;
	}
	lwgeom_free(lwgeom);
	PG_FREE_IF_COPY(geom, 0);
	PG_RETURN_INT32(ret);
}

/** 1-based offset */
PG_FUNCTION_INFO_V1(geography_geographyn_collection);
Datum geography_geographyn_collection(PG_FUNCTION_ARGS)
{
	GSERIALIZED *geog = PG_GETARG_GSERIALIZED_P(0);
	GSERIALIZED *result;
	int type = gserialized_get_type(geog);
	int32 idx;
	LWCOLLECTION *coll;
	LWGEOM *subgeog;

	/* elog(NOTICE, "GeometryN called"); */

	idx = PG_GETARG_INT32(1);
	idx -= 1; /* index is 1-based */

	/* call is valid on multi* geoms only */
	if (type==POINTTYPE || type==LINETYPE || type==CIRCSTRINGTYPE ||
	        type==COMPOUNDTYPE || type==POLYGONTYPE ||
		type==CURVEPOLYTYPE || type==TRIANGLETYPE)
	{
		if ( idx == 0 ) PG_RETURN_POINTER(geog);
		PG_RETURN_NULL();
	}

	coll = lwgeom_as_lwcollection(lwgeom_from_gserialized(geog));

	if ( idx < 0 ) PG_RETURN_NULL();
	if ( idx >= (int32) coll->ngeoms ) PG_RETURN_NULL();

	subgeog = coll->geoms[idx];
	subgeog->srid = coll->srid;

	/* COMPUTE_BBOX==TAINTING */
	// if ( coll->bbox ) lwgeom_add_bbox(subgeog);

	result = geography_serialize(subgeog);

	lwcollection_free(coll);
	PG_FREE_IF_COPY(geog, 0);

	PG_RETURN_POINTER(result);
}

/***********************************************************************
 * ST_NumPoints, ST_StartPoint, ST_EndPoint, ST_PointN for geographies
 ***********************************************************************/

/**
* numpoints(LINESTRING) -- return the number of points in the
* linestring, or NULL if it is not a linestring
*/
PG_FUNCTION_INFO_V1(geography_numpoints_linestring);
Datum geography_numpoints_linestring(PG_FUNCTION_ARGS)
{
	GSERIALIZED *geom = PG_GETARG_GSERIALIZED_P(0);
	LWGEOM *lwgeom = lwgeom_from_gserialized(geom);
	int count = -1;
	int type = lwgeom->type;

	if (type == LINETYPE || type == CIRCSTRINGTYPE || type == COMPOUNDTYPE)
		count = lwgeom_count_vertices(lwgeom);

	lwgeom_free(lwgeom);
	PG_FREE_IF_COPY(geom, 0);

	/* OGC says this functions is only valid on LINESTRING */
	if (count < 0)
		PG_RETURN_NULL();

	PG_RETURN_INT32(count);
}

/**
* ST_StartPoint(GEOMETRY)
* @return the first point of a linestring.
* 		Return NULL if there is no LINESTRING
*/
PG_FUNCTION_INFO_V1(geography_startpoint_linestring);
Datum geography_startpoint_linestring(PG_FUNCTION_ARGS)
{
	GSERIALIZED *geom = PG_GETARG_GSERIALIZED_P(0);
	LWGEOM *lwgeom = lwgeom_from_gserialized(geom);
	LWPOINT *lwpoint = NULL;
	int type = lwgeom->type;

	if (type == LINETYPE || type == CIRCSTRINGTYPE)
	{
		lwpoint = lwline_get_lwpoint((LWLINE*)lwgeom, 0);
	}
	else if (type == COMPOUNDTYPE)
	{
		lwpoint = lwcompound_get_startpoint((LWCOMPOUND*)lwgeom);
	}

	lwgeom_free(lwgeom);
	PG_FREE_IF_COPY(geom, 0);

	if (! lwpoint)
		PG_RETURN_NULL();

	PG_RETURN_POINTER(geography_serialize(lwpoint_as_lwgeom(lwpoint)));
}

/** EndPoint(GEOMETRY) -- find the first linestring in GEOMETRY,
 * @return the last point.
 * 	Return NULL if there is no LINESTRING(..) in GEOMETRY
 */
PG_FUNCTION_INFO_V1(geography_endpoint_linestring);
Datum geography_endpoint_linestring(PG_FUNCTION_ARGS)
{
	GSERIALIZED *geom = PG_GETARG_GSERIALIZED_P(0);
	LWGEOM *lwgeom = lwgeom_from_gserialized(geom);
	LWPOINT *lwpoint = NULL;
	int type = lwgeom->type;

	if (type == LINETYPE || type == CIRCSTRINGTYPE)
	{
		LWLINE *line = (LWLINE*)lwgeom;
		if (line->points)
			lwpoint = lwline_get_lwpoint((LWLINE*)lwgeom, line->points->npoints - 1);
	}
	else if (type == COMPOUNDTYPE)
	{
		lwpoint = lwcompound_get_endpoint((LWCOMPOUND*)lwgeom);
	}

	lwgeom_free(lwgeom);
	PG_FREE_IF_COPY(geom, 0);

	if (! lwpoint)
		PG_RETURN_NULL();

	PG_RETURN_POINTER(geography_serialize(lwpoint_as_lwgeom(lwpoint)));
}

/**
 * PointN(GEOMETRY,INTEGER) -- find the first linestring in GEOMETRY,
 * @return the point at index INTEGER (1 is 1st point).  Return NULL if
 * 		there is no LINESTRING(..) in GEOMETRY or INTEGER is out of bounds.
 */
PG_FUNCTION_INFO_V1(geography_pointn_linestring);
Datum geography_pointn_linestring(PG_FUNCTION_ARGS)
{
	GSERIALIZED *geom = PG_GETARG_GSERIALIZED_P(0);
	int where = PG_GETARG_INT32(1);
	LWGEOM *lwgeom = lwgeom_from_gserialized(geom);
	LWPOINT *lwpoint = NULL;
	int type = lwgeom->type;

	/* If index is negative, count backward */
	if (where < 1)
	{
		int count = -1;
		if (type == LINETYPE || type == CIRCSTRINGTYPE || type == COMPOUNDTYPE)
			count = lwgeom_count_vertices(lwgeom);
		if (count > 0)
		{
			/* only work if we found the total point number */
			/* converting where to positive backward indexing, +1 because 1 indexing */
			where = where + count + 1;
		}
		if (where < 1)
			PG_RETURN_NULL();
	}

	if (type == LINETYPE || type == CIRCSTRINGTYPE)
	{
		/* OGC index starts at one, so we substract first. */
		lwpoint = lwline_get_lwpoint((LWLINE*)lwgeom, (uint32_t) (where - 1));
	}
	else if (type == COMPOUNDTYPE)
	{
		lwpoint = lwcompound_get_lwpoint((LWCOMPOUND*)lwgeom, (uint32_t) (where - 1));
	}

	lwgeom_free(lwgeom);
	PG_FREE_IF_COPY(geom, 0);

	if (! lwpoint)
		PG_RETURN_NULL();

	PG_RETURN_POINTER(geography_serialize(lwpoint_as_lwgeom(lwpoint)));
}

/***********************************************************************
 * Closest point and closest line functions for geographies.
 ***********************************************************************/

LWGEOM *
geography_tree_closestpoint(const GSERIALIZED* g1, const GSERIALIZED* g2, double threshold)
{
	CIRC_NODE* circ_tree1 = NULL;
	CIRC_NODE* circ_tree2 = NULL;
	LWGEOM* lwgeom1 = NULL;
	LWGEOM* lwgeom2 = NULL;
	double min_dist = FLT_MAX;
	double max_dist = FLT_MAX;
	GEOGRAPHIC_POINT closest1, closest2;
	LWGEOM *result;
	POINT4D p;

	lwgeom1 = lwgeom_from_gserialized(g1);
	lwgeom2 = lwgeom_from_gserialized(g2);
	circ_tree1 = lwgeom_calculate_circ_tree(lwgeom1);
	circ_tree2 = lwgeom_calculate_circ_tree(lwgeom2);

	circ_tree_distance_tree_internal(circ_tree1, circ_tree2, threshold,
		&min_dist, &max_dist, &closest1, &closest2);

	p.x = rad2deg(closest1.lon);
	p.y = rad2deg(closest1.lat);
	result = (LWGEOM *)lwpoint_make2d(gserialized_get_srid(g1), p.x, p.y);

	circ_tree_free(circ_tree1);
	circ_tree_free(circ_tree2);
	lwgeom_free(lwgeom1);
	lwgeom_free(lwgeom2);
	return result;
}

/**
Returns the point in first input geography that is closest to the second input geography in 2d
*/

PG_FUNCTION_INFO_V1(geography_closestpoint);
Datum geography_closestpoint(PG_FUNCTION_ARGS)
{
	GSERIALIZED* g1 = NULL;
	GSERIALIZED* g2 = NULL;
	LWGEOM *point;
	GSERIALIZED* result;

	/* Get our geography objects loaded into memory. */
	g1 = PG_GETARG_GSERIALIZED_P(0);
	g2 = PG_GETARG_GSERIALIZED_P(1);

	error_if_srid_mismatch(gserialized_get_srid(g1), gserialized_get_srid(g2));

	/* Return NULL on empty arguments. */
	if ( gserialized_is_empty(g1) || gserialized_is_empty(g2) )
	{
		PG_FREE_IF_COPY(g1, 0);
		PG_FREE_IF_COPY(g2, 1);
		PG_RETURN_NULL();
	}

	point = geography_tree_closestpoint(g1, g2, FP_TOLERANCE);

	if (lwgeom_is_empty(point))
		PG_RETURN_NULL();

	result = geography_serialize(point);
	lwgeom_free(point);

	PG_FREE_IF_COPY(g1, 0);
	PG_FREE_IF_COPY(g2, 1);
	PG_RETURN_POINTER(result);
}

/*****************************************************************************/

LWGEOM *
geography_tree_shortestline(const GSERIALIZED* g1, const GSERIALIZED* g2,
	double threshold)
{
	CIRC_NODE* circ_tree1 = NULL;
	CIRC_NODE* circ_tree2 = NULL;
	LWGEOM* lwgeom1 = NULL;
	LWGEOM* lwgeom2 = NULL;
	double min_dist = FLT_MAX;
	double max_dist = FLT_MAX;
	GEOGRAPHIC_POINT closest1, closest2;
	LWGEOM *geoms[2];
	LWGEOM *result;
	POINT4D p1, p2;

	lwgeom1 = lwgeom_from_gserialized(g1);
	lwgeom2 = lwgeom_from_gserialized(g2);
	circ_tree1 = lwgeom_calculate_circ_tree(lwgeom1);
	circ_tree2 = lwgeom_calculate_circ_tree(lwgeom2);

	circ_tree_distance_tree_internal(circ_tree1, circ_tree2, threshold,
			&min_dist, &max_dist, &closest1, &closest2);

	p1.x = rad2deg(closest1.lon);
	p1.y = rad2deg(closest1.lat);
	p2.x = rad2deg(closest2.lon);
	p2.y = rad2deg(closest2.lat);

	geoms[0] = (LWGEOM *)lwpoint_make2d(gserialized_get_srid(g1), p1.x, p1.y);
	geoms[1] = (LWGEOM *)lwpoint_make2d(gserialized_get_srid(g1), p2.x, p2.y);
	result = (LWGEOM *)lwline_from_lwgeom_array(geoms[0]->srid, 2, geoms);

	lwgeom_free(geoms[0]);
	lwgeom_free(geoms[1]);
	circ_tree_free(circ_tree1);
	circ_tree_free(circ_tree2);
	lwgeom_free(lwgeom1);
	lwgeom_free(lwgeom2);
	return result;
}

/**
Returns the point in first input geography that is closest to the second input geography in 2d
*/

PG_FUNCTION_INFO_V1(geography_shortestline);
Datum geography_shortestline(PG_FUNCTION_ARGS)
{
	GSERIALIZED* g1 = NULL;
	GSERIALIZED* g2 = NULL;
	LWGEOM *line;
	GSERIALIZED* result;

	/* Get our geography objects loaded into memory. */
	g1 = PG_GETARG_GSERIALIZED_P(0);
	g2 = PG_GETARG_GSERIALIZED_P(1);

	error_if_srid_mismatch(gserialized_get_srid(g1), gserialized_get_srid(g2));

	/* Return NULL on empty arguments. */
	if ( gserialized_is_empty(g1) || gserialized_is_empty(g2) )
	{
		PG_FREE_IF_COPY(g1, 0);
		PG_FREE_IF_COPY(g2, 1);
		PG_RETURN_NULL();
	}

	line = geography_tree_shortestline(g1, g2, FP_TOLERANCE);

	if (lwgeom_is_empty(line))
		PG_RETURN_NULL();

	result = geography_serialize(line);
	lwgeom_free(line);

	PG_FREE_IF_COPY(g1, 0);
	PG_FREE_IF_COPY(g2, 1);
	PG_RETURN_POINTER(result);
}

/***********************************************************************
 * ST_LineSubstring for geographies
 ***********************************************************************/

/**
 * Find interpolation point p
 * between geography points p1 and p2
 * so that the len(p1,p) == len(p1,p2) * f
 * and p falls on p1,p2 segment.
 */
void
interpolate_point4d_sphere(
	const POINT3D *p1, const POINT3D *p2, /* 3-space points we are interpolating between */
	const POINT4D *v1, const POINT4D *v2, /* real values and z/m values */
	double f, /* fraction */
	POINT4D *p) /* write out results here */
{
	/* Calculate interpolated point */
	POINT3D mid;
	mid.x = p1->x + ((p2->x - p1->x) * f);
	mid.y = p1->y + ((p2->y - p1->y) * f);
	mid.z = p1->z + ((p2->z - p1->z) * f);
	normalize(&mid);

	/* Calculate z/m values */
	GEOGRAPHIC_POINT g;
	cart2geog(&mid, &g);
	p->x = rad2deg(g.lon);
	p->y = rad2deg(g.lat);
	p->z = v1->z + ((v2->z - v1->z) * f);
	p->m = v1->m + ((v2->m - v1->m) * f);
}

double ptarray_length_sphere(const POINTARRAY *pa)
{
	GEOGRAPHIC_POINT a, b;
	POINT4D p;
	uint32_t i;
	double length = 0.0;

	/* Return zero on non-sensical inputs */
	if ( ! pa || pa->npoints < 2 )
		return 0.0;

	/* Initialize first point */
	getPoint4d_p(pa, 0, &p);
	geographic_point_init(p.x, p.y, &a);

	/* Loop and sum the length for each segment */
	for ( i = 1; i < pa->npoints; i++ )
	{
		getPoint4d_p(pa, i, &p);
		geographic_point_init(p.x, p.y, &b);
		/* Add this segment length to the total */
		length +=  sphere_distance(&a, &b);
	}
	return length;
}

POINTARRAY *
geography_substring(POINTARRAY *ipa, double from, double to,
	double tolerance)
{
	POINTARRAY *dpa;
	POINT4D pt;
	POINT4D p1, p2;
	POINT3D q1, q2;
	GEOGRAPHIC_POINT g1, g2;
	int nsegs, i;
	double length, slength, tlength;
	int state = 0; /* 0 = before, 1 = inside */

	/*
	 * Create a dynamic pointarray with an initial capacity
	 * equal to full copy of input points
	 */
	dpa = ptarray_construct_empty((char) FLAGS_GET_Z(ipa->flags),
		(char) FLAGS_GET_M(ipa->flags), ipa->npoints);

	/* Compute total line length */
	length = ptarray_length_sphere(ipa);

	/* Get 'from' and 'to' lengths */
	from = length * from;
	to = length * to;
	tlength = 0;
	getPoint4d_p(ipa, 0, &p1);
	geographic_point_init(p1.x, p1.y, &g1);
	nsegs = ipa->npoints - 1;
	for (i = 0; i < nsegs; i++)
	{
		double dseg;
		getPoint4d_p(ipa, (uint32_t) i+1, &p2);
		geographic_point_init(p2.x, p2.y, &g2);

		/* Find the length of this segment */
		slength = sphere_distance(&g1, &g2);

		/*
		 * We are before requested start.
		 */
		if (state == 0) /* before */
		{
			if (fabs ( from - ( tlength + slength ) ) <= tolerance)
			{
				/*
				 * Second point is our start
				 */
				ptarray_append_point(dpa, &p2, LW_FALSE);
				state = 1; /* we're inside now */
				goto END;
			}
			else if (fabs(from - tlength) <= tolerance)
			{
				/*
				 * First point is our start
				 */
				ptarray_append_point(dpa, &p1, LW_FALSE);
				/*
				 * We're inside now, but will check
				 * 'to' point as well
				 */
				state = 1;
			}
			/*
			 * Didn't reach the 'from' point,
			 * nothing to do
			 */
			else if (from > tlength + slength)
				goto END;
			else  /* tlength < from < tlength+slength */
			{
				/*
				 * Our start is between first and second point
				 */
				dseg = (from - tlength) / slength;
				geog2cart(&g1, &q1);
				geog2cart(&g2, &q2);
				interpolate_point4d_sphere(&q1, &q2, &p1, &p2, dseg, &pt);
				ptarray_append_point(dpa, &pt, LW_FALSE);
				/*
				 * We're inside now, but will check 'to' point as well
				 */
				state = 1;
			}
		}

		if (state == 1) /* inside */
		{
			/*
			 * 'to' point is our second point.
			 */
			if (fabs(to - ( tlength + slength ) ) <= tolerance )
			{
				ptarray_append_point(dpa, &p2, LW_FALSE);
				break; /* substring complete */
			}
			/*
			 * 'to' point is our first point.
			 * (should only happen if 'to' is 0)
			 */
			else if (fabs(to - tlength) <= tolerance)
			{
				ptarray_append_point(dpa, &p1, LW_FALSE);
				break; /* substring complete */
			}
			/*
			 * Didn't reach the 'end' point,
			 * just copy second point
			 */
			else if (to > tlength + slength)
			{
				ptarray_append_point(dpa, &p2, LW_FALSE);
				goto END;
			}
			/*
			 * 'to' point falls on this segment
			 * Interpolate and break.
			 */
			else if (to < tlength + slength )
			{
				dseg = (to - tlength) / slength;
				geog2cart(&g1, &q1);
				geog2cart(&g2, &q2);
				interpolate_point4d_sphere(&q1, &q2, &p1, &p2, dseg, &pt);
				ptarray_append_point(dpa, &pt, LW_FALSE);
				break;
			}
		}
END:
		tlength += slength;
		memcpy(&p1, &p2, sizeof(POINT4D));
	}

	return dpa;
}

PG_FUNCTION_INFO_V1(geography_line_substring);
Datum geography_line_substring(PG_FUNCTION_ARGS)
{
	GSERIALIZED *gser = PG_GETARG_GSERIALIZED_P(0);
	double from_fraction = PG_GETARG_FLOAT8(1);
	double to_fraction = PG_GETARG_FLOAT8(2);
	LWLINE *lwline;
	LWGEOM *lwresult;
	POINTARRAY* opa;
	GSERIALIZED *result;

	if ( from_fraction < 0 || from_fraction > 1 )
	{
		elog(ERROR,"line_interpolate_point: 2nd arg isn't within [0,1]");
		PG_FREE_IF_COPY(gser, 0);
		PG_RETURN_NULL();
	}
	if ( to_fraction < 0 || to_fraction > 1 )
	{
		elog(ERROR,"line_interpolate_point: 3rd arg isn't within [0,1]");
		PG_FREE_IF_COPY(gser, 0);
		PG_RETURN_NULL();
	}
	if ( from_fraction > to_fraction )
	{
		elog(ERROR, "2nd arg must be smaller then 3rd arg");
		PG_RETURN_NULL();
	}
	if ( gserialized_get_type(gser) != LINETYPE )
	{
		elog(ERROR,"line_substring: 1st arg isn't a line");
		PG_FREE_IF_COPY(gser, 0);
		PG_RETURN_NULL();
	}

	lwline = lwgeom_as_lwline(lwgeom_from_gserialized(gser));
	opa = geography_substring(lwline->points, from_fraction, to_fraction,
		FP_TOLERANCE);

	lwgeom_free(lwline_as_lwgeom(lwline));
	PG_FREE_IF_COPY(gser, 0);

	if (opa->npoints <= 1)
	{
		lwresult = lwpoint_as_lwgeom(lwpoint_construct(lwline->srid, NULL, opa));
	} else {
		lwresult = lwline_as_lwgeom(lwline_construct(lwline->srid, NULL, opa));
	}

	lwgeom_set_geodetic(lwresult, true);
	result = geography_serialize(lwresult);
	lwgeom_free(lwresult);

	PG_RETURN_POINTER(result);
}

/***********************************************************************
 * Interpolate a point along a geographic line.
 ***********************************************************************/

POINTARRAY* lwline_interpolate_points_sphere(const LWLINE *line, double length_fraction,
	char repeat)
{
	POINT4D pt;
	uint32_t i;
	uint32_t points_to_interpolate;
	uint32_t points_found = 0;
	double length;
	double length_fraction_increment = length_fraction;
	double length_fraction_consumed = 0;
	char has_z = (char) lwgeom_has_z(lwline_as_lwgeom(line));
	char has_m = (char) lwgeom_has_m(lwline_as_lwgeom(line));
	const POINTARRAY* ipa = line->points;
	POINTARRAY* opa;
	POINT4D p1, p2;
	POINT3D q1, q2;
	GEOGRAPHIC_POINT g1, g2;

	/* Empty.InterpolatePoint == Point Empty */
	if ( lwline_is_empty(line) )
	{
		return ptarray_construct_empty(has_z, has_m, 0);
	}

	/* If distance is one of the two extremes, return the point on that
	 * end rather than doing any computations
	 */
	if ( length_fraction == 0.0 || length_fraction == 1.0 )
	{
		if ( length_fraction == 0.0 )
			getPoint4d_p(ipa, 0, &pt);
		else
			getPoint4d_p(ipa, ipa->npoints-1, &pt);

		opa = ptarray_construct(has_z, has_m, 1);
		ptarray_set_point4d(opa, 0, &pt);

		return opa;
	}

	/* Interpolate points along the line */
	length = ptarray_length_sphere(ipa);
	points_to_interpolate = repeat ? (uint32_t) floor(1 / length_fraction) : 1;
	opa = ptarray_construct(has_z, has_m, points_to_interpolate);

	getPoint4d_p(ipa, 0, &p1);
	geographic_point_init(p1.x, p1.y, &g1);
	for ( i = 0; i < ipa->npoints - 1 && points_found < points_to_interpolate; i++ )
	{
		getPoint4d_p(ipa, i+1, &p2);
		geographic_point_init(p2.x, p2.y, &g2);
		double segment_length_frac = sphere_distance(&g1, &g2) / length;

		/* If our target distance is before the total length we've seen
		 * so far. create a new point some distance down the current
		 * segment.
		 */
		while ( length_fraction < length_fraction_consumed + segment_length_frac && points_found < points_to_interpolate )
		{
			double segment_fraction = (length_fraction - length_fraction_consumed) / segment_length_frac;
			geog2cart(&g1, &q1);
			geog2cart(&g2, &q2);
			interpolate_point4d_sphere(&q1, &q2, &p1, &p2, segment_fraction, &pt);
			ptarray_set_point4d(opa, points_found++, &pt);
			length_fraction += length_fraction_increment;
		}

		length_fraction_consumed += segment_length_frac;

		p1 = p2;
		g1 = g2;
	}

	/* Return the last point on the line. This shouldn't happen, but
	 * could if there's some floating point rounding errors. */
	if (points_found < points_to_interpolate) {
		getPoint4d_p(ipa, ipa->npoints - 1, &pt);
		ptarray_set_point4d(opa, points_found, &pt);
	}

    return opa;
}

PG_FUNCTION_INFO_V1(geography_line_interpolate_point);
Datum geography_line_interpolate_point(PG_FUNCTION_ARGS)
{
	GSERIALIZED *gser = PG_GETARG_GSERIALIZED_P(0);
	double distance_fraction = PG_GETARG_FLOAT8(1);
	bool repeat = PG_NARGS() > 2 && PG_GETARG_BOOL(2);
	int srid = gserialized_get_srid(gser);
	LWLINE* lwline;
	LWGEOM* lwresult;
	POINTARRAY* opa;
	GSERIALIZED *result;

	if ( distance_fraction < 0 || distance_fraction > 1 )
	{
		elog(ERROR,"line_interpolate_point: 2nd arg isn't within [0,1]");
		PG_FREE_IF_COPY(gser, 0);
		PG_RETURN_NULL();
	}

	if ( gserialized_get_type(gser) != LINETYPE )
	{
		elog(ERROR,"line_interpolate_point: 1st arg isn't a line");
		PG_FREE_IF_COPY(gser, 0);
		PG_RETURN_NULL();
	}

	lwline = lwgeom_as_lwline(lwgeom_from_gserialized(gser));
	opa = lwline_interpolate_points_sphere(lwline, distance_fraction, repeat);

	lwgeom_free(lwline_as_lwgeom(lwline));
	PG_FREE_IF_COPY(gser, 0);

	if (opa->npoints <= 1)
	{
		lwresult = lwpoint_as_lwgeom(lwpoint_construct(srid, NULL, opa));
	} else {
		lwresult = lwmpoint_as_lwgeom(lwmpoint_construct(srid, opa));
	}

	lwgeom_set_geodetic(lwresult, true);
	result = geography_serialize(lwresult);
	lwgeom_free(lwresult);

	PG_RETURN_POINTER(result);
}

/***********************************************************************
 * Locate a point along a geographic line.
 ***********************************************************************/

double
ptarray_locate_point_sphere(const POINTARRAY *pa, const POINT4D *p,
	double tolerance, double *mindistout, POINT4D *closest)
{
	GEOGRAPHIC_EDGE e;
	GEOGRAPHIC_POINT a, b, nearest;
	POINT4D p1, p2;
	POINT2D p2d;
	uint32_t i, seg = 0;
	double distance, result;
	long double fraction, /* Used for computing Z and M values of the closest point */
		length, /* Length of the current segment */
		seglength, /* length of the segment where the closest point is located */
		partlength = 0.0, /* length from the beginning of the point array to the closest point */
		totlength = 0.0;  /* length of the point array */

	/* Initialize target point */
	geographic_point_init(p->x, p->y, &a);

	/* Handle point/point case here */
	if ( pa->npoints <= 1)
	{
		if ( pa->npoints == 1 && mindistout )
		{
			getPoint4d_p(pa, 0, &p1);
			geographic_point_init(p1.x, p1.y, &b);
			*mindistout = sphere_distance(&a, &b);
		}
		return 0.0;
	}

	/* Make distance really big, so that everything will be smaller than it */
	distance = FLT_MAX;

	/* Initialize first point of array */
	getPoint4d_p(pa, 0, &p1);
	geographic_point_init(p1.x, p1.y, &(e.start));

	/* Iterate through the edges in the line */
	for ( i = 1; i < pa->npoints; i++ )
	{
		double d;
		getPoint4d_p(pa, i, &p2);
		geographic_point_init(p2.x, p2.y, &(e.end));
		/* Get the spherical distance between point and edge */
		d = edge_distance_to_point(&e, &a, &b);
		/* New shortest distance! Record this distance/location/segment */
		if ( d < distance )
		{
			distance = d;
			nearest = b;
			seg = i - 1;
		}
		/* We've gotten closer than the tolerance... */
		if ( d < tolerance )
			break;

		e.start = e.end;
	}

	/* Initialize first point of array */
	getPoint4d_p(pa, 0, &p1);
	geographic_point_init(p1.x, p1.y, &a);

	/* Loop and sum the length for each segment */
	for ( i = 1; i < pa->npoints; i++ )
	{
		getPoint4d_p(pa, i, &p2);
		geographic_point_init(p2.x, p2.y, &b);

		/* Compute length of current segment */
		length = sphere_distance(&a, &b);

		/* Add segment length to the partial and total length */
		if (i < seg)
			partlength += length;
		else if (i == seg)
			seglength = length;
		totlength += length;

		/* B gets incremented in the next loop, so we save the value here */
		a = b;
	}

	/* Get the points defining the segment of the closest point */
	getPoint4d_p(pa, seg, &p1);
	getPoint4d_p(pa, seg + 1, &p2);

	/* Compute distance from beginning of the segment to closest point */
	geographic_point_init(p1.x, p1.y, &a);
	length = sphere_distance(&a, &nearest);

	/* Add this length to the partial length */
	partlength += length;

	/* Set output parameters */
	if ( mindistout )
		*mindistout = distance;
	if ( closest )
	{
		/* Set lon and lat for output parameter */
		closest->x = p2d.x = rad2deg(nearest.lon);
		closest->y = p2d.y = rad2deg(nearest.lat);

		/* For robustness, return the original point in line when
		 * closest point ~= one of the points in line */
		if (p2d_same(&p2d, getPoint2d_cp(pa, seg)))
			getPoint4d_p(pa, seg, closest);
		else if (p2d_same(&p2d, getPoint2d_cp(pa, seg + 1)))
			getPoint4d_p(pa, seg + 1, closest);
		else
		{
			if (ptarray_has_z(pa) || ptarray_has_m(pa))
			{
				fraction = length / seglength;
				closest->z = p1.z + (double) ((long double) (p2.z - p1.z) * fraction);
				closest->m = p1.m + (double) ((long double) (p2.m - p1.m) * fraction);
			}
			else
			{
				closest->z = NO_Z_VALUE;
				closest->m = NO_M_VALUE;
			}
		}
	}

	/* Location of any point on a zero-length line is 0 */
	/* See http://trac.osgeo.org/postgis/ticket/1772#comment:2 */
	if ( totlength == 0 )
		return 0.0;

	/* For robustness, force 0/1 when closest point == start/endpoint */
	getPoint4d_p(pa, 0, &p1);
	getPoint4d_p(pa, pa->npoints - 1, &p2);
	if ( seg == 0 && p4d_same(closest, &p1) )
		return 0.0;
	if ( seg >= (pa->npoints - 2) && p4d_same(closest, &p2) )
		return 1.0;

	result = (double) (partlength / totlength);
	return result;
}

Datum geography_line_locate_point(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(geography_line_locate_point);
Datum geography_line_locate_point(PG_FUNCTION_ARGS)
{
	GSERIALIZED *gser1 = PG_GETARG_GSERIALIZED_P(0);
	GSERIALIZED *gser2 = PG_GETARG_GSERIALIZED_P(1);
	LWLINE *lwline;
	LWPOINT *lwpoint;
	POINT4D p, proj;
	double ret;

	if ( gserialized_get_type(gser1) != LINETYPE )
	{
		elog(ERROR,"line_locate_point: 1st arg isn't a line");
		PG_RETURN_NULL();
	}
	if ( gserialized_get_type(gser2) != POINTTYPE )
	{
		elog(ERROR,"line_locate_point: 2st arg isn't a point");
		PG_RETURN_NULL();
	}

	error_if_srid_mismatch(gserialized_get_srid(gser1), gserialized_get_srid(gser2));

	lwline = lwgeom_as_lwline(lwgeom_from_gserialized(gser1));
	lwpoint = lwgeom_as_lwpoint(lwgeom_from_gserialized(gser2));

	lwpoint_getPoint4d_p(lwpoint, &p);

	ret = ptarray_locate_point_sphere(lwline->points, &p, FP_TOLERANCE, NULL, &proj);

	PG_RETURN_FLOAT8(ret);
}

/*****************************************************************************/
