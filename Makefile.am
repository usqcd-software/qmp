#
# Recurse down into src (to build the library)
#                   examples (build examples)
#
SUBDIRS = include lib examples
EXTRA_DIST = doc/Makefile.am doc/Makefile.in doc/QMPdoxyfile.in \
             doc/MessageAPI.htm doc/QMP_API.html doc/QMP-1-0.doc

#
# The end user's configuration script
# 
bin_SCRIPTS=qmp-config

#
# User explicitly has to make docs -- you may not want them
#
doc:
	cd docs ; $(MAKE)

doc-install: 
	cd docs ; $(MAKE) install

doc-uninstall:
	cd docs ; $(MAKE) uninstall

doc-clean:
	cd docs ; $(MAKE) clean

doc-distclean:
	cd docs ; $(MAKE) distclean

