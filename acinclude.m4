dnl George Fleming, 12/12/2002
dnl
dnl Stole this from mpich-1.2.4/mpe
dnl
dnl PAC_MPI_LINK_CC_FUNC( MPI_CC, MPI_CFLAGS, MPI_LIBS,
dnl                       MPI_VARS, MPI_FUNC,
dnl                       [action if working], [action if not working] )
dnl - MPI_CFLAGS  is the extra CFLAGS to CC, like "-I/usr/include" for mpi.h
dnl - MPI_LDFLAGS is the extra LDFLAGS to CC, like "-L/usr/lib" for libmpi.a
dnl - MPI_LIBS    is the LIBS to CC, like "-lmpi" for libmpi.a
dnl - MPI_VARS    is the the declaration of variables needed to call MPI_FUNC
dnl - MPI_FUNC    is the body of MPI function call to be checked for existence
dnl               e.g.  MPI_VARS="MPI_Request request; MPI_Fint a;"
dnl                     MPI_FUNC="a = MPI_Request_c2f( request );"
dnl               if MPI_FUNC is empty, assume linking with basic MPI program.
dnl               i.e. check if MPI definitions are valid
dnl
AC_DEFUN([PAC_MPI_LINK_CC_FUNC],[
dnl - set local parallel compiler environments
dnl   so input variables can be CFLAGS, LDFLAGS or LIBS
    pac_MPI_CFLAGS="$1"
    pac_MPI_LDFLAGS="$2"
    pac_MPI_LIBS="$3"
    AC_LANG_SAVE
    AC_LANG_C
dnl - save the original environment
    pac_saved_CFLAGS="$CFLAGS"
    pac_saved_LDFLAGS="$LDFLAGS"
    pac_saved_LIBS="$LIBS"
dnl - set the parallel compiler environment
    CFLAGS="$CFLAGS $pac_MPI_CFLAGS"
    LDFLAGS="$LDFLAGS $pac_MPI_LDFLAGS"
    LIBS="$LIBS $pac_MPI_LIBS"
    AC_TRY_LINK( [#include "mpi.h"], [
    int argc; char **argv;
    $4 ; 
    MPI_Init(&argc, &argv);
    $5 ;
    MPI_Finalize();
                 ], pac_mpi_working=yes, pac_mpi_working=no )
    CFLAGS="$pac_saved_CFLAGS"
    LDFLAGS="$pac_saved_LDFLAGS"
    LIBS="$pac_saved_LIBS"
    AC_LANG_RESTORE
    if test "$pac_mpi_working" = "yes" ; then
       ifelse([$6],,:,[$6])
    else
       ifelse([$7],,:,[$7])
    fi
])
dnl
dnl Balint Joo, 12/01/2003
dnl
dnl Stole this from mpich-1.2.4/mpe
dnl
dnl PAC_GM_LINK_CC_FUNC( GM_CC, GM_CFLAGS, GM_LIBS,
dnl                       GM_VARS, GM_FUNC,
dnl                       [action if working], [action if not working] )
dnl - GM_CFLAGS  is the extra CFLAGS to CC, like "-I/usr/include" for gm.h
dnl - GM_LDFLAGS is the extra LDFLAGS to CC, like "-L/usr/lib" for libgm.a
dnl - GM_LIBS    is the LIBS to CC, like "-lgm" for libgm.a
dnl - GM_VARS    is the the declaration of variables needed to call GM_FUNC
dnl - GM_FUNC    is the body of MPI function call to be checked for existence
dnl               e.g.  GM_VARS="gm_status_t status;"
dnl                     GM_FUNC="status = gm_open(....)"
dnl               if GM_FUNC is empty, assume linking with basic GM program.
dnl               i.e. check if definitions are valid -- try linking gm_init()
dnl
AC_DEFUN([PAC_GM_LINK_CC_FUNC],[
dnl - set local parallel compiler environments
dnl   so input variables can be CFLAGS, LDFLAGS or LIBS
    pac_GM_CFLAGS="$1"
    pac_GM_LDFLAGS="$2"
    pac_GM_LIBS="$3"
    AC_LANG_SAVE
    AC_LANG_C
dnl - save the original environment
    pac_saved_CFLAGS="$CFLAGS"
    pac_saved_LDFLAGS="$LDFLAGS"
    pac_saved_LIBS="$LIBS"
dnl - set the parallel compiler environment
    CFLAGS="$CFLAGS $pac_GM_CFLAGS"
    LDFLAGS="$LDFLAGS $pac_GM_LDFLAGS"
    LIBS="$LIBS $pac_GM_LIBS"
    AC_TRY_LINK( [#include <gm.h>], [
    int argc; char **argv;
    $4 ; 
    gm_init();
    $5 ;
    gm_finalize();], pac_gm_working=yes, pac_gm_working=no )
    CFLAGS="$pac_saved_CFLAGS"
    LDFLAGS="$pac_saved_LDFLAGS"
    LIBS="$pac_saved_LIBS"
    AC_LANG_RESTORE
    if test "$pac_gm_working" = "yes" ; then
       ifelse([$6],,:,[$6])
    else
       ifelse([$7],,:,[$7])
    fi
])
