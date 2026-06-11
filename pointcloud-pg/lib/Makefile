
include ../config.mk

CPPFLAGS = $(XML2_CPPFLAGS) $(ZLIB_CPPFLAGS) $(LAZPERF_CPPFLAGS)
LDFLAGS = $(XML2_LDFLAGS) $(ZLIB_LDFLAGS)
CFLAGS += -fPIC

OBJS = \
	pc_bytes.o \
	pc_dimstats.o \
	pc_filter.o \
	pc_mem.o \
	pc_patch.o \
	pc_patch_dimensional.o \
	pc_patch_uncompressed.o \
	pc_point.o \
	pc_pointlist.o \
	pc_schema.o \
	pc_sort.o \
	pc_stats.o \
	pc_util.o \
	pc_val.o \
	stringbuffer.o \
	hashtable.o \
	pc_patch_lazperf.o

OBJS_LAZPERF = \
	lazperf_adapter.o

all: $(LIB_A) $(LIB_A_LAZPERF)
	$(MAKE) -C cunit $@

$(LIB_A): $(OBJS)
	ar rs $@ $^

$(LIB_A_LAZPERF): $(OBJS_LAZPERF)
	ar rs $@ $^

clean:
	@rm -f $(OBJS) $(LIB_A) $(OBJS_LAZPERF) $(LIB_A_LAZPERF)
	$(MAKE) -C cunit $@

install uninstall:
	@echo "No install target in lib"

check:
	$(MAKE) -C cunit $@
