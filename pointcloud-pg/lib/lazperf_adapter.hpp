/***********************************************************************
 * lazperf_adapter.hpp
 *
 *				LazPerf compression/decompression
 *
 *	Copyright (c) 2016 Paul Blottiere, Oslandia
 *
 ***********************************************************************/

#pragma once

#include "pc_api_internal.h"

#ifdef HAVE_LAZPERF
#include <laz-perf/common/common.hpp>
#include <laz-perf/compressor.hpp>
#include <laz-perf/decoder.hpp>
#include <laz-perf/decompressor.hpp>
#include <laz-perf/encoder.hpp>
#include <laz-perf/formats.hpp>
#include <laz-perf/las.hpp>

/**********************************************************************
 * C API
 */
#include "lazperf_adapter.h"

/**********************************************************************
 * INTERNAL CPP
 */
// utility functions
void lazperf_dump(uint8_t *data, const size_t size);
void lazperf_dump(const PCPATCH_UNCOMPRESSED *p);
void lazperf_dump(const PCPATCH_LAZPERF *p);

// struct which capture data coming from the compressor
struct LazPerfBuf
{
  LazPerfBuf() : buf(), idx(0) {}

  const uint8_t *data()
  {
    return reinterpret_cast<const uint8_t *>(buf.data());
  }

  void putBytes(const unsigned char *b, size_t len)
  {
    while (len--)
    {
      buf.push_back(*b++);
    }
  }

  void putByte(const unsigned char b) { buf.push_back(b); }

  unsigned char getByte() { return buf[idx++]; }

  void getBytes(unsigned char *b, int len)
  {
    for (int i = 0; i < len; i++)
    {
      b[i] = getByte();
    }
  }

  std::vector<unsigned char> buf;
  size_t idx;
};

// some typedef
typedef laszip::encoders::arithmetic<LazPerfBuf> Encoder;
typedef laszip::decoders::arithmetic<LazPerfBuf> Decoder;

typedef laszip::formats::dynamic_field_compressor<Encoder>::ptr Compressor;
typedef laszip::formats::dynamic_field_decompressor<Decoder>::ptr Decompressor;

// LazPerf class
template <typename LazPerfEngine, typename LazPerfCoder> class LazPerf
{

public:
  LazPerf(const PCSCHEMA *pcschema, LazPerfBuf &buf);
  ~LazPerf();

  size_t pointsize() const { return _pointsize; }

protected:
  void initSchema();
  bool addField(const PCDIMENSION *dim);

  const PCSCHEMA *_pcschema;
  LazPerfCoder _coder;
  LazPerfEngine _engine;
  size_t _pointsize;
};

// compressor
class LazPerfCompressor : public LazPerf<Compressor, Encoder>
{

public:
  LazPerfCompressor(const PCSCHEMA *pcschema, LazPerfBuf &output);
  ~LazPerfCompressor();

  size_t compress(const uint8_t *input, const size_t inputsize);
};

// decompressor
class LazPerfDecompressor : public LazPerf<Decompressor, Decoder>
{

public:
  LazPerfDecompressor(const PCSCHEMA *pcschema, LazPerfBuf &input);
  ~LazPerfDecompressor();

  size_t decompress(uint8_t *data, const size_t datasize);
};
#endif // HAVE_LAZPERF
