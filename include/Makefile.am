## Process this file with automake to produce Makefile.in

## GTF: if qmp.hpp needs to end up in @prefix@/include, then add it here
include_HEADERS = qmp.h

## GTF: if qmp.hpp is just going along for the ride in the dist until it proves
## to be useful but is not currently not to be installed anywhere, put it here.
## JCO: qmp.hpp is out-of-date and not supported, so don't even distribute it.
EXTRA_DIST = \
	QMP_P_COMMON.h \
	QMP_P_MPI.h \
	QMP_P_BGSPI.h \
	QMP_profiling.h
