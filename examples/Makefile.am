## Process this file with automake to produce Makefile.in

check_PROGRAMS = QMP_test          \
                 QMP_msg           \
                 QMP_grid_test     \
                 QMP_loopback      \
                 QMP_qcd_test      \
                 QMP_stride_test   \
                 QMP_perf          \
                 QMP_broadcast     \
                 QMP_gcomm_perf    \
                 QMP_MILC_test     \
		 QMP_show_geom

## GTF: The whole point of an API is that you don't need to know where
## to find the header files for package on which you're building, e.g. GM,
## so don't add @QMP_COMMS_CFLAGS@ to AM_CFLAGS.  Unfortunately, you still
## need to know where to find the underlying libraries to resolve symbols.
AM_CFLAGS  = -I@top_srcdir@/include
AM_LDFLAGS = -L../lib @QMP_COMMS_LDFLAGS@
LDADD      = -lqmp @QMP_COMMS_LIBS@ -lm
