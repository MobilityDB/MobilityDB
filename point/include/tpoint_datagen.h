/*****************************************************************************
 *
 * tpoint_datagen.h
 *	  Data generator for MobilityDB.
 *
 * Portions Copyright (c) 2020, Esteban Zimanyi, Mahmoud Sakr,
 * 		Universite Libre de Bruxelles
 * Portions Copyright (c) 1996-2020, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 *****************************************************************************/

#ifndef __TPOINT_DATAGEN_H__
#define __TPOINT_DATAGEN_H__

#include <postgres.h>
#include <fmgr.h>

/*****************************************************************************/

extern Datum create_trip(PG_FUNCTION_ARGS);

/*****************************************************************************/

#endif
