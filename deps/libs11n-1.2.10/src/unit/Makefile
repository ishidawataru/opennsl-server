default: all
include toc.make
# CXX := /usr/bin/time $(CXX)

DIST_FILES += $(wildcard in.* *.cpp *.hpp)

BIN_PROGRAMS = unittests
unittests_bin_OBJECTS = main.o
BIN_PROGRAMS_LDADD += -rdynamic $(LIBS11N_CLIENT_INTREE_LDADD)
include $(TOC_MAKESDIR)/BIN_PROGRAMS.make

UNIT_TESTS = Casts \
	Exceptions \
	FormatsFrenzy \
	ListsTest \
	PodsTest \
	PointerDeser \
	ValArray

DIST_FILES += TestTemplate.cpp

CLEAN_FILES += $(patsubst %,%.s11n,$(UNIT_TESTS)) \
	FormatsFrenzy.*.s11n


########################################################################
# As long as each test is made up of exactly 1 .cpp file which has
# exactly the same base name as the test, then the following will
# generate the .so rules:
define UNIT_TESTS_DEFAULT_SO_STUFF
SHARED_LIBS += $(1)
$(1)_so_OBJECTS = $(1).o
endef
$(foreach TEST,$(UNIT_TESTS),$(eval $(call UNIT_TESTS_DEFAULT_SO_STUFF,$(TEST))))
# end .so generation
########################################################################


SHARED_LIBS_INSTALL = 0
SHARED_LIBS: $(BIN_PROGRAMS)
include $(TOC_MAKESDIR)/SHARED_LIBS.make


Makefile:
	@true
# %.o: Makefile # WTF are my .o files not rebuilt by this???

tests: SHARED_LIBS
	@LD_LIBRARY_PATH="$(top_srcdir)/src:.:$${LD_LIBRARY_PATH}"; \
	./unittests $(SHARED_LIBS)

all: BIN_PROGRAMS SHARED_LIBS
