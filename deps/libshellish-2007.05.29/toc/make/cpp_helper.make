# Sets up bootstrap vars and targets for C++ coding.
#

# The other cpp_*.make files may rely on vars set here.

#SOURCES += $(wildcard *.cpp *.c++ *.C)
#HEADERS += $(wildcard *.h *.hpp *.H)


DIST_FILES += $(SOURCES) $(HEADERS)

OBJECTS += $(patsubst %.cpp,%.o,$(SOURCES))

CLEAN_FILES += $(OBJECTS)
