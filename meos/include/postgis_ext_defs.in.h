#ifndef _LIBLWGEOM_H
#define _LIBLWGEOM_H

/******************************************************************/

/**
* Macros for manipulating the 'flags' byte. A uint8_t used as follows:
* VVSRGBMZ
* Version bit, followed by
* Validty, Solid, ReadOnly, Geodetic, HasBBox, HasM and HasZ flags.
*/
#define LWFLAG_Z        0x01
#define LWFLAG_M        0x02
#define LWFLAG_BBOX     0x04
#define LWFLAG_GEODETIC 0x08
#define LWFLAG_READONLY 0x10
#define LWFLAG_SOLID    0x20

#define FLAGS_GET_Z(flags)         ((flags) & LWFLAG_Z)
#define FLAGS_GET_M(flags)        (((flags) & LWFLAG_M)>>1)
#define FLAGS_GET_BBOX(flags)     (((flags) & LWFLAG_BBOX)>>2)
#define FLAGS_GET_GEODETIC(flags) (((flags) & LWFLAG_GEODETIC)>>3)
#define FLAGS_GET_READONLY(flags) (((flags) & LWFLAG_READONLY)>>4)
#define FLAGS_GET_SOLID(flags)    (((flags) & LWFLAG_SOLID)>>5)

#define FLAGS_SET_Z(flags, value) ((flags) = (value) ? ((flags) | LWFLAG_Z) : ((flags) & ~LWFLAG_Z))
#define FLAGS_SET_M(flags, value) ((flags) = (value) ? ((flags) | LWFLAG_M) : ((flags) & ~LWFLAG_M))
#define FLAGS_SET_BBOX(flags, value) ((flags) = (value) ? ((flags) | LWFLAG_BBOX) : ((flags) & ~LWFLAG_BBOX))
#define FLAGS_SET_GEODETIC(flags, value) ((flags) = (value) ? ((flags) | LWFLAG_GEODETIC) : ((flags) & ~LWFLAG_GEODETIC))
#define FLAGS_SET_READONLY(flags, value) ((flags) = (value) ? ((flags) | LWFLAG_READONLY) : ((flags) & ~LWFLAG_READONLY))
#define FLAGS_SET_SOLID(flags, value) ((flags) = (value) ? ((flags) | LWFLAG_SOLID) : ((flags) & ~LWFLAG_SOLID))

#define FLAGS_NDIMS(flags) (2 + FLAGS_GET_Z(flags) + FLAGS_GET_M(flags))
#define FLAGS_GET_ZM(flags) (FLAGS_GET_M(flags) + FLAGS_GET_Z(flags) * 2)
#define FLAGS_NDIMS_BOX(flags) (FLAGS_GET_GEODETIC(flags) ? 3 : FLAGS_NDIMS(flags))

/*
** Variants available for WKB and WKT output types
*/

#define WKB_ISO 0x01
#define WKB_SFSQL 0x02
#define WKB_EXTENDED 0x04
#define WKB_NDR 0x08
#define WKB_XDR 0x10
#define WKB_HEX 0x20
#define WKB_NO_NPOINTS 0x40 /* Internal use only */
#define WKB_NO_SRID 0x80 /* Internal use only */

#define WKT_ISO 0x01
#define WKT_SFSQL 0x02
#define WKT_EXTENDED 0x04

typedef uint16_t lwflags_t;

/******************************************************************/

typedef struct {
    double afac, bfac, cfac, dfac, efac, ffac, gfac, hfac, ifac, xoff, yoff, zoff;
} AFFINE;

/******************************************************************/

typedef struct
{
    double xmin, ymin, zmin;
    double xmax, ymax, zmax;
    int32_t srid;
}
BOX3D;

/******************************************************************
* GBOX structure.
* We include the flags (information about dimensionality),
* so we don't have to constantly pass them
* into functions that use the GBOX.
*/
typedef struct
{
    lwflags_t flags;
    double xmin;
    double xmax;
    double ymin;
    double ymax;
    double zmin;
    double zmax;
    double mmin;
    double mmax;
} GBOX;


/******************************************************************
* SPHEROID
*
*  Standard definition of an ellipsoid (what wkt calls a spheroid)
*    f = (a-b)/a
*    e_sq = (a*a - b*b)/(a*a)
*    b = a - fa
*/
typedef struct
{
    double  a;  /* semimajor axis */
    double  b;  /* semiminor axis b = (a - fa) */
    double  f;  /* flattening f = (a-b)/a */
    double  e;  /* eccentricity (first) */
    double  e_sq;   /* eccentricity squared (first) e_sq = (a*a-b*b)/(a*a) */
    double  radius;  /* spherical average radius = (2*a+b)/3 */
    char    name[20];  /* name of ellipse */
}
SPHEROID;

/******************************************************************
* POINT2D, POINT3D, POINT3DM, POINT4D
*/
typedef struct
{
    double x, y;
}
POINT2D;

typedef struct
{
    double x, y, z;
}
POINT3DZ;

typedef struct
{
    double x, y, z;
}
POINT3D;

typedef struct
{
    double x, y, m;
}
POINT3DM;

typedef struct
{
    double x, y, z, m;
}
POINT4D;

/******************************************************************
*  POINTARRAY
*  Point array abstracts a lot of the complexity of points and point lists.
*  It handles 2d/3d translation
*    (2d points converted to 3d will have z=0 or NaN)
*  DO NOT MIX 2D and 3D POINTS! EVERYTHING* is either one or the other
*/
typedef struct
{
    uint32_t npoints;   /* how many points we are currently storing */
    uint32_t maxpoints; /* how many points we have space for in serialized_pointlist */

    /* Use FLAGS_* macros to handle */
    lwflags_t flags;

    /* Array of POINT 2D, 3D or 4D, possibly misaligned. */
    uint8_t *serialized_pointlist;
}
POINTARRAY;

/******************************************************************
* GSERIALIZED
*/

typedef struct
{
    uint32_t size; /* For PgSQL use only, use VAR* macros to manipulate. */
    uint8_t srid[3]; /* 24 bits of SRID */
    uint8_t gflags; /* HasZ, HasM, HasBBox, IsGeodetic */
    uint8_t data[1]; /* See gserialized.txt */
} GSERIALIZED;

/******************************************************************
* LWGEOM (any geometry type)
*
* Abstract type, note that 'type', 'bbox' and 'srid' are available in
* all geometry variants.
*/
typedef struct
{
    GBOX *bbox;
    void *data;
    int32_t srid;
    lwflags_t flags;
    uint8_t type;
    char pad[1]; /* Padding to 24 bytes (unused) */
}
LWGEOM;

/* POINTYPE */
typedef struct
{
    GBOX *bbox;
    POINTARRAY *point;  /* hide 2d/3d (this will be an array of 1 point) */
    int32_t srid;
    lwflags_t flags;
    uint8_t type; /* POINTTYPE */
    char pad[1]; /* Padding to 24 bytes (unused) */
}
LWPOINT; /* "light-weight point" */

/* LINETYPE */
typedef struct
{
    GBOX *bbox;
    POINTARRAY *points; /* array of POINT3D */
    int32_t srid;
    lwflags_t flags;
    uint8_t type; /* LINETYPE */
    char pad[1]; /* Padding to 24 bytes (unused) */
}
LWLINE; /* "light-weight line" */

/* TRIANGLE */
typedef struct
{
    GBOX *bbox;
    POINTARRAY *points;
    int32_t srid;
    lwflags_t flags;
    uint8_t type;
    char pad[1]; /* Padding to 24 bytes (unused) */
}
LWTRIANGLE;

/* CIRCSTRINGTYPE */
typedef struct
{
    GBOX *bbox;
    POINTARRAY *points; /* array of POINT(3D/3DM) */
    int32_t srid;
    lwflags_t flags;
    uint8_t type; /* CIRCSTRINGTYPE */
    char pad[1]; /* Padding to 24 bytes (unused) */
}
LWCIRCSTRING; /* "light-weight circularstring" */

/* POLYGONTYPE */
typedef struct
{
    GBOX *bbox;
    POINTARRAY **rings; /* list of rings (list of points) */
    int32_t srid;
    lwflags_t flags;
    uint8_t type; /* POLYGONTYPE */
    char pad[1]; /* Padding to 24 bytes (unused) */
    uint32_t nrings;   /* how many rings we are currently storing */
    uint32_t maxrings; /* how many rings we have space for in **rings */
}
LWPOLY; /* "light-weight polygon" */

/* MULTIPOINTTYPE */
typedef struct
{
    GBOX *bbox;
    LWPOINT **geoms;
    int32_t srid;
    lwflags_t flags;
    uint8_t type; /* MULTYPOINTTYPE */
    char pad[1]; /* Padding to 24 bytes (unused) */
    uint32_t ngeoms;   /* how many geometries we are currently storing */
    uint32_t maxgeoms; /* how many geometries we have space for in **geoms */
}
LWMPOINT;

/* MULTILINETYPE */
typedef struct
{
    GBOX *bbox;
    LWLINE **geoms;
    int32_t srid;
    lwflags_t flags;
    uint8_t type; /* MULTILINETYPE */
    char pad[1]; /* Padding to 24 bytes (unused) */
    uint32_t ngeoms;   /* how many geometries we are currently storing */
    uint32_t maxgeoms; /* how many geometries we have space for in **geoms */
}
LWMLINE;

/* MULTIPOLYGONTYPE */
typedef struct
{
    GBOX *bbox;
    LWPOLY **geoms;
    int32_t srid;
    lwflags_t flags;
    uint8_t type; /* MULTIPOLYGONTYPE */
    char pad[1]; /* Padding to 24 bytes (unused) */
    uint32_t ngeoms;   /* how many geometries we are currently storing */
    uint32_t maxgeoms; /* how many geometries we have space for in **geoms */
}
LWMPOLY;

/* COLLECTIONTYPE */
typedef struct
{
    GBOX *bbox;
    LWGEOM **geoms;
    int32_t srid;
    lwflags_t flags;
    uint8_t type; /* COLLECTIONTYPE */
    char pad[1]; /* Padding to 24 bytes (unused) */
    uint32_t ngeoms;   /* how many geometries we are currently storing */
    uint32_t maxgeoms; /* how many geometries we have space for in **geoms */
}
LWCOLLECTION;

/* COMPOUNDTYPE */
typedef struct
{
    GBOX *bbox;
    LWGEOM **geoms;
    int32_t srid;
    lwflags_t flags;
    uint8_t type; /* COLLECTIONTYPE */
    char pad[1]; /* Padding to 24 bytes (unused) */
    uint32_t ngeoms;   /* how many geometries we are currently storing */
    uint32_t maxgeoms; /* how many geometries we have space for in **geoms */
}
LWCOMPOUND; /* "light-weight compound line" */

/* CURVEPOLYTYPE */
typedef struct
{
    GBOX *bbox;
    LWGEOM **rings;
    int32_t srid;
    lwflags_t flags;
    uint8_t type; /* CURVEPOLYTYPE */
    char pad[1]; /* Padding to 24 bytes (unused) */
    uint32_t nrings;    /* how many rings we are currently storing */
    uint32_t maxrings;  /* how many rings we have space for in **rings */
}
LWCURVEPOLY; /* "light-weight polygon" */

/* MULTICURVE */
typedef struct
{
    GBOX *bbox;
    LWGEOM **geoms;
    int32_t srid;
    lwflags_t flags;
    uint8_t type; /* MULTICURVE */
    char pad[1]; /* Padding to 24 bytes (unused) */
    uint32_t ngeoms;   /* how many geometries we are currently storing */
    uint32_t maxgeoms; /* how many geometries we have space for in **geoms */
}
LWMCURVE;

/* MULTISURFACETYPE */
typedef struct
{
    GBOX *bbox;
    LWGEOM **geoms;
    int32_t srid;
    lwflags_t flags;
    uint8_t type; /* MULTISURFACETYPE */
    char pad[1]; /* Padding to 24 bytes (unused) */
    uint32_t ngeoms;   /* how many geometries we are currently storing */
    uint32_t maxgeoms; /* how many geometries we have space for in **geoms */
}
LWMSURFACE;

/* POLYHEDRALSURFACETYPE */
typedef struct
{
    GBOX *bbox;
    LWPOLY **geoms;
    int32_t srid;
    lwflags_t flags;
    uint8_t type; /* POLYHEDRALSURFACETYPE */
    char pad[1]; /* Padding to 24 bytes (unused) */
    uint32_t ngeoms;   /* how many geometries we are currently storing */
    uint32_t maxgeoms; /* how many geometries we have space for in **geoms */
}
LWPSURFACE;

/* TINTYPE */
typedef struct
{
    GBOX *bbox;
    LWTRIANGLE **geoms;
    int32_t srid;
    lwflags_t flags;
    uint8_t type; /* TINTYPE */
    char pad[1]; /* Padding to 24 bytes (unused) */
    uint32_t ngeoms;   /* how many geometries we are currently storing */
    uint32_t maxgeoms; /* how many geometries we have space for in **geoms */
}
LWTIN;

extern LWPOINT *lwpoint_make(int32_t srid, int hasz, int hasm, const POINT4D *p);

extern LWGEOM *lwgeom_from_gserialized(const GSERIALIZED *g);
extern GSERIALIZED *gserialized_from_lwgeom(LWGEOM *geom, size_t *size);

extern LWPOINT *lwgeom_as_lwpoint(const LWGEOM *lwgeom);

extern int32_t lwgeom_get_srid(const LWGEOM *geom);

extern double lwpoint_get_x(const LWPOINT *point);
extern double lwpoint_get_y(const LWPOINT *point);
extern double lwpoint_get_z(const LWPOINT *point);
extern double lwpoint_get_m(const LWPOINT *point);

extern int lwgeom_has_z(const LWGEOM *geom);
extern int lwgeom_has_m(const LWGEOM *geom);

#endif              /* _LIBLWGEOM_H */
