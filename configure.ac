dnl Process this file with autoconf to produce a configure script.
AC_INIT(qmp, 1.2.7, chen@jlab.org)
AC_CONFIG_AUX_DIR(config)
AC_CONFIG_SRCDIR(src/QMP_grid.c)
AC_CONFIG_SRCDIR(examples/QMP_test.c)
AM_INIT_AUTOMAKE

dnl automake manual, ch 6 says to use this when SUBDIRS is used
AC_PROG_MAKE_SET

dnl Define these as defaults 

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
    [Build QMP on top of MPI or GM.  If MPI over GM is desired, choose MPI.
      Default is MPI.]),
  [case "${with_qmp_comms_type}" in
    mpi|MPI) QMP_COMMS_TYPE=MPI ;;
    gm|GM)   QMP_COMMS_TYPE=GM  ;;
    *)       AC_MSG_ERROR([bad value "${with_qmp_comms_type}"
      for --with-qmp-comms-type]) ;;
    esac],
  [QMP_COMMS_TYPE=MPI])

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
MPI)
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
GM)
  PAC_GM_LINK_CC_FUNC( ${QMP_COMMS_CFLAGS}, ${QMP_COMMS_LDFLAGS},
    ${QMP_COMMS_LIBS}, , , [gm_link_ok=yes], [gm_link_ok=no])
  AC_MSG_CHECKING([if we can compile/link of a simple GM program])
  if test "X${gm_link_ok}X" = "XyesX" ; then
    AC_MSG_RESULT(yes)
  else
    AC_MSG_RESULT(no)
    AC_MSG_ERROR([Cannot compile/link a basic GM C program!
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

AC_ARG_ENABLE( single-gm-port,
  AC_HELP_STRING([--enable-single-gm-port],
    [Using a single gm port to transfer messages using memory copy.
      Default is yes.]),
  [case "${enable_single_gm_port}" in
    no)
      AC_DEFINE(_QMP_USE_MULTI_PORTS)
      ;;
    yes) ;;
    *) AC_MSG_ERROR([bad value ${enable_single_gm_port}
      for --enable-single-gm-port]) ;;
    esac],
  [enable_single_gm_port=yes])

AC_ARG_ENABLE( quick-global-sum,
  AC_HELP_STRING([--enable-quick-global-sum],
    [Using a better algorithm to do global sum.  Default is yes.]),
  [case "${enable_quick_global_sum}" in
    yes)
      AC_DEFINE(_QMP_QUICK_GCOMM)
      ;;
    no) ;;
    *) AC_MSG_ERROR([bad value ${enable_quick_global_sum}
      for --enable-quick-global-sum]) ;;
    esac],
  [AC_DEFINE(_QMP_QUICK_GCOMM)])

# Currently, this implementation of QMP only supports switched or shared
# memory interconnect (IC) types.  If support for other types is added,
# something will have to change here.
AC_DEFINE(_QMP_IC_SWITCH)

# Needed by GM variant of QMP code
if test "X${QMP_COMMS_TYPE}X" = "XGMX" ; then
  AC_DEFINE(_QMP_USE_GM)
fi

############################################################
# Support for memory debugging with DMALLOC
############################################################

AM_WITH_DMALLOC

############################################################
# Now do some configuration
############################################################
#

AM_CONDITIONAL(QMP_MPI, [test "X${QMP_COMMS_TYPE}X" = "XMPIX"])
AM_CONDITIONAL(QMP_GM,  [test "X${QMP_COMMS_TYPE}X" = "XGMX" ])

AC_CHECK_PROG(DOXYGEN, doxygen, doxygen)

AC_CONFIG_FILES(Makefile)
dnl George T. Fleming, 03/03/2003
dnl
dnl Part of change from 'qmp_build_env.sh' to 'qmp-config' scripts
dnl AC_CONFIG_FILES(qmp_build_env.sh)
AC_CONFIG_FILES(qmp-config)
AC_CONFIG_FILES(src/Makefile)
AC_CONFIG_FILES(doc/Makefile)
AC_CONFIG_FILES(examples/Makefile)
AC_CONFIG_FILES(doc/QMPdoxyfile)
AC_OUTPUT
