/*
 * show layout geometry
 */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "qmp.h"

int
main(int argc, char **argv)
{
  int i, n, rank;
  QMP_status_t status;
  QMP_thread_level_t req, prv;

  req = QMP_THREAD_SINGLE;
  status = QMP_init_msg_passing(&argc, &argv, req, &prv);

  n = QMP_get_number_of_nodes();
  rank = QMP_get_node_number();

  if(rank==0) {
    printf("number of nodes = %i\n", n);
    if(QMP_get_msg_passing_type()==QMP_GRID) {
      printf("machine type is GRID\n");
    } else
    if(QMP_get_msg_passing_type()==QMP_SWITCH) {
      printf("machine type is SWITCH\n");
    } else {
      printf("machine type is not SWITCH or GRID\n");
    }
  }

  if(QMP_get_msg_passing_type()==QMP_GRID) {
    int nd;
    nd = QMP_get_allocated_number_of_dimensions();
    if(rank==0) {
      printf("allocated number of dimensions = %i\n", nd);
    }
    if(nd) {
      double *coord = (double *) malloc(2*nd*sizeof(double));
      const int *ad = QMP_get_allocated_dimensions();
      const int *ac = QMP_get_allocated_coordinates();

      status = QMP_declare_logical_topology(ad, nd);
      const int *ld = QMP_get_logical_dimensions();
      const int *lc = QMP_get_logical_coordinates();

      if(rank==0) {
	printf("allocated dimensions =");
	for(i=0; i<nd; i++) printf(" %i", ad[i]);
	printf("\n");
	printf("logical dimensions =");
	for(i=0; i<nd; i++) printf(" %i", ld[i]);
	printf("\n");
	printf("%-6s | %*s %*s %*s\n", "node", 2*nd+3, "alloc", 2*nd-2, "|", 2*nd+3, "logic");
      }

      for(i=0; i<n; i++) {
	if(i==rank) {
	  int j;
	  for(j=0; j<nd; j++) {
	    coord[j] = (double) ac[j];
	    coord[nd+j] = (double) lc[j];
	  }
	} else {
	  int j;
	  for(j=0; j<2*nd; j++) {
	    coord[j] = 0;
	  }
	}
	QMP_sum_double_array(coord, 2*nd);
	if(rank==0) {
	  int j;
	  printf("%-6i |", i);
	  for(j=0; j<nd; j++) printf(" %3g", coord[j]);
	  printf("  |");
	  for(j=0; j<nd; j++) printf(" %3g", coord[nd+j]);
	  printf("\n");
	}
      }
    }
  }

  QMP_finalize_msg_passing();
  return 0;
}
