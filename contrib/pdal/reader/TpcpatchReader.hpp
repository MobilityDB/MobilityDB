/**
 * @file TpcpatchReader.hpp
 * @brief PDAL stage <code>readers.tpcpatch</code> — reads MobilityDB
 *        @c tpcpatch values into a PDAL @c PointView.
 *
 * Connects to PostgreSQL via libpq, runs a user-supplied SQL query that
 * yields rows of @c (timestamp, pcpatch, pcid), decodes each pcpatch via
 * pgPointCloud's @c pc_patch_from_wkb (handling @c PC_NONE,
 * @c PC_DIMENSIONAL, and @c PC_LAZPERF transparently), and emits one PDAL
 * point per pcpatch point with the timestamp attached as a @c time_t
 * (microseconds since epoch) dimension.
 *
 * @ingroup pdal_plugin
 */
#pragma once

#include <pdal/Reader.hpp>
#include <pdal/Streamable.hpp>

#include <libpq-fe.h>

#include <cstdint>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

namespace pdal
{

/**
 * @class pdal::TpcpatchReader
 * @brief PDAL @c readers.tpcpatch stage.
 *
 * Streamable reader that decodes pgPointCloud-encoded @c pcpatch
 * values from a libpq result set and emits PDAL points carrying the
 * patch's per-point dimensions plus a @c time_t dimension. Patch
 * decode is delegated to pgPointCloud's @c libpc.a so all three
 * compression types are supported.
 */
class PDAL_EXPORT TpcpatchReader : public Reader, public Streamable
{
public:
    TpcpatchReader() = default;
    ~TpcpatchReader() override;

    TpcpatchReader(const TpcpatchReader&) = delete;
    TpcpatchReader& operator=(const TpcpatchReader&) = delete;

    std::string getName() const override;

private:
    void addArgs(ProgramArgs& args) override;
    void initialize() override;
    void addDimensions(PointLayoutPtr layout) override;
    void ready(PointTableRef table) override;
    point_count_t read(PointViewPtr view, point_count_t count) override;  // Flawfinder: ignore (PDAL Stage::read override, not POSIX read)
    void done(PointTableRef table) override;
    bool processOne(PointRef& point) override;

    void connectIfNeeded();
    void runQuery();
    void* fetchSchemaForPcid(uint32_t pcid); // PCSCHEMA*
    bool advanceRow();
    bool decodeNextPointFromCurrentPatch(PointRef& point);

    std::string m_connection;
    std::string m_query;
    std::string m_timeColumn{"t"};
    std::string m_patchColumn{"pcp"};
    std::string m_pcidColumn{"pcid"};

    PGconn* m_session = nullptr;
    PGresult* m_result = nullptr;

    int m_currentRow = -1;
    int m_totalRows = 0;
    uint64_t m_pointsRead = 0;
    uint64_t m_patchesRead = 0;
    uint64_t m_lastLoggedPatchCount = 0;

    std::unordered_map<uint32_t, void*> m_schemaCache; // PCSCHEMA*

    struct DimResolution
    {
        Dimension::Id id;
        uint32_t byteOffset;
        uint32_t interpretation;
        double scale;
        double offset;
        std::string name;
    };
    std::vector<DimResolution> m_currentPatchDims;

    void* m_currentPatchSchema = nullptr; // PCSCHEMA*
    void* m_currentPatch = nullptr;       // PCPATCH*
    uint32_t m_currentPatchPcid = 0;
    uint64_t m_currentPatchTimestamp = 0;
    int m_currentPatchPointCount = 0;
    int m_currentPatchPointIndex = 0;

    PointLayoutPtr m_layout = nullptr;
    Dimension::Id m_timeDim = Dimension::Id::Unknown;

    void rebuildDimResolution();
    static Dimension::Type interpretationToPdalType(uint32_t interp);
};

} // namespace pdal
