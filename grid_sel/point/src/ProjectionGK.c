/*****************************************************************************
 *
 * ProjectionOps.c
 *	  Implementation of the Gauss Krueger projection that is used in Secondo
 * 
 * This projection does not correspond to any standard projection in
 * http://www.epsg.org/
 *
 * Portions Copyright (c) 2019, Esteban Zimanyi, Arthur Lesuisse,
 *		Universite Libre de Bruxelles
 * Portions Copyright (c) 1996-2019, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 *****************************************************************************/

#include "TemporalPoint.h"

double Pi   = 3.1415926535897932384626433832795028841971693993751058209749445923078164;
double rho  = 0;
double awgs = 6378137.0;
double bwgs =  6356752.314;
double abes = 6377397.155;  	/* Bessel Semi-Major Axis = Equatorial Radius in meters */
double bbes = 6356078.962;		/* Bessel Semi-Minor Axis = Polar Radius in meters */
double cbes = 111120.6196;		/* Bessel latitude to Gauss-Krueger meters */
double dx   = -585.7;			/* Translation Parameter 1 */
double dy   = -87.0;			/* Translation Parameter 2 */
double dz   = -409.2;			/* Translation Parameter 3 */
double rotx = 2.540423689E-6;	/* Rotation Parameter 1 */
double roty = 7.514612057E-7;	/* Rotation Parameter 2 */
double rotz = -1.368144208E-5;	/* Rotation Parameter 3 */
double sc   = 0.99999122;		/* Scaling Factor */
double h1   = 0;
double eqwgs = 0;
double eqbes = 0;
double MDC = 2.0;		/* standard in Hagen, zone=2 */
bool useWGS = true;		/* usw coordinates in wgs ellipsoid */

static POINT2D
BesselBLToGaussKrueger(double b, double ll)
{
	POINT2D result;
	double l0 = 3.0 * MDC;
	l0 = Pi * l0 / 180.0;
	double l = ll - l0;
	double k = cos(b);
	double t = sin(b) / k;
	double eq = eqbes;
	double Vq = 1.0 + eq * k * k;
	double v = sqrt(Vq);
	double Ng = abes * abes / (bbes * v);
	double nk =(abes - bbes) / (abes + bbes);
	double X = ((Ng * t * k * k * l * l) / 2) + 
		((Ng * t * (9 * Vq - t * t - 4) * k * k * k * k * l * l * l * l) / 24);
	double gg = b + (((-3.0 * nk / 2.0) + (9.0 * nk * nk * nk / 16.0)) *
		sin(2 * b) + 15 * nk * nk * sin(4 * b) / 16 - 35 * nk * nk * nk * sin(6 * b) / 48);
	double SS = gg * 180.0 * cbes / Pi;
	double Ho = (SS + X);
	double Y = Ng * k * l + Ng * (Vq - t * t) * k * k * k * l * l * l / 6 + Ng *
		(5 - 18 * t * t + t * t * t * t) * k * k * k * k * k * l * l * l * l * l / 120;
	double kk = 500000;
	double RVV = MDC;
	double Re = RVV * 1000000.0 + kk + Y;
	result.x = Re;
	result.y = Ho;
	return result;
}

static POINT3D
HelmertTransformation(double x, double y, double z)
{
	POINT3D p;
	p.x = dx + (sc * (1 * x + rotz * y - roty * z));
	p.y = dy + (sc * (-rotz * x + 1 * y + rotx * z));
	p.z = dz + (sc * (roty * x - rotx * y + 1 * z));
	return p;
}

static double 
newF(double f, double x, double y, double p)
{
	double zw;
	double nnq;
	zw = abes / sqrt(1 - eqbes * sin(f) * sin(f));
	nnq = 1 - (eqbes * zw / (sqrt(x * x + y * y) / cos(f)));
	return (atan(p / nnq));
}

static POINT3D
BLRauenberg (double x, double y, double z)
{
	POINT3D result;
	double f = Pi * 50 / 180.0;
	double p = z / sqrt(x * x + y * y);
	double f1, f2;
	do
	{
		f1 = newF(f, x, y, p);
		f2 = f;
		f = f1;
	} while ((fabs(f2-f1) >= (10E-10)));

	result.x = f;
	result.y = atan(y / x);
	result.z = sqrt(x * x + y * y) / cos(f1) - 
		(abes / sqrt(1 - eqbes * sin(f1) * sin(f1)));
	return result;
}

/*
static POINT2D 
bessel2WGS(double geoDezRight1, double geoDezHeight1)
{
	POINT2D result;
	double aBessel = abes;
	double eeBessel = 0.0066743722296294277832;
	double ScaleFactor = 0.00000982;
	double RotXRad = -7.16069806998785E-06;
	double RotYRad = 3.56822869296619E-07;
	double RotZRad = 7.06858347057704E-06;
	double ShiftXMeters = 591.28;
	double ShiftYMeters = 81.35;
	double ShiftZMeters = 396.39;
	double aWGS84 = awgs;
	double eeWGS84 = 0.0066943799;
	double geoDezRight = (geoDezRight1 / 180) * Pi;
	double geoDezHeight = (geoDezHeight1 / 180) * Pi;
	double sinRight = sin(geoDezRight);
	double sinRight2 = sinRight * sinRight;
	double n = eeBessel * sinRight2;
	n = 1 - n;
	n = sqrt(n);
	n = aBessel / n;
	double cosRight = cos(geoDezRight);
	double cosHeight = cos(geoDezHeight);
	double sinHeight = sin(geoDezHeight);
	double CartesianXMeters = n * cosRight * cosHeight;
	double CartesianYMeters = n * cosRight * sinHeight;
	double CartesianZMeters = n * (1 - eeBessel) * sinRight;

	double CartOutputXMeters = (1 + ScaleFactor) *
		CartesianXMeters + RotZRad * CartesianYMeters -
		RotYRad * CartesianZMeters + ShiftXMeters;
	double CartOutputYMeters = -RotZRad * CartesianXMeters +
		(1 + ScaleFactor) * CartesianYMeters +
		RotXRad * CartesianZMeters + ShiftYMeters;
	double CartOutputZMeters = RotYRad * CartesianXMeters -
		RotXRad * CartesianYMeters +
		(1 + ScaleFactor) * CartesianZMeters + ShiftZMeters;
	geoDezHeight = atan(CartOutputYMeters / CartOutputXMeters);
	double Latitude = (CartOutputXMeters * CartOutputXMeters)+
		(CartOutputYMeters * CartOutputYMeters);
	Latitude = sqrt(Latitude);
	double InitLat = Latitude;
	Latitude = CartOutputZMeters/Latitude;
	Latitude = atan(Latitude);
	double LatitudeIt = 99999999;
	do {
		LatitudeIt = Latitude;
		double sinLat = sin(Latitude);
		n = 1 - eeWGS84 * sinLat * sinLat;
		n = sqrt(n);
		n = aWGS84 / n;
		Latitude = InitLat;
		Latitude = (CartOutputZMeters + eeWGS84 * n * sin(LatitudeIt)) / Latitude;
		Latitude = atan(Latitude);
	} while (fabs(Latitude - LatitudeIt) >= 0.000000000000001);
	result.x = (geoDezHeight / Pi) * 180;
	result.y = (Latitude / Pi) * 180;
	return result;
}

static POINT2D
gk2geo(double GKRight, double GKHeight)
{
	rho = 180 / Pi;
	POINT2D result;
	result.x = result.y = 0;
	if (GKRight < 1000000 || GKHeight < 1000000)
	{
		return result;
	} 
	double e2 = 0.0067192188;
	double c = 6398786.849;

	double bI = GKHeight / 10000855.7646;
	double bII = bI * bI;
	double bf = 325632.08677 * bI * ((((((0.00000562025 * bII + 0.00022976983) *
		bII - 0.00113566119) * bII + 0.00424914906) * bII - 0.00831729565) * bII + 1));
	bf /= 3600 * rho;
	double co = cos(bf);
	double g2 = e2 * (co * co);
	double g1 = c / sqrt(1 + g2);
	double t = tan(bf);
	double fa = (GKRight - floor(GKRight / 1000000) * 1000000 - 500000) / g1;
	double GeoDezRight = ((bf - fa * fa * t * (1 + g2) / 2 + fa * fa * fa * fa * t * 
		(5 + 3 * t * t + 6 * g2 - 6 * g2 * t * t) / 24) * rho);
	double dl = fa - fa * fa * fa * (1 + 2 * t * t + g2) / 6 + fa * fa * fa * fa * fa *
		(1 + 28 * t * t + 24 * t * t * t * t) / 120;
	double Mer = floor(GKRight / 1000000);
	double GeoDezHeight = dl * rho / co + Mer * 3;
	if (useWGS)
	{
		return bessel2WGS(GeoDezRight,GeoDezHeight);
	}
	else
	{
		result.x = GeoDezHeight;
		result.y = GeoDezRight;
		return result;
	}
}
*/

/* Get Datum from 2D point */

static Datum
point2d_get_datum(POINT2D point2D)
{
	LWPOINT *p = lwpoint_make2d(4326, point2D.x, point2D.y);
	GSERIALIZED *result = geometry_serialize((LWGEOM *)p);

	return PointerGetDatum(result);
}

static Datum
gk(Datum inst)
{
	eqwgs = (awgs * awgs - bwgs * bwgs) / (awgs * awgs);
	eqbes = (abes * abes - bbes * bbes) / (abes * abes);
	POINT2D point2D = datum_get_point2d(inst);
	POINT2D result;
	double x = point2D.x;
	double y = point2D.y;
	double a = (x / 180) * Pi;
	double b = (y / 180) * Pi;
	double l1 = a;
	double b1 = b;

	a = awgs;

	double eq = eqwgs;
	double N = a / sqrt(1 - eq * sin(b1) * sin(b1));
	double Xq = (N + h1) * cos(b1) * cos(l1);
	double Yq = (N + h1) * cos(b1) * sin(l1);
	double Zq = ((1 - eq) * N + h1) * sin(b1);

	POINT3D p = HelmertTransformation(Xq, Yq, Zq);
	double X = p.x;
	double Y = p.y;
	double Z = p.z;

	p = BLRauenberg(X, Y, Z);
	double b2 = p.x;
	double l2 = p.y;
	result = BesselBLToGaussKrueger(b2, l2);
	return point2d_get_datum(result);
}

/* Transform geometry to Gauss Kruger Projection */

static GSERIALIZED *
geometry_transform_gk_internal(GSERIALIZED *gs)
{
	GSERIALIZED *result;
	int geometryType = gserialized_get_type(gs);
	if(geometryType == POINTTYPE)
	{
		POINT2D point2D	= gs_get_point2d(gs);
		Datum geom = gk(point2d_get_datum(point2D));
		point2D	= datum_get_point2d(geom);
		LWPOINT *lwpoint = lwpoint_make2d(4326, point2D.x, point2D.y);
		result = geometry_serialize((LWGEOM *)lwpoint);
	}
	else if(geometryType == LINETYPE)
	{
		LWPOINT *lwpoint = NULL;
		LWGEOM *lwgeom = lwgeom_from_gserialized(gs);
		LWLINE *line = lwgeom_as_lwline(lwgeom);
		if(line->points->npoints < 1)
			ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE),
					errmsg("Invalid arguments for LineString")));

		uint32_t numPoints = line->points->npoints;
		LWPOINT **points = palloc(sizeof(LWPOINT *) * numPoints);
		for (uint32_t i = 0; i < numPoints; i++)
		{
			lwpoint = lwline_get_lwpoint((LWLINE*)lwgeom, i);
			Datum point2d_datum = PointerGetDatum(gserialized_from_lwgeom(lwpoint_as_lwgeom(lwpoint), 0));
			Datum geom = gk(point2d_datum);
			POINT2D point2D	= datum_get_point2d(geom);
			points[i] = lwpoint_make2d(4326, point2D.x, point2D.y);
		}

		line = lwline_from_ptarray(4326, numPoints, points);
		result = geometry_serialize(lwline_as_lwgeom(line));

		lwline_free(line);lwpoint_free(lwpoint);lwgeom_free(lwgeom);
		for( uint32_t i = 0; i < numPoints; i++)
			lwpoint_free(points[i]);
		pfree(points);
	}
	else
		ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE),
				errmsg("Component geometry/geography must be of type Point(Z)M or LineString")));

	return result;
}

static TemporalInst *
tgeompointinst_transform_gk(TemporalInst *inst)
{
	Datum geom = gk(temporalinst_value(inst));
	TemporalInst *result = temporalinst_make(geom, inst->t,
		type_oid(T_GEOMETRY));
	pfree(DatumGetPointer(geom));
	return result;
}

static TemporalI *
tgeompointi_transform_gk_internal(TemporalI *ti)
{
	TemporalInst **instants = palloc(sizeof(TemporalInst *) * ti->count);
	for (int i = 0; i < ti->count; i++)
	{
		TemporalInst *inst = temporali_inst_n(ti, i);
		instants[i] = tgeompointinst_transform_gk(inst);
	}
	TemporalI *result = temporali_from_temporalinstarr(instants, ti->count);

	for (int i = 0; i < ti->count; i++)
		pfree(instants[i]);
	pfree(instants);

	return result;
}

static TemporalSeq *
tgeompointseq_transform_gk_internal(TemporalSeq *seq)
{
	TemporalInst **instants = palloc(sizeof(TemporalInst *) * seq->count);
	for (int i = 0; i < seq->count; i++)
	{
		TemporalInst *inst = temporalseq_inst_n(seq, i);
		instants[i] = tgeompointinst_transform_gk(inst);
	}
	TemporalSeq *result = temporalseq_from_temporalinstarr(instants,
		seq->count, seq->period.lower_inc, seq->period.upper_inc, true);

	for (int i = 0; i < seq->count; i++)
		pfree(instants[i]);
	pfree(instants);

	return result;
}

static TemporalS *
tgeompoints_transform_gk_internal(TemporalS *ts)
{
	TemporalSeq **sequences = palloc(sizeof(TemporalSeq *) * ts->count);
	for (int i = 0; i < ts->count; i++)
	{
		TemporalSeq *seq = temporals_seq_n(ts, i);
		TemporalInst **instants = palloc(sizeof(TemporalInst *) * seq->count);
		for (int j = 0; j < seq->count; j++)
		{
			TemporalInst *inst = temporalseq_inst_n(seq, j);
			instants[j] = tgeompointinst_transform_gk(inst);
		}
		sequences[i] = temporalseq_from_temporalinstarr(instants,
			seq->count, seq->period.lower_inc, seq->period.upper_inc, true);
		for (int j = 0; j < seq->count; j++)
			pfree(instants[j]);
		pfree(instants);
	}
	TemporalS *result = temporals_from_temporalseqarr(sequences,
		ts->count, false);

	for (int i = 0; i < ts->count; i++)
		pfree(sequences[i]);
	pfree(sequences);

	return result;
}

PG_FUNCTION_INFO_V1(geometry_transform_gk);

PGDLLEXPORT Datum
geometry_transform_gk(PG_FUNCTION_ARGS)
{
	GSERIALIZED *gs = PG_GETARG_GSERIALIZED_P(0);
	GSERIALIZED *result = NULL;
	result = geometry_transform_gk_internal(gs);
	PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(tgeompoint_transform_gk);

PGDLLEXPORT Datum
tgeompoint_transform_gk(PG_FUNCTION_ARGS)
{
	Temporal *temp = PG_GETARG_TEMPORAL(0);
	Temporal *result;
	if (temp->type == TEMPORALINST)
		result = (Temporal *)tgeompointinst_transform_gk((TemporalInst *)temp);
	else if (temp->type == TEMPORALI)
		result = (Temporal *)tgeompointi_transform_gk_internal((TemporalI *)temp);
	else if (temp->type == TEMPORALSEQ)
		result = (Temporal *)tgeompointseq_transform_gk_internal((TemporalSeq *)temp);
	else if (temp->type == TEMPORALS)
		result = (Temporal *)tgeompoints_transform_gk_internal((TemporalS *)temp);
	else
		ereport(ERROR, (errcode(ERRCODE_INTERNAL_ERROR), 
			errmsg("Operation not supported")));
	PG_FREE_IF_COPY(temp, 0);
	PG_RETURN_POINTER(result);
}

/*****************************************************************************/


