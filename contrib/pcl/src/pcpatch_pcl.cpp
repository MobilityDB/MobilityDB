/**
 * @file pcpatch_pcl.cpp
 * @brief PCL bridge — PostgreSQL functions that serialize a
 *        pgPointCloud @c pcpatch to/from a PCL PCD bytea.
 *
 * Uses SPI to fetch the schema XML from @c pointcloud_formats and
 * libpc.a's @c pc_schema_from_xml (statically linked) to build a
 * @c PCSCHEMA. The patch's serialized representation (SERIALIZED_PATCH
 * varlena, compression == PC_NONE) is walked directly to extract X/Y/Z
 * dim values per point; WKB conversion is avoided so we don't need any
 * symbol resolution from pgPointCloud's PG extension @c .so. PCL I/O
 * uses a temp-file shim because PCL's PCD reader/writer take filesystem
 * paths.
 *
 * For now: PC_NONE compression only on input, since MobilityDB's
 * @c tpcpatch normalizes its internal storage to PC_NONE anyway.
 *
 * @ingroup pcl_bridge
 */

// PCL and standard C++ headers MUST come before PostgreSQL headers:
// PG's c.h #defines dngettext to a macro, which then conflicts with
// libintl.h's function declarations included transitively via PCL/boost.
#include <pcl/point_cloud.h>
#include <pcl/point_types.h>
#include <pcl/io/pcd_io.h>
#include <pcl/filters/voxel_grid.h>
#include <pcl/filters/statistical_outlier_removal.h>
#include <pcl/registration/icp.h>
#include <pcl/registration/gicp.h>
#include <pcl/features/normal_3d.h>
#include <pcl/search/kdtree.h>
#include <Eigen/Geometry>

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <string>
#include <unistd.h>
#include <strings.h>     /* strcasecmp */

extern "C" {
#include "postgres.h"
#include "fmgr.h"
#include "executor/spi.h"
#include "utils/builtins.h"
#include "utils/elog.h"
#include "utils/memutils.h"

PG_MODULE_MAGIC;

PG_FUNCTION_INFO_V1(pcpatch_to_pcd);
PG_FUNCTION_INFO_V1(pcpatch_from_pcd);
PG_FUNCTION_INFO_V1(pcpatch_voxel_grid);
PG_FUNCTION_INFO_V1(pcpatch_sor);
PG_FUNCTION_INFO_V1(pcpatch_icp);
PG_FUNCTION_INFO_V1(pcpatch_gicp);
PG_FUNCTION_INFO_V1(pcpatch_normals);

Datum pcpatch_to_pcd(PG_FUNCTION_ARGS);
Datum pcpatch_from_pcd(PG_FUNCTION_ARGS);
Datum pcpatch_voxel_grid(PG_FUNCTION_ARGS);
Datum pcpatch_sor(PG_FUNCTION_ARGS);
Datum pcpatch_icp(PG_FUNCTION_ARGS);
Datum pcpatch_gicp(PG_FUNCTION_ARGS);

#include "catalog/pg_type.h"
#include "utils/array.h"
#include "utils/lsyscache.h"

void _PG_init(void);
} // extern "C"

extern "C" {
#include "pc_api.h"
#include "pc_api_internal.h"
}

namespace
{

// ----- SERIALIZED_PATCH layout (matches pgPointCloud pgsql/pc_pgsql.h) -----
// uint32 size (varlena prefix); uint32 pcid; uint32 compression;
// uint32 npoints; PCBOUNDS bounds (4 doubles, 32 bytes); uint8 data[1].
// Stats and point data follow inside the data tail. For PC_NONE:
//   data starts at offsetof(SERIALIZED_PATCH, data) which is 48 in the
//   compiler's layout; followed by 3 * schema->size stats bytes and then
//   npoints * schema->size point bytes.
// We don't include pgPointCloud's pgsql/pc_pgsql.h here to avoid pulling
// PG headers from pointcloud's pgsql tree; we mirror the layout instead.
struct PcSerializedPatchHeader
{
    uint32_t vl_len;
    uint32_t pcid;
    uint32_t compression;
    uint32_t npoints;
    double xmin, xmax, ymin, ymax;
}; // 48 bytes

constexpr size_t kSerializedPatchDataOffset = 48;
constexpr uint32_t kCompressionNone = 0;

// Handler installation is handled by pgPointCloud's PG extension when
// pointcloud is loaded into the backend (it sets palloc/pfree as the
// pc_context handlers). We must NOT re-install default malloc/free
// handlers because that would mix allocators across the two .sos and
// crash on free. The user's session must have pgPointCloud loaded
// before any function from this extension is called — which is the
// natural expectation given our SQL signatures take pcpatch.
void ensurePcHandlers() {}

PCDIMENSION* findDim(const PCSCHEMA* schema, const char* name)
{
    for (uint32_t i = 0; i < schema->ndims; ++i)
    {
        PCDIMENSION* d = schema->dims[i];
        if (d && d->name && strcasecmp(d->name, name) == 0)
            return d;
    }
    return nullptr;
}

PCSCHEMA* fetchSchemaForPcid(uint32_t pcid)
{
    if (SPI_connect() != SPI_OK_CONNECT)
        elog(ERROR, "SPI_connect failed");

    char query[128];
    snprintf(query, sizeof(query),
             "SELECT schema FROM pointcloud_formats WHERE pcid = %u", pcid);

    int ret = SPI_execute(query, true, 1);
    if (ret != SPI_OK_SELECT || SPI_processed != 1)
    {
        SPI_finish();
        elog(ERROR, "no pointcloud_formats row for pcid=%u", pcid);
    }
    bool isnull = false;
    Datum xmlDatum = SPI_getbinval(SPI_tuptable->vals[0],
                                   SPI_tuptable->tupdesc, 1, &isnull);
    if (isnull)
    {
        SPI_finish();
        elog(ERROR, "pointcloud_formats.schema is NULL for pcid=%u", pcid);
    }
    char* xmlText = TextDatumGetCString(xmlDatum);
    PCSCHEMA* schema = pc_schema_from_xml(xmlText);
    pfree(xmlText);
    SPI_finish();

    if (!schema)
        elog(ERROR, "pc_schema_from_xml failed for pcid=%u", pcid);
    schema->pcid = pcid;
    return schema;
}

class TempFile
{
public:
    TempFile()
    {
        char tmpl[] = "/tmp/mobilitydb_pcl_XXXXXX";
        int fd = mkstemp(tmpl);
        if (fd < 0)
            throw std::runtime_error("mkstemp failed");
        m_path = tmpl;
        ::close(fd);
    }
    ~TempFile()
    {
        if (!m_path.empty())
            ::unlink(m_path.c_str());
    }
    TempFile(const TempFile&) = delete;
    TempFile& operator=(const TempFile&) = delete;
    const std::string& path() const { return m_path; }

private:
    std::string m_path;
};

/* I/O helpers throw std::runtime_error rather than calling
 * elog(ERROR) directly. PostgreSQL's elog(ERROR) longjmps out of the
 * frame and bypasses C++ destructors — the TempFile that owns the
 * filesystem path would never run its unlink, leaking /tmp/... on a
 * busy server. By raising a C++ exception instead, the TempFile
 * destructor runs during stack unwinding before the caller's
 * try/catch translates the message into elog(ERROR). */
bytea* fileToBytea(const std::string& path)
{
    std::ifstream in(path, std::ios::binary | std::ios::ate);
    if (!in)
        throw std::runtime_error("could not open temp file '" + path + "' for read");
    std::streamsize size = in.tellg();
    in.seekg(0, std::ios::beg);
    bytea* out = (bytea*) palloc(static_cast<size_t>(size) + VARHDRSZ);
    SET_VARSIZE(out, size + VARHDRSZ);
    if (!in.read(VARDATA(out), size))  // Flawfinder: ignore (std::ifstream::read into a palloc'd buffer of exactly 'size' bytes)
        throw std::runtime_error("short read from temp file '" + path + "'");
    return out;
}

void byteaToFile(const bytea* in, const std::string& path)
{
    std::ofstream out(path, std::ios::binary | std::ios::trunc);
    if (!out)
        throw std::runtime_error("could not open temp file '" + path + "' for write");
    int32 sz = VARSIZE_ANY_EXHDR(in);
    out.write(VARDATA_ANY(in), sz);
    if (!out)
        throw std::runtime_error("short write to temp file '" + path + "'");
}

} // namespace

extern "C" void _PG_init(void) { ensurePcHandlers(); }

/* -- pcpatch_to_pcd(pcpatch) RETURNS bytea ---------------------------- */
extern "C" Datum pcpatch_to_pcd(PG_FUNCTION_ARGS)
{
    ensurePcHandlers();

    bytea* serpatchVar = (bytea*) PG_DETOAST_DATUM(PG_GETARG_DATUM(0));
    if (VARSIZE_ANY_EXHDR(serpatchVar) + VARHDRSZ < kSerializedPatchDataOffset)
        elog(ERROR, "pcpatch is shorter than SERIALIZED_PATCH header");
    const PcSerializedPatchHeader* hdr =
        reinterpret_cast<const PcSerializedPatchHeader*>(serpatchVar);

    if (hdr->compression != kCompressionNone)
        elog(ERROR, "pcpatch_to_pcd: only PC_NONE compression supported "
                    "(got compression=%u). Use PC_Compress(p, 'auto') -> "
                    "tpcpatch round-trip to normalize.",
             hdr->compression);

    PCSCHEMA* schema = fetchSchemaForPcid(hdr->pcid);

    PCDIMENSION* xd = findDim(schema, "X");
    PCDIMENSION* yd = findDim(schema, "Y");
    PCDIMENSION* zd = findDim(schema, "Z");
    if (!xd || !yd || !zd)
    {
        pc_schema_free(schema);
        elog(ERROR, "pcpatch_to_pcd: schema (pcid=%u) is missing X/Y/Z",
             hdr->pcid);
    }

    size_t pointSize = schema->size;
    size_t statsSize = 3 * pointSize;
    const uint8_t* pointData =
        reinterpret_cast<const uint8_t*>(serpatchVar) +
        kSerializedPatchDataOffset + statsSize;

    PCDIMENSION* intensity = findDim(schema, "Intensity");
    PCDIMENSION* red       = findDim(schema, "Red");
    PCDIMENSION* green     = findDim(schema, "Green");
    PCDIMENSION* blue      = findDim(schema, "Blue");
    bool has_rgb = (red && green && blue);

    /* Encode + read-back inside a try block so the TempFile dtor (and
     * therefore the unlink of /tmp/mobilitydb_pcl_XXXXXX) runs even
     * when PCL save / read-back fails. We translate the C++ exception
     * to elog(ERROR) only after the dtor has run. */
    bytea* result = nullptr;
    std::string err;
    try
    {
        TempFile tf;
        int rc = -1;
        if (has_rgb)
        {
            // PointXYZRGB carries colour even if intensity is also present.
            pcl::PointCloud<pcl::PointXYZRGB> cloud;
            cloud.reserve(hdr->npoints);
            for (uint32_t i = 0; i < hdr->npoints; ++i)
            {
                const uint8_t* p = pointData + static_cast<size_t>(i) * pointSize;
                pcl::PointXYZRGB pt;
                pt.x = static_cast<float>(pc_double_from_ptr(p + xd->byteoffset, xd->interpretation));
                pt.y = static_cast<float>(pc_double_from_ptr(p + yd->byteoffset, yd->interpretation));
                pt.z = static_cast<float>(pc_double_from_ptr(p + zd->byteoffset, zd->interpretation));
                pt.r = static_cast<uint8_t>(pc_double_from_ptr(p + red->byteoffset,   red->interpretation));
                pt.g = static_cast<uint8_t>(pc_double_from_ptr(p + green->byteoffset, green->interpretation));
                pt.b = static_cast<uint8_t>(pc_double_from_ptr(p + blue->byteoffset,  blue->interpretation));
                cloud.push_back(pt);
            }
            cloud.width = cloud.size(); cloud.height = 1; cloud.is_dense = true;
            rc = pcl::io::savePCDFileBinary(tf.path(), cloud);
        }
        else if (intensity)
        {
            pcl::PointCloud<pcl::PointXYZI> cloud;
            cloud.reserve(hdr->npoints);
            for (uint32_t i = 0; i < hdr->npoints; ++i)
            {
                const uint8_t* p = pointData + static_cast<size_t>(i) * pointSize;
                pcl::PointXYZI pt;
                pt.x = static_cast<float>(pc_double_from_ptr(p + xd->byteoffset, xd->interpretation));
                pt.y = static_cast<float>(pc_double_from_ptr(p + yd->byteoffset, yd->interpretation));
                pt.z = static_cast<float>(pc_double_from_ptr(p + zd->byteoffset, zd->interpretation));
                pt.intensity = static_cast<float>(pc_double_from_ptr(
                    p + intensity->byteoffset, intensity->interpretation));
                cloud.push_back(pt);
            }
            cloud.width = cloud.size(); cloud.height = 1; cloud.is_dense = true;
            rc = pcl::io::savePCDFileBinary(tf.path(), cloud);
        }
        else
        {
            pcl::PointCloud<pcl::PointXYZ> cloud;
            cloud.reserve(hdr->npoints);
            for (uint32_t i = 0; i < hdr->npoints; ++i)
            {
                const uint8_t* p = pointData + static_cast<size_t>(i) * pointSize;
                pcl::PointXYZ pt;
                pt.x = static_cast<float>(pc_double_from_ptr(p + xd->byteoffset, xd->interpretation));
                pt.y = static_cast<float>(pc_double_from_ptr(p + yd->byteoffset, yd->interpretation));
                pt.z = static_cast<float>(pc_double_from_ptr(p + zd->byteoffset, zd->interpretation));
                cloud.push_back(pt);
            }
            cloud.width = cloud.size(); cloud.height = 1; cloud.is_dense = true;
            rc = pcl::io::savePCDFileBinary(tf.path(), cloud);
        }

        if (rc != 0)
            throw std::runtime_error("pcl::io::savePCDFileBinary failed");
        result = fileToBytea(tf.path());
        /* tf goes out of scope here -> unlink runs cleanly */
    }
    catch (const std::exception& e)
    {
        err = e.what();
    }

    pc_schema_free(schema);
    if (! err.empty())
        elog(ERROR, "pcpatch_to_pcd: %s", err.c_str());
    PG_RETURN_BYTEA_P(result);
}

/* -- pcd_to_pcpatch_wkb_hex(bytea, int) RETURNS text ------------------ *
 * Returns the pcpatch as WKB hex text. Wrap with ::pcpatch in SQL to
 * get a real pcpatch value. We keep WKB-only (not SERIALIZED_PATCH)
 * because constructing a SERIALIZED_PATCH from outside the extension
 * is layout-fragile (BUFFERALIGN slack offsets).
 */
extern "C" Datum pcpatch_from_pcd(PG_FUNCTION_ARGS)
{
    ensurePcHandlers();
    bytea* input = PG_GETARG_BYTEA_PP(0);
    int32 pcid = PG_GETARG_INT32(1);

    PCSCHEMA* schema = fetchSchemaForPcid(static_cast<uint32_t>(pcid));
    PCDIMENSION* xd = findDim(schema, "X");
    PCDIMENSION* yd = findDim(schema, "Y");
    PCDIMENSION* zd = findDim(schema, "Z");
    if (!xd || !yd || !zd)
    {
        pc_schema_free(schema);
        elog(ERROR, "pcpatch_from_pcd: schema (pcid=%d) missing X/Y/Z", pcid);
    }
    PCDIMENSION* intensity = findDim(schema, "Intensity");
    PCDIMENSION* red       = findDim(schema, "Red");
    PCDIMENSION* green     = findDim(schema, "Green");
    PCDIMENSION* blue      = findDim(schema, "Blue");
    bool schema_has_rgb = (red && green && blue);

    /* Wrap the PCD-load + decode in a try block so the TempFile dtor
     * (and unlink) runs cleanly even when PCL load / field-introspect
     * fail. Any thrown std::runtime_error captures into err, the dtor
     * fires during stack unwinding, and we elog(ERROR) only after. */
    uint8_t* wkb = nullptr;
    size_t wkbsize = 0;
    std::string err;
    try
    {
        TempFile tf;
        byteaToFile(input, tf.path());

        // Read into PCLPointCloud2 — the type-agnostic container — so we
        // can introspect the PCD's actual fields and only copy across what
        // both the PCD and the schema declare.
        pcl::PCLPointCloud2 cloud2;
        if (pcl::io::loadPCDFile(tf.path(), cloud2) != 0)
            throw std::runtime_error("pcl::io::loadPCDFile failed");

        auto findField = [&cloud2](const char* name) -> const pcl::PCLPointField* {
            for (const auto& f : cloud2.fields)
                if (strcasecmp(f.name.c_str(), name) == 0) return &f;
            return nullptr;
        };
        const pcl::PCLPointField* fx = findField("x");
        const pcl::PCLPointField* fy = findField("y");
        const pcl::PCLPointField* fz = findField("z");
        if (!fx || !fy || !fz)
            throw std::runtime_error("PCD is missing x/y/z fields");

        const pcl::PCLPointField* f_intensity = findField("intensity");
        const pcl::PCLPointField* f_rgb       = findField("rgb");

        auto readFloat = [](const pcl::PCLPointField* f, const uint8_t* row) -> float {
            float v;
            std::memcpy(&v, row + f->offset, sizeof(float));
            return v;
        };

        size_t row_step = cloud2.point_step;
        size_t npoints = cloud2.width * cloud2.height;
        PCPOINTLIST* pl = pc_pointlist_make(static_cast<uint32_t>(npoints));
        for (size_t i = 0; i < npoints; ++i)
        {
            const uint8_t* row = cloud2.data.data() + i * row_step;
            PCPOINT* pt = pc_point_make(schema);
            pc_double_to_ptr(pt->data + xd->byteoffset, xd->interpretation,
                             static_cast<double>(readFloat(fx, row)));
            pc_double_to_ptr(pt->data + yd->byteoffset, yd->interpretation,
                             static_cast<double>(readFloat(fy, row)));
            pc_double_to_ptr(pt->data + zd->byteoffset, zd->interpretation,
                             static_cast<double>(readFloat(fz, row)));
            if (intensity && f_intensity)
            {
                pc_double_to_ptr(pt->data + intensity->byteoffset, intensity->interpretation,
                                 static_cast<double>(readFloat(f_intensity, row)));
            }
            if (schema_has_rgb && f_rgb)
            {
                // pcl::PointXYZRGB packs r/g/b into a single 32-bit float
                // (PCL's union trick). Read as uint32 from the same offset.
                uint32_t packed;
                std::memcpy(&packed, row + f_rgb->offset, sizeof(uint32_t));
                uint8_t r = static_cast<uint8_t>((packed >> 16) & 0xFF);
                uint8_t g = static_cast<uint8_t>((packed >> 8)  & 0xFF);
                uint8_t b = static_cast<uint8_t>(packed         & 0xFF);
                pc_double_to_ptr(pt->data + red->byteoffset,   red->interpretation,   r);
                pc_double_to_ptr(pt->data + green->byteoffset, green->interpretation, g);
                pc_double_to_ptr(pt->data + blue->byteoffset,  blue->interpretation,  b);
            }
            pc_pointlist_add_point(pl, pt);
        }
        PCPATCH* patch = (PCPATCH*) pc_patch_uncompressed_from_pointlist(pl);
        pc_pointlist_free(pl);
        if (!patch)
            throw std::runtime_error("pc_patch_uncompressed_from_pointlist failed");
        wkb = pc_patch_to_wkb(patch, &wkbsize);
        pc_patch_free(patch);
        if (!wkb || wkbsize == 0)
            throw std::runtime_error("pc_patch_to_wkb returned empty");
        /* tf goes out of scope here -> unlink runs cleanly */
    }
    catch (const std::exception& e)
    {
        err = e.what();
    }

    pc_schema_free(schema);
    if (! err.empty())
        elog(ERROR, "pcpatch_from_pcd: %s", err.c_str());

    // Hex-encode the WKB into a Postgres text Datum.
    static const char* k = "0123456789ABCDEF";
    text* result = (text*) palloc(VARHDRSZ + wkbsize * 2);
    SET_VARSIZE(result, VARHDRSZ + wkbsize * 2);
    char* dst = VARDATA(result);
    for (size_t i = 0; i < wkbsize; ++i)
    {
        dst[2 * i] = k[(wkb[i] >> 4) & 0x0F];
        dst[2 * i + 1] = k[wkb[i] & 0x0F];
    }
    pcfree(wkb);
    PG_RETURN_TEXT_P(result);
}

// ---------------------------------------------------------------------
// Filter helpers shared by pcpatch_voxel_grid and pcpatch_sor.
// ---------------------------------------------------------------------

namespace
{

struct DimRefs
{
    PCDIMENSION *xd, *yd, *zd;
    PCDIMENSION *intensity;
    PCDIMENSION *red, *green, *blue;
};

// Per-PointT loaders / writers via overloads. Each fills point
// fields from the patch's row bytes (load) or vice versa (store).

inline void load_point(const uint8_t* p, pcl::PointXYZ& pt, const DimRefs& d)
{
    pt.x = static_cast<float>(pc_double_from_ptr(p + d.xd->byteoffset, d.xd->interpretation));
    pt.y = static_cast<float>(pc_double_from_ptr(p + d.yd->byteoffset, d.yd->interpretation));
    pt.z = static_cast<float>(pc_double_from_ptr(p + d.zd->byteoffset, d.zd->interpretation));
}
inline void load_point(const uint8_t* p, pcl::PointXYZI& pt, const DimRefs& d)
{
    pt.x = static_cast<float>(pc_double_from_ptr(p + d.xd->byteoffset, d.xd->interpretation));
    pt.y = static_cast<float>(pc_double_from_ptr(p + d.yd->byteoffset, d.yd->interpretation));
    pt.z = static_cast<float>(pc_double_from_ptr(p + d.zd->byteoffset, d.zd->interpretation));
    pt.intensity = static_cast<float>(pc_double_from_ptr(
        p + d.intensity->byteoffset, d.intensity->interpretation));
}
inline void load_point(const uint8_t* p, pcl::PointXYZRGB& pt, const DimRefs& d)
{
    pt.x = static_cast<float>(pc_double_from_ptr(p + d.xd->byteoffset, d.xd->interpretation));
    pt.y = static_cast<float>(pc_double_from_ptr(p + d.yd->byteoffset, d.yd->interpretation));
    pt.z = static_cast<float>(pc_double_from_ptr(p + d.zd->byteoffset, d.zd->interpretation));
    pt.r = static_cast<uint8_t>(pc_double_from_ptr(p + d.red->byteoffset,   d.red->interpretation));
    pt.g = static_cast<uint8_t>(pc_double_from_ptr(p + d.green->byteoffset, d.green->interpretation));
    pt.b = static_cast<uint8_t>(pc_double_from_ptr(p + d.blue->byteoffset,  d.blue->interpretation));
}

inline void store_point(PCPOINT* pt, const pcl::PointXYZ& p, const DimRefs& d)
{
    pc_double_to_ptr(pt->data + d.xd->byteoffset, d.xd->interpretation, static_cast<double>(p.x));
    pc_double_to_ptr(pt->data + d.yd->byteoffset, d.yd->interpretation, static_cast<double>(p.y));
    pc_double_to_ptr(pt->data + d.zd->byteoffset, d.zd->interpretation, static_cast<double>(p.z));
}
inline void store_point(PCPOINT* pt, const pcl::PointXYZI& p, const DimRefs& d)
{
    pc_double_to_ptr(pt->data + d.xd->byteoffset, d.xd->interpretation, static_cast<double>(p.x));
    pc_double_to_ptr(pt->data + d.yd->byteoffset, d.yd->interpretation, static_cast<double>(p.y));
    pc_double_to_ptr(pt->data + d.zd->byteoffset, d.zd->interpretation, static_cast<double>(p.z));
    pc_double_to_ptr(pt->data + d.intensity->byteoffset, d.intensity->interpretation,
                     static_cast<double>(p.intensity));
}
inline void store_point(PCPOINT* pt, const pcl::PointXYZRGB& p, const DimRefs& d)
{
    pc_double_to_ptr(pt->data + d.xd->byteoffset, d.xd->interpretation, static_cast<double>(p.x));
    pc_double_to_ptr(pt->data + d.yd->byteoffset, d.yd->interpretation, static_cast<double>(p.y));
    pc_double_to_ptr(pt->data + d.zd->byteoffset, d.zd->interpretation, static_cast<double>(p.z));
    pc_double_to_ptr(pt->data + d.red->byteoffset,   d.red->interpretation,   static_cast<double>(p.r));
    pc_double_to_ptr(pt->data + d.green->byteoffset, d.green->interpretation, static_cast<double>(p.g));
    pc_double_to_ptr(pt->data + d.blue->byteoffset,  d.blue->interpretation,  static_cast<double>(p.b));
}

/// Apply @p apply_filter to a freshly-loaded cloud<PointT> and write the
/// filtered result to a new PCPATCH (uncompressed). Caller frees the
/// returned patch via pc_patch_free.
template<typename PointT, typename ApplyFilter>
PCPATCH* run_pcl_filter(const PcSerializedPatchHeader* hdr,
                        const uint8_t* pointData, size_t pointSize,
                        PCSCHEMA* schema, const DimRefs& d,
                        ApplyFilter apply_filter)
{
    typename pcl::PointCloud<PointT>::Ptr in(new pcl::PointCloud<PointT>());
    in->reserve(hdr->npoints);
    for (uint32_t i = 0; i < hdr->npoints; ++i)
    {
        const uint8_t* p = pointData + static_cast<size_t>(i) * pointSize;
        PointT pt;
        load_point(p, pt, d);
        in->push_back(pt);
    }
    in->width = in->size(); in->height = 1; in->is_dense = true;

    typename pcl::PointCloud<PointT>::Ptr out(new pcl::PointCloud<PointT>());
    apply_filter(in, out);

    PCPOINTLIST* pl = pc_pointlist_make(static_cast<uint32_t>(out->size()));
    for (const auto& p : out->points)
    {
        PCPOINT* pt = pc_point_make(schema);
        store_point(pt, p, d);
        pc_pointlist_add_point(pl, pt);
    }
    PCPATCH* patch = (PCPATCH*) pc_patch_uncompressed_from_pointlist(pl);
    pc_pointlist_free(pl);
    return patch;
}

/// Build a text* containing hex of the patch's WKB, free the patch and
/// schema. Used by both filter entry points.
text* patch_to_wkb_hex_and_free(PCPATCH* patch, PCSCHEMA* schema,
                                 const char* fn_name)
{
    if (!patch)
    {
        pc_schema_free(schema);
        elog(ERROR, "%s: pc_patch_uncompressed_from_pointlist failed", fn_name);
    }
    size_t wkbsize = 0;
    uint8_t* wkb = pc_patch_to_wkb(patch, &wkbsize);
    pc_patch_free(patch);
    pc_schema_free(schema);
    if (!wkb || wkbsize == 0)
        elog(ERROR, "%s: pc_patch_to_wkb returned empty", fn_name);

    static const char* k = "0123456789ABCDEF";
    text* result = (text*) palloc(VARHDRSZ + wkbsize * 2);
    SET_VARSIZE(result, VARHDRSZ + wkbsize * 2);
    char* dst = VARDATA(result);
    for (size_t i = 0; i < wkbsize; ++i)
    {
        dst[2 * i] = k[(wkb[i] >> 4) & 0x0F];
        dst[2 * i + 1] = k[wkb[i] & 0x0F];
    }
    pcfree(wkb);
    return result;
}

/// Common entry-point preamble for filters: extract the pcpatch header,
/// validate compression, fetch schema, resolve dim refs.
struct FilterEntry
{
    const PcSerializedPatchHeader* hdr;
    const uint8_t* pointData;
    size_t pointSize;
    PCSCHEMA* schema;
    DimRefs dims;
    bool has_rgb;
    bool has_i;
};

FilterEntry filter_entry(bytea* serpatchVar, const char* fn_name)
{
    if (VARSIZE_ANY_EXHDR(serpatchVar) + VARHDRSZ < kSerializedPatchDataOffset)
        elog(ERROR, "pcpatch is shorter than SERIALIZED_PATCH header");
    const PcSerializedPatchHeader* hdr =
        reinterpret_cast<const PcSerializedPatchHeader*>(serpatchVar);
    if (hdr->compression != kCompressionNone)
        elog(ERROR, "%s: only PC_NONE compression supported (got compression=%u)",
             fn_name, hdr->compression);

    PCSCHEMA* schema = fetchSchemaForPcid(hdr->pcid);
    DimRefs d {};
    d.xd = findDim(schema, "X");
    d.yd = findDim(schema, "Y");
    d.zd = findDim(schema, "Z");
    if (!d.xd || !d.yd || !d.zd)
    {
        pc_schema_free(schema);
        elog(ERROR, "%s: schema (pcid=%u) missing X/Y/Z", fn_name, hdr->pcid);
    }
    d.intensity = findDim(schema, "Intensity");
    d.red       = findDim(schema, "Red");
    d.green     = findDim(schema, "Green");
    d.blue      = findDim(schema, "Blue");

    FilterEntry e;
    e.hdr = hdr;
    e.pointSize = schema->size;
    e.pointData = reinterpret_cast<const uint8_t*>(serpatchVar)
                  + kSerializedPatchDataOffset + 3 * e.pointSize;
    e.schema = schema;
    e.dims = d;
    e.has_rgb = (d.red && d.green && d.blue);
    e.has_i   = (d.intensity != nullptr);
    return e;
}

} // namespace

/* -- pcpatch_voxel_grid(pcpatch, leaf double) RETURNS text ----------- *
 * Apply PCL's VoxelGrid filter to the patch's points (down-sampling).
 * leaf is the cell edge in the schema's spatial unit; a typical value
 * for metre-scaled point clouds is 0.05 (5 cm). Non-spatial dims
 * (Intensity, R/G/B) survive the filter when present in the schema —
 * VoxelGrid averages them within each voxel.
 */
extern "C" Datum pcpatch_voxel_grid(PG_FUNCTION_ARGS)
{
    ensurePcHandlers();

    bytea* serpatchVar = (bytea*) PG_DETOAST_DATUM(PG_GETARG_DATUM(0));
    double leaf = PG_GETARG_FLOAT8(1);
    if (!(leaf > 0.0))
        elog(ERROR, "pcpatch_voxel_grid: leaf must be positive (got %g)", leaf);

    FilterEntry e = filter_entry(serpatchVar, "pcpatch_voxel_grid");
    PCPATCH* patch = nullptr;
    float lf = static_cast<float>(leaf);

    if (e.has_rgb)
    {
        patch = run_pcl_filter<pcl::PointXYZRGB>(
            e.hdr, e.pointData, e.pointSize, e.schema, e.dims,
            [lf](pcl::PointCloud<pcl::PointXYZRGB>::Ptr in,
                 pcl::PointCloud<pcl::PointXYZRGB>::Ptr out) {
                pcl::VoxelGrid<pcl::PointXYZRGB> vg;
                vg.setInputCloud(in);
                vg.setLeafSize(lf, lf, lf);
                vg.filter(*out);
            });
    }
    else if (e.has_i)
    {
        patch = run_pcl_filter<pcl::PointXYZI>(
            e.hdr, e.pointData, e.pointSize, e.schema, e.dims,
            [lf](pcl::PointCloud<pcl::PointXYZI>::Ptr in,
                 pcl::PointCloud<pcl::PointXYZI>::Ptr out) {
                pcl::VoxelGrid<pcl::PointXYZI> vg;
                vg.setInputCloud(in);
                vg.setLeafSize(lf, lf, lf);
                vg.filter(*out);
            });
    }
    else
    {
        patch = run_pcl_filter<pcl::PointXYZ>(
            e.hdr, e.pointData, e.pointSize, e.schema, e.dims,
            [lf](pcl::PointCloud<pcl::PointXYZ>::Ptr in,
                 pcl::PointCloud<pcl::PointXYZ>::Ptr out) {
                pcl::VoxelGrid<pcl::PointXYZ> vg;
                vg.setInputCloud(in);
                vg.setLeafSize(lf, lf, lf);
                vg.filter(*out);
            });
    }
    PG_RETURN_TEXT_P(patch_to_wkb_hex_and_free(patch, e.schema,
                                                "pcpatch_voxel_grid"));
}

/* -- pcpatch_sor(pcpatch, k int, stddev double) RETURNS text --------- *
 * Apply PCL's StatisticalOutlierRemoval filter to the patch's X/Y/Z
 * points. k is the number of neighbours to consider per point;
 * stddev_mul is the standard-deviation multiplier — points whose mean
 * distance to their k nearest neighbours falls outside (mean ±
 * stddev_mul × stddev) are dropped. Returns pcpatch WKB hex. Typical
 * values: k=50, stddev_mul=1.0.
 */
extern "C" Datum pcpatch_sor(PG_FUNCTION_ARGS)
{
    ensurePcHandlers();

    bytea* serpatchVar = (bytea*) PG_DETOAST_DATUM(PG_GETARG_DATUM(0));
    int32 k = PG_GETARG_INT32(1);
    double stddev_mul = PG_GETARG_FLOAT8(2);
    if (k < 2)
        elog(ERROR, "pcpatch_sor: k must be >= 2 (got %d)", k);
    if (!(stddev_mul > 0.0))
        elog(ERROR, "pcpatch_sor: stddev_mul must be positive (got %g)",
             stddev_mul);

    FilterEntry e = filter_entry(serpatchVar, "pcpatch_sor");
    PCPATCH* patch = nullptr;

    if (e.has_rgb)
    {
        patch = run_pcl_filter<pcl::PointXYZRGB>(
            e.hdr, e.pointData, e.pointSize, e.schema, e.dims,
            [k, stddev_mul](pcl::PointCloud<pcl::PointXYZRGB>::Ptr in,
                            pcl::PointCloud<pcl::PointXYZRGB>::Ptr out) {
                pcl::StatisticalOutlierRemoval<pcl::PointXYZRGB> sor;
                sor.setInputCloud(in);
                sor.setMeanK(k);
                sor.setStddevMulThresh(stddev_mul);
                sor.filter(*out);
            });
    }
    else if (e.has_i)
    {
        patch = run_pcl_filter<pcl::PointXYZI>(
            e.hdr, e.pointData, e.pointSize, e.schema, e.dims,
            [k, stddev_mul](pcl::PointCloud<pcl::PointXYZI>::Ptr in,
                            pcl::PointCloud<pcl::PointXYZI>::Ptr out) {
                pcl::StatisticalOutlierRemoval<pcl::PointXYZI> sor;
                sor.setInputCloud(in);
                sor.setMeanK(k);
                sor.setStddevMulThresh(stddev_mul);
                sor.filter(*out);
            });
    }
    else
    {
        patch = run_pcl_filter<pcl::PointXYZ>(
            e.hdr, e.pointData, e.pointSize, e.schema, e.dims,
            [k, stddev_mul](pcl::PointCloud<pcl::PointXYZ>::Ptr in,
                            pcl::PointCloud<pcl::PointXYZ>::Ptr out) {
                pcl::StatisticalOutlierRemoval<pcl::PointXYZ> sor;
                sor.setInputCloud(in);
                sor.setMeanK(k);
                sor.setStddevMulThresh(stddev_mul);
                sor.filter(*out);
            });
    }
    PG_RETURN_TEXT_P(patch_to_wkb_hex_and_free(patch, e.schema, "pcpatch_sor"));
}

/* -- pcpatch_icp(source pcpatch, target pcpatch, ...) RETURNS double[] *
 * Run PCL's IterativeClosestPoint to align @c source onto @c target.
 * Returns an 8-element double[]:
 *   [0..2] tx, ty, tz   — translation
 *   [3]    qw           — quaternion w
 *   [4..6] qx, qy, qz   — quaternion x/y/z
 *   [7]    fitness      — final mean-squared error (PCL's getFitnessScore)
 *
 * Compose into a MobilityDB pose with:
 *   SELECT pose_make_3d(r[1], r[2], r[3], r[4], r[5], r[6], r[7], srid)
 *   FROM pcpatch_icp(src, tgt) r;
 *
 * Optional 3rd / 4th arguments:
 *   max_iterations  (default 50)
 *   max_correspondence_distance (default 1.0, in schema units)
 */
/// Shared body for ICP and GICP. Reg is one of
/// pcl::IterativeClosestPoint<PointXYZ,PointXYZ> or
/// pcl::GeneralizedIterativeClosestPoint<PointXYZ,PointXYZ>; both
/// expose the same setInputSource / setInputTarget / setMaximumIterations
/// / setMaxCorrespondenceDistance / align / getFinalTransformation /
/// getFitnessScore / hasConverged interface.
template<typename Reg>
static Datum run_pcpatch_registration(PG_FUNCTION_ARGS, const char* fn_name)
{
    ensurePcHandlers();

    bytea* srcVar = (bytea*) PG_DETOAST_DATUM(PG_GETARG_DATUM(0));
    bytea* tgtVar = (bytea*) PG_DETOAST_DATUM(PG_GETARG_DATUM(1));
    int32 max_iter = (PG_NARGS() > 2 && !PG_ARGISNULL(2))
                       ? PG_GETARG_INT32(2) : 50;
    double max_corr = (PG_NARGS() > 3 && !PG_ARGISNULL(3))
                        ? PG_GETARG_FLOAT8(3) : 1.0;
    if (max_iter < 1)
        elog(ERROR, "%s: max_iterations must be >= 1 (got %d)", fn_name, max_iter);
    if (!(max_corr > 0.0))
        elog(ERROR, "%s: max_correspondence_distance must be positive "
                    "(got %g)", fn_name, max_corr);

    if (VARSIZE_ANY_EXHDR(srcVar) + VARHDRSZ < kSerializedPatchDataOffset ||
        VARSIZE_ANY_EXHDR(tgtVar) + VARHDRSZ < kSerializedPatchDataOffset)
        elog(ERROR, "%s: pcpatch shorter than SERIALIZED_PATCH header",
             fn_name);
    const PcSerializedPatchHeader* shdr =
        reinterpret_cast<const PcSerializedPatchHeader*>(srcVar);
    const PcSerializedPatchHeader* thdr =
        reinterpret_cast<const PcSerializedPatchHeader*>(tgtVar);
    if (shdr->compression != kCompressionNone ||
        thdr->compression != kCompressionNone)
        elog(ERROR, "%s: only PC_NONE compression supported", fn_name);
    if (shdr->pcid != thdr->pcid)
        elog(ERROR, "%s: source pcid=%u != target pcid=%u",
             fn_name, shdr->pcid, thdr->pcid);

    PCSCHEMA* schema = fetchSchemaForPcid(shdr->pcid);
    DimRefs dims {};
    dims.xd = findDim(schema, "X");
    dims.yd = findDim(schema, "Y");
    dims.zd = findDim(schema, "Z");
    if (!dims.xd || !dims.yd || !dims.zd)
    {
        pc_schema_free(schema);
        elog(ERROR, "%s: schema (pcid=%u) missing X/Y/Z", fn_name, shdr->pcid);
    }
    size_t pointSize = schema->size;
    size_t statsSize = 3 * pointSize;
    const uint8_t* sData = reinterpret_cast<const uint8_t*>(srcVar)
                           + kSerializedPatchDataOffset + statsSize;
    const uint8_t* tData = reinterpret_cast<const uint8_t*>(tgtVar)
                           + kSerializedPatchDataOffset + statsSize;

    auto loadXYZ = [&dims, pointSize](const PcSerializedPatchHeader* hdr,
                                       const uint8_t* data) {
        pcl::PointCloud<pcl::PointXYZ>::Ptr cloud(
            new pcl::PointCloud<pcl::PointXYZ>());
        cloud->reserve(hdr->npoints);
        for (uint32_t i = 0; i < hdr->npoints; ++i)
        {
            const uint8_t* p = data + static_cast<size_t>(i) * pointSize;
            pcl::PointXYZ pt;
            load_point(p, pt, dims);
            cloud->push_back(pt);
        }
        cloud->width = cloud->size(); cloud->height = 1; cloud->is_dense = true;
        return cloud;
    };
    pcl::PointCloud<pcl::PointXYZ>::Ptr src = loadXYZ(shdr, sData);
    pcl::PointCloud<pcl::PointXYZ>::Ptr tgt = loadXYZ(thdr, tData);

    Reg reg;
    reg.setInputSource(src);
    reg.setInputTarget(tgt);
    reg.setMaximumIterations(max_iter);
    reg.setMaxCorrespondenceDistance(max_corr);

    pcl::PointCloud<pcl::PointXYZ> aligned;
    reg.align(aligned);

    pc_schema_free(schema);

    if (!reg.hasConverged())
        elog(WARNING, "%s: did not converge within %d iterations",
             fn_name, max_iter);

    Eigen::Matrix4f T = reg.getFinalTransformation();
    double tx = T(0, 3), ty = T(1, 3), tz = T(2, 3);
    Eigen::Matrix3f R = T.block<3, 3>(0, 0);
    Eigen::Quaternionf q(R);
    q.normalize();
    double qw = q.w(), qx = q.x(), qy = q.y(), qz = q.z();
    double fitness = reg.getFitnessScore();

    double vals[8] = { tx, ty, tz, qw, qx, qy, qz, fitness };
    Datum elems[8];
    for (int i = 0; i < 8; ++i)
        elems[i] = Float8GetDatum(vals[i]);
    int dims_arr[1] = { 8 };
    int lbs[1] = { 1 };
    ArrayType* result = construct_md_array(elems, NULL, 1, dims_arr, lbs,
                                            FLOAT8OID, sizeof(float8),
                                            FLOAT8PASSBYVAL, 'd');
    PG_RETURN_ARRAYTYPE_P(result);
}

extern "C" Datum pcpatch_icp(PG_FUNCTION_ARGS)
{
    return run_pcpatch_registration<
        pcl::IterativeClosestPoint<pcl::PointXYZ, pcl::PointXYZ>>(
            fcinfo, "pcpatch_icp");
}

/* -- pcpatch_gicp(source pcpatch, target pcpatch, ...) RETURNS double[] *
 * Same shape as pcpatch_icp but uses PCL's
 * GeneralizedIterativeClosestPoint, which models each point's local
 * surface as a Gaussian and minimises a Mahalanobis distance. More
 * robust to non-planar surfaces and noisy correspondences than vanilla
 * ICP at the cost of higher per-iteration compute.
 */
extern "C" Datum pcpatch_gicp(PG_FUNCTION_ARGS)
{
    return run_pcpatch_registration<
        pcl::GeneralizedIterativeClosestPoint<pcl::PointXYZ, pcl::PointXYZ>>(
            fcinfo, "pcpatch_gicp");
}

/* -- pcpatch_normals(pcpatch, k int DEFAULT 10) RETURNS double[] ----- *
 * Per-point surface-normal + curvature estimation via PCL's
 * NormalEstimation<PointXYZ, pcl::Normal>, k nearest neighbours.
 * Returns a flat double precision array of length 4 * npoints, laid
 * out as [nx_0, ny_0, nz_0, curv_0, nx_1, ny_1, nz_1, curv_1, ...]
 * (same shape convention as pcpatch_icp's pose array).
 *
 * Use cases:
 *   - oriented-surface change detection: compare per-point normals
 *     across two repeat surveys of the same site;
 *   - Potree high-quality shading export: per-point normals feed the
 *     PotreeConverter --normal flag;
 *   - photogrammetric mesh extraction: normals seed downstream
 *     surface reconstruction (Poisson, MLS).
 *
 * Size note: a 100k-point patch returns a 3.2 MB array. For large
 * patches consider applying pcpatch_voxel_grid first to down-sample,
 * then running normals on the result.
 */
extern "C" Datum pcpatch_normals(PG_FUNCTION_ARGS)
{
    ensurePcHandlers();

    bytea* serpatchVar = (bytea*) PG_DETOAST_DATUM(PG_GETARG_DATUM(0));
    int k = PG_GETARG_INT32(1);
    if (k < 3)
        elog(ERROR, "pcpatch_normals: k must be >= 3 (got %d) — NormalEstimation needs at least 3 neighbours to fit a tangent plane", k);

    FilterEntry e = filter_entry(serpatchVar, "pcpatch_normals");

    pcl::PointCloud<pcl::PointXYZ>::Ptr cloud(
        new pcl::PointCloud<pcl::PointXYZ>());
    cloud->reserve(e.hdr->npoints);
    for (uint32_t i = 0; i < e.hdr->npoints; ++i)
    {
        const uint8_t* p = e.pointData + static_cast<size_t>(i) * e.pointSize;
        pcl::PointXYZ pt;
        load_point(p, pt, e.dims);
        cloud->push_back(pt);
    }
    cloud->width = cloud->size(); cloud->height = 1; cloud->is_dense = true;

    pcl::NormalEstimation<pcl::PointXYZ, pcl::Normal> ne;
    ne.setInputCloud(cloud);
    pcl::search::KdTree<pcl::PointXYZ>::Ptr tree(
        new pcl::search::KdTree<pcl::PointXYZ>());
    ne.setSearchMethod(tree);
    ne.setKSearch(k);

    pcl::PointCloud<pcl::Normal>::Ptr normals(
        new pcl::PointCloud<pcl::Normal>());
    ne.compute(*normals);
    pc_schema_free(e.schema);

    if (normals->size() != cloud->size())
        elog(ERROR, "pcpatch_normals: NormalEstimation returned %zu normals for %zu points",
             normals->size(), cloud->size());

    const size_t nout = 4 * normals->size();
    Datum* elems = (Datum*) palloc(sizeof(Datum) * nout);
    for (size_t i = 0; i < normals->size(); ++i)
    {
        const pcl::Normal& n = normals->points[i];
        elems[4 * i + 0] = Float8GetDatum(static_cast<double>(n.normal_x));
        elems[4 * i + 1] = Float8GetDatum(static_cast<double>(n.normal_y));
        elems[4 * i + 2] = Float8GetDatum(static_cast<double>(n.normal_z));
        elems[4 * i + 3] = Float8GetDatum(static_cast<double>(n.curvature));
    }
    int dims_arr[1] = { static_cast<int>(nout) };
    int lbs[1] = { 1 };
    ArrayType* result = construct_md_array(elems, NULL, 1, dims_arr, lbs,
                                            FLOAT8OID, sizeof(float8),
                                            FLOAT8PASSBYVAL, 'd');
    PG_RETURN_ARRAYTYPE_P(result);
}
