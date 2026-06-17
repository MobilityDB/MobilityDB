/*****************************************************************************
 *
 * This MobilityDB code is provided under The PostgreSQL License.
 * Copyright (c) 2016-2025, Université libre de Bruxelles and MobilityDB
 * contributors
 *
 *****************************************************************************/

/**
 * @brief Internal declarations for the TPCBox bounding-box type.
 */

#ifndef __TPCBOX_H__
#define __TPCBOX_H__

/* PostgreSQL */
#include <postgres.h>
/* MEOS */
#include <meos.h>
#include <meos_pointcloud.h>

/*****************************************************************************
 * fmgr macros
 *****************************************************************************/

#define DatumGetTpcboxP(X)         ((TPCBox *) DatumGetPointer(X))
#define TpcboxPGetDatum(X)         PointerGetDatum(X)
#define PG_GETARG_TPCBOX_P(X)      DatumGetTpcboxP(PG_GETARG_DATUM(X))
#define PG_RETURN_TPCBOX_P(X)      return TpcboxPGetDatum(X)

/*****************************************************************************/

#endif /* __TPCBOX_H__ */
