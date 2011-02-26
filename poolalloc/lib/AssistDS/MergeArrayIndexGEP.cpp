//===-- MergeGEP.cpp - Merge GEPs for indexing in arrays ------------ ----===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// 
//===----------------------------------------------------------------------===//
#define DEBUG_TYPE "mergearraygep"

#include "llvm/Instructions.h"
#include "llvm/Module.h"
#include "llvm/Pass.h"
#include "llvm/Instructions.h"
#include "llvm/Constants.h"
#include "llvm/Support/GetElementPtrTypeIterator.h"
#include "llvm/Transforms/Utils/Cloning.h"
#include "llvm/ADT/Statistic.h"
#include "llvm/Support/FormattedStream.h"
#include "llvm/Support/Debug.h"
#include <vector>

using namespace llvm;


namespace {
  class MergeArrayGEP : public ModulePass {
  public:
    static char ID;
    MergeArrayGEP() : ModulePass(&ID) {}
    bool runOnModule(Module& M) {
      for (Module::iterator F = M.begin(); F != M.end(); ++F){
        for (Function::iterator B = F->begin(), FE = F->end(); B != FE; ++B) {      
          for (BasicBlock::iterator I = B->begin(), BE = B->end(); I != BE;) {
            GetElementPtrInst *GEP = dyn_cast<GetElementPtrInst>(I);
            I++;
            if(GEP == NULL)
              continue;
            simplifyGEP(GEP);
          }
        }
      }
      return true;
    }
    static void simplifyGEP(GetElementPtrInst *GEP) {
      Value *PtrOp = GEP->getOperand(0);
      if (GEPOperator *Src = dyn_cast<GEPOperator>(PtrOp)) {
        // Note that if our source is a gep chain itself that we wait for that
        // chain to be resolved before we perform this transformation.  This
        // avoids us creating a TON of code in some cases.
        //
        if (GetElementPtrInst *SrcGEP =
            dyn_cast<GetElementPtrInst>(Src->getOperand(0)))
          if (SrcGEP->getNumOperands() == 2)
            return;   // Wait until our source is folded to completion.

        SmallVector<Value*, 8> Indices;

        // Find out whether the last index in the source GEP is a sequential idx.
        bool EndsWithSequential = false;
        for (gep_type_iterator I = gep_type_begin(*Src), E = gep_type_end(*Src);
             I != E; ++I)
          EndsWithSequential = !(*I)->isStructTy();

        // Can we combine the two pointer arithmetics offsets?
        if (EndsWithSequential) {
          // Replace: gep (gep %P, long B), long A, ...
          // With:    T = long A+B; gep %P, T, ...
          //
          Value *Sum;
          Value *SO1 = Src->getOperand(Src->getNumOperands()-1);
          Value *GO1 = GEP->getOperand(1);
          if (SO1 == Constant::getNullValue(SO1->getType())) {
            Sum = GO1;
          } else if (GO1 == Constant::getNullValue(GO1->getType())) {
            Sum = SO1;
          } else {
            // If they aren't the same type, then the input hasn't been processed
            // by the loop above yet (which canonicalizes sequential index types to
            // intptr_t).  Just avoid transforming this until the input has been
            // normalized.
            if (SO1->getType() != GO1->getType())
              return;
            Sum = llvm::BinaryOperator::Create(BinaryOperator::Add,SO1, GO1, PtrOp->getName()+".sum",GEP);
          }

          // Update the GEP in place if possible.
          if (Src->getNumOperands() == 2) {
            GEP->setOperand(0, Src->getOperand(0));
            GEP->setOperand(1, Sum);
            return;
          }
          Indices.append(Src->op_begin()+1, Src->op_end()-1);
          Indices.push_back(Sum);
          Indices.append(GEP->op_begin()+2, GEP->op_end());
        } else if (isa<Constant>(GEP->idx_begin()) &&
                   cast<Constant>(GEP->idx_begin())->isNullValue() &&
                   Src->getNumOperands() != 1) {
          // Otherwise we can do the fold if the first index of the GEP is a zero
          Indices.append(Src->op_begin()+1, Src->op_end());
          Indices.append(GEP->idx_begin()+1, GEP->idx_end());
        }

        if (!Indices.empty()){
          GetElementPtrInst *GEPNew =  (GEP->isInBounds() && Src->isInBounds()) ?
            GetElementPtrInst::CreateInBounds(Src->getOperand(0), Indices.begin(),
                                              Indices.end(), GEP->getName(), GEP) :
            GetElementPtrInst::Create(Src->getOperand(0), Indices.begin(),
                                      Indices.end(), GEP->getName(), GEP);
          GEP->replaceAllUsesWith(GEPNew);
          GEP->eraseFromParent();
        }
      }


    }
  };
}

char MergeArrayGEP::ID = 0;
static RegisterPass<MergeArrayGEP>
X("mergearrgep", "Merge GEPs for arrays indexing");