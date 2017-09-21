
default: all
include toc.make
%.o: Makefile

DIST_FILES += $(wildcard in.* *.cpp *.hpp) s11nconvert.1

SOURCES = main.cpp

BIN_PROGRAMS = s11nconvert
s11nconvert_bin_OBJECTS = main.o argv_parser.o
BIN_PROGRAMS_LDADD = $(LIBS11N_CLIENT_INTREE_LDADD)
include $(TOC_MAKESDIR)/BIN_PROGRAMS.make

INSTALL_MAN1 = s11nconvert.1

INSTALL_BINS = $(BIN_PROGRAMS)
install-BINS: strip-bins

########################################################################
# start conditional demo/test stuff...
SC = $(wildcard ./s11nconvert)
SCARGS += -DL mysql
ifneq (,$(SC))
    SC := $(SC) $(SCARGS)
    SERIALIZERS = $(shell $(SC) --known-serializers)
    DEMO_IN = in.demo


showser: s11nconvert
	@echo -n "Available Serializers: "; $(SC) --known-serializers

demo:
	LD_LIBRARY_PATH="${LD_LIBRARY_PATH}:.:$(top_srcdir)/lib"; \
		for s in $(SERIALIZERS); do \
			cmd="$(SC) -f $(DEMO_IN) -s $$s -o out.$$s"; \
			echo $$cmd; $$cmd || exit; \
		done
	ls -la out.{$(shell echo $(SERIALIZERS) | sed -e 's/ /,/g')} || true
        #          ^^^ i am SUCH a lazy bastard!
CLEAN_FILES += $(wildcard my.* out.*)
endif
# ^^^ $(SC)
# end conditional demo/test stuff.
########################################################################

all: BIN_PROGRAMS
################################################################################


