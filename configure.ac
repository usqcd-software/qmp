dnl Process this file with autoconf to produce a configure script.
AC_INIT(qmp, 2.0.0, josborn@physics.bu.edu)
AC_CONFIG_AUX_DIR(config)
AC_CONFIG_SRCDIR(lib/QMP_grid.c)
AC_CONFIG_SRCDIR(examples/QMP_test.c)
AM_INIT_AUTOMAKE

dnl automake manual, ch 6 says to use this when SUBDIRS is used
AC_PROG_MAKE_SET

dnl Define these as defaults 
dnl Needed below to check if compiler works

AC_PROG_CC(gcc cc)
AC_ISC_POSIX
AC_PROG_RANLIB

dnl George Fleming, 12/12/2002
dnl
dnl This is simply a complete rewrite of the --enable and --with options
dnl to support different implementations of MPI or GM upon which QMP
dnl is built.  In some ways it duplicates what is already available
dnl in scripts like 'mpicc' or 'mpcc' but since those don't always exist
dnl or can't be trusted to point to the implementation the user intended,
dnl this puts the onus on the user to set all the flags correctly.

AC_ARG_WITH(qmp-comms-type,
  AC_HELP_STRING([--with-qmp-comms-type=TYPE],
    [Build QMP for a single node (SINGLE) or on top of MPI (MPI).
     Default is SINGLE.]),
  [case "${with_qmp_comms_type}" in
    single|SINGLE) QMP_COMMS_TYPE=SINGLE ;;
    mpi|MPI) QMP_COMMS_TYPE=MPI ;;
    *)       AC_MSG_ERROR([bad value "${with_qmp_comms_type}"
      for --with-qmp-comms-type]) ;;
    esac],
  [QMP_COMMS_TYPE=SINGLE])

AC_SUBST(QMP_COMMS_TYPE)

AC_ARG_WITH( qmp-comms-cflags,
  AC_HELP_STRING([--with-qmp-comms-cflags=QMP_COMMS_CFLAGS],
    [To pass optional include path to comms header files via compiler flag
      -I]),
  [QMP_COMMS_CFLAGS="$with_qmp_comms_cflags"] )

AC_SUBST(QMP_COMMS_CFLAGS)

AC_ARG_WITH( qmp-comms-ldflags,
  AC_HELP_STRING([--with-qmp-comms-ldflags=QMP_COMMS_LDFLAGS],
    [To pass optional library search path to comms libraries via compiler
      flag -L]),
  [QMP_COMMS_LDFLAGS="$with_qmp_comms_ldflags"])

AC_SUBST(QMP_COMMS_LDFLAGS)

AC_ARG_WITH( qmp-comms-libs,
  AC_HELP_STRING([--with-qmp-comms-libs=QMP_COMMS_LIBS],
    [To pass optional list of comms libraries to linker via compiler flag -l]),
  [QMP_COMMS_LIBS="$with_qmp_comms_libs"])

AC_SUBST(QMP_COMMS_LIBS)

# If $QMP_COMMS_TYPE=MPI then perform the following link test

case "${QMP_COMMS_TYPE}" in 
SINGLE)
  echo Building QMP for SINGLE node use
  ;;
MPI)
  echo Building QMP for use with MPI
  PAC_MPI_LINK_CC_FUNC( ${QMP_COMMS_CFLAGS}, ${QMP_COMMS_LDFLAGS},
    ${QMP_COMMS_LIBS}, , , [mpi_link_ok=yes], [mpi_link_ok=no])
  AC_MSG_CHECKING([if we can compile/link of a simple MPI program])
  if test "X${mpi_link_ok}X" = "XyesX" ; then
    AC_MSG_RESULT(yes)
  else
    AC_MSG_RESULT(no)
    AC_MSG_ERROR([Cannot compile/link a basic MPI C program!
      Check QMP_COMMS_CFLAGS, QMP_COMMS_LDFLAGS, QMP_COMMS_LIBS.])
  fi
  ;;
*)
  AC_MSG_ERROR([Shouldnt reach this point]);
  ;;
esac

AC_ARG_ENABLE( extra-debugging,
  AC_HELP_STRING([--enable-extra-debugging],
    [Extra debugging messages useful for QMP developers.  Default is no.]),
  [case "${enable_extra_debugging}" in
    yes)
      AC_DEFINE(_QMP_DEBUG)
      AC_DEFINE(_QMP_TRACE)
      ;;
    no) ;;
    *) AC_MSG_ERROR([bad value ${enable_extra_debugging}
      for --enable-extra-debugging]) ;;
    esac],
  [enable_extra_debugging=no])

# Currently, this implementation of QMP only supports switched or shared
# memory interconnect (IC) types.  If support for other types is added,
# something will have to change here.
AC_DEFINE(_QMP_IC_SWITCH)

AM_CONDITIONAL(QMP_SINGLE, [test "X${QMP_COMMS_TYPE}X" = "XSINGLEX"])
AM_CONDITIONAL(QMP_MPI, [test "X${QMP_COMMS_TYPE}X" = "XMPIX"])

############################################################
# Support for memory debugging with DMALLOC
############################################################

AM_WITH_DMALLOC

############################################################
# Check for build programs
############################################################

AC_CHECK_PROG(DOXYGEN, doxygen, doxygen)

############################################################
# Check for library functions
############################################################


############################################################
# Define files to configure and do it
############################################################

AC_CONFIG_FILES(Makefile)
dnl George T. Fleming, 03/03/2003
dnl
dnl Part of change from 'qmp_build_env.sh' to 'qmp-config' scripts
dnl AC_CONFIG_FILES(qmp_build_env.sh)
AC_CONFIG_FILES(include/Makefile)
AC_CONFIG_FILES(lib/Makefile)
AC_CONFIG_FILES(bin/Makefile)
AC_CONFIG_FILES(bin/qmp-config)
AC_CONFIG_FILES(examples/Makefile)
AC_CONFIG_FILES(doc/Makefile)
AC_CONFIG_FILES(doc/QMPdoxyfile)
AC_OUTPUT
