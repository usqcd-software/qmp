#! /usr/bin/bash
 
INSTALL="$HOME/Devel/SciDAC/install/qmp_generic"
export CC="mpcc_r "
export CFLAGS="-O2 -qfuncsect"
 
./configure --prefix=${INSTALL} \
        --with-qmp-comms-type=MPI \
        --host=powerpc-ibm-aix \
        --build=none
