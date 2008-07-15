#ifndef INSERT_BOUNDS_H
#define INSERT_BOUNDS_H

#include "safecode/Config/config.h"
#include "ConvertUnsafeAllocas.h"
#include "llvm/Analysis/ScalarEvolutionExpressions.h"
#include "llvm/Pass.h"

#ifndef LLVA_KERNEL
#include "SafeDynMemAlloc.h"
#include "poolalloc/PoolAllocate.h"
#endif

#include <map>
#include <set>

#ifdef LLVA_KERNEL
#define LLVA_ICONTEXT_SIZE     (18*4)
#define LLVA_INTEGERSTATE_SIZE (18*4)
#define LLVA_FPSTATE_SIZE      (27*4)
#endif
namespace llvm {

ModulePass *creatInsertPoolChecks();
using namespace CUA;

struct AlignCheckAnalysis : public ModulePass {
  public:
    virtual bool runOnModule (Module & M);
    virtual void getAnalysisUsage(AnalysisUsage &AU) const {
#ifdef LLVA_KERNEL
      AU.addRequired<TDDataStructures>();
#else
      AU.addRequired<EquivClassGraphs>();
      AU.addRequired<PoolAllocate>();
#endif
      AU.setPreservesAll();
    }

    bool nodeNeedsAlignment (DSNode * Node) {
      return ((AlignmentNodes.find (Node)) != (AlignmentNodes.end()));
    }
  private:
    // Private variables
#ifdef LLVA_KERNEL
    TDDataStructures * TDPass;
#else
    PoolAllocate     * paPass;
    EquivClassGraphs * equivPass;
#endif

    // Set of DSNodes that require alignment checks
    std::set<DSNode *> AlignmentNodes;

    // Private methods
    void addLinksNeedingAlignment (DSNode * Node);
    DSGraph & getDSGraph (Function & F);
};

struct PreInsertPoolChecks : public ModulePass {
  public:
    virtual bool runOnModule (Module & M);
    virtual void getAnalysisUsage(AnalysisUsage &AU) const {
      // Required passes
      AU.addRequired<ConvertUnsafeAllocas>();
      AU.addRequired<TDDataStructures>();
      AU.addRequired<TargetData>();
#ifndef LLVA_KERNEL
      AU.addRequired<EquivClassGraphs>();
      AU.addRequired<PoolAllocate>();
      AU.addRequired<EmbeCFreeRemoval>();
#endif

      // Preserved passes
      AU.addPreserved<ConvertUnsafeAllocas>();
      AU.addPreserved<TDDataStructures>();
      AU.addPreserved<TargetData>();
    }

    bool nodeNeedsAlignment (DSNode * Node) {
      return ((AlignmentNodes.find (Node)) != (AlignmentNodes.end()));
    }
  private:
#ifndef  LLVA_KERNEL
    PoolAllocate * paPass;
    EquivClassGraphs *equivPass;
    EmbeCFreeRemoval *efPass;
#endif  

    // Private variables
    CUA::ConvertUnsafeAllocas * cuaPass;
    TDDataStructures * TDPass;
    TargetData * TD;

    // Set of DSNodes that require alignment checks
    std::set<DSNode *> AlignmentNodes;

    // External functions in the SAFECode run-time library
    Function *PoolCheck;
    Function *PoolCheckArray;
    Function *PoolCheckIArray;
    Function *ExactCheck;
    Function *FunctionCheck;
    Function *FunctionCheckT;
    Function *FunctionCheckG;
    Function *ICCheck;
    Function *BoundsCheck;
    Function *UIBoundsCheck;
    Function *getBounds;
    Function *UIgetBounds;
    Function *ExactCheck2;
    Function *ExactCheck3;
    Function *GetActualValue;
    Function *PoolRegister;
    Function *StackRegister;
    Function *ObjFree;
    Function *StackFree;
    Function *FuncRegister;
    Function *PoolRegMP;
    Function *PoolFindMP;
    Function *getBegin;
    Function *getEnd;

    // Private methods
    void addPoolCheckProto(Module &M);
    void registerGlobalArraysWithGlobalPools(Module &M);
    void addLinksNeedingAlignment (DSNode * Node);
#ifndef LLVA_KERNEL
    Value * getPoolHandle(const Value *V, Function *F, PA::FuncInfo &FI, bool collapsed = false);
#endif
    Value * createPoolHandle (const Value * V, Function * F);
    Value * createPoolHandle (Module & M, DSNode * Node);
    DSGraph & getDSGraph (Function & F);
    Value* getPD(DSNode* N, Module& M) { 
      if (!N) return 0;
      createPoolHandle (M, N);
      if (N->getMP())
        return N->getMP()->getMetaPoolValue();
      else
        return 0;
    }
};

struct InsertPoolChecks : public FunctionPass {
  public :
    const char *getPassName() const { return "Inserting pool checks for array bounds "; }
    virtual bool doInitialization (Module &M);
    virtual bool doFinalization   (Module &M);
    virtual bool runOnFunction (Function & F);
    virtual void getAnalysisUsage(AnalysisUsage &AU) const {
      AU.addRequired<ConvertUnsafeAllocas>();
      AU.addRequired<PreInsertPoolChecks>();
      AU.addPreserved<PreInsertPoolChecks>();
#if 0
      AU.addRequired<CompleteBUDataStructures>();
      AU.addRequired<TDDataStructures>();
#endif
#ifndef LLVA_KERNEL      
      AU.addRequired<EquivClassGraphs>();
      AU.addRequired<PoolAllocate>();
      AU.addRequired<EmbeCFreeRemoval>();
      AU.addRequired<TargetData>();
#else 
      AU.addRequired<ScalarEvolution>();
      AU.addRequired<LoopInfo>();
      AU.addRequired<TDDataStructures>();
      AU.addRequired<TargetData>();
#endif
    };

  private :
    // Prerequisite passes
    PreInsertPoolChecks * preSCPass;
    CUA::ConvertUnsafeAllocas * cuaPass;
    TargetData * TD;
#ifndef  LLVA_KERNEL
    PoolAllocate * paPass;
    EquivClassGraphs *equivPass;
    EmbeCFreeRemoval *efPass;
#else
    TDDataStructures * TDPass;
    ScalarEvolution  * scevPass;
    LoopInfo         * LI;
#endif  

    // Map of DSNode to Function List Global Variables
    std::map<Value *, GlobalVariable *> FuncListMap;

    // Set of DSNodes for which we do full bounds checks
    std::set<DSNode *> PHNeeded;

    // Functions
    Function *PoolCheck;
    Function *PoolCheckIO;
    Function *PoolCheckIOI;
    Function *PoolCheckAlign;
    Function *PoolCheckUI;
    Function *PoolCheckAlignUI;
    Function *PoolCheckArray;
    Function *PoolCheckIArray;
    Function *ExactCheck;
    Function *FunctionCheck;
    Function *FunctionCheckT;
    Function *FunctionCheckG;
    Function *ICCheck;
    Function *BoundsCheck;
    Function *UIBoundsCheck;
    Function *getBounds;
    Function *UIgetBounds;
    Function *ExactCheck2;
    Function *ExactCheck3;
    Function *GetActualValue;
    Function *PoolRegister;
    Function *StackRegister;
    Function *IORegister;
    Function *IOFree;
    Function *ObjFree;
    Function *StackFree;
    Function *IO;
    Function *FuncRegister;
    Function *KmemCachegetSize;
    Function *PoolRegMP;
    Function *PoolFindMP;
    Function *getBegin;
    Function *getEnd;
    Function *declareStack;

    void simplifyGEPList();
    void addObjFrees(Module& M);
    void addMetaPools(Module& M, MetaPool* MP, DSNode* N);
    void addPoolCheckProto(Module &M);
    void addPoolChecks(Module &M);
    void addGetElementPtrChecks(Module &M);
    DSGraph & getDSGraph (Function & F);
    DSNode* getDSNode(const Value *V, Function *F);
    unsigned getDSNodeOffset(const Value *V, Function *F);
    void addLoadStoreChecks (Function & F);
    void addDeclaredStackChecks (Function & F);
    void insertAlignmentCheck (LoadInst * LI);
    void TransformFunction(Function &F);
    void handleCallInst(CallInst *CI);
    void handleGetElementPtr(GetElementPtrInst *MAI);
    void addGetActualValue(SetCondInst *SCI, unsigned operand);
    void registerAllocaInst(AllocaInst *AI, AllocaInst *AIOrig);
    GlobalVariable * getOrCreateFunctionTable (DSNode * Node, Value * PH, Module * M);
    void addExactCheck  (Instruction * GEP, Value * Index, Value * Bound);
    void addExactCheck  (Value * Pointer, Value * Index, Value * Bound, Instruction * InsertPt);
    void addExactCheck2 (Value * Base, Value * Result, Value * Bound, Instruction * InsertPt);
    Value * addExactCheck3 (Value * Source, Value * Result, Value * Bound, Instruction * Next);
    bool insertExactCheck (GetElementPtrInst * GEP);
    bool insertExactCheck (Instruction * , Value *, Value *, Instruction *);
    void insertFunctionCheck(CallInst* CI);
    void insertICCheck (Value * Pointer, Instruction * InsertPt);
    Value * insertBoundsCheck (Instruction * , Value *, Value *, Instruction *);
    bool AggregateGEPs (GetElementPtrInst * GEP, std::set<Instruction *> & GEPs);
    bool findCheckedPointer (Value * PointerOperand);
    void addHeapRegs (Module & M);

#ifndef LLVA_KERNEL  
    void addLSChecks(Value *Vnew, const Value *V, Instruction *I, Function *F);
    Value * getPoolHandle(const Value *V, Function *F, PA::FuncInfo &FI, bool collapsed = false);
    Value * getPoolHandle(const Value *V, Function *F, bool collapsed = false);
#else
    Value* getPD(DSNode* N, Module& M) { 
      if (!N) return 0;
      addMetaPools(M, N->getMP(), N);
      if (N->getMP())
        return N->getMP()->getMetaPoolValue();
      else
        return 0;
    }
    void addLSChecks(Value *V, Instruction *I, Function *F);
    void addIOLSChecks (Value *V, Instruction *I, Function *F);
    Value * getPoolHandle(const Value *V, Function *F);
#endif  
};
}
#endif
