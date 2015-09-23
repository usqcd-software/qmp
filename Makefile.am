#
# Recurse down into src (to build the library)
#                   examples (build examples)
#
SUBDIRS = include lib bin examples doc

DISTCHECK_CONFIGURE_FLAGS = @DISTCHECK_CONFIGURE_FLAGS@

# QMPConfig.cmake is a file, that a CMake system
# Can use to find out about the QMP Installation
# It is not configured by autoconf and just
# needs to be included in the root of the qmp installation
#
# This adds the file, so make dist will pick it up
EXTRA_DIST = QMPConfig.cmake

# THese two lines will install the file in the target directory
cmakeconfdir = $(prefix)
cmakeconf_DATA = QMPConfig.cmake


#
# User explicitly has to make docs -- you may not want them
#
doc:
	cd doc && $(MAKE) doc

doc-install:
	cd doc && $(MAKE) doc-install

doc-uninstall:
	cd doc && $(MAKE) doc-uninstall

doc-clean:
	cd doc && $(MAKE) doc-clean

doc-distclean:
	cd doc && $(MAKE) doc-distclean

.PHONY: doc doc-install doc-uninstall doc-clean doc-distclean
