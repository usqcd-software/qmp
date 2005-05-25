#!/bin/sh

../configure --prefix=/home/edwards/arch/qmp/mpich-p4 --with-qmp-comms-type=mpi --with-qmp-comms-cflags="-g -O1 -I/usr/local/mpich-p4-1.2.6/include" --with-qmp-comms-ldflags="-L/usr/local/mpich-p4-1.2.6/lib" --with-qmp-comms-libs="-lpmpich" --without-dmalloc
