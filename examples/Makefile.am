## Process this file with automake to produce Makefile.in

if QMP_GM
  QMP_GM_CHECK = gm_loop
endif

check_PROGRAMS = QMP_test          \
                 QMP_msg           \
                 QMP_grid_test     \
                 QMP_loopback      \
                 QMP_qcd_test      \
                 QMP_perf          \
		 QMP_gcomm_perf	   \
		 QMP_MILC_test	   \
                 $(QMP_GM_CHECK)

AM_CFLAGS  = -I@top_srcdir@/src @QMP_COMMS_CFLAGS@
AM_LDFLAGS = -L../src @QMP_COMMS_LDFLAGS@
LDADD      = -lqmp @QMP_COMMS_LIBS@

