
include ../../config.mk

CPPFLAGS = $(XML2_CPPFLAGS) $(CUNIT_CPPFLAGS) $(ZLIB_CPPFLAGS) -I..
LDFLAGS = $(XML2_LDFLAGS) $(CUNIT_LDFLAGS) $(ZLIB_LDFLAGS)

EXE = cu_tester

# ADD YOUR NEW TEST FILE HERE (1/1)
OBJS =	\
	cu_tester.o \
	cu_pc_bytes.o \
	cu_pc_schema.o \
	cu_pc_point.o \
	cu_pc_patch.o \
	cu_pc_patch_lazperf.o \
	cu_pc_sort.o \
	cu_pc_util.o

ifeq ($(CUNIT_LDFLAGS),)
# No cunit? Emit message and continue

all:
	@echo "CUnit not found, skipping build"

check:
	@echo "CUnit not found, skipping tests"

else
# Yes cunit? Build tests and run

# Build the unit tester
all: $(EXE)

# Build and run the unit tester
check: $(EXE)
	@./$(EXE)

endif

# Build the main unit test executable
$(EXE): $(OBJS) ../$(LIB_A) ../$(LIB_A_LAZPERF)
	$(CC) -o $@ $^ $(LDFLAGS) -lm -lstdc++

../$(LIB_A):
	$(MAKE) -C .. $(LIB_A)

../$(LIB_A_LAZPERF):
	$(MAKE) -C .. $(LIB_A_LAZPERF)

# Clean target
clean:
	@rm -f $(OBJS)
	@rm -f $(EXE)

