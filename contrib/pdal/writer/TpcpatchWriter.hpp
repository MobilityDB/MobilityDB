/**
 * @file TpcpatchWriter.hpp
 * @brief PDAL stage <code>writers.tpcpatch</code> — writes a PDAL
 *        @c PointView into a MobilityDB @c tpcpatch column.
 *
 * Groups the input points by their @c time_t dimension into per-timestamp
 * @c pcpatch values, encodes each via pgPointCloud's
 * @c pc_patch_to_wkb (supporting @c PC_NONE, @c PC_DIMENSIONAL,
 * @c PC_LAZPERF compression), and issues an @c INSERT or @c UPDATE
 * against the destination table using MobilityDB's
 * @c tpcpatch(pcpatch,timestamptz) and @c tpcpatchSeq(pcpatch[])
 * constructors.
 *
 * @ingroup pdal_plugin
 */
#pragma once

#include <pdal/Writer.hpp>
#include <pdal/Streamable.hpp>

#include <libpq-fe.h>

#include <cstdint>
#include <string>
#include <vector>

namespace pdal
{

/**
 * @class pdal::TpcpatchWriter
 * @brief PDAL @c writers.tpcpatch stage.
 *
 * Streamable writer: groups consecutive same-timestamp points into one
 * pgPointCloud @c pcpatch via @c pc_patch_to_wkb as the points arrive,
 * accumulating completed patches in WKB-hex form. Bounded by
 * @c flush_threshold patches, the accumulator is flushed to PostgreSQL
 * via MobilityDB's @c tpcpatch / @c tpcpatchSeq constructors. For
 * @c mode=insert / @c mode=update the tail flush in @c done() emits a
 * single @c INSERT or @c UPDATE; for @c mode=append / @c mode=upsert
 * each batch flush appends incrementally via @c merge() so the live
 * memory footprint stays @c O(flush_threshold) regardless of survey
 * size — the design point that makes multi-GB LAS ingest
 * tractable.
 */
class PDAL_EXPORT TpcpatchWriter : public NoFilenameWriter, public Streamable
{
public:
    TpcpatchWriter() = default;
    ~TpcpatchWriter() override;

    TpcpatchWriter(const TpcpatchWriter&) = delete;
    TpcpatchWriter& operator=(const TpcpatchWriter&) = delete;

    std::string getName() const override;

private:
    void addArgs(ProgramArgs& args) override;
    void initialize() override;
    void prepared(PointTableRef table) override;
    void ready(PointTableRef table) override;
    void write(const PointViewPtr view) override;
    bool processOne(PointRef& point) override;
    void done(PointTableRef table) override;

    void connectIfNeeded();
    void* fetchSchemaForPcid(uint32_t pcid); // PCSCHEMA*
    void appendPointToCurrentBucket(uint64_t ts, const PointRef& point);
    void encodeAndQueueCurrentBucket();
    void flushPending(bool finalFlush);
    void insertTpcpatch(const std::vector<uint64_t>& timestamps,
                        const std::vector<std::vector<uint8_t>>& patches,
                        bool isFirstBatch);
    static std::string hexEncode(const std::vector<uint8_t>& bytes);

    std::string m_connection;
    std::string m_table;
    std::string m_column{"traj"};
    std::string m_idColumn;
    std::string m_idValue;
    std::string m_compression{"none"};  // none / dimensional / laz
    std::string m_mode{"insert"};       // insert / update / append / upsert
    std::string m_timeDimName{"time_t"};
    std::string m_timeFormat{"unix_microseconds"};
    uint32_t m_pcid = 0;
    uint32_t m_flushThreshold = 1024;
    uint64_t m_patchesWritten = 0;
    uint64_t m_pointsWritten = 0;
    uint64_t m_batchesFlushed = 0;

    PGconn* m_session = nullptr;
    void* m_schema = nullptr; // PCSCHEMA*

    void* m_currentBucketPL = nullptr;     // PCPOINTLIST* — points sharing m_currentBucketTs
    uint64_t m_currentBucketTs = 0;
    uint32_t m_currentBucketCount = 0;
    std::vector<uint64_t> m_pendingTs;
    std::vector<std::vector<uint8_t>> m_pendingPatches;

    struct DimResolution
    {
        Dimension::Id id;
        uint32_t byteOffset;
        uint32_t interpretation;
        double scale;
        double offset;
        std::string name;
    };
    std::vector<DimResolution> m_schemaDims;
    Dimension::Id m_timeDim = Dimension::Id::Unknown;
};

} // namespace pdal
