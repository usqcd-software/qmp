##
## This is perhaps the yuckiest of the lot
##
## Automake file for making the doxygen documentation
## Balint Joo 25-11-2002 bj@ph.ed.ac.uk

# Define the installation documentation directory
docdir=@prefix@/doc

DOCSRCDIR=@top_srcdir@/doc
VPATH=$(DOCSRCDIR)

# A make rule for the oxydocs target
doc: oxydocs
oxydocs:
	doxygen QMPdoxyfile

# A 'special' install rule for the DATA (ie oxydocs) files to be 
# installed in docdir
doc-install: oxydocs
	mkdir -p ${docdir}
	cp -r oxydocs/* ${docdir}

# Likewise a special (and rather drastic) uninstall rule
doc-uninstall:
	-rm -rf ${docdir}

# Remove the whole doxygen generated stuff
doc-clean:
	-rm -rf oxydocs

doc-distclean: doc-clean

# also clean on regular make clean
clean-local: doc-clean

EXTRA_DIST = MessageAPI.htm QMP_API.html QMP-1-0.doc \
             boy6.gif constru3.gif id2.gif note02.gif

.PHONY: doc doc-install doc-uninstall doc-clean doc-distclean

