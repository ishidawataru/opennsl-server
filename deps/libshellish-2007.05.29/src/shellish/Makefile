include toc.make

SOURCES = $(addsuffix .cpp, \
	aliaser \
	arguments \
	argv_parser \
	builtins \
	environment \
	hnd_random \
	key_value_parser \
	path_finder \
	plugin \
	property_store \
	session \
	shell_process \
	shellish \
	stdstring_tokenizer \
	strtool \
	ui \
)

HEADERS = $(addsuffix .hpp, \
	aliaser \
	arguments \
	argv_parser \
	builtins \
	environment \
	factory \
	factory_reg \
	key_value_parser \
	path_finder \
	plugin \
	property_store \
	session \
	shell_process \
	shellish \
	shellish_debuggering_macros \
	stdstring_tokenizer \
	strtool \
)


GENERATED_HEADERS = shellish_config.hpp

OBJECTS = $(patsubst %.cpp,%.o,$(SOURCES))
# pedantically correct, but annoying: $(OBJECTS): Makefile

DISTCLEAN_FILES += $(GENERATED_HEADERS)

DIST_FILES += $(SOURCES) $(HEADERS) test.cpp shellish_config.hpp.at demomenu.emenu \
		plugin.noop.cpp plugin.dl.cpp plugin.win32.cpp \
		client.cpp


############################## shared libs:
lib_instdir = $(prefix)/lib/shellish
SHARED_LIBS = libshellish
# libshellish_builtins libshellish_ui

ifeq (1,0)
SIGSLOT_OBJECTS = $(top_srcdir)/src/sigslot/sigslot.o
else
SIGSLOT_OBJECTS =
endif

libshellish_so_OBJECTS = $(addsuffix .o,  aliaser \
					arguments \
					argv_parser \
					builtins \
					environment \
					hnd_random \
					key_value_parser \
					path_finder \
					plugin \
					property_store \
					session \
					shell_process \
					shellish \
					stdstring_tokenizer \
					strtool \
					ui \
			) \
			$(SIGSLOT_OBJECTS)

libshellish_so_VERSION = $(PACKAGE_VERSION)
libshellish_so_LDADD += $(SHELLISH_LIB_LDADD)

#libshellish_builtins_so_OBJECTS = builtins.o
#libshellish_builtins_so_INSTALL_DEST = $(lib_instdir)
#libshellish_builtins_so_VERSION = $(PACKAGE_VERSION)
#libshellish_ui_so_OBJECTS = ui.o
#libshellish_ui_so_INSTALL_DEST = $(lib_instdir)
#libshellish_ui_so_VERSION = $(PACKAGE_VERSION)

include $(TOC_MAKESDIR)/SHARED_LIBS.make


INSTALL_PACKAGE_HEADERS = $(HEADERS) $(GENERATED_HEADERS)

BIN_PROGRAMS = shellish noshell
shellish_bin_OBJECTS = client.o
noshell_bin_OBJECTS = test.o
# t10n_bin_OBJECTS = t10n.o
gene_bin_OBJECTS = gene.o
# noshell_bin_LDADD = -lpthread
BIN_PROGRAMS_LDADD = -L. -lshellish
include $(TOC_MAKESDIR)/BIN_PROGRAMS.make
INSTALL_BINS = 

SYMLINK_HEADERS = $(INSTALL_PACKAGE_HEADERS)
SYMLINK_HEADERS_DEST = $(top_srcdir)/include/s11n.net/shellish
include $(TOC_MAKESDIR)/SYMLINK_HEADERS.make

test: SHARED_LIBS
	LD_LIBRARY_PATH=.:${LD_LIBRARY_PATH} ./shellish --shellish-debug --session=./session.s11n

all: SYMLINK_HEADERS SHARED_LIBS BIN_PROGRAMS

