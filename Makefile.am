#
# Recurse down into src (to build the library)
#                   examples (build examples)
#
SUBDIRS = include lib examples doc

#
# The end user's configuration script
# 
bin_SCRIPTS=qmp-config

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
