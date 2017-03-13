#/usr/bin/make -f

include toc.make

SUBDIRS = src

ifneq (,$(strip $(filter dist testdist clean distclean uninstall install,$(MAKECMDGOALS))))
# only build these SUBDIRS when running certain targets
      SUBDIRS += include doc toc
endif


INSTALL_BINS = $(PACKAGE_NAME)-config
INSTALL_PKGCONFIG = $(PACKAGE_NAME).pc


DIST_FILES += ChangeLog INSTALL INSTALL.nobuildfiles LICENSE.$(PACKAGE_NAME) \
	README.src README.WIN32 create_generated_tree.sh \
	configure configure.$(PACKAGE_NAME) postconfig.$(PACKAGE_NAME) \
	toc.$(PACKAGE_NAME).make.at toc.$(PACKAGE_NAME).help \
	find_toc.sh


DISTCLEAN_FILES += $(PACKAGE_NAME)-config $(PACKAGE_NAME).pc

all: subdirs



########################################################################
# Check if tree appears to be built, and do some stuff if it is...
ifneq (,$(wildcard src/lite/s11nlite.o))
S11NCONF = $(wildcard ./libs11n-config)
LIB_WITH_VERSION = libs11n-$(shell $(S11NCONF) --version)

NOBUILD_DIR = nobuildfiles# must match name used by ./create_generated_tree.sh
NOBUILD_PKG_DIR = $(LIB_WITH_VERSION)-$(NOBUILD_DIR)
NOBUILD_ZIP = $(NOBUILD_PKG_DIR).zip
##################################################
# The nobuild target creates a source zip file
# for cross-platform use.
.PHONY: nobuild
nobuild:
	@test -d $(NOBUILD_DIR) && rm -fr $(NOBUILD_DIR); exit 0
	./create_generated_tree.sh
	@test -d $(NOBUILD_PKG_DIR) && rm -fr $(NOBUILD_PKG_DIR); exit 0
	mv $(NOBUILD_DIR) $(NOBUILD_PKG_DIR)
	@test -f $(NOBUILD_ZIP) && rm $(NOBUILD_ZIP); exit 0
	zip -q -r $(NOBUILD_ZIP) $(NOBUILD_PKG_DIR)
	@ls -la $(NOBUILD_ZIP)
CLEAN_FILES += $(NOBUILD_PKG_DIR)
########################################################################
# Tree isn't built yet.
else
nobuild:
	@echo "Creating the fully generated source tree requires a configured and built source tree."; exit 1
endif
# end built-tree check.
########################################################################
