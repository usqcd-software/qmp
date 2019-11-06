#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#define __USE_UNIX98 /* needed to get gethostname from GNU unistd.h */
#include <unistd.h>
#include <ctype.h>
#include <stdarg.h>

#include "QMP_P_COMMON.h"

// static library data
static QMP_args_t QMP_args_s = {QMP_ARGS_INIT};
QMP_args_t *QMP_args = &QMP_args_s;

static QMP_machine_t QMP_machine_s = {QMP_MACHINE_INIT};
QMP_machine_t *QMP_machine = &QMP_machine_s;

static struct QMP_comm_struct QMP_allocated_comm_s = {QMP_COMM_INIT};
QMP_comm_t QMP_allocated_comm = &QMP_allocated_comm_s;

//static struct QMP_comm_struct QMP_job_comm_s = {QMP_COMM_INIT};
//QMP_comm_t QMP_job_comm = &QMP_job_comm_s;
QMP_comm_t QMP_job_comm = &QMP_allocated_comm_s;

//static struct QMP_comm_struct QMP_default_comm_s = {QMP_COMM_INIT};
//QMP_comm_t QMP_default_comm = &QMP_default_comm_s;
QMP_comm_t QMP_default_comm = &QMP_allocated_comm_s;

// for debugging
int QMP_stack_level = 0;

/**
 * Get the allocated communicator.
 */
QMP_comm_t
QMP_comm_get_allocated(void)
{
  ENTER;
  LEAVE;
  return QMP_allocated_comm;
}

/**
 * Set the allocated communicator.
 */
QMP_status_t
QMP_comm_set_allocated(QMP_comm_t comm)
{
  ENTER;
  QMP_allocated_comm = comm;
  LEAVE;
  return QMP_SUCCESS;
}

/**
 * Get the job communicator.
 */
QMP_comm_t
QMP_comm_get_job(void)
{
  ENTER;
  LEAVE;
  return QMP_job_comm;
}

/**
 * Set the job communicator.
 */
QMP_status_t
QMP_comm_set_job(QMP_comm_t comm)
{
  ENTER;
  QMP_job_comm = comm;
  LEAVE;
  return QMP_SUCCESS;
}

/**
 * Get the default communicator.
 */
QMP_comm_t
QMP_comm_get_default(void)
{
  ENTER;
  LEAVE;
  return QMP_default_comm;
}

/**
 * Set the default communicator.
 */
QMP_status_t
QMP_comm_set_default(QMP_comm_t comm)
{
  ENTER;
  QMP_default_comm = comm;
  LEAVE;
  return QMP_SUCCESS;
}


static int 
lex_rank(const int coords[], int dim, int size[])
{
  int d;
  int rank = coords[dim-1];

  for(d = dim-2; d >= 0; d--) {
    rank = rank * size[d] + coords[d];
  }
  return rank;
}

static void
get_arg(int argc, char **argv, const char *tag, int *first, int *last,
	char **c, int **a)
{
  int i;
  *first = -1;
  *last = -1;
  *c = NULL;
  *a = NULL;
  for(i=1; i<argc; i++) {
    if(strcmp(argv[i], tag)==0) {
      *first = i;
      //printf("%i %i\n", i, argc);
      if( ((i+1)<argc) && !(isdigit(argv[i+1][0])) ) {
	//printf("c %i %s\n", i+1, argv[i+1]);
	*c = argv[i+1];
	*last = i+1;
      } else {
	//printf("a %i %s\n", i+1, argv[i+1]);
	while( (++i<argc) && isdigit(argv[i][0]) );
	*last = i-1;
	int n = *last - *first;
	if(n) {
	  int j;
	  //*a = (int *) malloc(n*sizeof(int));
	  QMP_alloc(*a, int, n);
	  //printf("%i %p\n", n, *a);
	  for(j=0; j<n; j++) {
	    (*a)[j] = atoi(argv[*first+1+j]);
	    //printf(" %i", (*a)[j]);
	  }
	  //printf("\n");
	}
      }
    }
  }
}


static void
remove_from_args(int *argc, char ***argv, int first, int last)
{
  int n = last - first;
  if(first>=0) {
    // shift arguments down including final NULL in *argc
    for(int i=last+1; i<=*argc; i++) (*argv)[i-n-1] = (*argv)[i];
    *argc -= n + 1;
  }
}


static int *
get_int_array(int *len, const char *tag, int *argc, char ***argv)
{
  int first, last, *a=NULL;
  char *c=NULL;
  get_arg(*argc, *argv, tag, &first, &last, &c, &a);
  *len = last - first;
  if(c && strcmp(c,"native")==0) *len = -1;
  //printf("%s %i %i\n", tag, first, last);
  remove_from_args(argc, argv, first, last);
  return a;
}


static int
get_color(void)
{
  int c=0;
  if(QMP_args->jobgeom) {
    if(QMP_comm_logical_topology_is_declared(QMP_allocated_comm)) {
      int i, ndim;
      const int *lc = QMP_comm_get_logical_coordinates(QMP_allocated_comm);
      QMP_assert(QMP_args->njobdim==QMP_get_allocated_number_of_dimensions())
      ndim = QMP_args->njobdim;
      int jc[ndim];
      const int *ad = QMP_get_allocated_dimensions();
      for(i=0; i<ndim; i++) {
	QMP_assert(QMP_args->jobgeom[i]>0);
	QMP_assert(ad[i]%QMP_args->jobgeom[i]==0);
	jc[i] = (lc[i]*QMP_args->jobgeom[i])/ad[i];
      }
      c = lex_rank(jc, ndim, QMP_args->jobgeom);
    } else {
      QMP_assert(QMP_args->njobdim==1);
      QMP_assert(QMP_args->jobgeom[0]>0);
      QMP_assert(QMP_allocated_comm->num_nodes%QMP_args->jobgeom[0]==0);
      c = (QMP_allocated_comm->nodeid*QMP_args->jobgeom[0])/QMP_allocated_comm->num_nodes;
    }
  }
  return c;
}


/* Process QMP command line arguments */
static void
process_args(int* argc, char*** argv)
{
  ENTER;

#ifdef QMP_MPI
  int* nmpi = get_int_array(&QMP_args->geomlen, "-qmp-nmpi", argc, argv);
  if (nmpi)
    {
      if (!QMP_allocated_comm->nodeid)
	printf("nmpi = %d, reducing number of MPI processes being used accordingly\n",*nmpi);

      int my_color = QMP_allocated_comm->nodeid < *nmpi ? 0 : 1;

      MPI_Comm new_comm;
      MPI_Comm_split( QMP_allocated_comm->mpicomm , my_color , QMP_allocated_comm->nodeid , &new_comm );

      QMP_allocated_comm->mpicomm = new_comm;
      QMP_allocated_comm->num_nodes = *nmpi;
      
      if (!QMP_allocated_comm->nodeid)
	printf("MPI communicator replaced with new split communitator\n");

      if (QMP_allocated_comm->nodeid >= *nmpi)
	{
	  printf("process %d exits now\n",QMP_allocated_comm->nodeid);
	  MPI_Finalize();
	  exit(0);
	}
    }
#endif    

  QMP_args->geom = get_int_array(&QMP_args->geomlen, "-qmp-geom", argc, argv);
  QMP_args->amap = get_int_array(&QMP_args->amaplen, "-qmp-alloc-map", argc, argv);
  QMP_args->lmap = get_int_array(&QMP_args->lmaplen, "-qmp-logic-map", argc, argv);
  QMP_args->jobgeom = get_int_array(&QMP_args->njobdim, "-qmp-job", argc, argv);

  QMP_assert(QMP_args->amaplen>=0);
  QMP_assert(QMP_args->lmaplen>=0);

  // set allocated topology (if any)
  QMP_comm_declare_logical_topology_map(QMP_allocated_comm, QMP_args->geom, QMP_args->geomlen,
					QMP_args->amap, QMP_args->amaplen);
  if(QMP_comm_logical_topology_is_declared(QMP_allocated_comm)) QMP_machine->ic_type = QMP_MESH;

  // set job communicator and topology (if any)
  int color = get_color();
  QMP_comm_split(QMP_allocated_comm, color, 0, &QMP_job_comm);
  if(QMP_args->jobgeom && QMP_get_msg_passing_type()==QMP_MESH) {
    int i, ndim = QMP_args->njobdim;
    int geom[ndim];
    const int *ad = QMP_get_allocated_dimensions();
    for(i=0; i<ndim; i++) geom[i] = ad[i] / QMP_args->jobgeom[i];
    QMP_comm_declare_logical_topology_map(QMP_job_comm, geom, ndim,
					  QMP_args->amap, QMP_args->amaplen);
  }

  // set default communicator
  QMP_comm_split(QMP_job_comm, 0, 0, &QMP_default_comm);

  LEAVE;
}


/* Initialize QMP */
QMP_status_t
QMP_init_msg_passing (int* argc, char*** argv, QMP_thread_level_t required,
                      QMP_thread_level_t *provided)
{
  ENTER_INIT;

  QMP_assert(QMP_machine->inited==QMP_FALSE);
  QMP_machine->inited = QMP_TRUE;
  QMP_machine->err_code = QMP_SUCCESS;

#ifdef QMP_INIT_MACHINE
  QMP_INIT_MACHINE(argc, argv, required, provided);
#else
#define MAX_HOST_NAME 256
  QMP_alloc(QMP_machine->host, char, MAX_HOST_NAME);
  gethostname(QMP_machine->host, MAX_HOST_NAME);
  QMP_machine->hostlen = strlen(QMP_machine->host);
  QMP_allocated_comm->num_nodes = 1;
  QMP_allocated_comm->nodeid = 0;
  QMP_allocated_comm->ncolors = 1;
  QMP_allocated_comm->color = 0;
  QMP_allocated_comm->key = 0;
  *provided = required;
#endif
  QMP_machine->mnodes = QMP_allocated_comm->num_nodes;
  QMP_machine->mnodeid = QMP_allocated_comm->nodeid;
  QMP_machine->thread_level = *provided;

  process_args(argc, argv);

#ifdef QMP_INIT_FINISH
  QMP_INIT_FINISH();
#endif

  QMP_barrier();

  LEAVE_INIT;
  return QMP_machine->err_code;
}


/* Shutdown the machine */
void
QMP_finalize_msg_passing(void)
{
  ENTER_INIT;
  QMP_machine->inited = QMP_FALSE;
#ifdef QMP_FINALIZE_MSG_PASSING
  QMP_FINALIZE_MSG_PASSING();
#endif
  LEAVE_INIT;
}


/* Abort the program */
void 
QMP_abort(int error_code)
{
  ENTER_INIT;
  QMP_error("abort: %i", error_code);
#ifdef QMP_ABORT
  QMP_ABORT(error_code);
#endif
  exit(error_code);
  LEAVE_INIT;
}


/* Print string and abort the program */
void
QMP_abort_string(int error_code, char *message)
{
  ENTER_INIT;
  QMP_error(message);
  QMP_error("abort: %i", error_code);
#ifdef QMP_ABORT
  QMP_ABORT(error_code);
#endif
  exit(error_code);
  LEAVE_INIT;
}
