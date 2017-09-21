default: all
include toc.make

DIST_FILES += $(wildcard in.* *.cpp *.hpp)

SHARED_LIBS = SerializableOne SerializableTwo
ifeq (1,$(s11n_CONFIG_ENABLE_PLUGINS))
SerializableOne_so_OBJECTS = SerializableOne.o
SerializableOne_so_INSTALL = 0
SerializableTwo_so_OBJECTS = SerializableTwo.o
SerializableTwo_so_INSTALL = 0
plugins_DEP_OBJECTS = SerializableOne.so SerializableTwo.so
include $(TOC_MAKESDIR)/SHARED_LIBS.make
endif
######## ^^^^ $(s11n_CONFIG_ENABLE_PLUGINS))


BIN_PROGRAMS = \
		arrays \
		cleanup \
		compspeed \
		demo_coord \
		enondas \
		exceptions \
		experimental \
		gene \
		graph \
		mylite \
		patrick \
		pointerdeser \
		pool \
		radford \
		rattlemouse \
		sdj \
		sdj2 \
		statics \
		templates


BIN_PROGRAMS += classname


arrays_bin_OBJECTS = arrays.o
classname_bin_OBJECTS = classname.o
cleanup_bin_OBJECTS = cleanup.o
compspeed_bin_OBJECTS = compspeed.o
demo_coord_bin_OBJECTS = demo_coord.o
enondas_bin_OBJECTS = enondas.o argv_parser.o
exceptions_bin_OBJECTS = exceptions.o
experimental_bin_OBJECTS = experimental.o
gene_bin_OBJECTS = gene.o
graph_bin_OBJECTS = graph.o
mylite_bin_OBJECTS = mylite.o
patrick_bin_OBJECTS = patrick.o
pointerdeser_bin_OBJECTS = pointerdeser.o
pool_bin_OBJECTS = pool.o
radford_bin_OBJECTS = radford.o
rattlemouse_bin_OBJECTS = rattlemouse.o
sdj_bin_OBJECTS = sdj.o
sdj2_bin_OBJECTS = sdj2.o
statics_bin_OBJECTS = statics.o
templates_bin_OBJECTS = templates.o
ushort_bin_OBJECTS = ushort.o
BIN_PROGRAMS_LDADD += -rdynamic $(LIBS11N_CLIENT_INTREE_LDADD)

ifeq (1,$(s11n_CONFIG_ENABLE_PLUGINS))
# causes a re-link on each build ---> plugins: SHARED_LIBS
  plugins: $(plugins_DEP_OBJECTS)
  BIN_PROGRAMS += plugins
  plugins_bin_OBJECTS = plugins.o SerializableBase.o SerializableFinal.o
endif
include $(TOC_MAKESDIR)/BIN_PROGRAMS.make
Makefile:
	@true
%.o: Makefile # WTF are my .o files not rebuilt by this???


DIST_FILES += enondas.s11n enondas2.s11n

CLEAN_FILES += graph.s11n graph.s11n.2

run: BIN_PROGRAMS
	@for i in $(BIN_PROGRAMS); do \
		LD_LIBRARY_PATH=$(top_srcdir)/src:$$LD_LIBRARY_PATH ./$$i || { \
			err=$$?; echo "$$i failed with code $$err"; exit $$err; \
		}; \
	done; \
	echo "All bins returned exit code 0. $(TOC_EMOTICON_OK)"

all: BIN_PROGRAMS
