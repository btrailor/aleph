/* net_protected.h
 * bees
 * aleph
 *
 * private header for network types
 */

#ifndef _NET_PROTECTED_H_
#define _NET_PROTECTED_H_

//! use dynamically allocated memory and vectors 
#define NET_USE_MALLOC 0

#if NET_USE_MALLOC
#include <stdlib.h>
#endif

#include "module_common.h"
#include "net.h"
#include "op.h"

// Compatibility macros for dynamic vs static allocation
#ifdef DYNAMIC_NETWORK_ENABLED
  #define NET_OPS_CAPACITY(net)    (net->opsCapacity)
  #define NET_INS_CAPACITY(net)    (net->insCapacity)
  #define NET_OUTS_CAPACITY(net)   (net->outsCapacity)
  #define NET_PARAMS_CAPACITY(net) (net->paramsCapacity)
#else
  #define NET_OPS_CAPACITY(net)    NET_OPS_MAX
  #define NET_INS_CAPACITY(net)    NET_INS_MAX
  #define NET_OUTS_CAPACITY(net)   NET_OUTS_MAX
  #define NET_PARAMS_CAPACITY(net) NET_PARAMS_MAX
#endif
#include "op_derived.h"
#include "param_scaler.h"
#include "types.h"
#include "util.h"

//! input node type
typedef struct _inode {
  //! parent op index in net list
  s32 opIdx;
  //! input index in parent op list
  u8 opInIdx;
  //! play inclusion flag
  u8 play;
} inode_t;

//! output node type (index into inode list)
typedef struct _onode {
  //! output idx in parent op's output list
  u8 opOutIdx;
  //! target input idx in net list
  s16 target;
  //! parent op's index in net list
  s32 opIdx;
} onode_t;

//! parameter I/O node
typedef struct _pnode {
  ParamDesc desc;
  ParamData data;
  ParamScaler scaler;
  //! play inclusion flag
  /// must be separate from inputs list for large input counts!
  u8 play;
} pnode_t;


//! big old class for the network
#ifdef DYNAMIC_NETWORK_ENABLED
// Use dynamic allocation for network arrays
typedef struct _ctlnet {
  //!  op pointers (dynamic)
  op_t** ops;
  //! number of instantiated operators
  u16 numOps;
  //! number of instantiated inputs
  u16 numIns;
  //! number of instantiated outputs
  u16 numOuts;
  //! number of instantiated params
  u16 numParams;
  
  //! allocated capacities
  u16 opsCapacity;
  u16 insCapacity;
  u16 outsCapacity;
  u16 paramsCapacity;

  //! inputs (dynamic)
  inode_t* ins;
  //! outputs (dynamic)
  onode_t* outs;
  //! DSP params (dynamic)
  pnode_t* params;

} ctlnet_t;
#else
// Original fixed allocation for compatibility
typedef struct _ctlnet {
  //!  op pointers
  op_t * ops[NET_OPS_MAX];
  //! number of instantiated operators
  u16 numOps;
  //! number of instantiated inputs
  u16 numIns;
  //! number of instantiated outputs
  u16 numOuts;
  //! number of instantiated params
  u16 numParams;

  //! inputs
  inode_t ins[NET_INS_MAX];
  //! outputs
  onode_t outs[NET_OUTS_MAX];
  //! DSP params
  pnode_t params[NET_PARAMS_MAX];

} ctlnet_t;
#endif

// Include dynamic network functions after struct definitions
#ifdef DYNAMIC_NETWORK_ENABLED
#include "dynamic_network.h"
#endif

////! external variables

//! pointer to network!
extern ctlnet_t* net;

//! flag for legacy scene format (Random operator without SEED input)
extern u8 legacyRandomFormat;


//! --- pointers to system-created ops

//! encoders
extern op_enc_t* opSysEnc[4];
//! function keys and footswitches
extern op_sw_t* opSysSw[6];
//! adc
extern op_adc_t* opSysAdc;
//! preset
extern op_preset_t* opSysPreset;

//! set active
extern void net_set_active(bool v);

#endif // header guard
