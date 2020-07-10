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
 * Functions copied from PostGIS since they are not exported
 ***********************************************************************/

extern int ptarray_has_z(const POINTARRAY *pa);
extern int ptarray_has_m(const POINTARRAY *pa);


#define CIRC_NODE_SIZE 8

extern int circ_tree_get_point(const CIRC_NODE* node, POINT2D* pt);
extern int circ_tree_contains_point(const CIRC_NODE* node, const POINT2D* pt, const POINT2D* pt_outside, int* on_boundary);
extern uint32_t edge_intersects(const POINT3D *A1, const POINT3D *A2, const POINT3D *B1, const POINT3D *B2);
extern int edge_intersection(const GEOGRAPHIC_EDGE *e1, const GEOGRAPHIC_EDGE *e2, GEOGRAPHIC_POINT *g);
extern double edge_distance_to_edge(const GEOGRAPHIC_EDGE *e1, const GEOGRAPHIC_EDGE *e2, GEOGRAPHIC_POINT *closest1, GEOGRAPHIC_POINT *closest2);
extern void circ_tree_free(CIRC_NODE* node);
extern CIRC_NODE* lwgeom_calculate_circ_tree(const LWGEOM* lwgeom);
extern double circ_tree_distance_tree(const CIRC_NODE* n1, const CIRC_NODE* n2, const SPHEROID *spheroid, double threshold);

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
	return;
}

double
circ_tree_distance_tree_internal(const CIRC_NODE* n1, const CIRC_NODE* n2, double threshold,
		double* min_dist, double* max_dist, GEOGRAPHIC_POINT* closest1, GEOGRAPHIC_POINT* closest2)
{
	double max;
	double d, d_min;

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
	if ( n1->geom_type == POLYGONTYPE && n2->geom_type && ! lwtype_is_collection(n2->geom_type) )
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
	if ( n2->geom_type == POLYGONTYPE && n1->geom_type && ! lwtype_is_collection(n1->geom_type) )
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
	if ( circ_node_is_leaf(n1) && circ_node_is_leaf(n2) )
	{
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
		if ( n1->geom_type && lwtype_is_collection(n1->geom_type) )
		{
			circ_internal_nodes_sort(n1->nodes, n1->num_nodes, n2);
			for ( i = 0; i < n1->num_nodes; i++ )
			{
				d = circ_tree_distance_tree_internal(n1->nodes[i], n2, threshold, min_dist, max_dist, closest1, closest2);
				d_min = FP_MIN(d_min, d);
			}
		}
		else if ( n2->geom_type && lwtype_is_collection(n2->geom_type) )
		{
            uint32_t i;
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
 * Closest point and closest line functions for geographies.
 * These functions are an extension to PostGIS
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

	/* Quietly decrease the threshold just a little to avoid cases where */
	/* the actual spheroid distance is larger than the sphere distance */
	/* causing the return value to be larger than the threshold value */
	// double threshold_radians = 0.95 * threshold / spheroid->radius;
	double threshold_radians = threshold / WGS84_RADIUS;

	circ_tree_distance_tree_internal(circ_tree1, circ_tree2, threshold_radians,
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

	result = geometry_serialize(point);
	lwgeom_free(point);

	PG_FREE_IF_COPY(g1, 0);
	PG_FREE_IF_COPY(g2, 1);
	PG_RETURN_POINTER(result);
}

/*****************************************************************************/

LWGEOM *
geography_tree_shortestline(const GSERIALIZED* g1, const GSERIALIZED* g2,
	double threshold, const SPHEROID *spheroid)
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

	/* Quietly decrease the threshold just a little to avoid cases where */
	/* the actual spheroid distance is larger than the sphere distance */
	/* causing the return value to be larger than the threshold value */
	// double threshold_radians = 0.95 * threshold / spheroid->radius;
	double threshold_radians = threshold / spheroid->radius;

	circ_tree_distance_tree_internal(circ_tree1, circ_tree2, threshold_radians,
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
	bool use_spheroid = true;
	SPHEROID s;

	/* Get our geography objects loaded into memory. */
	g1 = PG_GETARG_GSERIALIZED_P(0);
	g2 = PG_GETARG_GSERIALIZED_P(1);

	error_if_srid_mismatch(gserialized_get_srid(g1), gserialized_get_srid(g2));

	/* Read calculation type */
	if ( PG_NARGS() > 2 && ! PG_ARGISNULL(2) )
		use_spheroid = PG_GETARG_BOOL(2);

	/* Return NULL on empty arguments. */
	if ( gserialized_is_empty(g1) || gserialized_is_empty(g2) )
	{
		PG_FREE_IF_COPY(g1, 0);
		PG_FREE_IF_COPY(g2, 1);
		PG_RETURN_NULL();
	}

	/* Initialize spheroid */
	/* We currently cannot use the following statement since PROJ4 API is not
	 * available directly to MobilityDB. */
	// spheroid_init_from_srid(fcinfo, srid, &s);
	spheroid_init(&s, WGS84_MAJOR_AXIS, WGS84_MINOR_AXIS);

	/* Set to sphere if requested */
	if ( ! use_spheroid )
		s.a = s.b = s.radius;

	line = geography_tree_shortestline(g1, g2, FP_TOLERANCE, &s);

	if (lwgeom_is_empty(line))
		PG_RETURN_NULL();

	result = geometry_serialize(line);
	lwgeom_free(line);

	PG_FREE_IF_COPY(g1, 0);
	PG_FREE_IF_COPY(g2, 1);
	PG_RETURN_POINTER(result);
}

/***********************************************************************
 * Interpolate a point along a geographic line.
 ***********************************************************************/

/**
 * Find interpolation point p
 * between geography points p1 and p2
 * so that the len(p1,p) == len(p1,p2) * f
 * and p falls on p1,p2 segment.
 */
void
geography_interpolate_point4d(
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

POINTARRAY* geography_interpolate_points(const LWLINE *line, double length_fraction,
	const SPHEROID *s, char repeat)
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
	length = ptarray_length_spheroid(ipa, s);
	points_to_interpolate = repeat ? (uint32_t) floor(1 / length_fraction) : 1;
	opa = ptarray_construct(has_z, has_m, points_to_interpolate);

	getPoint4d_p(ipa, 0, &p1);
	geographic_point_init(p1.x, p1.y, &g1);
	for ( i = 0; i < ipa->npoints - 1 && points_found < points_to_interpolate; i++ )
	{
		getPoint4d_p(ipa, i+1, &p2);
		geographic_point_init(p2.x, p2.y, &g2);
		double segment_length_frac = spheroid_distance(&g1, &g2, s) / length;

		/* If our target distance is before the total length we've seen
		 * so far. create a new point some distance down the current
		 * segment.
		 */
		while ( length_fraction < length_fraction_consumed + segment_length_frac && points_found < points_to_interpolate )
		{
			geog2cart(&g1, &q1);
			geog2cart(&g2, &q2);
			double segment_fraction = (length_fraction - length_fraction_consumed) / segment_length_frac;
			geography_interpolate_point4d(&q1, &q2, &p1, &p2, segment_fraction, &pt);
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

void spheroid_init(SPHEROID *s, double a, double b)
{
	s->a = a;
	s->b = b;
	s->f = (a - b) / a;
	s->e_sq = (a*a - b*b)/(a*a);
	s->radius = (2.0 * a + b ) / 3.0;
}

PG_FUNCTION_INFO_V1(geography_line_interpolate_point);
Datum geography_line_interpolate_point(PG_FUNCTION_ARGS)
{
	GSERIALIZED *gser = PG_GETARG_GSERIALIZED_P(0);
	double distance_fraction = PG_GETARG_FLOAT8(1);
	/* Read calculation type */
	bool use_spheroid = true;
	if ( PG_NARGS() > 2 && ! PG_ARGISNULL(2) )
		use_spheroid = PG_GETARG_BOOL(2);
	/* Read repeat mode */
	bool repeat = PG_NARGS() > 3 && PG_GETARG_BOOL(3);
	int srid = gserialized_get_srid(gser);
	LWLINE* lwline;
	LWGEOM* lwresult;
	POINTARRAY* opa;
	SPHEROID s;
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

	/* Initialize spheroid */
	/* We currently cannot use the following statement since PROJ4 API is not
	 * available directly to MobilityDB. */
	// spheroid_init_from_srid(fcinfo, srid, &s);
	spheroid_init(&s, WGS84_MAJOR_AXIS, WGS84_MINOR_AXIS);

	/* Set to sphere if requested */
	if ( ! use_spheroid )
		s.a = s.b = s.radius;

	lwline = lwgeom_as_lwline(lwgeom_from_gserialized(gser));
	opa = geography_interpolate_points(lwline, distance_fraction, &s, repeat);

	lwgeom_free(lwline_as_lwgeom(lwline));
	PG_FREE_IF_COPY(gser, 0);

	if (opa->npoints <= 1)
	{
		lwresult = lwpoint_as_lwgeom(lwpoint_construct(srid, NULL, opa));
	} else {
		lwresult = lwmpoint_as_lwgeom(lwmpoint_construct(srid, opa));
	}

	lwgeom_set_geodetic(lwresult, true);
	result = geometry_serialize(lwresult);
	lwgeom_free(lwresult);

	PG_RETURN_POINTER(result);
}

/***********************************************************************
 * Locate a point along a geographic line.
 ***********************************************************************/

double
ptarray_locate_point_spheroid(const POINTARRAY *pa, const POINT4D *p4d,
	const SPHEROID *s, double tolerance, double *mindistout, POINT4D *proj4d)
{
	GEOGRAPHIC_EDGE e;
	GEOGRAPHIC_POINT a, b, nearest;
	POINT4D p1, p2;
	const POINT2D *p;
	POINT2D proj;
	uint32_t i, seg = 0;
	int use_sphere = (s->a == s->b ? 1 : 0);
	int hasz;
	double za = 0.0, zb = 0.0;
	double distance,
		length, 	/* Used for computing lengths */
		seglength, /* length of the segment where the closest point is located */
		partlength, /* length from the beginning of the point array to the closest point */
		totlength;  /* length of the point array */

	/* Initialize our point */
	geographic_point_init(p4d->x, p4d->y, &a);

	/* Handle point/point case here */
	if ( pa->npoints <= 1)
	{
		if ( pa->npoints == 1 )
		{
			p = getPoint2d_cp(pa, 0);
			geographic_point_init(p->x, p->y, &b);
			/* Sphere special case, axes equal */
			*mindistout = s->radius * sphere_distance(&a, &b);
			/* If close or greater than tolerance, get the real answer to be sure */
			if ( ! use_sphere || *mindistout > 0.95 * tolerance )
				*mindistout = spheroid_distance(&a, &b, s);
		}
		return 0.0;
	}

	/* Make result really big, so that everything will be smaller than it */
	distance = FLT_MAX;

	/* Initialize start of line */
	p = getPoint2d_cp(pa, 0);
	geographic_point_init(p->x, p->y, &(e.start));

	/* Iterate through the edges in our line */
	for ( i = 1; i < pa->npoints; i++ )
	{
		double d;
		p = getPoint2d_cp(pa, i);
		geographic_point_init(p->x, p->y, &(e.end));
		/* Get the spherical distance between point and edge */
		d = s->radius * edge_distance_to_point(&e, &a, &b);
		/* New shortest distance! Record this distance / location / segment */
		if ( d < distance )
		{
			distance = d;
			nearest = b;
			seg = i - 1;
		}
		/* We've gotten closer than the tolerance... */
		if ( d < tolerance )
		{
			/* Working on a sphere? The answer is correct, return */
			if ( use_sphere )
			{
				break;
			}
			/* Far enough past the tolerance that the spheroid calculation won't change things */
			else if ( d < tolerance * 0.95 )
			{
				break;
			}
			/* On a spheroid and near the tolerance? Confirm that we are *actually* closer than tolerance */
			else
			{
				d = spheroid_distance(&a, &nearest, s);
				/* Yes, closer than tolerance, return! */
				if ( d < tolerance )
					break;
			}
		}
		e.start = e.end;
	}

	if ( mindistout ) *mindistout = distance;

	/* See if we have a third dimension */
	hasz = FLAGS_GET_Z(pa->flags);

	/* Initialize first point of array */
	getPoint4d_p(pa, 0, &p1);
	geographic_point_init(p1.x, p1.y, &a);
	if ( hasz )
		za = p1.z;

	partlength = 0.0;
	totlength = 0.0;

	/* Loop and sum the length for each segment */
	for ( i = 1; i < pa->npoints; i++ )
	{
		getPoint4d_p(pa, i, &p1);
		geographic_point_init(p1.x, p1.y, &b);
		if ( hasz )
			zb = p1.z;

		/* Special sphere case */
		if ( s->a == s->b )
			length = s->radius * sphere_distance(&a, &b);
		/* Spheroid case */
		else
			length = spheroid_distance(&a, &b, s);

		/* Add in the vertical displacement if we're in 3D */
		if ( hasz )
			length = sqrt( (zb-za)*(zb-za) + length*length );

		/* Add this segment length to the total length */
		totlength += length;

		/* Add this segment length to the partial length */
		if (i < seg)
			partlength += length;
		else if (i == seg)
			/* Save segment length */
			seglength = length;

		/* B gets incremented in the next loop, so we save the value here */
		a = b;
		za = zb;
	}

	/* Copy nearest into 2D/4D holder */
	proj4d->x = proj.x = rad2deg(nearest.lon);
	proj4d->y = proj.y = rad2deg(nearest.lat);

	/* Compute distance from beginning of the segment to closest point */

	/* Start of the segment */
	getPoint4d_p(pa, seg, &p1);
	geographic_point_init(p1.x, p1.y, &a);

	/* Closest point */
	geographic_point_init(proj4d->x, proj4d->y, &b);

	/* Special sphere case */
	if ( s->a == s->b )
		length = s->radius * sphere_distance(&a, &b);
	/* Spheroid case */
	else
		length = spheroid_distance(&a, &b, s);

	if ( hasz )
	{
		/* Compute Z and M values for closest point */
		double f = length / seglength;
		getPoint4d_p(pa, seg + 1, &p2);
		proj4d->z = p1.z + ((p2.z - p1.z) * f);
		proj4d->m = p1.m + ((p2.m - p1.m) * f);
		/* Add in the vertical displacement if we're in 3D */
		za = p1.z;
		zb = proj4d->z;
		length = sqrt( (zb-za)*(zb-za) + length*length );
	}

	/* Add this segment length to the total */
	partlength += length;

	/* Location of any point on a zero-length line is 0 */
	/* See http://trac.osgeo.org/postgis/ticket/1772#comment:2 */
	if ( totlength == 0 )
		return 0;

	/* For robustness, force 1 when closest point == endpoint */
	p = getPoint2d_cp(pa, pa->npoints - 1);
	if ( (seg >= (pa->npoints-2)) && p2d_same(&proj, p) )
		return 1.0;

	return partlength / totlength;
}

Datum geography_line_locate_point(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(geography_line_locate_point);
Datum geography_line_locate_point(PG_FUNCTION_ARGS)
{
	GSERIALIZED *gser1 = PG_GETARG_GSERIALIZED_P(0);
	GSERIALIZED *gser2 = PG_GETARG_GSERIALIZED_P(1);
	bool use_spheroid = true;
	/* Read our calculation type */
	if ( PG_NARGS() > 2 && ! PG_ARGISNULL(2) )
		use_spheroid = PG_GETARG_BOOL(2);
	double tolerance = FP_TOLERANCE;
	SPHEROID s;
	LWLINE *lwline;
	LWPOINT *lwpoint;
	POINTARRAY *pa;
	POINT4D p, p_proj;
	double ret;

	/* Initialize spheroid */
	/* We currently cannot use the following statement since PROJ4 API is not
	 * available directly to MobilityDB. */
	// spheroid_init_from_srid(fcinfo, gserialized_get_srid(gser1), &s);
	spheroid_init(&s, WGS84_MAJOR_AXIS, WGS84_MINOR_AXIS);

	/* Set to sphere if requested */
	if ( ! use_spheroid )
		s.a = s.b = s.radius;

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

	/* User requests spherical calculation, turn our spheroid into a sphere */
	if ( ! use_spheroid )
		s.a = s.b = s.radius;
	else
		/* Initialize spheroid */
		/* We cannot use the following statement since PROJ4 API is not
		 * available directly to MobilityDB. */
		// spheroid_init_from_srid(fcinfo, srid, &s);
		spheroid_init(&s, WGS84_MAJOR_AXIS, WGS84_MINOR_AXIS);

	error_if_srid_mismatch(gserialized_get_srid(gser1), gserialized_get_srid(gser2));

	lwline = lwgeom_as_lwline(lwgeom_from_gserialized(gser1));
	lwpoint = lwgeom_as_lwpoint(lwgeom_from_gserialized(gser2));

	pa = lwline->points;
	lwpoint_getPoint4d_p(lwpoint, &p);

	ret = ptarray_locate_point_spheroid(pa, &p, &s, tolerance, NULL, &p_proj);

	PG_RETURN_FLOAT8(ret);
}

/***********************************************************************
 * New version of ST_Segmentize that allows to perform the calculation
 * on a sphere or spheroid
 ***********************************************************************/

static int ptarray_segmentize_spheroid_edge_recursive (
	const POINT3D *p1, const POINT3D *p2, /* 3-space points we are interpolating between */
	const POINT4D *v1, const POINT4D *v2, /* real values and z/m values */
	double d, double max_seg_length, /* current segment length and segment limit */
	POINTARRAY *pa) /* write out results here */
{
	GEOGRAPHIC_POINT g;
	/* Reached the terminal leaf in recursion. Add */
	/* the left-most point to the pointarray here */
	/* We recurse down the left side first, so outputs should */
	/* end up added to the array in order this way */
	if (d <= max_seg_length)
	{
		POINT4D p;
		cart2geog(p1, &g);
		p.x = v1->x;
		p.y = v1->y;
		p.z = v1->z;
		p.m = v1->m;
		return ptarray_append_point(pa, &p, LW_FALSE);
	}
	/* Find the mid-point and recurse on the left and then the right */
	else
	{
		/* Calculate mid-point */
		POINT3D mid;
		mid.x = (p1->x + p2->x) / 2.0;
		mid.y = (p1->y + p2->y) / 2.0;
		mid.z = (p1->z + p2->z) / 2.0;
		normalize(&mid);

		/* Calculate z/m mid-values */
		POINT4D midv;
		cart2geog(&mid, &g);
		midv.x = rad2deg(g.lon);
		midv.y = rad2deg(g.lat);
		midv.z = (v1->z + v2->z) / 2.0;
		midv.m = (v1->m + v2->m) / 2.0;
		/* Recurse on the left first */
		ptarray_segmentize_spheroid_edge_recursive(p1, &mid, v1, &midv, d/2.0, max_seg_length, pa);
		ptarray_segmentize_spheroid_edge_recursive(&mid, p2, &midv, v2, d/2.0, max_seg_length, pa);
		return LW_SUCCESS;
	}
}

/**
* Create a new point array with no segment longer than the input segment length (expressed in radians!)
* @param pa_in - input point array pointer
* @param max_seg_length - maximum output segment length in radians
*/
static POINTARRAY*
ptarray_segmentize_spheroid(const POINTARRAY *pa_in, double max_seg_length, const SPHEROID *s)
{
	POINTARRAY *pa_out;
	int hasz = ptarray_has_z(pa_in);
	int hasm = ptarray_has_m(pa_in);
	POINT4D p1, p2;
	POINT3D q1, q2;
	GEOGRAPHIC_POINT g1, g2;
	uint32_t i;

	/* Just crap out on crazy input */
	if ( ! pa_in )
		lwerror("%s: null input pointarray", __func__);
	if ( max_seg_length <= 0.0 )
		lwerror("%s: maximum segment length must be positive", __func__);

	/* Empty starting array */
	pa_out = ptarray_construct_empty(hasz, hasm, pa_in->npoints);

	/* Simple loop per edge */
	for (i = 1; i < pa_in->npoints; i++)
	{
		getPoint4d_p(pa_in, i-1, &p1);
		getPoint4d_p(pa_in, i, &p2);
		geographic_point_init(p1.x, p1.y, &g1);
		geographic_point_init(p2.x, p2.y, &g2);

		/* Skip duplicate points (except in case of 2-point lines!) */
		if ((pa_in->npoints > 2) && p4d_same(&p1, &p2))
			continue;

		/* How long is this edge? */
		double d;
		/* Special sphere case */
		if ( s->a == s->b )
			d = s->radius * sphere_distance(&g1, &g2);
		/* Spheroid case */
		else
			d = spheroid_distance(&g1, &g2, s);

		if (d > max_seg_length)
		{
			geog2cart(&g1, &q1);
			geog2cart(&g2, &q2);
			/* 3-d end points, XYZM end point, current edge size, min edge size */
			ptarray_segmentize_spheroid_edge_recursive(&q1, &q2, &p1, &p2, d, max_seg_length, pa_out);
		}
		/* If we don't segmentize, we need to add first point manually */
		else
		{
			ptarray_append_point(pa_out, &p1, LW_TRUE);
		}
	}
	/* Always add the last point */
	ptarray_append_point(pa_out, &p2, LW_TRUE);
	return pa_out;
}

/**
* Create a new, densified geometry where no segment is longer than max_seg_length.
* Input geometry is not altered, output geometry must be freed by caller.
* @param lwg_in = input geometry
* @param max_seg_length = maximum segment length in radians
*/
LWGEOM*
lwgeom_segmentize_spheroid(const LWGEOM *lwg_in, double max_seg_length, const SPHEROID *s)
{
	POINTARRAY *pa_out;
	LWLINE *lwline;
	LWPOLY *lwpoly_in, *lwpoly_out;
	LWCOLLECTION *lwcol_in, *lwcol_out;
	uint32_t i;

	/* Reflect NULL */
	if ( ! lwg_in )
		return NULL;

	/* Clone empty */
	if ( lwgeom_is_empty(lwg_in) )
		return lwgeom_clone(lwg_in);

	switch (lwg_in->type)
	{
	case MULTIPOINTTYPE:
	case POINTTYPE:
		return lwgeom_clone_deep(lwg_in);
		break;
	case LINETYPE:
		lwline = lwgeom_as_lwline(lwg_in);
		pa_out = ptarray_segmentize_spheroid(lwline->points, max_seg_length, s);
		return lwline_as_lwgeom(lwline_construct(lwg_in->srid, NULL, pa_out));
		break;
	case POLYGONTYPE:
		lwpoly_in = lwgeom_as_lwpoly(lwg_in);
		lwpoly_out = lwpoly_construct_empty(lwg_in->srid, lwgeom_has_z(lwg_in), lwgeom_has_m(lwg_in));
		for ( i = 0; i < lwpoly_in->nrings; i++ )
		{
			pa_out = ptarray_segmentize_spheroid(lwpoly_in->rings[i], max_seg_length, s);
			lwpoly_add_ring(lwpoly_out, pa_out);
		}
		return lwpoly_as_lwgeom(lwpoly_out);
		break;
	case MULTILINETYPE:
	case MULTIPOLYGONTYPE:
	case COLLECTIONTYPE:
		lwcol_in = lwgeom_as_lwcollection(lwg_in);
		lwcol_out = lwcollection_construct_empty(lwg_in->type, lwg_in->srid, lwgeom_has_z(lwg_in), lwgeom_has_m(lwg_in));
		for ( i = 0; i < lwcol_in->ngeoms; i++ )
		{
			lwcollection_add_lwgeom(lwcol_out, lwgeom_segmentize_spheroid(lwcol_in->geoms[i], max_seg_length, s));
		}
		return lwcollection_as_lwgeom(lwcol_out);
		break;
	default:
		lwerror("lwgeom_segmentize_spheroid: unsupported input geometry type: %d - %s",
		        lwg_in->type, lwtype_name(lwg_in->type));
		break;
	}

	lwerror("lwgeom_segmentize_spheroid got to the end of the function, should not happen");
	return NULL;
}

/*
** geography_segmentize(GSERIALIZED *g1, double max_seg_length, boolean use_spheroid)
** returns densified geometry with no segment longer than max
*/
PG_FUNCTION_INFO_V1(geography_segmentize1);
Datum geography_segmentize1(PG_FUNCTION_ARGS)
{
	LWGEOM *lwgeom1 = NULL;
	LWGEOM *lwgeom2 = NULL;
	GSERIALIZED *g1 = NULL;
	GSERIALIZED *g2 = NULL;
	double max_seg_length;
	bool use_spheroid = true;
	uint32_t type1;
	SPHEROID s;

	/* Get our geometry object loaded into memory. */
	g1 = PG_GETARG_GSERIALIZED_P(0);
	type1 = gserialized_get_type(g1);

	/* Get max_seg_length in metric units */
	max_seg_length = PG_GETARG_FLOAT8(1);

	/* Read calculation type */
	if ( PG_NARGS() > 2 && ! PG_ARGISNULL(2) )
		use_spheroid = PG_GETARG_BOOL(2);

	/* We can't densify points or points, reflect them back */
	if ( type1 == POINTTYPE || type1 == MULTIPOINTTYPE || gserialized_is_empty(g1) )
		PG_RETURN_POINTER(g1);

	/* Initialize spheroid */
	/* We currently cannot use the following statement since PROJ4 API is not
	 * available directly to MobilityDB. */
	// spheroid_init_from_srid(fcinfo, srid, &s);
	spheroid_init(&s, WGS84_MAJOR_AXIS, WGS84_MINOR_AXIS);

	/* Set to sphere if requested */
	if ( ! use_spheroid )
		s.a = s.b = s.radius;

	/* Deserialize */
	lwgeom1 = lwgeom_from_gserialized(g1);

	/* Calculate the densified geometry */
	lwgeom2 = lwgeom_segmentize_spheroid(lwgeom1, max_seg_length, &s);

	/*
	** Set the geodetic flag so subsequent
	** functions do the right thing.
	*/
	lwgeom_set_geodetic(lwgeom2, true);

	/* Recalculate the boxes after re-setting the geodetic bit */
	lwgeom_drop_bbox(lwgeom2);

	/* We are trusting geography_serialize will add a box if needed */
	g2 = geography_serialize(lwgeom2);

	/* Clean up */
	lwgeom_free(lwgeom1);
	lwgeom_free(lwgeom2);
	PG_FREE_IF_COPY(g1, 0);

	PG_RETURN_POINTER(g2);
}

/*****************************************************************************/

/*****************************************************************************/

/*****************************************************************************/

/*****************************************************************************/

