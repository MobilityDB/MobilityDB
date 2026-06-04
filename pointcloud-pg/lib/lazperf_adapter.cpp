/***********************************************************************
 * lazperf_adapter.cpp
 *
 *  LazPerf compression/decompression
 *
 *  Copyright (c) 2016 Paul Blottiere, Oslandia
 *
 ***********************************************************************/

#include "lazperf_adapter.hpp"

#ifdef HAVE_LAZPERF

/**********************************************************************
 * C API
 */
size_t lazperf_compress_from_uncompressed(const PCPATCH_UNCOMPRESSED *pa,
                                          uint8_t **compressed)
{
  size_t size = 1;

  LazPerfBuf buf;
  LazPerfCompressor engine(pa->schema, buf);

  if (engine.compress(pa->data, pa->datasize) == pa->npoints)
  {
    size = buf.buf.size();
    *compressed = (uint8_t *)malloc(size);
    *compressed = (uint8_t *)memcpy(*compressed, buf.data(), size);
  }

  // log
  // lazperf_dump(pa);
  // lazperf_dump(*compressed, size);

  return size;
}

size_t lazperf_uncompress_from_compressed(const PCPATCH_LAZPERF *pa,
                                          uint8_t **decompressed)
{
  size_t size = -1;
  size_t datasize = pa->schema->size * pa->npoints;

  LazPerfBuf buf;
  buf.putBytes(pa->lazperf, pa->lazperfsize);
  LazPerfDecompressor engine(pa->schema, buf);

  *decompressed = (uint8_t *)malloc(datasize);

  if (engine.decompress(*decompressed, datasize) == pa->npoints)
    size = buf.buf.size();

  // log
  // lazperf_dump(pa);
  // lazperf_dump(*decompressed, datasize);

  return size;
}

/**********************************************************************
 * INTERNAL CPP
 */
// utility functions
void lazperf_dump(uint8_t *data, const size_t size)
{
  std::cout << "DUMP DATA: " << std::endl;
  std::cout << "	- datasize: " << size << std::endl;
  std::cout << "	- data: ";
  for (int i = 0; i < size; ++i)
    printf("%02x ", data[i]);
  std::cout << std::endl;
}

void lazperf_dump(const PCPATCH_UNCOMPRESSED *p)
{
  std::cout << std::endl;
  std::cout << "DUMP UNCOMPRESSED PATCH: " << std::endl;
  std::cout << "	- type: " << p->type << std::endl;
  std::cout << "	- schema->size " << p->schema->size << std::endl;
  std::cout << "	- readonly: " << p->readonly << std::endl;
  std::cout << "	- npoints: " << p->npoints << std::endl;
  std::cout << "	- maxpoints: " << p->maxpoints << std::endl;
  std::cout << "	- datasize: " << p->datasize << std::endl;
  std::cout << "	- data: ";
  for (int i = 0; i < p->datasize; ++i)
    printf("%02x ", p->data[i]);
  std::cout << std::endl;
}

void lazperf_dump(const PCPATCH_LAZPERF *p)
{
  std::cout << std::endl;
  std::cout << "DUMP LAZPERF PATCH: " << std::endl;
  std::cout << "	- type: " << p->type << std::endl;
  std::cout << "	- schema->size " << p->schema->size << std::endl;
  std::cout << "	- readonly: " << p->readonly << std::endl;
  std::cout << "	- npoints: " << p->npoints << std::endl;
  std::cout << "	- lazperfsize: " << p->lazperfsize << std::endl;
  std::cout << "	- lazperf: ";
  for (int i = 0; i < p->lazperfsize; ++i)
    printf("%02x ", p->lazperf[i]);
  std::cout << std::endl;
}

// LazPerf class
template <typename LazPerfEngine, typename LazPerfCoder>
LazPerf<LazPerfEngine, LazPerfCoder>::LazPerf(const PCSCHEMA *pcschema,
                                              LazPerfBuf &buf)
    : _pcschema(pcschema), _coder(buf), _pointsize(0)
{
}

template <typename LazPerfEngine, typename LazPerfCoder>
LazPerf<LazPerfEngine, LazPerfCoder>::~LazPerf()
{
}

template <typename LazPerfEngine, typename LazPerfCoder>
void LazPerf<LazPerfEngine, LazPerfCoder>::initSchema()
{
  for (int i = 0; i < _pcschema->ndims; i++)
    addField(_pcschema->dims[i]);
}

template <typename LazPerfEngine, typename LazPerfCoder>
bool LazPerf<LazPerfEngine, LazPerfCoder>::addField(const PCDIMENSION *dim)
{
  bool rc = true;

  switch (dim->interpretation)
  {
  case PC_INT8:
  {
    _engine->template add_field<I8>();
    break;
  }
  case PC_UINT8:
  {
    _engine->template add_field<U8>();
    break;
  }
  case PC_INT16:
  {
    _engine->template add_field<I16>();
    break;
  }
  case PC_UINT16:
  {
    _engine->template add_field<U16>();
    break;
  }
  case PC_INT32:
  {
    _engine->template add_field<I32>();
    break;
  }
  case PC_UINT32:
  {
    _engine->template add_field<U32>();
    break;
  }
  case PC_INT64:
  {
    _engine->template add_field<I32>();
    _engine->template add_field<I32>();
    break;
  }
  case PC_UINT64:
  {
    _engine->template add_field<U32>();
    _engine->template add_field<U32>();
    break;
  }
  case PC_DOUBLE:
  {
    _engine->template add_field<U32>();
    _engine->template add_field<U32>();
    break;
  }
  case PC_FLOAT:
  {
    _engine->template add_field<I32>();
    break;
  }
  case PC_UNKNOWN:
  default:
    rc = false;
  }

  if (rc)
    _pointsize += dim->size;

  return rc;
}

// LazPerf Compressor
LazPerfCompressor::LazPerfCompressor(const PCSCHEMA *pcschema,
                                     LazPerfBuf &output)
    : LazPerf(pcschema, output)
{
  _engine = laszip::formats::make_dynamic_compressor(_coder);
  initSchema();
}

LazPerfCompressor::~LazPerfCompressor() {}

size_t LazPerfCompressor::compress(const uint8_t *input, const size_t inputsize)
{
  size_t size = 0;

  const uint8_t *end = input + inputsize;

  while (input + _pointsize <= end)
  {
    _engine->compress((const char *)input);
    input += _pointsize;
    size++;
  }

  _coder.done();

  return size;
}

// LazPerf Decompressor
LazPerfDecompressor::LazPerfDecompressor(const PCSCHEMA *pcschema,
                                         LazPerfBuf &input)
    : LazPerf(pcschema, input)
{
  _engine = laszip::formats::make_dynamic_decompressor(_coder);
  initSchema();
}

LazPerfDecompressor::~LazPerfDecompressor() {}

size_t LazPerfDecompressor::decompress(uint8_t *output, const size_t outputsize)
{
  size_t size = 0;

  const uint8_t *end = output + outputsize;

  while (output + _pointsize <= end)
  {
    _engine->decompress((char *)output);
    output += _pointsize;
    size++;
  }

  return size;
}

#endif // HAVE_LAZPERF
