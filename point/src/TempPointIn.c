/*****************************************************************************
 *
 * TempPointIn.c
 *	  Input of temporal points in WKT, EWKT and MF-JSON format
 *
 * Portions Copyright (c) 2019, Esteban Zimanyi, Arthur Lesuisse,
 *		Universite Libre de Bruxelles
 * Portions Copyright (c) 1996-2019, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 *****************************************************************************/

#include "TemporalPoint.h"

#include <json-c/json.h>
#include <json-c/json_object_private.h>

/*****************************************************************************
 * Input in MFJSON format 
 *****************************************************************************/

/* Function taken from PostGIS file lwin_geojson.c */

static json_object*
findMemberByName(json_object* poObj, const char* pszName )
{
	json_object* poTmp;
	json_object_iter it;

	poTmp = poObj;

	if (pszName == NULL || poObj == NULL)
		return NULL;

	it.key = NULL;
	it.val = NULL;
	it.entry = NULL;

	if (json_object_get_object(poTmp) != NULL)
	{
		if (json_object_get_object(poTmp)->head == NULL)
		{
            ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE), 
                errmsg("Invalid MFJSON representation")));
			return NULL;
		}

		for (it.entry = json_object_get_object(poTmp)->head;
		        ( it.entry ?
		          ( it.key = (char*)it.entry->k,
		            it.val = (json_object*)it.entry->v, it.entry) : 0);
		        it.entry = it.entry->next)
		{
			if (strcasecmp((char *)it.key, pszName) == 0)
				return it.val;
		}
	}

	return NULL;
}

static bool
parse_mfjson_coord(json_object *poObj, bool *hasz, POINT3DZ *pt)
{
	if (json_type_array == json_object_get_type(poObj))
	{
		json_object *poObjCoord = NULL;
		const int nSize = json_object_array_length(poObj);
		if (nSize < 2)
		{
            ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE), 
                errmsg("Too few coordinates in MFJSON representation")));
			return false;
		}
		else if (nSize > 3)
		{
            ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE), 
                errmsg("Too many coordinates in MFJSON representation")));
			return false;
		}

		/* Read X coordinate */
		poObjCoord = json_object_array_get_idx(poObj, 0);
		pt->x = json_object_get_double(poObjCoord);

		/* Read Y coordinate */
		poObjCoord = json_object_array_get_idx(poObj, 1);
		pt->y = json_object_get_double(poObjCoord);

		if (nSize == 3) /* should this be >= 3 ? */
		{
			/* Read Z coordinate */
			poObjCoord = json_object_array_get_idx(poObj, 2);
			pt->z = json_object_get_double(poObjCoord);
			*hasz = true;
		}
	}
	else
	{
		/* If it's not an array, just don't handle it */
		return false;
	}

	return true;
}

/*****************************************************************************/
