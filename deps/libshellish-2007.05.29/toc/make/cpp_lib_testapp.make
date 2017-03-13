# makefile snippet for running e-based apps
# If you are going to include cpp_{static,dynamic}_lib.make, You should 
# include those before including this one.
# 
# you must define:
#   THISLIB_TESTS = list of app names
# then include this file.
# for each FOO in $(THISLIBLIB_TESTS) you must have the file FOO.cpp
# in place for this to work. (Sorry, it's braindeaded-level automation.)
# If cpp_static_lib has been included it builds a statically-linked binary
# named "FOO" from FOO.o and $(THISLIB_STATIC).
# If cpp_dynamic_lib has been included it builds a dynamically-linked binary
# "FOO_dynamic" from FOO.o and $(THISLIB_SHARED).
# (This FOO_dynamic naming convention will change - i don't like it.)
# todo: need better way of handling name descrepancy between static and dyn-linked bins.

libtests: $(THISLIB_TESTS)
submakes: $(THISLIB_TESTS)

SUBMAKE_SOURCES =  $(patsubst %,%.cpp,$(THISLIB_TESTS))
DIST_FILES += $(SUBMAKE_SOURCES)
SUBMAKE_OBJECTS += $(patsubst %,%.o,$(THISLIB_TESTS))
SUBMAKE_COMMON_DEPS += Makefile $(THISLIB_STATIC)

ifneq ($(THISLIB_STATIC),) # shared?
CLEAN_FILES += $(THISLIB_TESTS) $(patsubst %,%.o,$(THISLIB_TESTS))
endif
ifneq ($(THISLIB_SHARED),) # shared?
CLEAN_FILES += $(patsubst %,%_dynamic,$(THISLIB_TESTS)) $(patsubst %,%_dynamic.o,$(THISLIB_TESTS))
endif

#fore_namexform = $(patsubst .,_,$(1))
#foreach(fore_namexform,$(THISLIB_TESTS),

# LDFLAGS += -export-dynamic
call_linkmessage = echo '$(1) linking binary: $(3)'
$(THISLIB_TESTS): $(SUBMAKE_COMMON_DEPS) $(SUBMAKE_OBJECTS) 
ifneq ($(THISLIB_STATIC),) # static?
	@$(call call_linkmessage,Statically,$@,$@)
	$(CXX) $(LDFLAGS) -o $@ $@.o $(THISLIB_STATIC) $(THISLIB_EXTRAOBJECTS)  -lstdc++
        # note that the order of the libs here is significant :/
endif # build shared versions?
ifneq ($(THISLIB_SHARED),) # shared?
	@$(call call_linkmessage,Dynamically,$@,$(@)_dynamic)
	$(CXX) $(LDFLAGS) -export-dynamic -o $(@)_dynamic $@.o $(THISLIB_SHARED) $(THISLIB_EXTRAOBJECTS) -lstdc++
endif # build shared versions?

# Note that the above usage of $(LDFLAGS) is not known to be valid. :/ It works in my
# extremely simple cases.

runtests: # does not account for dyn-linked bins...
	@for t in $(THISLIB_TESTS); do echo "Running $t..."; ./$$t || exit $$?; done
