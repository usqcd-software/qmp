#!/bin/sh

../configure --prefix=/usr/local/share/qmp/mpi --with-qmp-comms-type=MPI --with-qmp-comms-cflags=-I/usr/local/share/mpich/include --with-qmp-comms-ldflags=-L/usr/local/share/mpich/lib --with-qmp-comms-libs="-lmpich -lmpe"
