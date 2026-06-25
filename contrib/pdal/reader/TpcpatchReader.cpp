/**
 * @file TpcpatchReader.cpp
 * @brief Implementation of the PDAL @c readers.tpcpatch stage.
 */
#include "TpcpatchReader.hpp"

extern "C" {
#include "pc_api.h"
#include "pc_api_internal.h"
}

#include <pdal/PluginHelper.hpp>
#include <pdal/PointView.hpp>
#include <pdal/util/ProgramArgs.hpp>

#include <cstring>
#include <sstream>
#include <stdexcept>

namespace pdal
{

static PluginInfo const s_info{
    "readers.tpcpatch",
    "MobilityDB temporal point-cloud patch (tpcpatch) reader.",
    "https://github.com/MobilityDB/MobilityDB/wiki/"
    "Generalizing-pgPointCloud-types-in-MobilityDB"};

CREATE_SHARED_STAGE(TpcpatchReader, s_info)

std::string TpcpatchReader::getName() const { return s_info.name; }

namespace
{

inline uint8_t hexValue(char c)
{
    if (c >= '0' && c <= '9') return static_cast<uint8_t>(c - '0');
    if (c >= 'a' && c <= 'f') return static_cast<uint8_t>(c - 'a' + 10);
    if (c >= 'A' && c <= 'F') return static_cast<uint8_t>(c - 'A' + 10);
    throw std::invalid_argument("non-hex character in patch bytes");
}

std::vector<uint8_t> hexDecode(const char* hex)
{
    if (hex[0] == '\\' && hex[1] == 'x')
        hex += 2;
    size_t len = std::strlen(hex);
    if (len % 2 != 0)
        throw std::invalid_argument("odd-length hex patch payload");
    std::vector<uint8_t> out(len / 2);
    for (size_t i = 0; i < len; i += 2)
        out[i / 2] = static_cast<uint8_t>((hexValue(hex[i]) << 4) |
                                          hexValue(hex[i + 1]));
    return out;
}

uint64_t parsePostgresTimestamp(const char* value)
{
    char* end = nullptr;
    unsigned long long v = std::strtoull(value, &end, 10);
    return (end == value) ? 0 : static_cast<uint64_t>(v);
}

bool g_pcHandlersInstalled = false;

void ensurePcHandlers()
{
    if (!g_pcHandlersInstalled)
    {
        pc_install_default_handlers();
        g_pcHandlersInstalled = true;
    }
}

} // namespace

TpcpatchReader::~TpcpatchReader()
{
    if (m_currentPatch)
        pc_patch_free(static_cast<PCPATCH*>(m_currentPatch));
    for (auto& kv : m_schemaCache)
        pc_schema_free(static_cast<PCSCHEMA*>(kv.second));
    if (m_result)
        PQclear(m_result);
    if (m_session)
        PQfinish(m_session);
}

void TpcpatchReader::addArgs(ProgramArgs& args)
{
    args.add("connection", "libpq connection string", m_connection)
        .setPositional();
    args.add("query",
             "SQL query returning rows of (timestamp, pcpatch, pcid). "
             "Timestamp must be expressed as microseconds since epoch.",
             m_query)
        .setPositional();
    args.add("time_column", "name of the timestamp column", m_timeColumn);
    args.add("patch_column", "name of the pcpatch column", m_patchColumn);
    args.add("pcid_column", "name of the pcid column", m_pcidColumn);
}

void TpcpatchReader::initialize()
{
    ensurePcHandlers();
    connectIfNeeded();
}

void TpcpatchReader::connectIfNeeded()
{
    if (m_session)
        return;
    m_session = PQconnectdb(m_connection.c_str());
    if (PQstatus(m_session) != CONNECTION_OK)
    {
        std::string err = PQerrorMessage(m_session);
        PQfinish(m_session);
        m_session = nullptr;
        throwError("libpq connection failed: " + err);
    }
}

void* TpcpatchReader::fetchSchemaForPcid(uint32_t pcid)
{
    auto it = m_schemaCache.find(pcid);
    if (it != m_schemaCache.end())
        return it->second;

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
    m_schemaCache.emplace(pcid, schema);
    return schema;
}

void TpcpatchReader::runQuery()
{
    if (m_result) { PQclear(m_result); m_result = nullptr; }
    m_result = PQexec(m_session, m_query.c_str());
    if (PQresultStatus(m_result) != PGRES_TUPLES_OK)
    {
        std::string err = PQresultErrorMessage(m_result);
        PQclear(m_result); m_result = nullptr;
        throwError("query failed: " + err);
    }
    m_totalRows = PQntuples(m_result);
    m_currentRow = -1;
}

Dimension::Type TpcpatchReader::interpretationToPdalType(uint32_t interp)
{
    switch (interp)
    {
    case PC_INT8:    return Dimension::Type::Signed8;
    case PC_UINT8:   return Dimension::Type::Unsigned8;
    case PC_INT16:   return Dimension::Type::Signed16;
    case PC_UINT16:  return Dimension::Type::Unsigned16;
    case PC_INT32:   return Dimension::Type::Signed32;
    case PC_UINT32:  return Dimension::Type::Unsigned32;
    case PC_INT64:   return Dimension::Type::Signed64;
    case PC_UINT64:  return Dimension::Type::Unsigned64;
    case PC_DOUBLE:  return Dimension::Type::Double;
    case PC_FLOAT:   return Dimension::Type::Float;
    default:         return Dimension::Type::None;
    }
}

void TpcpatchReader::addDimensions(PointLayoutPtr layout)
{
    m_layout = layout;
    runQuery();
    if (m_totalRows == 0)
    {
        m_timeDim = layout->registerOrAssignDim("time_t",
                                                Dimension::Type::Unsigned64);
        return;
    }

    int pcidCol = PQfnumber(m_result, m_pcidColumn.c_str());
    if (pcidCol < 0)
        throwError("query result is missing pcid column '" + m_pcidColumn +
                   "'");
    uint32_t pcid =
        static_cast<uint32_t>(std::stoul(PQgetvalue(m_result, 0, pcidCol)));
    PCSCHEMA* schema = static_cast<PCSCHEMA*>(fetchSchemaForPcid(pcid));

    for (uint32_t i = 0; i < schema->ndims; ++i)
    {
        PCDIMENSION* d = schema->dims[i];
        Dimension::Type t = interpretationToPdalType(d->interpretation);
        if (t != Dimension::Type::None)
            layout->registerOrAssignDim(d->name, t);
    }
    m_timeDim =
        layout->registerOrAssignDim("time_t", Dimension::Type::Unsigned64);
}

void TpcpatchReader::ready(PointTableRef /*table*/)
{
    m_currentRow = -1;
    m_currentPatchPointIndex = 0;
    m_currentPatchPointCount = 0;
    if (m_currentPatch)
    {
        pc_patch_free(static_cast<PCPATCH*>(m_currentPatch));
        m_currentPatch = nullptr;
    }
}

void TpcpatchReader::rebuildDimResolution()
{
    m_currentPatchDims.clear();
    if (!m_currentPatchSchema) return;
    PCSCHEMA* schema = static_cast<PCSCHEMA*>(m_currentPatchSchema);
    for (uint32_t i = 0; i < schema->ndims; ++i)
    {
        PCDIMENSION* d = schema->dims[i];
        DimResolution r;
        r.id = m_layout ? m_layout->findDim(d->name) : Dimension::Id::Unknown;
        r.byteOffset = d->byteoffset;
        r.interpretation = d->interpretation;
        r.scale = d->scale;
        r.offset = d->offset;
        r.name = d->name;
        m_currentPatchDims.push_back(r);
    }
}

bool TpcpatchReader::advanceRow()
{
    m_currentRow++;
    if (m_currentRow >= m_totalRows)
        return false;

    int patchCol = PQfnumber(m_result, m_patchColumn.c_str());
    int timeCol  = PQfnumber(m_result, m_timeColumn.c_str());
    int pcidCol  = PQfnumber(m_result, m_pcidColumn.c_str());
    if (patchCol < 0 || timeCol < 0 || pcidCol < 0)
        throwError("query result is missing required columns");

    uint32_t pcid = static_cast<uint32_t>(
        std::stoul(PQgetvalue(m_result, m_currentRow, pcidCol)));
    if (pcid != m_currentPatchPcid)
    {
        m_currentPatchSchema = fetchSchemaForPcid(pcid);
        m_currentPatchPcid = pcid;
        rebuildDimResolution();
    }

    m_currentPatchTimestamp = parsePostgresTimestamp(
        PQgetvalue(m_result, m_currentRow, timeCol));

    const char* patchHex = PQgetvalue(m_result, m_currentRow, patchCol);
    std::vector<uint8_t> bytes = hexDecode(patchHex);

    if (m_currentPatch)
    {
        pc_patch_free(static_cast<PCPATCH*>(m_currentPatch));
        m_currentPatch = nullptr;
    }
    PCPATCH* patch = pc_patch_from_wkb(
        static_cast<PCSCHEMA*>(m_currentPatchSchema),
        bytes.data(), bytes.size());
    if (!patch)
        throwError("pc_patch_from_wkb failed at row " +
                   std::to_string(m_currentRow));
    m_currentPatch = patch;

    m_currentPatchPointCount = static_cast<int>(patch->npoints);
    m_currentPatchPointIndex = 0;
    m_patchesRead++;
    m_pointsRead += static_cast<uint64_t>(patch->npoints);
    if (m_patchesRead - m_lastLoggedPatchCount >= 1000)
    {
        log()->get(LogLevel::Info)
            << getName() << ": " << m_patchesRead << " patches / "
            << m_pointsRead << " points decoded so far"
            << std::endl;
        m_lastLoggedPatchCount = m_patchesRead;
    }
    return true;
}

bool TpcpatchReader::decodeNextPointFromCurrentPatch(PointRef& point)
{
    if (m_currentPatchPointIndex >= m_currentPatchPointCount)
        return false;

    PCPOINT* pt = pc_patch_pointn(
        static_cast<PCPATCH*>(m_currentPatch),
        m_currentPatchPointIndex + 1);
    if (!pt)
        throwError("pc_patch_pointn returned null at index " +
                   std::to_string(m_currentPatchPointIndex));

    for (auto& d : m_currentPatchDims)
    {
        if (d.id == Dimension::Id::Unknown)
            continue;
        double v = pc_double_from_ptr(pt->data + d.byteOffset, d.interpretation);
        v = v * d.scale + d.offset;
        point.setField(d.id, v);
    }
    point.setField(m_timeDim, m_currentPatchTimestamp);

    pc_point_free(pt);
    m_currentPatchPointIndex++;
    return true;
}

bool TpcpatchReader::processOne(PointRef& point)
{
    while (true)
    {
        if (m_currentPatchPointIndex < m_currentPatchPointCount)
            return decodeNextPointFromCurrentPatch(point);
        if (!advanceRow())
            return false;
    }
}

point_count_t TpcpatchReader::read(PointViewPtr view, point_count_t count)  // Flawfinder: ignore (PDAL Stage::read override, not POSIX read)
{
    point_count_t emitted = 0;
    PointId id = view->size();
    PointRef point(*view, id);
    while (emitted < count)
    {
        point.setPointId(id);
        if (!processOne(point))
            break;
        ++id;
        ++emitted;
    }
    return emitted;
}

void TpcpatchReader::done(PointTableRef /*table*/)
{
    log()->get(LogLevel::Info)
        << getName() << ": done — total " << m_pointsRead
        << " points across " << m_patchesRead << " patches"
        << std::endl;
    if (m_currentPatch)
    {
        pc_patch_free(static_cast<PCPATCH*>(m_currentPatch));
        m_currentPatch = nullptr;
    }
    if (m_result) { PQclear(m_result); m_result = nullptr; }
    if (m_session) { PQfinish(m_session); m_session = nullptr; }
}

} // namespace pdal
