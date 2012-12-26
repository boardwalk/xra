#ifndef XRA_COMPILER_HPP
#define XRA_COMPILER_HPP

#include "visitor.hpp"

namespace xra {

struct Compiler : Visitor<Compiler, const Expr>
{
  llvm::Module& module;
  llvm::IRBuilder<> builder;
  llvm::Value* value;

  Compiler(llvm::Module& module_) :
    module(module_),
    builder(module_.getContext()),
    value(nullptr)
  {}

  void VisitVoid(const EVoid& expr);
  void VisitCall(const ECall& expr);
};

} // namespace xra

#endif // XRA_COMPILER_HPP
