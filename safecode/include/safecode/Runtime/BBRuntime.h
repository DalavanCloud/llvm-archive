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

#ifndef _BB_RUNTIME_H_
#define _BB_RUNTIME_H_

#include "safecode/SAFECode.h"
#include "safecode/Runtime/BitmapAllocator.h"

#include <iosfwd>

NAMESPACE_SC_BEGIN

//
// Enumerated Type: allocType
//
// Description:
//  This enumerated type lists the different types of allocations that can be
//  made.
//
enum allocType {
  Global,   // Global object
  Stack,    // Stack-allocated object
  Heap      // Heap-allocated object
};

//
// Structure: DebugMetaData
//
// Description:
//  This structure contains information on the error to be reported.
//
// Fields:
//  allocID    : The ID number of the allocation of the object.
//  freeID     : The ID number of the deallocation of the object.
//  allocPC    : The program counter at which the object was last allocated.
//  freePC     : The program counter at which the object was last deallocated.
//  canonAddr  : The canonical address of the memory reference.
//  SourceFile : A string containing the source file to which the erring
//               instruction is found.
//  lineno     : The line number in the source file to which the erring
//               instruction is found.
//
typedef struct DebugMetaData {
  unsigned allocID;
  unsigned freeID;
  void * allocPC;
  void * freePC;
  void * canonAddr;

  // Allocation type (global, stack, or heap object)
  allocType allocationType;

  // Source filename
  void * SourceFile;

  // Source filename for deallocation
  void * FreeSourceFile;

  // Line number
  unsigned lineno;

  // Line number for deallocation
  unsigned Freelineno;

  void print(std::ostream & OS) const;
} DebugMetaData;
typedef DebugMetaData * PDebugMetaData;

struct DebugPoolTy : public BitmapPoolTy {
};

void * rewrite_ptr (DebugPoolTy * Pool, const void * p, const void * ObjStart,
const void * ObjEnd, const char * SourceFile, unsigned lineno);
void installAllocHooks (void);

NAMESPACE_SC_END

// Use macros so that I won't polluate the namespace

#define PPOOL NAMESPACE_SC::DebugPoolTy*
#define TAG unsigned
#define SRC_INFO const char *, unsigned int

#define UNSET_MASK 0x7fffffffffff
#define SET_MASK 0xffff800000000000

extern "C" {
  void pool_init_runtime(unsigned Dangling,
                         unsigned RewriteOOB,
                         unsigned Terminate);
  void * __sc_bb_newpool(unsigned NodeSize);
  void __sc_bb_pooldestroy(PPOOL);

  void * __sc_bb_poolinit(PPOOL, unsigned NodeSize, unsigned);
  void * __sc_bb_poolalloc(PPOOL, unsigned NumBytes);
  void * __sc_bb_src_poolalloc (PPOOL, unsigned Size, TAG, SRC_INFO);

  void * __sc_bb_poolargvregister (int argc, char ** argv);

  void __sc_bb_poolregister(PPOOL, void *allocaptr, unsigned NumBytes);
  void __sc_bb_src_poolregister (PPOOL, void * p, unsigned size, TAG, SRC_INFO);
  void __sc_bb_poolregister_stack (PPOOL, void * p, unsigned size);
  void __sc_bb_src_poolregister_stack (PPOOL, void * p, unsigned size, TAG, SRC_INFO);
  void __sc_bb_poolregister_global (PPOOL, void * p, unsigned size);
  void __sc_bb_src_poolregister_global_debug (PPOOL, void * p, unsigned size, TAG, SRC_INFO);

  void __sc_bb_poolunregister(PPOOL, void *allocaptr);
  void __sc_bb_poolunregister_stack(PPOOL, void *allocaptr);
  void __sc_bb_poolunregister_debug(PPOOL, void *allocaptr, TAG, SRC_INFO);
  void __sc_bb_poolunregister_stack_debug(PPOOL, void *allocaptr, TAG, SRC_INFO);
  void __sc_bb_poolfree(PPOOL, void *Node);
  void __sc_bb_src_poolfree (PPOOL, void *, TAG, SRC_INFO);

  void * __sc_bb_poolcalloc (PPOOL, unsigned Number, unsigned NumBytes, TAG);
  void * __sc_bb_src_poolcalloc (PPOOL,
                                unsigned Number, unsigned NumBytes,
                                  TAG, SRC_INFO);

  void * __sc_bb_poolrealloc(PPOOL, void *Node, unsigned NumBytes);
  void * __sc_bb_poolrealloc_debug(PPOOL, void *Node, unsigned NumBytes, TAG, SRC_INFO);
  void * __sc_bb_poolstrdup (PPOOL, const char * Node);
  void * __sc_bb_poolstrdup_debug (PPOOL, const char * Node, TAG, SRC_INFO);
  void * __sc_bb_poolmemalign(PPOOL, unsigned Alignment, unsigned NumBytes);

  void bb_poolcheck(PPOOL, void *Node);
  void bb_poolcheckui(PPOOL, void *Node, TAG);
  void bb_poolcheck_debug (PPOOL, void * Node, TAG, SRC_INFO);

  void bb_poolcheckalign(PPOOL, void *Node, unsigned Offset);
  void bb_poolcheckalign_debug (PPOOL, void *Node, unsigned Offset, TAG, SRC_INFO);

  void * bb_boundscheck   (PPOOL, void * Source, void * Dest);
  void * bb_boundscheckui (PPOOL, void * Source, void * Dest);
  void * bb_boundscheckui_debug (PPOOL, void * S, void * D, TAG, SRC_INFO);
  void * bb_boundscheck_debug (PPOOL, void * S, void * D, TAG, SRC_INFO);

  // CStdLib
  void * bb_pool_memcpy(PPOOL dstPool, PPOOL srcPool, void *dst, const void *src, size_t n);
  void * bb_pool_memmove(PPOOL dstPool, PPOOL srcPool, void *dst, const void *src, size_t n);
  void * bb_pool_memset(PPOOL sPool, void *s, int c, size_t n);
  char * bb_pool_strcpy(PPOOL dstPool, PPOOL srcPool, char *dst, const char *src);
  char * bb_pool_strcpy_debug(PPOOL dstPool, PPOOL srcPool, char *dst, const char *src, TAG, SRC_INFO);
  size_t bb_pool_strlen(PPOOL stringPool, const char *string);
  char * pool_strncpy(PPOOL dstPool, PPOOL srcPool, char *dst, const char *src, size_t n);

#ifdef _GNU_SOURCE
  void * bb_pool_mempcpy(PPOOL dstPool, PPOOL srcPool, void *dst, const void *src, size_t n);
#endif

  // Exact checks
  void * bb_exactcheck2 (const char *base, const char *result, unsigned size);
  void * bb_exactcheck2_debug (const char *base, const char *result, unsigned size,
                            TAG, SRC_INFO);

  void __sc_bb_funccheck (unsigned num, void *f, void *g, ...);
  void * pchk_getActualValue (PPOOL, void * src);

  // Change memory protections to detect dangling pointers
  void * bb_pool_shadow (void * Node, unsigned NumBytes);
  void * bb_pool_unshadow (void * Node);
}

#undef PPOOL
#undef TAG
#undef SRC_INFO
#endif
