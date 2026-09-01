/*****************************************************************************
 *
 * This MobilityDB code is provided under The PostgreSQL License.
 * Copyright (c) 2016-2026, Université libre de Bruxelles and MobilityDB
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
 * @brief Basic functions for static pose chain objects.
 */

/* C */
#include <math.h>
#include <float.h>
#include <limits.h>
/* Postgres */
#include <postgres.h>
#include <pgtypes.h>
#include <varatt.h>
#include <common/hashfn.h>
#include <utils/float.h>
/* MEOS */
#include <meos.h>
#include <meos_pose.h>
#include <meos_internal.h>
#include <meos_internal_geo.h>
#include "temporal/set.h"
#include "temporal/type_inout.h"
#include "temporal/type_parser.h"
#include "temporal/type_util.h"
#include "geo/geo_funcs.h"
#include "geo/meos_transform.h"
#include "geo/tgeo_spatialfuncs.h"
#include "geo/tspatial.h"
#include "geo/tspatial_parser.h"
#include "pose/pose.h"
#include "pose/posechain.h"

/** Buffer size for the output of a single link of a pose chain */
#define MAXPOSELEN    256

/*****************************************************************************
 * Validity functions
 *****************************************************************************/

/**
 * @brief Ensure that a pose chain has the same SRID and the same dimension as
 * another one
 */
bool
ensure_valid_posechain_posechain(const PoseChain *pc1, const PoseChain *pc2)
{
  VALIDATE_NOT_NULL(pc1, false); VALIDATE_NOT_NULL(pc2, false);
  if (! ensure_same_srid(posechain_srid(pc1), posechain_srid(pc2)) ||
      MEOS_FLAGS_GET_Z(pc1->flags) != MEOS_FLAGS_GET_Z(pc2->flags))
    return false;
  return true;
}

/**
 * @brief Return true if a set and a pose chain are valid for set operations
 * @param[in] s Set
 * @param[in] pc Value
 */
bool
ensure_valid_posechainset_posechain(const Set *s, const PoseChain *pc)
{
  /* Ensure the validity of the arguments */
  VALIDATE_POSECHAINSET(s, false); VALIDATE_NOT_NULL(pc, false);
  if (! ensure_same_srid(spatialset_srid(s), posechain_srid(pc)) ||
      MEOS_FLAGS_GET_Z(pc->flags) != MEOS_FLAGS_GET_Z(s->flags))
    return false;
  return true;
}

/*****************************************************************************
 * Composition of the links of a chain
 *
 * Clause 4.2.6 of OGC GeoPose 1.0 defines a frame transform as a pair of
 * frames in which "the outer frame is the domain; the inner frame is the
 * range", and clause 4.2.8 forbids an inner frame from being topocentric.
 * A chain therefore composes as a fold from the outside in: every link
 * carries the rigid transform from its parent's frame to its own, and the
 * world pose of a link is the composition of every transform above it.
 *****************************************************************************/

/*****************************************************************************
 * Constructor functions
 *****************************************************************************/

static Pose *posechain_link_pose(const PoseChain *pc, int n, bool outer);

/**
 * @brief Return an uninitialized pose chain of a given dimension and length
 */
static PoseChain *
posechain_alloc(int count, bool hasz, bool geodetic, int32_t srid)
{
  size_t memsize = DOUBLE_PAD(sizeof(PoseChain)) +
    (size_t) count * (hasz ? 7 : 3) * sizeof(double);
  PoseChain *result = palloc0(memsize);
  SET_VARSIZE(result, memsize);
  MEOS_FLAGS_SET_X(result->flags, true);
  MEOS_FLAGS_SET_Z(result->flags, hasz);
  MEOS_FLAGS_SET_GEODETIC(result->flags, geodetic);
  posechain_set_srid_int(result, srid);
  result->count = count;
  return result;
}

/**
 * @brief Ensure that a pose is admissible as the link at a given position of
 * a chain whose outer link is given
 * @details The outer link fixes the frame of the whole chain. Clause 4.2.8 of
 * OGC GeoPose 1.0 states that an inner frame "Cannot be a topocentric frame",
 * so no link but the first carries a frame of its own: an inner link is a
 * rigid transform read in the axes of its parent, and it must be neither
 * geodetic nor bound to an SRID that disagrees with the chain's.
 */
static bool
ensure_valid_chain_link(const Pose *outer, const Pose *pose, int n)
{
  if (MEOS_FLAGS_GET_Z(pose->flags) != MEOS_FLAGS_GET_Z(outer->flags))
  {
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
      "All the links of a pose chain must be of the same dimension");
    return false;
  }
  if (n == 0)
    return true;
  if (MEOS_FLAGS_GET_GEODETIC(pose->flags))
  {
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
      "Only the first link of a pose chain may be geodetic: link %d is "
      "expressed in the frame the link before it defines", n + 1);
    return false;
  }
  int32_t srid = pose_srid(pose);
  if (srid != SRID_UNKNOWN && srid != pose_srid(outer))
  {
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
      "Only the first link of a pose chain may carry an SRID: link %d has "
      "SRID %d", n + 1, srid);
    return false;
  }
  return true;
}

/**
 * @ingroup meos_posechain_base_constructor
 * @brief Construct a pose chain from an array of poses
 * @param[in] poses Poses, ordered from the outermost frame inwards
 * @param[in] count Number of poses, at least one
 * @csqlfn #Posechain_constructor()
 */
PoseChain *
posechain_make(const Pose **poses, int count)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(poses, NULL);
  if (! ensure_positive(count))
    return NULL;
  for (int i = 0; i < count; i++)
    if (! ensure_not_null((void *) poses[i]) ||
        ! ensure_valid_chain_link(poses[0], poses[i], i))
      return NULL;

  bool hasz = MEOS_FLAGS_GET_Z(poses[0]->flags);
  PoseChain *result = posechain_alloc(count, hasz,
    MEOS_FLAGS_GET_GEODETIC(poses[0]->flags), pose_srid(poses[0]));
  int nvalues = hasz ? 7 : 3;
  for (int i = 0; i < count; i++)
    memcpy(POSECHAIN_LINK_PTR(result, i), poses[i]->data,
      (size_t) nvalues * sizeof(double));
  return result;
}

/**
 * @ingroup meos_posechain_base_constructor
 * @brief Copy a pose chain
 * @param[in] pc Pose chain
 */
PoseChain *
posechain_copy(const PoseChain *pc)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(pc, NULL);
  PoseChain *result = palloc(VARSIZE(pc));
  memcpy(result, pc, VARSIZE(pc));
  return result;
}

/**
 * @ingroup meos_posechain_base_constructor
 * @brief Return a pose chain with a pose appended as its innermost link
 * @param[in] pc Pose chain
 * @param[in] pose Pose, read in the frame the last link of @p pc defines
 * @csqlfn #Posechain_append_pose()
 */
PoseChain *
posechain_append(const PoseChain *pc, const Pose *pose)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(pc, NULL); VALIDATE_NOT_NULL(pose, NULL);
  if (MEOS_FLAGS_GET_Z(pose->flags) != MEOS_FLAGS_GET_Z(pc->flags))
  {
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
      "All the links of a pose chain must be of the same dimension");
    return NULL;
  }
  if (MEOS_FLAGS_GET_GEODETIC(pose->flags))
  {
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
      "Only the first link of a pose chain may be geodetic");
    return NULL;
  }
  int32_t srid = pose_srid(pose);
  if (srid != SRID_UNKNOWN && srid != posechain_srid(pc))
  {
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
      "Only the first link of a pose chain may carry an SRID: the pose has "
      "SRID %d", srid);
    return NULL;
  }

  int nvalues = POSECHAIN_LINK_SIZE(pc);
  PoseChain *result = posechain_alloc(pc->count + 1,
    MEOS_FLAGS_GET_Z(pc->flags), MEOS_FLAGS_GET_GEODETIC(pc->flags),
    posechain_srid(pc));
  memcpy(result->data, pc->data,
    (size_t) pc->count * nvalues * sizeof(double));
  memcpy(POSECHAIN_LINK_PTR(result, pc->count), pose->data,
    (size_t) nvalues * sizeof(double));
  return result;
}

/*****************************************************************************
 * Input/output functions
 *****************************************************************************/

/**
 * @brief Parse a pose chain value from the buffer
 */
PoseChain *
posechain_parse(const char **str, bool end)
{
  assert(str);
  const char *type_str = meostype_name(T_POSECHAIN);

  /* Determine whether the chain has an SRID */
  int32_t srid;
  srid_parse(str, &srid);

  if (pg_strncasecmp(*str, "POSECHAIN", 9) != 0)
  {
    meos_error(ERROR, MEOS_ERR_TEXT_INPUT,
      "Could not parse %s value: Missing prefix 'PoseChain'", type_str);
    return NULL;
  }
  *str += 9;
  p_whitespace(str);

  /* Parse opening parenthesis */
  if (! ensure_oparen(str, type_str))
    return NULL;

  /* A chain is at least one link, so say that rather than let the pose parser
   * report a missing delimiter against the closing parenthesis */
  p_whitespace(str);
  if (**str == ')')
  {
    meos_error(ERROR, MEOS_ERR_TEXT_INPUT,
      "Could not parse %s value: A pose chain requires at least one link",
      type_str);
    return NULL;
  }

  /* Parse the links. The outer link is a pose in the frame of the chain and
   * states that frame, exactly as a pose states its own; every later link is
   * a rigid transform in the frame the link before it defines, and names no
   * frame */
  int count = 0, maxcount = 4;
  Pose **poses = palloc(sizeof(Pose *) * maxcount);
  for (;;)
  {
    p_whitespace(str);
    Pose *pose = pose_parse(str, false);
    if (! pose)
    {
      pfree_array((void **) poses, count);
      return NULL;
    }
    if (count == maxcount)
    {
      maxcount *= 2;
      poses = repalloc(poses, sizeof(Pose *) * maxcount);
    }
    poses[count++] = pose;
    p_whitespace(str);
    if (! p_comma(str))
      break;
  }

  /* An SRID written before the chain applies to its outer frame */
  if (srid != SRID_UNKNOWN && pose_srid(poses[0]) == SRID_UNKNOWN)
    pose_set_srid_int(poses[0], srid);

  PoseChain *result = posechain_make((const Pose **) poses, count);
  pfree_array((void **) poses, count);
  if (! result)
    return NULL;

  /* Parse closing parenthesis */
  p_whitespace(str);
  if (! ensure_cparen(str, type_str) ||
        (end && ! ensure_end_input(str, type_str)))
  {
    pfree(result);
    return NULL;
  }
  return result;
}

/**
 * @ingroup meos_posechain_base_inout
 * @brief Return a pose chain from its string representation
 * @param[in] str String
 * @csqlfn #Posechain_in()
 */
PoseChain *
posechain_in(const char *str)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(str, NULL);
  return posechain_parse(&str, true);
}

/**
 * @brief Return the string representation of one link of a pose chain
 * @param[in] pc Pose chain
 * @param[in] n Number of the link
 * @param[in] maxdd Maximum number of decimal digits
 * @details The link is written as a pose in the frame of the link before it,
 * so it carries neither an SRID nor the geodetic marker of the chain
 */
static char *
posechain_link_out(const PoseChain *pc, int n, int maxdd)
{
  Pose *pose = posechain_link_pose(pc, n, n == 0);
  char *result = pose_out(pose, maxdd);
  pfree(pose);
  return result;
}

/**
 * @ingroup meos_posechain_base_inout
 * @brief Return the string representation of a pose chain
 * @param[in] pc Pose chain
 * @param[in] maxdd Maximum number of decimal digits
 * @csqlfn #Posechain_out()
 */
char *
posechain_out(const PoseChain *pc, int maxdd)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(pc, NULL);
  if (! ensure_not_negative(maxdd))
    return NULL;

  char **links = palloc(sizeof(char *) * pc->count);
  size_t len = 0;
  for (int i = 0; i < pc->count; i++)
  {
    links[i] = posechain_link_out(pc, i, maxdd);
    len += strlen(links[i]) + 2; /* ", " between the links */
  }
  const char *chaintype = "POSECHAIN";
  len += strlen(chaintype) + 3; /* Type() and the final '\0' */
  char *result = palloc(len);
  size_t pos = (size_t) snprintf(result, len, "%s(", chaintype);
  for (int i = 0; i < pc->count; i++)
  {
    if (i > 0)
      pos += (size_t) snprintf(result + pos, len - pos, ", ");
    pos += (size_t) snprintf(result + pos, len - pos, "%s", links[i]);
  }
  snprintf(result + pos, len - pos, ")");
  pfree_array((void **) links, pc->count);
  return result;
}

/**
 * @brief Output a pose chain in the Well-Known Text (WKT) representation
 */
char *
posechain_wkt_out(const PoseChain *pc, bool extended, int maxdd)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(pc, NULL);
  if (! ensure_not_negative(maxdd))
    return NULL;

  char **links = palloc(sizeof(char *) * pc->count);
  size_t len = 0;
  for (int i = 0; i < pc->count; i++)
  {
    Pose *pose = posechain_link_pose(pc, i, i == 0);
    links[i] = pose_wkt_out(pose, false, maxdd);
    pfree(pose);
    len += strlen(links[i]) + 1; /* One ',' */
  }
  const char *chaintype = "PoseChain";
  char srid_str[32] = "";
  if (extended)
  {
    int32_t srid = posechain_srid(pc);
    if (srid != SRID_UNKNOWN)
      snprintf(srid_str, sizeof(srid_str), "SRID=%d;", srid);
  }
  len += strlen(chaintype) + strlen(srid_str) + 3;
  char *result = palloc(len);
  size_t pos = (size_t) snprintf(result, len, "%s%s(", srid_str, chaintype);
  for (int i = 0; i < pc->count; i++)
  {
    if (i > 0)
      pos += (size_t) snprintf(result + pos, len - pos, ",");
    pos += (size_t) snprintf(result + pos, len - pos, "%s", links[i]);
  }
  snprintf(result + pos, len - pos, ")");
  pfree_array((void **) links, pc->count);
  return result;
}

/**
 * @ingroup meos_posechain_base_inout
 * @brief Return the Well-Known Text (WKT) representation of a pose chain
 * @param[in] pc Pose chain
 * @param[in] maxdd Maximum number of decimal digits
 * @csqlfn #Posechain_as_text()
 */
char *
posechain_as_text(const PoseChain *pc, int maxdd)
{
  return posechain_wkt_out(pc, false, maxdd);
}

/**
 * @ingroup meos_posechain_base_inout
 * @brief Return the Extended Well-Known Text (EWKT) representation of a pose
 * chain
 * @param[in] pc Pose chain
 * @param[in] maxdd Maximum number of decimal digits
 * @csqlfn #Posechain_as_ewkt()
 */
char *
posechain_as_ewkt(const PoseChain *pc, int maxdd)
{
  return posechain_wkt_out(pc, true, maxdd);
}

/*****************************************************************************/

/**
 * @ingroup meos_posechain_base_inout
 * @brief Return a pose chain from its Well-Known Binary (WKB) representation
 * @param[in] wkb WKB string
 * @param[in] size Size of the string
 * @csqlfn #Posechain_recv(), #Posechain_from_wkb()
 */
PoseChain *
posechain_from_wkb(const uint8_t *wkb, size_t size)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(wkb, NULL);
  return (PoseChain *) type_from_wkb(wkb, size, T_POSECHAIN);
}

/**
 * @ingroup meos_posechain_base_inout
 * @brief Return a pose chain from its ASCII hex-encoded Well-Known Binary
 * (WKB) representation
 * @param[in] hexwkb HexWKB string
 * @csqlfn #Posechain_from_hexwkb()
 */
PoseChain *
posechain_from_hexwkb(const char *hexwkb)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(hexwkb, NULL);
  return (PoseChain *) type_from_hexwkb(hexwkb, strlen(hexwkb), T_POSECHAIN);
}

/**
 * @ingroup meos_posechain_base_inout
 * @brief Return the Well-Known Binary (WKB) representation of a pose chain
 * @param[in] pc Pose chain
 * @param[in] variant Output variant
 * @param[out] size_out Size of the result
 * @csqlfn #Posechain_send(), #Posechain_as_wkb()
 */
uint8_t *
posechain_as_wkb(const PoseChain *pc, uint8_t variant, size_t *size_out)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(pc, NULL); VALIDATE_NOT_NULL(size_out, NULL);
  return datum_as_wkb(PointerGetDatum(pc), T_POSECHAIN, variant, size_out);
}

/**
 * @ingroup meos_posechain_base_inout
 * @brief Return the ASCII hex-encoded Well-Known Binary (WKB) representation
 * of a pose chain
 * @param[in] pc Pose chain
 * @param[in] variant Output variant
 * @param[out] size_out Size of the result
 * @csqlfn #Posechain_as_hexwkb()
 */
char *
posechain_as_hexwkb(const PoseChain *pc, uint8_t variant, size_t *size_out)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(pc, NULL); VALIDATE_NOT_NULL(size_out, NULL);
  return (char *) datum_as_wkb(PointerGetDatum(pc), T_POSECHAIN,
    variant | (uint8_t) WKB_HEX, size_out);
}

/*****************************************************************************
 * Conversion functions
 *****************************************************************************/

/**
 * @brief Return the pose that a link of a chain holds, in the frame of the
 * link before it
 */
static Pose *
posechain_link_pose(const PoseChain *pc, int n, bool outer)
{
  const double *v = POSECHAIN_LINK_PTR(pc, n);
  bool geodetic = outer && MEOS_FLAGS_GET_GEODETIC(pc->flags);
  int32_t srid = outer ? posechain_srid(pc) : SRID_UNKNOWN;
  return MEOS_FLAGS_GET_Z(pc->flags) ?
    pose_make_3d(v[0], v[1], v[2], v[3], v[4], v[5], v[6], geodetic, srid) :
    pose_make_2d(v[0], v[1], v[2], geodetic, srid);
}

/**
 * @ingroup meos_posechain_base_conversion
 * @brief Convert a pose into a pose chain of a single link
 * @param[in] pose Pose
 * @csqlfn #Pose_to_posechain()
 */
PoseChain *
pose_to_posechain(const Pose *pose)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(pose, NULL);
  return posechain_make(&pose, 1);
}

/**
 * @ingroup meos_posechain_base_conversion
 * @brief Return the pose of the frame the first @p n links of a pose chain
 * define, in the outer frame of the chain
 * @param[in] pc Pose chain
 * @param[in] n Number of links to compose, from 1 to the length of the chain
 * @details Composing the whole chain is what gives a chain the surface of a
 * pose: distance, spatial relationships, bounding boxes and indexing all
 * read the composed value. Composing a prefix answers where an intermediate
 * joint is, which is a place in its own right
 * @csqlfn #Posechain_prefix_pose()
 */
Pose *
posechain_prefix_pose(const PoseChain *pc, int n)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(pc, NULL);
  if (! ensure_positive(n))
    return NULL;
  if (n > pc->count)
  {
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
      "The pose chain has %d links, cannot compose %d of them", pc->count, n);
    return NULL;
  }

  bool hasz = MEOS_FLAGS_GET_Z(pc->flags);
  bool geodetic = MEOS_FLAGS_GET_GEODETIC(pc->flags);
  int nvalues = hasz ? 7 : 3;
  double acc[7], next[7];
  memcpy(acc, POSECHAIN_LINK_PTR(pc, 0), (size_t) nvalues * sizeof(double));
  for (int i = 1; i < n; i++)
  {
    pose_compose_values(acc, POSECHAIN_LINK_PTR(pc, i), hasz, geodetic,
      next);
    memcpy(acc, next, (size_t) nvalues * sizeof(double));
  }
  int32_t srid = posechain_srid(pc);
  return hasz ?
    pose_make_3d(acc[0], acc[1], acc[2], acc[3], acc[4], acc[5], acc[6],
      geodetic, srid) :
    pose_make_2d(acc[0], acc[1], acc[2], geodetic, srid);
}

/**
 * @ingroup meos_posechain_base_conversion
 * @brief Convert a pose chain into the pose of its innermost frame, read in
 * the outer frame of the chain
 * @param[in] pc Pose chain
 * @csqlfn #Posechain_to_pose()
 */
Pose *
posechain_to_pose(const PoseChain *pc)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(pc, NULL);
  return posechain_prefix_pose(pc, pc->count);
}

/**
 * @brief Return the pose of the innermost frame of a pose chain
 */
Datum
datum_posechain_pose(Datum pc)
{
  return PointerGetDatum(posechain_to_pose(DatumGetPoseChainP(pc)));
}

/**
 * @ingroup meos_posechain_base_conversion
 * @brief Convert a pose chain into the geometry point of its innermost frame
 * @param[in] pc Pose chain
 * @csqlfn #Posechain_to_point()
 */
GSERIALIZED *
posechain_to_point(const PoseChain *pc)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(pc, NULL);
  Pose *pose = posechain_to_pose(pc);
  if (! pose)
    return NULL;
  GSERIALIZED *result = pose_to_point(pose);
  pfree(pose);
  return result;
}

/*****************************************************************************
 * Accessor functions
 *****************************************************************************/

/**
 * @ingroup meos_posechain_base_accessor
 * @brief Return the number of links of a pose chain
 * @param[in] pc Pose chain
 * @return On error return @p INT_MAX
 * @csqlfn #Posechain_num_poses()
 */
int
posechain_num_poses(const PoseChain *pc)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(pc, INT_MAX);
  return pc->count;
}

/**
 * @ingroup meos_posechain_base_accessor
 * @brief Return a copy of the outermost link of a pose chain
 * @param[in] pc Pose chain
 * @csqlfn #Posechain_start_pose()
 */
Pose *
posechain_start_pose(const PoseChain *pc)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(pc, NULL);
  return posechain_link_pose(pc, 0, true);
}

/**
 * @ingroup meos_posechain_base_accessor
 * @brief Return a copy of the innermost link of a pose chain
 * @param[in] pc Pose chain
 * @csqlfn #Posechain_end_pose()
 */
Pose *
posechain_end_pose(const PoseChain *pc)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(pc, NULL);
  return posechain_link_pose(pc, pc->count - 1, pc->count == 1);
}

/**
 * @ingroup meos_posechain_base_accessor
 * @brief Return a copy of the n-th link of a pose chain
 * @param[in] pc Pose chain
 * @param[in] n Number of the link, from 1 to the length of the chain
 * @details The link is returned as it is stored, that is, in the frame the
 * link before it defines. The pose of that frame in the outer frame of the
 * chain is what #posechain_prefix_pose() returns
 * @csqlfn #Posechain_pose_n()
 */
Pose *
posechain_pose_n(const PoseChain *pc, int n)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(pc, NULL);
  if (n < 1 || n > pc->count)
    return NULL;
  return posechain_link_pose(pc, n - 1, n == 1);
}

/**
 * @ingroup meos_posechain_base_accessor
 * @brief Return the array of links of a pose chain
 * @param[in] pc Pose chain
 * @param[out] count Number of elements of the output array
 * @csqlfn #Posechain_poses()
 */
Pose **
posechain_poses(const PoseChain *pc, int *count)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(pc, NULL); VALIDATE_NOT_NULL(count, NULL);
  Pose **result = palloc(sizeof(Pose *) * pc->count);
  for (int i = 0; i < pc->count; i++)
    result[i] = posechain_link_pose(pc, i, i == 0);
  *count = pc->count;
  return result;
}

/*****************************************************************************
 * Transformation functions
 *****************************************************************************/

/**
 * @ingroup meos_posechain_base_transf
 * @brief Return a pose chain with the values of its links rounded to a number
 * of decimal digits
 * @param[in] pc Pose chain
 * @param[in] maxdd Maximum number of decimal digits
 * @csqlfn #Posechain_round()
 */
PoseChain *
posechain_round(const PoseChain *pc, int maxdd)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(pc, NULL);
  if (! ensure_not_negative(maxdd))
    return NULL;

  PoseChain *result = posechain_copy(pc);
  int nvalues = POSECHAIN_LINK_SIZE(pc);
  for (int i = 0; i < pc->count; i++)
  {
    Pose *pose = posechain_link_pose(pc, i, false);
    Pose *rounded = pose_round(pose, maxdd);
    memcpy(POSECHAIN_LINK_PTR(result, i), rounded->data,
      (size_t) nvalues * sizeof(double));
    pfree(pose); pfree(rounded);
  }
  return result;
}

/**
 * @brief Return a pose chain with the values of its links rounded to a number
 * of decimal digits
 */
Datum
datum_posechain_round(Datum pc, Datum size)
{
  return PointerGetDatum(posechain_round(DatumGetPoseChainP(pc),
    DatumGetInt32(size)));
}

/*****************************************************************************
 * Interpolation functions
 *
 * A chain interpolates link by link, each link as a pose does: linearly in
 * position and along the shortest arc in rotation. Two chains are values of
 * one temporal chain only when they hold the same number of links, since a
 * chain that gains a joint is a different structure rather than a later value
 * of the same one, and that constant count is what makes the link-wise
 * interpolation definable at all.
 *****************************************************************************/

/**
 * @brief Ensure that two pose chains hold the same number of links
 * @param[in] pc1,pc2 Pose chains
 */
bool
ensure_same_count_posechain(const PoseChain *pc1, const PoseChain *pc2)
{
  if (pc1->count != pc2->count)
  {
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
      "Operation on pose chains of %d and %d links", pc1->count, pc2->count);
    return false;
  }
  return true;
}

/**
 * @brief Return the pose chain interpolated from two pose chains and a ratio
 * @param[in] start,end Pose chains
 * @param[in] ratio Value in [0,1] representing the duration of the timestamps
 * associated to `start` and the result divided by the duration of the
 * timestamps associated to `start` and `end`
 * @return On error return @p NULL
 */
PoseChain *
posechainsegm_interpolate(const PoseChain *start, const PoseChain *end,
  double ratio)
{
  if (! ensure_valid_posechain_posechain(start, end) ||
      ! ensure_same_count_posechain(start, end))
    return NULL;

  Pose **poses = palloc(sizeof(Pose *) * start->count);
  for (int i = 0; i < start->count; i++)
  {
    Pose *pose1 = posechain_link_pose(start, i, i == 0);
    Pose *pose2 = posechain_link_pose(end, i, i == 0);
    poses[i] = posesegm_interpolate(pose1, pose2, ratio);
    pfree(pose1); pfree(pose2);
    if (poses[i] == NULL)
    {
      pfree_array((void **) poses, i);
      return NULL;
    }
  }
  PoseChain *result = posechain_make((const Pose **) poses, start->count);
  pfree_array((void **) poses, start->count);
  return result;
}

/**
 * @brief Return the ratio at which a pose chain sits on the segment defined
 * by two pose chains
 * @param[in] start,end Pose chains defining the segment
 * @param[in] value Pose chain to locate
 * @details Every link locates at the ratio the whole chain moves through, so
 * the value sits on the segment only where all of them agree on one ratio.
 * @note The function returns -1.0 when the value is not on the segment, which
 * is what the lifting infrastructure reads to decide there is no crossing. A
 * chain holding another number of links is a different structure and so is
 * never on the segment, while the two chains defining the segment are values
 * of one temporal chain and must agree on that number
 */
long double
posechainsegm_locate(const PoseChain *start, const PoseChain *end,
  const PoseChain *value)
{
  if (! ensure_valid_posechain_posechain(start, end) ||
      ! ensure_valid_posechain_posechain(start, value) ||
      ! ensure_same_count_posechain(start, end))
    return -1.0;

  if (start->count != value->count)
    return -1.0;

  long double result = -1.0;
  for (int i = 0; i < start->count; i++)
  {
    Pose *pose1 = posechain_link_pose(start, i, i == 0);
    Pose *pose2 = posechain_link_pose(end, i, i == 0);
    Pose *pose = posechain_link_pose(value, i, i == 0);
    long double ratio = posesegm_locate(pose1, pose2, pose);
    pfree(pose1); pfree(pose2); pfree(pose);
    /* A link that does not move constrains no ratio */
    if (ratio < 0.0)
      continue;
    if (result < 0.0)
      result = ratio;
    else if (fabs((double) (result - ratio)) > MEOS_EPSILON)
      return -1.0;
  }
  return result;
}

/**
 * @brief Return true if the three pose chains are collinear
 * @param[in] pc1,pc2,pc3 Pose chains
 * @param[in] ratio Value in [0,1] representing the duration of the timestamps
 * associated to `pc1` and `pc2` divided by the duration of the timestamps
 * associated to `pc1` and `pc3`
 */
bool
posechain_collinear(const PoseChain *pc1, const PoseChain *pc2,
  const PoseChain *pc3, double ratio)
{
  assert(pc1); assert(pc2); assert(pc3);
  PoseChain *interpolated = posechainsegm_interpolate(pc1, pc3, ratio);
  if (interpolated == NULL)
    return false;
  bool result = posechain_same(pc2, interpolated);
  pfree(interpolated);
  return result;
}

/*****************************************************************************
 * Spatial reference system functions
 *****************************************************************************/

/**
 * @ingroup meos_posechain_base_srid
 * @brief Return the SRID of the outer frame of a pose chain
 * @param[in] pc Pose chain
 * @csqlfn #Posechain_srid()
 */
int32_t
posechain_srid(const PoseChain *pc)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(pc, SRID_INVALID);

  int32_t srid = 0;
  srid = (pc->srid[0] << 16);
  srid = srid | (pc->srid[1] << 8);
  srid = srid | (pc->srid[2]);
  /* Only the first 21 bits are set. Slide up and back to pull
     the negative bits down, if we need them. */
  srid = (srid << 11) >> 11;

  /* 0 is our internal unknown value. We'll map back and forth here for now */
  return (srid == 0) ? SRID_UNKNOWN : srid;
}

/**
 * @ingroup meos_internal_posechain_base_srid
 * @brief Set the outer frame of a pose chain to an SRID
 * @param[in] pc Pose chain
 * @param[in] srid SRID
 */
void
posechain_set_srid_int(PoseChain *pc, int32_t srid)
{
  assert(pc);
  /* 0 is our internal unknown value.
   * We'll map back and forth here for now */
  if (srid == SRID_UNKNOWN)
    srid = 0;
  pc->srid[0] = (srid & 0x001F0000) >> 16;
  pc->srid[1] = (srid & 0x0000FF00) >> 8;
  pc->srid[2] = (srid & 0x000000FF);
}

/**
 * @ingroup meos_posechain_base_srid
 * @brief Return a pose chain with its outer frame set to an SRID
 * @param[in] pc Pose chain
 * @param[in] srid SRID
 * @csqlfn #Posechain_set_srid()
 */
PoseChain *
posechain_set_srid(const PoseChain *pc, int32_t srid)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(pc, NULL);
  if (srid == SRID_INVALID)
    return NULL;
  PoseChain *result = posechain_copy(pc);
  posechain_set_srid_int(result, srid);
  return result;
}

/**
 * @brief Return a pose chain transformed to another SRID using a pipeline
 * @details Only the outer link names a frame, so only the outer link is
 * transformed: every other link is a rigid transform read in the axes of its
 * parent, and a change of the outer frame leaves those axes as they were
 */
PoseChain *
posechain_transf_pj(const PoseChain *pc, int32_t srid_to, const LWPROJ *pj)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(pc, NULL); VALIDATE_NOT_NULL(pj, NULL);

  Pose *outer = posechain_link_pose(pc, 0, true);
  Pose *transf = pose_transf_pj(outer, srid_to, pj);
  pfree(outer);
  if (! transf)
    return NULL;

  PoseChain *result = posechain_copy(pc);
  memcpy(result->data, transf->data,
    (size_t) POSECHAIN_LINK_SIZE(pc) * sizeof(double));
  MEOS_FLAGS_SET_GEODETIC(result->flags,
    MEOS_FLAGS_GET_GEODETIC(transf->flags));
  posechain_set_srid_int(result, srid_to);
  pfree(transf);
  return result;
}

/**
 * @ingroup meos_posechain_base_srid
 * @brief Return a pose chain transformed to another SRID
 * @param[in] pc Pose chain
 * @param[in] srid_to Target SRID
 * @csqlfn #Posechain_transform()
 */
PoseChain *
posechain_transform(const PoseChain *pc, int32_t srid_to)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(pc, NULL);
  int32_t srid_from = posechain_srid(pc);
  if (! ensure_srid_known(srid_from) || ! ensure_srid_known(srid_to))
    return NULL;
  if (srid_from == srid_to)
    return posechain_copy(pc);

  /* Get the structure with information about the projection */
  LWPROJ *pj;
  if (! lwproj_lookup(srid_from, srid_to, &pj))
    return NULL;

  return posechain_transf_pj(pc, srid_to, pj);
}

/**
 * @ingroup meos_posechain_base_srid
 * @brief Return a pose chain transformed to another SRID using a
 * transformation pipeline
 * @param[in] pc Pose chain
 * @param[in] pipeline Pipeline string
 * @param[in] srid_to Target SRID
 * @param[in] is_forward True when the transformation is applied in the
 * forward direction
 * @csqlfn #Posechain_transform_pipeline()
 */
PoseChain *
posechain_transform_pipeline(const PoseChain *pc, const char *pipeline,
  int32_t srid_to, bool is_forward)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(pc, NULL); VALIDATE_NOT_NULL(pipeline, NULL);

  if (! ensure_srid_known(srid_to))
    return NULL;

  /* Get the structure with information about the projection */
  LWPROJ *pj = lwproj_from_str_pipeline(pipeline, is_forward);
  if (! pj)
    return NULL;

  PoseChain *result = posechain_transf_pj(pc, srid_to, pj);
  proj_destroy(pj->pj); pfree(pj);
  return result;
}

/*****************************************************************************
 * Comparison functions
 *****************************************************************************/

/**
 * @ingroup meos_posechain_base_comp
 * @brief Return true if the first pose chain is equal to the second one
 * @param[in] pc1,pc2 Pose chains
 * @csqlfn #Posechain_eq()
 */
bool
posechain_eq(const PoseChain *pc1, const PoseChain *pc2)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(pc1, false); VALIDATE_NOT_NULL(pc2, false);

  if (pc1->count != pc2->count ||
      MEOS_FLAGS_GET_Z(pc1->flags) != MEOS_FLAGS_GET_Z(pc2->flags) ||
      MEOS_FLAGS_GET_GEODETIC(pc1->flags) !=
        MEOS_FLAGS_GET_GEODETIC(pc2->flags) ||
      posechain_srid(pc1) != posechain_srid(pc2))
    return false;
  int nvalues = POSECHAIN_LINK_SIZE(pc1) * pc1->count;
  for (int i = 0; i < nvalues; i++)
    if (! float8_eq(pc1->data[i], pc2->data[i]))
      return false;
  return true;
}

/**
 * @ingroup meos_posechain_base_comp
 * @brief Return true if the first pose chain is not equal to the second one
 * @param[in] pc1,pc2 Pose chains
 * @csqlfn #Posechain_ne()
 */
bool
posechain_ne(const PoseChain *pc1, const PoseChain *pc2)
{
  return ! posechain_eq(pc1, pc2);
}

/**
 * @ingroup meos_posechain_base_comp
 * @brief Return true if the first pose chain is equal to the second one up to
 * the tolerance of the comparison of floating-point values
 * @param[in] pc1,pc2 Pose chains
 * @csqlfn #Posechain_same()
 */
bool
posechain_same(const PoseChain *pc1, const PoseChain *pc2)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(pc1, false); VALIDATE_NOT_NULL(pc2, false);

  if (pc1->count != pc2->count ||
      MEOS_FLAGS_GET_Z(pc1->flags) != MEOS_FLAGS_GET_Z(pc2->flags) ||
      MEOS_FLAGS_GET_GEODETIC(pc1->flags) !=
        MEOS_FLAGS_GET_GEODETIC(pc2->flags) ||
      posechain_srid(pc1) != posechain_srid(pc2))
    return false;
  int nvalues = POSECHAIN_LINK_SIZE(pc1) * pc1->count;
  for (int i = 0; i < nvalues; i++)
    if (! MEOS_FP_EQ(pc1->data[i], pc2->data[i]))
      return false;
  return true;
}

/**
 * @ingroup meos_posechain_base_comp
 * @brief Return true if the first pose chain is not equal to the second one
 * up to the tolerance of the comparison of floating-point values
 * @param[in] pc1,pc2 Pose chains
 */
bool
posechain_nsame(const PoseChain *pc1, const PoseChain *pc2)
{
  return ! posechain_same(pc1, pc2);
}

/**
 * @ingroup meos_posechain_base_comp
 * @brief Return -1, 0, or 1 depending on whether the first pose chain is less
 * than, equal to, or greater than the second one
 * @param[in] pc1,pc2 Pose chains
 * @return On error return @p INT_MAX
 * @csqlfn #Posechain_cmp()
 */
int
posechain_cmp(const PoseChain *pc1, const PoseChain *pc2)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(pc1, INT_MAX); VALIDATE_NOT_NULL(pc2, INT_MAX);

  /* Compare first the dimension, then the SRID, then the links in order,
     and last the number of links, so that a chain precedes every chain
     that extends it */
  bool hasz1 = MEOS_FLAGS_GET_Z(pc1->flags),
       hasz2 = MEOS_FLAGS_GET_Z(pc2->flags);
  if (hasz1 != hasz2)
    return (hasz1 ? 1 : -1);

  int32_t srid1 = posechain_srid(pc1), srid2 = posechain_srid(pc2);
  if (srid1 < srid2)
    return -1;
  if (srid1 > srid2)
    return 1;

  int count = Min(pc1->count, pc2->count) * (hasz1 ? 7 : 3);
  for (int i = 0; i < count; i++)
  {
    if (pc1->data[i] < pc2->data[i])
      return -1;
    if (pc1->data[i] > pc2->data[i])
      return 1;
  }
  if (pc1->count < pc2->count)
    return -1;
  if (pc1->count > pc2->count)
    return 1;
  return 0;
}

/**
 * @ingroup meos_posechain_base_comp
 * @brief Return true if the first pose chain is less than the second one
 * @param[in] pc1,pc2 Pose chains
 * @csqlfn #Posechain_lt()
 */
bool
posechain_lt(const PoseChain *pc1, const PoseChain *pc2)
{
  return posechain_cmp(pc1, pc2) < 0;
}

/**
 * @ingroup meos_posechain_base_comp
 * @brief Return true if the first pose chain is less than or equal to the
 * second one
 * @param[in] pc1,pc2 Pose chains
 * @csqlfn #Posechain_le()
 */
bool
posechain_le(const PoseChain *pc1, const PoseChain *pc2)
{
  return posechain_cmp(pc1, pc2) <= 0;
}

/**
 * @ingroup meos_posechain_base_comp
 * @brief Return true if the first pose chain is greater than the second one
 * @param[in] pc1,pc2 Pose chains
 * @csqlfn #Posechain_gt()
 */
bool
posechain_gt(const PoseChain *pc1, const PoseChain *pc2)
{
  return posechain_cmp(pc1, pc2) > 0;
}

/**
 * @ingroup meos_posechain_base_comp
 * @brief Return true if the first pose chain is greater than or equal to the
 * second one
 * @param[in] pc1,pc2 Pose chains
 * @csqlfn #Posechain_ge()
 */
bool
posechain_ge(const PoseChain *pc1, const PoseChain *pc2)
{
  return posechain_cmp(pc1, pc2) >= 0;
}

/*****************************************************************************
 * Function for defining hash indexes
 *****************************************************************************/

/* Prototype for liblwgeom/lookup3.c */
void hashlittle2(const void *key, size_t length, uint32_t *pc, uint32_t *pb);

/**
 * @ingroup meos_posechain_base_accessor
 * @brief Return the 32-bit hash value of a pose chain
 * @param[in] pc Pose chain
 * @return On error return @p UINT32_MAX
 * @csqlfn #Posechain_hash()
 */
uint32
posechain_hash(const PoseChain *pc)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(pc, UINT32_MAX);

  /* Use the same code as pose_hash */
  int32_t hval;
  int32_t pb = 0, pc2 = 0;
  /* Point to just the flags/count/coordinate part of the buffer */
  size_t hsz1 = 8; /* varsize (4) + flags (1) + srid (3) */
  const uint8_t *b1 = (const uint8_t *) pc + hsz1;
  size_t bsz1 = VARSIZE(pc) - hsz1;
  int32_t srid = posechain_srid(pc);
  size_t bsz2 = bsz1 + sizeof(int);
  uint8_t *b2 = palloc(bsz2);
  /* Copy the SRID into the front of the combined buffer */
  memcpy(b2, &srid, sizeof(int));
  memcpy(b2 + sizeof(int), b1, bsz1);
  hashlittle2(b2, bsz2, (uint32_t *) &pb, (uint32_t *) &pc2);
  pfree(b2);
  hval = pb ^ pc2;
  return hval;
}

/**
 * @ingroup meos_posechain_base_accessor
 * @brief Return the 64-bit hash value of a pose chain using a seed
 * @param[in] pc Pose chain
 * @param[in] seed Seed
 * @csqlfn #Posechain_hash_extended()
 */
uint64
posechain_hash_extended(const PoseChain *pc, uint64 seed)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(pc, UINT64_MAX);
  return DatumGetUInt64(hash_any_extended(
    (unsigned char *) VARDATA_ANY(pc), VARSIZE_ANY_EXHDR(pc), seed));
}

/*****************************************************************************/

/*****************************************************************************
 * Bounding box functions
 *****************************************************************************/

/**
 * @ingroup meos_internal_box_constructor
 * @brief Return in the last argument a spatiotemporal box constructed from a
 * pose chain
 * @param[in] pc Pose chain
 * @param[out] box Spatiotemporal box
 * @details The box covers the composed position of every prefix of the chain,
 * not only of the whole of it. Every joint of a chain is a place, and a query
 * window that meets the elbow but not the hand still meets the chain
 */
bool
posechain_set_stbox(const PoseChain *pc, STBox *box)
{
  assert(pc); assert(box);
  bool found = false;
  for (int i = 1; i <= pc->count; i++)
  {
    Pose *pose = posechain_prefix_pose(pc, i);
    if (! pose)
      return false;
    STBox box1;
    bool ok = pose_set_stbox(pose, &box1);
    pfree(pose);
    if (! ok)
      return false;
    if (! found)
    {
      memcpy(box, &box1, sizeof(STBox));
      found = true;
    }
    else
      stbox_expand(&box1, box);
  }
  return found;
}

/**
 * @ingroup meos_internal_box_constructor
 * @brief Return in the last argument a spatiotemporal box constructed from an
 * array of pose chains
 * @param[in] values Pose chains
 * @param[in] count Number of elements in the array
 * @param[out] box Spatiotemporal box
 */
void
posechainarr_set_stbox(const Datum *values, int count, STBox *box)
{
  assert(values); assert(box); assert(count > 0);
  posechain_set_stbox(DatumGetPoseChainP(values[0]), box);
  for (int i = 1; i < count; i++)
  {
    STBox box1;
    posechain_set_stbox(DatumGetPoseChainP(values[i]), &box1);
    stbox_expand(&box1, box);
  }
  return;
}

/**
 * @ingroup meos_posechain_base_conversion
 * @brief Convert a pose chain into a spatiotemporal box
 * @param[in] pc Pose chain
 * @csqlfn #Posechain_to_stbox()
 */
STBox *
posechain_to_stbox(const PoseChain *pc)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(pc, NULL);
  STBox box;
  if (! posechain_set_stbox(pc, &box))
    return NULL;
  return stbox_copy(&box);
}

/*****************************************************************************/
