## Process this file with automake to produce Makefile.in
#
# Toplevel Makefile.am -- Deal with source and Doc subdirctories
#
## Balint Joo, 25/11/2002
##
## Add the QMPdoxyfile and the QMP run script (which are not part 
## of anything that gets built or is a _SOURCE or any of automakes
## other automatics) to the distro, so that make dist willl work out
## OK.
##
## George Fleming, 11/12/2002
##
## Moved non-library sources into examples directory and made them compile
## only under 'make check'.
##
## George Fleming, 03/03/2003
##
## Implemented qmp-config script in lieu of qmp_build_env.sh script

SUBDIRS=src doc examples
EXTRA_DIST=doc/QMPdoxyfile.in src/QMP_run

bin_SCRIPTS=qmp-config
