//===-- Compiler.cpp - Java bytecode compiler -------------------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file was developed by the LLVM research group and is distributed under
// the University of Illinois Open Source License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file contains Java bytecode to LLVM bytecode compiler.
//
//===----------------------------------------------------------------------===//

#define DEBUG_TYPE "javacompiler"

#include <llvm/Java/Bytecode.h>
#include <llvm/Java/BytecodeParser.h>
#include <llvm/Java/ClassFile.h>
#include <llvm/Java/Compiler.h>
#include <llvm/Constants.h>
#include <llvm/DerivedTypes.h>
#include <llvm/Instructions.h>
#include <llvm/Value.h>
#include <llvm/Type.h>
#include <Support/Debug.h>
#include <Support/SetVector.h>
#include <Support/STLExtras.h>
#include <Support/StringExtras.h>
#include <iostream>
#include <stack>
#include <vector>

using namespace llvm;
using namespace llvm::Java;

namespace llvm { namespace Java { namespace {

    const std::string TMP("tmp");

    typedef std::vector<BasicBlock*> BC2BBMap;
    typedef std::stack<Value*, std::vector<Value*> > OperandStack;
    typedef std::vector<Value*> Locals;

    inline bool isTwoSlotValue(const Value* v) {
        return v->getType() == Type::LongTy | v->getType() == Type::DoubleTy;
    }

    inline bool isOneSlotValue(const Value* v) {
        return !isTwoSlotValue(v);
    }

    llvm::Constant* getConstant(Constant* c) {
        if (dynamic_cast<ConstantString*>(c))
            assert(0 && "not implemented");
        else if (ConstantInteger* i =
                 dynamic_cast<ConstantInteger*>(c))
            return ConstantSInt::get(Type::IntTy, i->getValue());
        else if (ConstantFloat* f = dynamic_cast<ConstantFloat*>(c))
            return ConstantFP::get(Type::FloatTy, f->getValue());
        else if (ConstantLong* l = dynamic_cast<ConstantLong*>(c))
            return ConstantSInt::get(Type::LongTy, l->getValue());
        else if (ConstantDouble* d = dynamic_cast<ConstantDouble*>(c))
            return ConstantFP::get(Type::DoubleTy, d->getValue());
        else
            return NULL; // FIXME: throw something
    }

    struct Bytecode2BasicBlockMapper
        : public BytecodeParser<Bytecode2BasicBlockMapper> {
    public:
        Bytecode2BasicBlockMapper(Function& f,
                                  BC2BBMap& m,
                                  CodeAttribute& c)
            : function_(f), bc2bbMap_(m), codeAttr_(c) { }

        void compute() {
            bc2bbMap_.clear();
            bc2bbMap_.assign(codeAttr_.getCodeSize(), NULL);

            BasicBlock* bb = new BasicBlock("entry", &function_);

            parse(codeAttr_.getCode(), codeAttr_.getCodeSize());

            for (unsigned i = 0; i < bc2bbMap_.size(); ++i) {
                if (bc2bbMap_[i])
                    bb = bc2bbMap_[i];
                else
                    bc2bbMap_[i] = bb;
            }

            assert(function_.getEntryBlock().getName() == "entry");
        }

        void do_if(unsigned bcI, JSetCC cc, JType type,
                   unsigned t, unsigned f) {
            if (!bc2bbMap_[t])
                bc2bbMap_[t] =
                    new BasicBlock("bc" + utostr(t), &function_);
            if (!bc2bbMap_[f])
                bc2bbMap_[f] =
                    new BasicBlock("bc" + utostr(f), &function_);
        }

        void do_ifcmp(unsigned bcI, JSetCC cc,
                      unsigned t, unsigned f) {
            if (!bc2bbMap_[t])
                bc2bbMap_[t] =
                    new BasicBlock("bc" + utostr(t), &function_);
            if (!bc2bbMap_[f])
                bc2bbMap_[f] =
                    new BasicBlock("bc" + utostr(f), &function_);
        }

        void do_switch(unsigned bcI,
                       unsigned defTarget,
                       const SwitchCases& sw) {
            for (unsigned i = 0; i < sw.size(); ++i) {
                unsigned target = sw[i].second;
                if (!bc2bbMap_[target])
                    bc2bbMap_[target] =
                        new BasicBlock("bc" + utostr(target), &function_);
            }
            if (!bc2bbMap_[defTarget])
                bc2bbMap_[defTarget] =
                    new BasicBlock("bc" + utostr(defTarget), &function_);
        }

    private:
        Function& function_;
        BC2BBMap& bc2bbMap_;
        const CodeAttribute& codeAttr_;
    };

    struct CompilerImpl :
        public BytecodeParser<CompilerImpl> {
    private:
        Module* module_;
        ClassFile* cf_;
        OperandStack opStack_;
        Locals locals_;
        BC2BBMap bc2bbMap_;
        BasicBlock* prologue_;
        typedef SetVector<Function*> FunctionSet;
        FunctionSet toCompileFunctions_;
        typedef std::map<std::string, Type*> Class2TypeMap;
        Class2TypeMap c2tMap_;

    private:
        BasicBlock* getBBAt(unsigned bcI) { return bc2bbMap_[bcI]; }

    private:
        Type* getType(JType type) {
            switch (type) {
            case REFERENCE:
                return PointerType::get(getTypeForClass("java/lang/Object"));
            case BOOLEAN: return Type::BoolTy;
            case CHAR: return Type::UShortTy;
            case FLOAT: return Type::FloatTy;
            case DOUBLE: return Type::DoubleTy;
            case BYTE: return Type::SByteTy;
            case SHORT: return Type::ShortTy;
            case INT: return Type::IntTy;
            case LONG: return Type::LongTy;
            default: assert(0 && "Invalid JType to Type conversion!");
            }

            return NULL;
        }

        Instruction::BinaryOps getSetCC(JSetCC cc) {
            switch (cc) {
            case EQ: return Instruction::SetEQ;
            case NE: return Instruction::SetNE;
            case LT: return Instruction::SetLT;
            case GE: return Instruction::SetGE;
            case GT: return Instruction::SetGT;
            case LE: return Instruction::SetLE;
            default: assert(0 && "Invalid JSetCC to BinaryOps conversion!");
            }
            return static_cast<Instruction::BinaryOps>(-1);
        }

        Type* getType(ConstantUtf8* descr) {
            unsigned i = 0;
            return getTypeHelper(descr->str(), i);
        }

        Type* getTypeHelper(const std::string& descr, unsigned& i) {
            assert(i < descr.size());
            switch (descr[i++]) {
            case 'B': return Type::SByteTy;
            case 'C': return Type::UShortTy;
            case 'D': return Type::DoubleTy;
            case 'F': return Type::FloatTy;
            case 'I': return Type::IntTy;
            case 'J': return Type::LongTy;
            case 'S': return Type::ShortTy;
            case 'Z': return Type::BoolTy;
            case 'V': return Type::VoidTy;
            case 'L': {
                unsigned e = descr.find(';', i);
                std::string className = descr.substr(i, e - i);
                i = e + 1;
                return PointerType::get(getTypeForClass(className));
            }
            case '[':
                // FIXME: this should really be a new class
                // represeting the array of the following type
                return PointerType::get(
                    ArrayType::get(getTypeHelper(descr, i), 0));
            case '(': {
                std::vector<const Type*> params;
                while (descr[i] != ')')
                    params.push_back(getTypeHelper(descr, i));
                return FunctionType::get(getTypeHelper(descr, ++i),
                                         params, false);
            }
            // FIXME: Throw something
            default:  return NULL;
            }
        }

        Type* getTypeForClass(const std::string& className) {
            Class2TypeMap::iterator it = c2tMap_.lower_bound(className);
            if (it == c2tMap_.end() || it->first != className) {
                ClassFile* cf = ClassFile::getClassFile(className);
                OpaqueType* newType = OpaqueType::get();
                it = c2tMap_.insert(it, std::make_pair(className, newType));
                std::vector<const Type*> elements;
                if (ConstantClass* super = cf->getSuperClass())
                    elements.push_back
                        (getTypeForClass(super->getName()->str()));
                const Fields& fields = cf->getFields();
                for (unsigned i = 0, e = fields.size(); i != e; ++i) {
                    Field* field = fields[i];
                    if (!field->isStatic())
                        elements.push_back(getType(field->getDescriptor()));
                }
                PATypeHolder holder = newType;
                newType->refineAbstractTypeTo(StructType::get(elements));
                it->second = holder.get();
                DEBUG(std::cerr << "Adding " << className << " = "
                      << *it->second << " to type map\n");
                module_->addTypeName(className, it->second);
            }
            return it->second;
        }

        Value* getOrCreateLocal(unsigned index, Type* type) {
            if (!locals_[index] ||
                cast<PointerType>(locals_[index]->getType())->getElementType() != type) {
                locals_[index] = new AllocaInst
                    (type, NULL, "local" + utostr(index), prologue_);
            }

            return locals_[index];
        }

        GlobalVariable* getStaticField(unsigned index) {
            ConstantFieldRef* fieldRef =
                (ConstantFieldRef*)(cf_->getConstantPool()[index]);
            ConstantNameAndType* nameAndType = fieldRef->getNameAndType();

            std::string globalName =
                fieldRef->getClass()->getName()->str() + '/' +
                nameAndType->getName()->str();

            GlobalVariable* global = module_->getGlobalVariable
                (globalName, getType(nameAndType->getDescriptor()));

            return global;
        }

        Function* compileMethodOnly(const std::string& classMethodDesc) {
            DEBUG(std::cerr << "Compiling method: " << classMethodDesc << '\n');

            Method* method;
            tie(cf_, method) = findClassAndMethod(classMethodDesc);

            std::string name = cf_->getThisClass()->getName()->str();
            name += '/';
            name += method->getName()->str();
            name += method->getDescriptor()->str();

            Function* function = module_->getOrInsertFunction
                (name, cast<FunctionType>(getType(method->getDescriptor())));
            function->setLinkage(method->isPrivate() ?
                                 Function::InternalLinkage :
                                 Function::ExternalLinkage);

            Java::CodeAttribute* codeAttr = method->getCodeAttribute();

            while (!opStack_.empty())
                opStack_.pop();

            locals_.clear();
            locals_.assign(codeAttr->getMaxLocals(), NULL);

            Bytecode2BasicBlockMapper mapper(*function, bc2bbMap_, *codeAttr);
            mapper.compute();

            prologue_ = new BasicBlock("prologue");

            parse(codeAttr->getCode(), codeAttr->getCodeSize());

            // if the prologue is not empty, make it the entry block
            // of the function with entry as its only successor
            if (prologue_->empty())
                delete prologue_;
            else {
                function->getBasicBlockList().push_front(prologue_);
                new BranchInst(prologue_->getNext(), prologue_);
            }

            return function;
        }

        void emitStaticInitializers(const ClassFile* classfile) {
            const Method* method = classfile->getMethod("<clinit>()V");
            if (!method)
                return;

            std::string name = classfile->getThisClass()->getName()->str();
            name += '/';
            name += method->getName()->str();
            name += method->getDescriptor()->str();

            Function* hook =
                module_->getOrInsertFunction("llvm_java_static_init",
                                             Type::VoidTy, 0);
            Function* init =
                module_->getOrInsertFunction(name, Type::VoidTy, 0);

            // if this is the first time we scheduled this function
            // for compilation insert a call to it right before the
            // terminator of the only basic block in
            // llvm_java_static_init
            if (toCompileFunctions_.insert(init)) {
                assert(hook->front().getTerminator() &&
                       "llvm_java_static_init should have a terminator!");
                new CallInst(init, "", hook->front().getTerminator());
                // we also create the global variables of this class
                const Fields& fields = classfile->getFields();
                for (unsigned i = 0, e = fields.size(); i != e; ++i) {
                    Field* field = fields[i];
                    if (field->isStatic()) {
                        llvm::Constant* init = NULL;
                        if (ConstantValueAttribute* cv =
                            field->getConstantValueAttribute())
                            init = getConstant(cv->getValue());

                        new GlobalVariable(getType(field->getDescriptor()),
                                           field->isFinal(),
                                           (field->isPrivate() & bool(init) ?
                                            GlobalVariable::InternalLinkage :
                                            GlobalVariable::ExternalLinkage),
                                           init,
                                           classfile->getThisClass()->getName()->str() + '/' + field->getName()->str(),
                                           module_);
                    }
                }
            }
        }

        std::pair<ClassFile*, Method*>
        findClassAndMethod(const std::string& classMethodDesc) {
            unsigned slash = classMethodDesc.find('/');
            std::string className = classMethodDesc.substr(0, slash);
            std::string methodNameAndDescr = classMethodDesc.substr(slash+1);

            ClassFile* classfile = ClassFile::getClassFile(className);
            emitStaticInitializers(classfile);
            Method* method = classfile->getMethod(methodNameAndDescr);

            if (!method)
                throw InvocationTargetException(
                    "Method " + methodNameAndDescr +
                    " not found in class " + className);


            return std::make_pair(classfile, method);
        }

    public:
        Function* compileMethod(Module& module,
                                const std::string& classMethodDesc) {
            module_ = &module;
            // initialize the static initializer function
            Function* staticInit =
                module_->getOrInsertFunction("llvm_java_static_init",
                                             Type::VoidTy, 0);
            BasicBlock* staticInitBB = new BasicBlock("entry", staticInit);
            new ReturnInst(NULL, staticInitBB);

            // insert an opaque type for java.lang.Object. This is
            // defined in runtime.ll
            c2tMap_.insert(std::make_pair("java/lang/Object",
                                          OpaqueType::get()));
            module.addTypeName("java/lang/Object", c2tMap_["java/lang/Object"]);

            // compile the method requested
            Function* function = compileMethodOnly(classMethodDesc);
            // compile all other methods called by this method recursively
            for (unsigned i = 0; i != toCompileFunctions_.size(); ++i) {
                Function* f = toCompileFunctions_[i];
                compileMethodOnly(f->getName());
            }

            return function;
        }

        void do_aconst_null(unsigned bcI) {
            opStack_.push(llvm::Constant::getNullValue(getType(REFERENCE)));
        }

        void do_iconst(unsigned bcI, int value) {
            opStack_.push(ConstantSInt::get(Type::IntTy, value));
        }

        void do_lconst(unsigned bcI, long long value) {
            opStack_.push(ConstantSInt::get(Type::LongTy, value));
        }

        void do_fconst(unsigned bcI, float value) {
            opStack_.push(ConstantFP::get(Type::FloatTy, value));
        }

        void do_dconst(unsigned bcI, double value) {
            opStack_.push(ConstantFP::get(Type::DoubleTy, value));
        }

        void do_ldc(unsigned bcI, unsigned index) {
            Constant* c = cf_->getConstantPool()[index];
            assert(getConstant(c) && "Java constant not handled!");
            opStack_.push(getConstant(c));
        }

        void do_load(unsigned bcI, JType type, unsigned index) {
            opStack_.push(new LoadInst(getOrCreateLocal(index, getType(type)),
                                       TMP, getBBAt(bcI)));
        }

        void do_aload(unsigned bcI, JType type) {
            assert(0 && "not implemented");
        }

        void do_store(unsigned bcI, JType type, unsigned index) {
            Value* v1 = opStack_.top(); opStack_.pop();
            opStack_.push(
                new StoreInst(v1, getOrCreateLocal(index, getType(type)),
                              getBBAt(bcI)));
        }


        void do_astore(unsigned bcI, JType type) {
            assert(0 && "not implemented");
        }

        void do_pop(unsigned bcI) {
            opStack_.pop();
        }

        void do_pop2(unsigned bcI) {
            Value* v1 = opStack_.top(); opStack_.pop();
            if (isOneSlotValue(v1))
                opStack_.pop();
        }

        void do_dup(unsigned bcI) {
            opStack_.push(opStack_.top());
        }

        void do_dup_x1(unsigned bcI) {
            Value* v1 = opStack_.top(); opStack_.pop();
            Value* v2 = opStack_.top(); opStack_.pop();
            opStack_.push(v1);
            opStack_.push(v2);
            opStack_.push(v1);
        }

        void do_dup_x2(unsigned bcI) {
            Value* v1 = opStack_.top(); opStack_.pop();
            Value* v2 = opStack_.top(); opStack_.pop();
            if (isOneSlotValue(v2)) {
                Value* v3 = opStack_.top(); opStack_.pop();
                opStack_.push(v1);
                opStack_.push(v3);
                opStack_.push(v2);
                opStack_.push(v1);
            }
            else {
                opStack_.push(v1);
                opStack_.push(v2);
                opStack_.push(v1);
            }
        }

        void do_dup2(unsigned bcI) {
            Value* v1 = opStack_.top(); opStack_.pop();
            if (isOneSlotValue(v1)) {
                Value* v2 = opStack_.top(); opStack_.pop();
                opStack_.push(v2);
                opStack_.push(v1);
                opStack_.push(v2);
                opStack_.push(v1);
            }
            else {
                opStack_.push(v1);
                opStack_.push(v1);
            }
        }

        void do_dup2_x1(unsigned bcI) {
            Value* v1 = opStack_.top(); opStack_.pop();
            Value* v2 = opStack_.top(); opStack_.pop();
            if (isOneSlotValue(v1)) {
                Value* v3 = opStack_.top(); opStack_.pop();
                opStack_.push(v2);
                opStack_.push(v1);
                opStack_.push(v3);
                opStack_.push(v2);
                opStack_.push(v1);
            }
            else {
                opStack_.push(v1);
                opStack_.push(v2);
                opStack_.push(v1);
            }
        }

        void do_dup2_x2(unsigned bcI) {
            Value* v1 = opStack_.top(); opStack_.pop();
            Value* v2 = opStack_.top(); opStack_.pop();
            if (isOneSlotValue(v1)) {
                Value* v3 = opStack_.top(); opStack_.pop();
                if (isOneSlotValue(v3)) {
                    Value* v4 = opStack_.top(); opStack_.pop();
                    opStack_.push(v2);
                    opStack_.push(v1);
                    opStack_.push(v4);
                    opStack_.push(v3);
                    opStack_.push(v2);
                    opStack_.push(v1);
                }
                else {
                    opStack_.push(v2);
                    opStack_.push(v1);
                    opStack_.push(v3);
                    opStack_.push(v2);
                    opStack_.push(v1);
                }
            }
            else {
                if (isOneSlotValue(v2)) {
                    Value* v3 = opStack_.top(); opStack_.pop();
                    opStack_.push(v1);
                    opStack_.push(v3);
                    opStack_.push(v2);
                    opStack_.push(v1);
                }
                else {
                    opStack_.push(v1);
                    opStack_.push(v2);
                    opStack_.push(v1);
                }
            }
        }

        void do_swap(unsigned bcI) {
            Value* v1 = opStack_.top(); opStack_.pop();
            Value* v2 = opStack_.top(); opStack_.pop();
            opStack_.push(v1);
            opStack_.push(v2);
        }

        void do_add(unsigned bcI) {
            do_binary_op_common(bcI, Instruction::Add);
        }

        void do_sub(unsigned bcI) {
            do_binary_op_common(bcI, Instruction::Sub);
        }

        void do_mul(unsigned bcI) {
            do_binary_op_common(bcI, Instruction::Mul);
        }

        void do_div(unsigned bcI) {
            do_binary_op_common(bcI, Instruction::Div);
        }

        void do_rem(unsigned bcI) {
            do_binary_op_common(bcI, Instruction::Rem);
        }

        void do_neg(unsigned bcI) {
            Value* v1 = opStack_.top(); opStack_.pop();
            opStack_.push(
                BinaryOperator::createNeg(v1, TMP, getBBAt(bcI)));
        }

        void do_shl(unsigned bcI) {
            do_shift_common(bcI, Instruction::Shl);
        }

        void do_shr(unsigned bcI) {
            do_shift_common(bcI, Instruction::Shr);
        }

        void do_ushr(unsigned bcI) {
            // cast value to be shifted into its unsigned version
            do_swap(bcI);
            Value* value = opStack_.top(); opStack_.pop();
            value = new CastInst(value,
                                 value->getType()->getUnsignedVersion(),
                                 TMP, getBBAt(bcI));
            opStack_.push(value);
            do_swap(bcI);

            do_shift_common(bcI, Instruction::Shr);

            value = opStack_.top(); opStack_.pop();
            // cast shifted value back to its original signed version
            opStack_.push(new CastInst(value,
                                       value->getType()->getSignedVersion(),
                                       TMP, getBBAt(bcI)));
        }

        void do_shift_common(unsigned bcI, Instruction::OtherOps op) {
            Value* amount = opStack_.top(); opStack_.pop();
            Value* value = opStack_.top(); opStack_.pop();
            amount = new CastInst(amount, Type::UByteTy, TMP, getBBAt(bcI));
            opStack_.push(new ShiftInst(op, value, amount, TMP, getBBAt(bcI)));
        }

        void do_and(unsigned bcI) {
            do_binary_op_common(bcI, Instruction::And);
        }

        void do_or(unsigned bcI) {
            do_binary_op_common(bcI, Instruction::Or);
        }

        void do_xor(unsigned bcI) {
            do_binary_op_common(bcI, Instruction::Xor);
        }

        void do_binary_op_common(unsigned bcI, Instruction::BinaryOps op) {
            Value* v2 = opStack_.top(); opStack_.pop();
            Value* v1 = opStack_.top(); opStack_.pop();
            opStack_.push(BinaryOperator::create(op, v1, v2, TMP,getBBAt(bcI)));
        }


        void do_iinc(unsigned bcI, unsigned index, int amount) {
            Value* v = new LoadInst(getOrCreateLocal(index, Type::IntTy),
                                    TMP, getBBAt(bcI));
            BinaryOperator::createAdd(v, ConstantSInt::get(Type::IntTy, amount),
                                      TMP, getBBAt(bcI));
            new StoreInst(v, getOrCreateLocal(index, Type::IntTy),
                          getBBAt(bcI));
        }

        void do_convert(unsigned bcI, JType to) {
            Value* v1 = opStack_.top(); opStack_.pop();
            opStack_.push(new CastInst(v1, getType(to), TMP, getBBAt(bcI)));
        }

        void do_lcmp(unsigned bcI) {
            Value* v2 = opStack_.top(); opStack_.pop();
            Value* v1 = opStack_.top(); opStack_.pop();
            Value* c = BinaryOperator::createSetGT(v1, v2, TMP, getBBAt(bcI));
            Value* r = new SelectInst(c, ConstantSInt::get(Type::IntTy, 1),
                                      ConstantSInt::get(Type::IntTy, 0), TMP,
                                      getBBAt(bcI));
            c = BinaryOperator::createSetLT(v1, v2, TMP, getBBAt(bcI));
            r = new SelectInst(c, ConstantSInt::get(Type::IntTy, -1), r, TMP,
                               getBBAt(bcI));
            opStack_.push(r);
        }

        void do_cmpl(unsigned bcI) {
            do_cmp_common(bcI, -1);
        }

        void do_cmpg(unsigned bcI) {
            do_cmp_common(bcI, 1);
        }

        void do_cmp_common(unsigned bcI, int valueIfUnordered) {
            Value* v2 = opStack_.top(); opStack_.pop();
            Value* v1 = opStack_.top(); opStack_.pop();
            Value* c = BinaryOperator::createSetGT(v1, v2, TMP, getBBAt(bcI));
            Value* r = new SelectInst(c, ConstantSInt::get(Type::IntTy, 1),
                                      ConstantSInt::get(Type::IntTy, 0), TMP,
                                      getBBAt(bcI));
            c = BinaryOperator::createSetLT(v1, v2, TMP, getBBAt(bcI));
            r = new SelectInst(c, ConstantSInt::get(Type::IntTy, -1), r, TMP,
                               getBBAt(bcI));
            c = new CallInst(module_->getOrInsertFunction
                             ("llvm.isunordered",
                              Type::BoolTy, v1->getType(), v2->getType(), 0),
                v1, v2, TMP, getBBAt(bcI));
            r = new SelectInst(c,
                               ConstantSInt::get(Type::IntTy, valueIfUnordered),
                               r, TMP, getBBAt(bcI));
            opStack_.push(r);
        }

        void do_if(unsigned bcI, JSetCC cc, JType type,
                   unsigned t, unsigned f) {
            Value* v2 = llvm::Constant::getNullValue(getType(type));
            Value* v1 = opStack_.top(); opStack_.pop();
            Value* c = new SetCondInst(getSetCC(cc), v1, v2, TMP, getBBAt(bcI));
            new BranchInst(getBBAt(t), getBBAt(f), c, getBBAt(bcI));
        }

        void do_ifcmp(unsigned bcI, JSetCC cc,
                      unsigned t, unsigned f) {
            Value* v2 = opStack_.top(); opStack_.pop();
            Value* v1 = opStack_.top(); opStack_.pop();
            Value* c = new SetCondInst(getSetCC(cc), v1, v2, TMP, getBBAt(bcI));
            new BranchInst(getBBAt(t), getBBAt(f), c, getBBAt(bcI));
        }

        void do_goto(unsigned bcI, unsigned target) {
            new BranchInst(getBBAt(target), getBBAt(bcI));
        }

        void do_jsr(unsigned bcI, unsigned target) {
            assert(0 && "not implemented");
        }

        void do_ret(unsigned bcI, unsigned index) {
            assert(0 && "not implemented");
        }

        void do_switch(unsigned bcI,
                       unsigned defTarget,
                       const SwitchCases& sw) {
            Value* v1 = opStack_.top(); opStack_.pop();
            SwitchInst* in =
                new SwitchInst(v1, getBBAt(defTarget), getBBAt(bcI));
            for (unsigned i = 0; i < sw.size(); ++i)
                in->addCase(ConstantSInt::get(Type::IntTy, sw[i].first),
                            getBBAt(sw[i].second));
        }

        void do_return(unsigned bcI) {
            Value* v1 = opStack_.top(); opStack_.pop();
            new ReturnInst(v1, getBBAt(bcI));
        }

        void do_return_void(unsigned bcI) {
            new ReturnInst(NULL, getBBAt(bcI));
        }

        void do_getstatic(unsigned bcI, unsigned index) {
            Value* v = new LoadInst(getStaticField(index), TMP, getBBAt(bcI));
            opStack_.push(v);
        }

        void do_putstatic(unsigned bcI, unsigned index) {
            Value* v = opStack_.top(); opStack_.pop();
            new StoreInst(v, getStaticField(index), getBBAt(bcI));            
        }

        void do_getfield(unsigned bcI, unsigned index) {
            assert(0 && "not implemented");
        }

        void do_putfield(unsigned bcI, unsigned index) {
            assert(0 && "not implemented");
        }

        void do_invokevirtual(unsigned bcI, unsigned index) {
            assert(0 && "not implemented");
        }

        void do_invokespecial(unsigned bcI, unsigned index) {
            assert(0 && "not implemented");
        }

        void do_invokestatic(unsigned bcI, unsigned index) {
            ConstantMethodRef* methodRef =
                (ConstantMethodRef*)(cf_->getConstantPool()[index]);
            ConstantNameAndType* nameAndType = methodRef->getNameAndType();

            std::string funcName =
                methodRef->getClass()->getName()->str() + '/' +
                nameAndType->getName()->str() +
                nameAndType->getDescriptor()->str();

            FunctionType* funcType =
                cast<FunctionType>(getType(nameAndType->getDescriptor()));
            std::vector<Value*> params(funcType->getNumParams(), NULL);
            for (unsigned i = funcType->getNumParams(); i > 0; ) {
                Value* p = opStack_.top(); opStack_.pop();
                params[--i] = p;
            }

            Function* function =
                module_->getOrInsertFunction(funcName, funcType);
            toCompileFunctions_.insert(function);
            Value* r = new CallInst(function, params, TMP, getBBAt(bcI));
            opStack_.push(r);
        }

        void do_invokeinterface(unsigned bcI, unsigned index) {
            assert(0 && "not implemented");
        }

        void do_new(unsigned bcI, unsigned index) {
            assert(0 && "not implemented");
        }

        void do_newarray(unsigned bcI, JType type) {
            assert(0 && "not implemented");
        }

        void do_anewarray(unsigned bcI, unsigned index) {
            assert(0 && "not implemented");
        }

        void do_arraylength(unsigned bcI) {
            assert(0 && "not implemented");
        }

        void do_athrow(unsigned bcI) {
            assert(0 && "not implemented");
        }

        void do_checkcast(unsigned bcI, unsigned index) {
            assert(0 && "not implemented");
        }

        void do_instanceof(unsigned bcI, unsigned index) {
            assert(0 && "not implemented");
        }

        void do_monitorenter(unsigned bcI) {
            assert(0 && "not implemented");
        }

        void do_monitorexit(unsigned bcI) {
            assert(0 && "not implemented");
        }

        void do_multianewarray(unsigned bcI,
                               unsigned index,
                               unsigned dims) {
            assert(0 && "not implemented");
        }
    };

} } } // namespace llvm::Java::

Compiler::Compiler()
    : compilerImpl_(new CompilerImpl())
{

}

Compiler::~Compiler()
{
    delete compilerImpl_;
}

void Compiler::compile(Module& m, const std::string& className)
{
    DEBUG(std::cerr << "Compiling class: " << className << '\n');

    Function* main =
        compilerImpl_->compileMethod(m,
                                     className + "/main([Ljava/lang/String;)V");
    Function* javaMain = m.getOrInsertFunction
        ("llvm_java_main", Type::VoidTy,
         Type::IntTy, PointerType::get(PointerType::get(Type::SByteTy)), NULL);

    BasicBlock* bb = new BasicBlock("entry", javaMain);
    const FunctionType* mainTy = main->getFunctionType();
    new CallInst(main,
                 // FIXME: Forward correct params from llvm_java_main
                 llvm::Constant::getNullValue(mainTy->getParamType(0)),
                 "",
                 bb);
    new ReturnInst(NULL, bb);
}
