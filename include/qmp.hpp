/*----------------------------------------------------------------------------
 * Copyright (c) 2001      Southeastern Universities Research Association,
 *                         Thomas Jefferson National Accelerator Facility
 *
 * This software was developed under a United States Government license
 * described in the NOTICE file included as part of this distribution.
 *
 * Jefferson Lab HPC Group, 12000 Jefferson Ave., Newport News, VA 23606
 *----------------------------------------------------------------------------
 *
 * Description:
 *     Qcd Message Passing Pacakge C++ Public Header File 
 *
 * Author:  
 *      Chulwoo Jung
 *      Physics Department,Coulmbia University
 *
 * Revision History:
 *   $Log: not supported by cvs2svn $
 *   Revision 1.2  2003/02/13 16:41:16  chen
 *   minor change
 *
 *   Revision 1.1  2003/02/13 16:37:55  chen
 *   add qmp.hpp remove qmp.hh
 *
 *   Revision 1.1  2003/02/13 16:22:24  chen
 *   qmp version 1.2
 *
 *
 *
 */

/** 
 * Declareing SIGN as an enum class enables setting defaults for
 * priority and overloading SendTo and SendRelative. 
 */
#ifndef _QMP_CPP_H
#define _QMP_CPP_H

enum SIGN {PLUS = 1, MINUS = -1};

namespace QMP { 

  QMP_status_t      init(int* argc, char ***argv, 
			 QMP_smpaddr_type_t option = QMP_SMP_ONE_ADDRESS);

  void              finalize(void);

  QMP_u32_t         getSMPCount (void);
  QMP_ictype_t      getMsgPassingType (void);
  QMP_u32_t         getNumberOfNodes (void);
  QMP_u32_t         getNodeNumber(void);

  const QMP_u32_t   getAllocatedNumberOfDimensions(void);
  const QMP_u32_t*  getAllocatedDimensions(void);
  const QMP_u32_t*  getAllocatedCoordinates(void);

  QMP_bool_t        declareLogicalTopology (const QMP_u32_t *dims, 
					    QMP_u32_t ndim);
  QMP_bool_t        logicalTopologyIsDeclared(void);
  const QMP_u32_t   getLogicalNumberOfDimensions(void);
  const QMP_u32_t*  getLogicalDimensions(void);
  const QMP_u32_t*  getLogicalCoordinates(void);

  const QMP_u32_t   getNodeNumberFrom (const QMP_u32_t *coor);
  const QMP_u32_t*  getLogicalCoordinatedFrom (QMP_u32_t nodenum);

  QMP_bool_t        layoutGrid (QMP_u32_t *lattDim, QMP_u32_t ndims);
  const QMP_u32_t*  getSubgridDimensions(void);

  const char*       errorString(QMP_status_t code);

  void*             allocateAlignedMemory(QMP_u32_t nbytes);
  void              freeAlignedMemory(void *ab);

  class MessageMemory {

    MessageMemory  (void){};
    ~MessageMemory (void);

    MessageMemory (const void *buffer, QMP_u32_t blksize, 
		   QMP_u32_t nblocks = 1, QMP_u32_t stride = 0);

    MessageMemory (const void **buffer, QMP_u32_t *blksize, 
		   QMP_u32_t *nblocks , QMP_u32_t *stride, QMP_u32_t n);

    void Init     (const void *buffer, QMP_u32_t blksize, 
		   QMP_u32_t nblocks = 1, QMP_u32_t stride = 0);

  };

  class MessageOperation{

    virtual QMP_status_t start(void)  = 0;
    virtual QMP_status_t isComplete(void) = 0;
    virtual QMP_status_t wait(void) = 0;
    virtual QMP_status_t getErrorNumber (void) const  = 0;
    virtual const char * getErrorString (void)  = 0;
  };

  class SingleOperation : public MessageOperation{

    /**
     *  SingleOperation refers to a (possibly multiple strided)
     * MessageOperation to a specific wire or a node.
     */
    SingleOperation (void){};
    ~SingleOperation(void){};
    
    QMP_status_t declareSend (MessageMemory *mm, 
			      QMP_s32_t dimension, SIGN sign, 
			      QMP_s32_t priority = DEFAULT_PRIORITY);

    QMP_status_t declareSend (MessageMemory *mm, QMP_s32_t remoteHost, 
			      QMP_s32_t priority = DEFAULT_PRIORITY);

    QMP_status_t declareReceive (MessageMemory *mm, QMP_s32_t dimension, 
				 SIGN sign, 
				 QMP_s32_t priority  = DEFAULT_PRIORITY);
    QMP_status_t declareReceive (MessageMemory *mm, QMP_s32_t remoteHos, 
				 QMP_s32_t priority  = DEFAULT_PRIORITY);

    /**
     *  It is NOT allowed to "re-use" MessageOperations
     */
    QMP_status_t start (void);
    QMP_status_t isComplete (void);
    QMP_status_t wait (void);
    QMP_status_t getErrorNumber (void) const;
    const char * getErrorString (void);
  };


  class MultiOperation : public MessageOperation{

    /**
     *  MultiOperation  refers to a collection of SingleOperation's
     *  No 2 SingleOperation's should have same wire or the node.
     *  Receiving and Sending operation is considered separate.
     */
    MultiOperation (void){};
    ~MultiOperation(void){};

    MultiOperation(SingleOperation *msgops, QMP_u32_t nmsgops);
    MultiOperation(SingleOperation **msgops, QMP_u32_t nmsgops);

    void init(SingleOperation *msgops, QMP_u32_t nmsgops);
    void init(SingleOperation **msgops, QMP_u32_t nmsgops);

    QMP_status_t start(void);
    QMP_status_t isComplete(void);
    QMP_status_t wait(void);
    QMP_status_t getErrorNumber (void) const;
    const char * getErrorString (void);
  };


  QMP_status_t sumInt(QMP_s32_t *i);
  QMP_status_t sumFloat(QMP_float_t *x);
  QMP_status_t sumDouble(QMP_double_t *x);

  QMP_status_t sumFloatArray(QMP_float_t *x, QMP_u32_t length);
  QMP_status_t sumDoubleArray(QMP_double_t *x, QMP_u32_t length);

  QMP_status_t maxInt(QMP_s32_t *i);
  QMP_status_t maxFloat(QMP_float_t *x);
  QMP_status_t maxDouble(QMP_double_t *x);

  QMP_status_t minInt(QMP_s32_t *i);
  QMP_status_t minFloat(QMP_float_t *x);
  QMP_status_t minDouble(QMP_double_t *x);

  QMP_status_t binaryReduction( void *localvalue, QMP_u32_t nbytes, QMP_binary_func funcptr);
  typedef void (*QMP_binary_func) (void* inoutvec, void* invec);

  /**
   * Maybe using QMP_u64_t is more consistent instead of using long as in C binding?
   */
  QMP_status_t globalXor(QMP_u64_t *lval);
  QMP_status_t broadcast(void *buf, QMP_u32_t nbytes);
  QMP_status_t waitForBarrier(QMP_s32_t milliseconds);
}
#endif
