#!/bin/sh

../configure --prefix=/usr/local/qmp/mpich-gm-1.2.5 --with-qmp-comms-type=mpi --with-qmp-comms-cflags=-I/usr/local/mpich-gm-1.2.5/include --with-qmp-comms-ldflags="-L/usr/local/mpich-gm-1.2.5/lib -L/usr/local/gm-1.6.4/lib" --with-qmp-comms-libs="-lmpich -lgm -lpthread"
