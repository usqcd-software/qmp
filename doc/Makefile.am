##
## This is perhaps the yuckiest of the lot
##
## Automake file for making the doxygen documentation
## Balint Joo 25-11-2002 bj@ph.ed.ac.uk

# Define the installation documentation directory
docdir=@prefix@/doc

DOCSRCDIR=@top_srcdir@/doc
VPATH=$(DOCSRCDIR)

# Define the 'target'. This is basically the directory containing the docs
doc_DATA = oxydocs

# A make rule for the oxydocs target
oxydocs:
	doxygen QMPdoxyfile

# A 'special' install rule for the DATA (ie oxydocs) files to be 
# installed in docdir
install-docDATA:
	mkdir -p ${docdir}
	cp -r oxydocs/* ${docdir}

# Likewise a special (and rather drastic) uninstall rule
uninstall-docDATA:
	rm -rf ${docdir}/*

# Now some cleaning rules according to the automake heuristics
.PHONY: clean_doxygen mostlyclean clean distclean maintainer-clean

# Remove the whole doxygen generated stuff
clean_doxygen:
	rm -rf oxydocs

#
# The four GNU clean targets. Since doxygen you may want to rebuild 
# all the time it should become 'mostlyclean'
# All the rest are just the same
mostlyclean: clean_doxygen

clean: clean_doxygen

distclean: clean_doxygen

maintainer-clean: clean_doxygen

