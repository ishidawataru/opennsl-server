include toc.make

SUBDIRS = src

DIST_FILES += configure configure.$(PACKAGE_NAME) toc.$(PACKAGE_NAME).make.at \
	ChangeLog README.$(PACKAGE_NAME) LICENSE.$(PACKAGE_NAME) \
	find_toc.sh


INSTALL_PKGCONFIG = $(PACKAGE_NAME).pc

DISTCLEAN_FILES += $(PACKAGE_NAME)-config $(PACKAGE_NAME).pc

INSTALL_BINS += $(PACKAGE_NAME)-config

all: subdirs

