/**
 * @file TpcpatchWriter.cpp
 * @brief Implementation of the PDAL @c writers.tpcpatch stage.
 */
#include "TpcpatchWriter.hpp"

extern "C" {
#include "pc_api.h"
#include "pc_api_internal.h"
}

#include <pdal/PluginHelper.hpp>
#include <pdal/PointView.hpp>
#include <pdal/util/ProgramArgs.hpp>

#include <algorithm>
#include <cstring>
#include <sstream>
#include <stdexcept>

namespace pdal
{

static PluginInfo const s_info{
    "writers.tpcpatch",
    "MobilityDB temporal point-cloud patch (tpcpatch) writer.",
    "https://github.com/MobilityDB/MobilityDB/wiki/"
    "Generalizing-pgPointCloud-types-in-MobilityDB"};

CREATE_SHARED_STAGE(TpcpatchWriter, s_info)

std::string TpcpatchWriter::getName() const { return s_info.name; }

namespace
{

bool g_pcHandlersInstalled = false;
void ensurePcHandlers()
{
    if (!g_pcHandlersInstalled)
    {
        pc_install_default_handlers();
        g_pcHandlersInstalled = true;
    }
}

int compressionFromString(const std::string& s)
{
    std::string l;
    l.reserve(s.size());
    std::transform(s.begin(), s.end(), std::back_inserter(l), ::tolower);
    if (l == "none" || l == "uncompressed" || l == "")
        return PC_NONE;
    if (l == "dimensional" || l == "dim")
        return PC_DIMENSIONAL;
    if (l == "laz" || l == "lazperf")
        return PC_LAZPERF;
    return -1;
}

/* Convert a raw time value to microseconds since the Unix epoch.
 * 'gps_adjusted' implements LAS-1.4 'Adjusted GPS Standard Time':
 *   - +1e9 to recover seconds-since-GPS-epoch (1980-01-06 UTC),
 *   - +315964782 to bridge the GPS-Unix epoch difference
 *     (315964800 - 18 leap seconds in 2024),
 *   - *1e6 to scale to microseconds.
 * The leap-second offset is the value at the time of writing (2024-04);
 * bump it whenever the IERS announces a new leap second. */
uint64_t toUnixMicroseconds(double v, const std::string& fmt)
{
    if (fmt == "unix_microseconds") return static_cast<uint64_t>(v);
    if (fmt == "unix_seconds")      return static_cast<uint64_t>(v * 1.0e6);
    if (fmt == "gps_adjusted")
        return static_cast<uint64_t>((v + 1.0e9 + 315964782.0 - 18.0) * 1.0e6);
    return static_cast<uint64_t>(v); /* validated upstream */
}

} // namespace

TpcpatchWriter::~TpcpatchWriter()
{
    if (m_schema)
        pc_schema_free(static_cast<PCSCHEMA*>(m_schema));
    if (m_session)
        PQfinish(m_session);
}

void TpcpatchWriter::addArgs(ProgramArgs& args)
{
    args.add("connection", "libpq connection string", m_connection)
        .setPositional();
    args.add("table", "destination table name", m_table).setPositional();
    args.add("column", "tpcpatch column name (default 'traj')", m_column);
    args.add("id_column", "optional row-id column name to update in place",
             m_idColumn);
    args.add("id_value", "value for id_column when updating in place",
             m_idValue);
    args.add("pcid",
             "pgPointCloud schema id (pcid) — must match an entry in "
             "pointcloud_formats",
             m_pcid);
    args.add("compression",
             "patch compression: 'none' (default), 'dimensional', 'laz'",
             m_compression, std::string("none"));
    args.add("mode",
             "write mode: 'insert' (default — new row per pipeline run), "
             "'update' (overwrite the row identified by id_column=id_value), "
             "'append' (concatenate via appendSequence to the existing row), "
             "'upsert' (insert if absent, append if present)",
             m_mode, std::string("insert"));
    args.add("time_dim",
             "name of the source time dimension (default 'time_t' — set to "
             "'GpsTime' to read LAS timestamps directly without a "
             "filters.assign stage)",
             m_timeDimName, std::string("time_t"));
    args.add("time_format",
             "interpretation of the source time dimension: "
             "'unix_microseconds' (default), 'unix_seconds', "
             "'gps_adjusted' (LAS-1.4 'Adjusted GPS Standard Time': adds "
             "10^9 + 315964782 - 18 leap-seconds before scaling to "
             "microseconds since Unix epoch). Drone LAS files in ASGT "
             "mode use 'gps_adjusted'.",
             m_timeFormat, std::string("unix_microseconds"));
    args.add("flush_threshold",
             "streaming flush threshold: number of completed per-instant "
             "patches accumulated in memory before issuing an INSERT/append. "
             "Default 1024. Lower values cap libpq transport buffers tighter; "
             "higher values amortise round-trip cost. Honoured for "
             "mode=append/upsert; mode=insert/update flush only at done() "
             "since each pipeline run produces a single row.",
             m_flushThreshold, static_cast<uint32_t>(1024));
}

void TpcpatchWriter::initialize()
{
    if (m_pcid == 0)
        throwError("'pcid' option is required");
    if (compressionFromString(m_compression) < 0)
        throwError("invalid 'compression' value '" + m_compression +
                   "' (expected 'none', 'dimensional', or 'laz')");
    if (m_mode != "insert" && m_mode != "update" &&
        m_mode != "append" && m_mode != "upsert")
        throwError("invalid 'mode' value '" + m_mode +
                   "' (expected 'insert', 'update', 'append', or 'upsert')");
    if ((m_mode == "update" || m_mode == "append" || m_mode == "upsert") &&
        (m_idColumn.empty() || m_idValue.empty()))
        throwError("'mode'='" + m_mode +
                   "' requires both 'id_column' and 'id_value'");
    if (m_timeFormat != "unix_microseconds" && m_timeFormat != "unix_seconds" &&
        m_timeFormat != "gps_adjusted")
        throwError("invalid 'time_format' value '" + m_timeFormat +
                   "' (expected 'unix_microseconds', 'unix_seconds', or "
                   "'gps_adjusted')");
    ensurePcHandlers();
    connectIfNeeded();
    m_schema = fetchSchemaForPcid(m_pcid);

    PCSCHEMA* schema = static_cast<PCSCHEMA*>(m_schema);
    for (uint32_t i = 0; i < schema->ndims; ++i)
    {
        PCDIMENSION* d = schema->dims[i];
        DimResolution r;
        r.id = Dimension::Id::Unknown;
        r.byteOffset = d->byteoffset;
        r.interpretation = d->interpretation;
        r.scale = d->scale;
        r.offset = d->offset;
        r.name = d->name;
        m_schemaDims.push_back(r);
    }
}

void TpcpatchWriter::connectIfNeeded()
{
    if (m_session) return;
    m_session = PQconnectdb(m_connection.c_str());
    if (PQstatus(m_session) != CONNECTION_OK)
    {
        std::string err = PQerrorMessage(m_session);
        PQfinish(m_session);
        m_session = nullptr;
        throwError("libpq connection failed: " + err);
    }
}

void* TpcpatchWriter::fetchSchemaForPcid(uint32_t pcid)
{
    std::ostringstream sql;
    sql << "SELECT schema FROM pointcloud_formats WHERE pcid = " << pcid;
    PGresult* res = PQexec(m_session, sql.str().c_str());
    if (PQresultStatus(res) != PGRES_TUPLES_OK || PQntuples(res) != 1)
    {
        std::string err = PQresultErrorMessage(res);
        PQclear(res);
        throwError("could not fetch schema for pcid=" + std::to_string(pcid) +
                   ": " + err);
    }
    std::string xml(PQgetvalue(res, 0, 0));
    PQclear(res);

    PCSCHEMA* schema = pc_schema_from_xml(xml.c_str());
    if (!schema)
        throwError("pc_schema_from_xml failed for pcid=" +
                   std::to_string(pcid));
    schema->pcid = pcid;
    return schema;
}

void TpcpatchWriter::prepared(PointTableRef table)
{
    PointLayoutPtr layout = table.layout();
    m_timeDim = layout->findDim(m_timeDimName);
    if (m_timeDim == Dimension::Id::Unknown)
        throwError("input is missing the configured time dimension '" +
                   m_timeDimName + "'. Set 'time_dim' to the source dim "
                   "(e.g. 'GpsTime' for LAS) and 'time_format' to its "
                   "epoch convention ('unix_microseconds', 'unix_seconds', "
                   "or 'gps_adjusted'), or insert a filters.assign stage "
                   "that derives 'time_t' (Unsigned64 microseconds since "
                   "Unix epoch).");
    for (auto& d : m_schemaDims)
    {
        d.id = layout->findDim(d.name);
        if (d.id == Dimension::Id::Unknown)
            throwError("input is missing dimension '" + d.name +
                       "' required by pcid=" + std::to_string(m_pcid));
    }
}

void TpcpatchWriter::ready(PointTableRef /*table*/) {}

std::string TpcpatchWriter::hexEncode(const std::vector<uint8_t>& bytes)
{
    static const char* k = "0123456789ABCDEF";
    std::string out;
    out.resize(bytes.size() * 2);
    for (size_t i = 0; i < bytes.size(); ++i)
    {
        out[2 * i] = k[(bytes[i] >> 4) & 0x0F];
        out[2 * i + 1] = k[bytes[i] & 0x0F];
    }
    return out;
}

void TpcpatchWriter::appendPointToCurrentBucket(uint64_t ts,
                                                 const PointRef& point)
{
    PCSCHEMA* schema = static_cast<PCSCHEMA*>(m_schema);
    if (m_currentBucketCount == 0)
        m_currentBucketTs = ts;
    if (!m_currentBucketPL)
        m_currentBucketPL = pc_pointlist_make(64);

    PCPOINT* pt = pc_point_make(schema);
    for (auto& d : m_schemaDims)
    {
        double v = point.getFieldAs<double>(d.id);
        v = (v - d.offset) / d.scale;
        pc_double_to_ptr(pt->data + d.byteOffset, d.interpretation, v);
    }
    pc_pointlist_add_point(static_cast<PCPOINTLIST*>(m_currentBucketPL), pt);
    m_currentBucketCount++;
}

void TpcpatchWriter::encodeAndQueueCurrentBucket()
{
    if (m_currentBucketCount == 0) return;
    PCSCHEMA* schema = static_cast<PCSCHEMA*>(m_schema);
    PCPOINTLIST* pl = static_cast<PCPOINTLIST*>(m_currentBucketPL);

    PCPATCH* upatch = (PCPATCH*) pc_patch_uncompressed_from_pointlist(pl);
    pc_pointlist_free(pl);
    m_currentBucketPL = nullptr;
    if (!upatch)
        throw std::runtime_error("pc_patch_uncompressed_from_pointlist failed");

    PCPATCH* finalPatch = upatch;
    int comp = compressionFromString(m_compression);
    if (comp != PC_NONE)
    {
        uint32_t saved = schema->compression;
        schema->compression = static_cast<uint32_t>(comp);
        finalPatch = pc_patch_compress(upatch, nullptr);
        schema->compression = saved;
        if (finalPatch && finalPatch != upatch)
            pc_patch_free(upatch);
        if (!finalPatch)
            throw std::runtime_error("pc_patch_compress failed");
    }

    size_t wkbsize = 0;
    uint8_t* wkb = pc_patch_to_wkb(finalPatch, &wkbsize);
    if (!wkb || wkbsize == 0)
    {
        pc_patch_free(finalPatch);
        throw std::runtime_error("pc_patch_to_wkb returned empty");
    }
    m_pendingTs.push_back(m_currentBucketTs);
    m_pendingPatches.emplace_back(wkb, wkb + wkbsize);
    pcfree(wkb);
    pc_patch_free(finalPatch);

    m_pointsWritten += m_currentBucketCount;
    m_currentBucketCount = 0;
}

void TpcpatchWriter::flushPending(bool finalFlush)
{
    if (m_pendingTs.empty()) return;
    /* mode=insert / mode=update both produce a single row per pipeline
     * run, so we cannot flush mid-stream — accumulate everything until
     * done() runs the single INSERT/UPDATE. mode=append / mode=upsert
     * fold each batch into the same row via merge() so any flush is
     * safe; honour the threshold. */
    const bool deferred = (m_mode == "insert" || m_mode == "update");
    if (!finalFlush && deferred) return;
    if (!finalFlush && m_pendingTs.size() < m_flushThreshold) return;

    insertTpcpatch(m_pendingTs, m_pendingPatches, m_batchesFlushed == 0);
    m_patchesWritten += m_pendingTs.size();
    m_batchesFlushed++;
    log()->get(LogLevel::Info)
        << getName() << ": batch " << m_batchesFlushed
        << " flushed " << m_pendingTs.size() << " patches ("
        << m_pointsWritten << " pts / " << m_patchesWritten
        << " patches total, mode=" << m_mode << ")"
        << std::endl;
    m_pendingTs.clear();
    m_pendingPatches.clear();
}

void TpcpatchWriter::insertTpcpatch(
    const std::vector<uint64_t>& timestamps,
    const std::vector<std::vector<uint8_t>>& patches,
    bool isFirstBatch)
{
    if (timestamps.empty()) return;

    /* Build the tpcpatch literal expression once; every mode reuses it.
     * For append/update/upsert we always wrap in tpcpatchSeq so the new
     * value carries Sequence subtype, matching the column for merge().
     * For mode=insert, keep the bare Instant literal when there's only
     * one timestamp — that is the simpler stored form. */
    const bool wrapAsSequence =
        (m_mode != "insert") || timestamps.size() > 1;
    std::ostringstream lit;
    if (! wrapAsSequence)
    {
        lit << "tpcpatch('" << hexEncode(patches[0])
            << "'::pcpatch, to_timestamp(" << timestamps[0]
            << "::numeric / 1000000.0)::timestamptz)";
    }
    else
    {
        lit << "tpcpatchSeq(ARRAY[";
        for (size_t i = 0; i < timestamps.size(); ++i)
        {
            if (i > 0) lit << ", ";
            lit << "tpcpatch('" << hexEncode(patches[i])
                << "'::pcpatch, to_timestamp(" << timestamps[i]
                << "::numeric / 1000000.0)::timestamptz)";
        }
        lit << "])";
    }
    const std::string newPatch = lit.str();

    std::ostringstream sql;
    /* Legacy compatibility: id_column without an explicit mode is the
     * pre-existing UPDATE behaviour. */
    std::string mode = m_mode;
    if (mode == "insert" && !m_idColumn.empty())
        mode = "update";
    /* Subsequent batches in append/upsert always operate on the
     * already-existing row via merge(); keep using append form. */
    if (!isFirstBatch && mode == "upsert")
        mode = "append";

    if (mode == "insert")
    {
        sql << "INSERT INTO " << m_table << " (" << m_column
            << ") VALUES (" << newPatch << ")";
    }
    else if (mode == "update")
    {
        sql << "UPDATE " << m_table << " SET " << m_column << " = "
            << newPatch
            << " WHERE " << m_idColumn << " = " << m_idValue;
    }
    else if (mode == "append")
    {
        /* merge() is the only tpcpatch concatenation primitive exposed
         * as a SQL aggregate — feed (existing UNION ALL new) through it. */
        sql << "UPDATE " << m_table << " t SET " << m_column
            << " = (SELECT merge(p) FROM ("
            << "SELECT " << m_column << " AS p FROM " << m_table
            <<   " WHERE " << m_idColumn << " = " << m_idValue
            << " UNION ALL SELECT " << newPatch << " AS p"
            << ") s)"
            << " WHERE t." << m_idColumn << " = " << m_idValue;
    }
    else /* upsert */
    {
        /* INSERT … ON CONFLICT DO UPDATE is atomic; the merge subquery
         * sees the existing row's value (table-qualified) and the
         * proposed-new-value (EXCLUDED). Each successive pipeline run
         * appends one more flight onto the same row. */
        sql << "INSERT INTO " << m_table << " ("
            <<   m_idColumn << ", " << m_column << ") VALUES ("
            <<   m_idValue << ", " << newPatch << ") "
            <<   "ON CONFLICT (" << m_idColumn << ") DO UPDATE SET "
            <<   m_column << " = (SELECT merge(p) FROM ("
            <<     "SELECT " << m_table << "." << m_column << " AS p"
            <<     " UNION ALL SELECT EXCLUDED." << m_column << " AS p"
            <<   ") s)";
    }

    PGresult* res = PQexec(m_session, sql.str().c_str());
    if (PQresultStatus(res) != PGRES_COMMAND_OK)
    {
        std::string err = PQresultErrorMessage(res);
        PQclear(res);
        throwError(mode + " failed: " + err);
    }

    /* Confirm the UPDATE / append actually matched a row — silent
     * 0-row updates are the most painful silent failure mode. */
    if (mode == "update" || mode == "append")
    {
        const char* tag = PQcmdTuples(res);
        if (tag && std::string(tag) == "0")
        {
            PQclear(res);
            throwError("'mode'='" + mode + "' matched no row in '" +
                       m_table + "' WHERE " + m_idColumn + " = " +
                       m_idValue + " (use 'mode'='insert' or 'upsert' to "
                       "create the row)");
        }
    }
    PQclear(res);
}

bool TpcpatchWriter::processOne(PointRef& point)
{
    if (m_timeDim == Dimension::Id::Unknown)
        throwError("processOne() called before prepared()");
    uint64_t ts = toUnixMicroseconds(
        point.getFieldAs<double>(m_timeDim), m_timeFormat);
    /* Timestamp boundary: close out the previous bucket and check the
     * flush threshold. The pending list grows under append/upsert until
     * m_flushThreshold completed patches; insert/update accumulate the
     * full pipeline before the single done()-time INSERT/UPDATE. */
    if (m_currentBucketCount > 0 && ts != m_currentBucketTs)
    {
        encodeAndQueueCurrentBucket();
        flushPending(false);
    }
    appendPointToCurrentBucket(ts, point);
    return true;
}

void TpcpatchWriter::write(const PointViewPtr view)
{
    /* Non-streaming path: PDAL has accumulated the full PointView into
     * memory before calling write(). Iterate and feed processOne so the
     * encoding and flushing logic stays in one place. The OOM risk of
     * a multi-GB LAS file under non-streaming mode is on PDAL's side
     * (the PointView itself); the writer's own footprint is bounded by
     * one bucket of points + m_flushThreshold encoded WKB strings. */
    if (!view || view->size() == 0) return;
    PointRef p(*view, 0);
    for (PointId i = 0; i < view->size(); ++i)
    {
        p.setPointId(i);
        processOne(p);
    }
}

void TpcpatchWriter::done(PointTableRef /*table*/)
{
    /* Final-flush the pending tail: close any open bucket, then flush
     * everything regardless of threshold or deferred-mode semantics. */
    if (m_currentBucketCount > 0)
        encodeAndQueueCurrentBucket();
    flushPending(true);

    if (m_currentBucketPL)
    {
        pc_pointlist_free(static_cast<PCPOINTLIST*>(m_currentBucketPL));
        m_currentBucketPL = nullptr;
    }
    log()->get(LogLevel::Info)
        << getName() << ": done — total " << m_pointsWritten
        << " points across " << m_patchesWritten
        << " per-instant patches in " << m_batchesFlushed
        << " batch(es) into " << m_table << "." << m_column
        << " (pcid " << m_pcid << ", compression=" << m_compression
        << ", mode=" << m_mode
        << ", flush_threshold=" << m_flushThreshold << ")"
        << std::endl;
    if (m_session) { PQfinish(m_session); m_session = nullptr; }
}

} // namespace pdal
