dnl Process this file with autoconf to produce a configure script.
AC_INIT(qmp, 2.5.0, [detar at physics.utah.edu])
AC_CONFIG_AUX_DIR(config)
AC_CONFIG_SRCDIR([lib/QMP_grid.c])
AC_CONFIG_SRCDIR([examples/QMP_test.c])
AC_CONFIG_HEADER([include/qmp_config.h])
AM_INIT_AUTOMAKE([subdir-objects])

dnl automake manual, ch 6 says to use this when SUBDIRS is used
AC_PROG_MAKE_SET

dnl Define these as defaults 
dnl Needed below to check if compiler works

AC_PROG_CC(c99 cc gcc)
AC_ISC_POSIX
AC_PROG_RANLIB
AC_CHECK_TOOL(AR, ar, [ar])
AM_PROG_CC_C_O

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
    mpi|MPI) QMP_COMMS_TYPE=MPI
             AC_DEFINE([HAVE_MPI],[],[compiling for MPI]) ;;
    *)       AC_MSG_ERROR([bad value "${with_qmp_comms_type}"
      for --with-qmp-comms-type]) ;;
    esac],
  [QMP_COMMS_TYPE=SINGLE])

AC_ARG_WITH( qmp-comms-cflags,
  AC_HELP_STRING([--with-qmp-comms-cflags=QMP_COMMS_CFLAGS],
    [To pass optional include path to comms header files via compiler flag
      -I]),
  [QMP_COMMS_CFLAGS="$with_qmp_comms_cflags"] )

AC_ARG_WITH( qmp-comms-ldflags,
  AC_HELP_STRING([--with-qmp-comms-ldflags=QMP_COMMS_LDFLAGS],
    [To pass optional library search path to comms libraries via compiler
      flag -L]),
  [QMP_COMMS_LDFLAGS="$with_qmp_comms_ldflags"])

AC_ARG_WITH( qmp-comms-libs,
  AC_HELP_STRING([--with-qmp-comms-libs=QMP_COMMS_LIBS],
    [To pass optional list of comms libraries to linker via compiler flag -l]),
  [QMP_COMMS_LIBS="$with_qmp_comms_libs"])

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
      AC_DEFINE([_QMP_DEBUG],[],[enable extra debugging])
      AC_DEFINE([_QMP_TRACE],[],[enable extra tracing])
      ;;
    no) ;;
    *) AC_MSG_ERROR([bad value ${enable_extra_debugging}
      for --enable-extra-debugging]) ;;
    esac],
  [enable_extra_debugging=no])

dnl --enable-profiling
AC_ARG_ENABLE(
  profiling,
  AC_HELP_STRING([--enable-profiling],
    [build QMP with weak linkage allowing profiling library to be inserted]),
  [if test "X$enableval" != "Xno"; then
    AC_DEFINE([QMP_BUILD_PROFILING],[],[build QMP to allow profiling])
  fi]
)

dnl --enable-timing
AC_ARG_ENABLE(
  timing,
  AC_HELP_STRING([--enable-timing],
    [turn on timing of all QMP functions]),
  [if test "X$enableval" != "Xno"; then
    AC_DEFINE([QMP_BUILD_TIMING],[],[build QMP with function timing])
  fi]
)

dnl --enable-bgl
AC_ARG_ENABLE(
  bgl,
  AC_HELP_STRING([--enable-bgl],
    [Use BG/L personality to set native machine geometry.]),
  [if test "X$enableval" != "Xno"; then
    AC_DEFINE([HAVE_BGL],[],[compiling for BG/L])
    AC_MSG_NOTICE([Using BG/L personality.])
  fi]
)

dnl --enable-bgp
AC_ARG_ENABLE(
  bgp,
  AC_HELP_STRING([--enable-bgp],
    [Use BG/P personality to set native machine geometry.]),
  [if test "X$enableval" != "Xno"; then
    AC_DEFINE([HAVE_BGP],[],[compiling for BG/P])
    AC_MSG_NOTICE([Using BG/P personality.])
    QMP_COMMS_CFLAGS="-I/bgsys/drivers/ppcfloor/arch/include $QMP_COMMS_CFLAGS"
  fi]
)

dnl --enable-bgq
AC_ARG_ENABLE(
  bgq,
  AC_HELP_STRING([--enable-bgq],
    [Use BG/Q personality to set native machine geometry.]),
  [if test "X$enableval" != "Xno"; then
    AC_DEFINE([HAVE_BGQ],[],[compiling for BG/Q])
    AC_MSG_NOTICE([Using BG/Q personality.])
    QMP_COMMS_CFLAGS="-I/bgsys/drivers/ppcfloor/spi/include $QMP_COMMS_CFLAGS"
  fi]
)

dnl --enable-bgspi
AC_ARG_ENABLE(
  bgspi,
  AC_HELP_STRING([--enable-bgspi],
    [Use Blue Gene SPI low-level communications.]),
  [if test "X$enableval" != "Xno"; then
    AC_DEFINE([HAVE_BGSPI],[],[compiling for SPI])
    AC_MSG_NOTICE([Using Blue Gene SPI.])
    HAVE_BGSPI=1
    #QMP_COMMS_CFLAGS="-I/bgsys/drivers/ppcfloor/spi/include $QMP_COMMS_CFLAGS"
  fi]
)

AC_SUBST(QMP_COMMS_TYPE)
AC_SUBST(QMP_COMMS_CFLAGS)
AC_SUBST(QMP_COMMS_LDFLAGS)
AC_SUBST(QMP_COMMS_LIBS)

AM_CONDITIONAL(QMP_SINGLE, [test "X${QMP_COMMS_TYPE}X" = "XSINGLEX"])
AM_CONDITIONAL(QMP_MPI, [test "X${QMP_COMMS_TYPE}X" = "XMPIX"])
AM_CONDITIONAL(QMP_BGSPI, [test "X${HAVE_BGSPI}X" = "X1X"])

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
# Pass configuration to distcheck target
############################################################
DISTCHECK_CONFIGURE_FLAGS="--enable-parallel-arch=$QMP_COMMS_TYPE CC=\"$CC\" CFLAGS=\"$CFLAGS\" QMP_COMMS_CFLAGS=\"$QMP_COMMS_CFLAGS\" QMP_COMMS_LDFLAGS=\"$QMP_COMMS_LDFLAGS\" QMP_COMMS_LIBS=\"$QMP_COMMS_LIBS\""
AC_SUBST(DISTCHECK_CONFIGURE_FLAGS)

############################################################
# Define files to configure and do it
############################################################

AC_CONFIG_FILES(Makefile)
AC_CONFIG_FILES(include/Makefile)
AC_CONFIG_FILES(lib/Makefile)
AC_CONFIG_FILES(bin/Makefile)
AC_CONFIG_FILES(bin/qmp-config)
AC_CONFIG_FILES(examples/Makefile)
AC_CONFIG_FILES(doc/Makefile)
AC_CONFIG_FILES(doc/QMPdoxyfile)
AC_OUTPUT
