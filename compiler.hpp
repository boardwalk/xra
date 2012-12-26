#ifndef XRA_COMPILER_HPP
#define XRA_COMPILER_HPP

#include "visitor.hpp"

namespace xra {

struct Compiler : Visitor<Compiler, const Expr>
{
  llvm::Module& module;
  llvm::IRBuilder<> builder;
  map<string, llvm::AllocaInst*> values;
  llvm::Value* result;

  Compiler(llvm::Module& module_) :
    module(module_),
    builder(module_.getContext()),
    result(nullptr)
  {}

  void VisitEVariable(const EVariable&);
  void VisitEBoolean(const EBoolean&);
  void VisitEFunction(const EFunction&);
  void VisitECall(const ECall&);
  void VisitEList(const EList&);
};

} // namespace xra

#endif // XRA_COMPILER_HPP
