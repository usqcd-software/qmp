#ifndef _QMP_P_BGSPI_H
#define _QMP_P_BGSPI_H

#include <QMP_P_MPI.h>
#include <../lib/bgspi/qspi.h>

// machine specific datatypes

#undef MH_TYPES
#define MH_TYPES MH_TYPES_BGSPI MH_TYPES_MPI
#define MH_TYPES_BGSPI int useSPI; qspi_msg_t qspimsg,qspicts;


// machine specific routines

#undef QMP_INIT_MACHINE
#define QMP_INIT_MACHINE QMP_INIT_MACHINE_BGSPI
#undef QMP_FINALIZE_MSG_PASSING
#define QMP_FINALIZE_MSG_PASSING QMP_FINALIZE_MSG_PASSING_BGSPI
#undef QMP_ALLOC_MSGHANDLE
#define QMP_ALLOC_MSGHANDLE QMP_ALLOC_MSGHANDLE_BGSPI
#undef QMP_FREE_MSGHANDLE
#define QMP_FREE_MSGHANDLE QMP_FREE_MSGHANDLE_BGSPI
//#undef QMP_DECLARE_MULTIPLE
//#define QMP_DECLARE_MULTIPLE QMP_DECLARE_MULTIPLE_BGSPI
#undef QMP_CHANGE_ADDRESS
#define QMP_CHANGE_ADDRESS QMP_CHANGE_ADDRESS_BGSPI
#undef QMP_START
#define QMP_START QMP_START_BGSPI
#undef QMP_IS_COMPLETE
#define QMP_IS_COMPLETE QMP_IS_COMPLETE_BGSPI
#undef QMP_WAIT
#define QMP_WAIT QMP_WAIT_BGSPI
#undef QMP_COMM_BROADCAST
#define QMP_COMM_BROADCAST QMP_COMM_BROADCAST_BGSPI

#define QMP_CLEAR_TO_SEND QMP_CLEAR_TO_SEND_BGSPI

//#define QMP_TIME MPI_Wtime

#define QMP_INIT_MACHINE_BGSPI QMP_init_machine_bgspi
extern QMP_status_t QMP_init_machine_bgspi(int* argc, char*** argv,
					   QMP_thread_level_t required,
					   QMP_thread_level_t *provided);

#define QMP_FINALIZE_MSG_PASSING_BGSPI QMP_finalize_msg_passing_bgspi
void QMP_finalize_msg_passing_bgspi(void);

#define QMP_ALLOC_MSGHANDLE_BGSPI QMP_alloc_msghandle_bgspi
void QMP_alloc_msghandle_bgspi(QMP_msghandle_t mh);

#define QMP_FREE_MSGHANDLE_BGSPI QMP_free_msghandle_bgspi
void QMP_free_msghandle_bgspi(QMP_msghandle_t mh);

//#define QMP_DECLARE_MULTIPLE_BGSPI QMP_declare_multiple_bgspi
//void QMP_declare_multiple_bgspi(QMP_msghandle_t mh);

#define QMP_CHANGE_ADDRESS_BGSPI QMP_change_address_bgspi
void QMP_change_address_bgspi(QMP_msghandle_t mh);

#define QMP_CLEAR_TO_SEND_BGSPI QMP_clear_to_send_bgspi
QMP_status_t QMP_clear_to_send_bgspi(QMP_msghandle_t mh);

#define QMP_START_BGSPI QMP_start_bgspi
QMP_status_t QMP_start_bgspi(QMP_msghandle_t mh);

#define QMP_IS_COMPLETE_BGSPI QMP_is_complete_bgspi
QMP_bool_t QMP_is_complete_bgspi(QMP_msghandle_t mh);

#define QMP_WAIT_BGSPI QMP_wait_bgspi
QMP_status_t QMP_wait_bgspi(QMP_msghandle_t mh);

#define QMP_COMM_BROADCAST_BGSPI QMP_comm_broadcast_bgspi
QMP_status_t QMP_comm_broadcast_bgspi(QMP_comm_t comm, void *send_buf, size_t count);


#endif /* _QMP_P_BGSPI_H */
