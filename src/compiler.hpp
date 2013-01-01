#ifndef XRA_COMPILER_HPP
#define XRA_COMPILER_HPP

#include "visitor.hpp"

namespace xra {

class Compiler : public Visitor<Compiler, const Expr>
{
public:
  llvm::Module& module;
  llvm::IRBuilder<> builder;
  map<string, llvm::AllocaInst*> values;
  llvm::BasicBlock* endLoopBlock;
  llvm::Value* result;

  Compiler(llvm::Module& module_) :
    module(module_),
    builder(module_.getContext()),
    endLoopBlock(nullptr),
    result(nullptr)
  {}

  llvm::Value* Load(llvm::Value* val)
  {
    if(val && isa<llvm::AllocaInst>(val))
      return builder.CreateLoad(val);
    return val;
  }

  void VisitEVariable(const EVariable&);
  void VisitEBoolean(const EBoolean&);
  void VisitEInteger(const EInteger&);
  void VisitEFloat(const EFloat&);
  void VisitEString(const EString&);
  void VisitEFunction(const EFunction&);
  void VisitECall(const ECall&);
  void VisitEList(const EList&);
  void VisitEExtern(const EExtern&);
};

} // namespace xra

#endif // XRA_COMPILER_HPP
