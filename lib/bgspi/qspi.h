#ifndef QSPI_H
#define QSPI_H

#include <stddef.h>

typedef struct qspi_msg_t *qspi_msg_t;

/* Initialize qspi  ------------*/
void qspi_init(void);

/* Prepare sending a message -----------
  int dest                    destination rank number
  void *buf                   sending buffer address
  int size                    sending buffer size in bytes
  qspi_msg_t send_msg         communication message. It should be declared before 
                              qspi_set_send is called. */
void qspi_set_send(int dest, void *buf, size_t size, qspi_msg_t send_msg);

void qspi_set_send_multi(int dest, void *buf[], size_t size[], int n_msg, qspi_msg_t send_msg);

/* Prepare receiving a message -----------
  int src                    source rank number
  void *buf                  receiving buffer address
  int size                   receiving  buffer size in bytes
  qspi_msg_t recv_msg        communication message. It should be declared before 
                             qspi_set_recv is called. */
void qspi_set_recv(int src, void *buf, size_t size, qspi_msg_t recv_msg);

void qspi_prepare(qspi_msg_t msgs[], int num);


/* Start message communication -----------
   It will send or receive as the msg is given.*/
void qspi_start(qspi_msg_t msg);

/* Wait message communication -----------
   It will send or receive as the msg is given.*/
void qspi_wait(qspi_msg_t msg);

/* Finalize qspi  */
void qspi_finalize(void);

/* create a mssage 
   returns an address of the message */
qspi_msg_t qspi_create_msg(void);

/* free a message */
void qspi_free_msg( qspi_msg_t  msg);

void qspi_sum_double(double *x);

#endif // QSPI_H
