//===- SAFECodeRuntime.h -- Runtime interface of SAFECode ------*- C++ -*-===//
// 
//                     The LLVM Compiler Infrast`ructure
//
// This file was developed by the LLVM research group and is distributed under
// the University of Illinois Open Source License. See LICENSE.TXT for details.
// 
//===----------------------------------------------------------------------===//
//
// This file defines the interface of SAFECode runtime library.
//
//===----------------------------------------------------------------------===//

#ifndef _SAFECODE_RUNTIME_H_
#define _SAFECODE_RUNTIME_H_

#include "AtomicOps.h"

#ifdef __cplusplus
extern "C" {
#endif

  struct PoolTy;

  typedef llvm::safecode::ConditionalCounter SCSyncToken;


  ///  All functions that perform checking return a synchronization token
  SCSyncToken * __sc_poolcheck(PoolTy *Pool, void *Node);
  SCSyncToken * __sc_poolcheckui(PoolTy *Pool, void *Node);
  SCSyncToken * __sc_boundscheck   (PoolTy * Pool, void * Source, void * Dest);
  SCSyncToken * __sc_boundscheckui (PoolTy * Pool, void * Source, void * Dest) __attribute__ ((always_inline));
  void __sc_wait_for_completion(SCSyncToken * token);
  void __sc_spec_runtime_init (void);
  void __sc_spec_runtime_cleanup (void);
#ifdef __cplusplus
}
#endif

#endif
