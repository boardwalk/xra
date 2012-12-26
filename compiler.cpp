#include "common.hpp"
#include "compiler.hpp"
#include <llvm/Analysis/Verifier.h>

namespace xra {

void Compiler::VisitEList(const EList& expr)
{
  if(expr.exprs.empty())
    return;

  // TODO
}

void Compiler::VisitEVariable(const EVariable& expr)
{
  auto local = dyn_cast<VLocal>(expr.value.get());
  if(local) {
    auto& alloc = values[expr.name];
    if(!alloc)
      alloc = builder.CreateAlloca(ToLLVM(*expr.value->type, module.getContext()), nullptr, expr.name);
    result = alloc;
    return;
  }

  // Otherwise do nothing
}

void Compiler::VisitEBoolean(const EBoolean& expr)
{
  result = expr.value ? builder.getTrue() : builder.getFalse();
}

void Compiler::VisitEFunction(const EFunction& expr)
{
  auto previousBlock = builder.GetInsertBlock();
  map<string, llvm::AllocaInst*> previousValues;
  previousValues.swap(values);

  // create function
  auto funcType = static_cast<llvm::FunctionType*>(ToLLVM(*expr.value->type, module.getContext())->getPointerElementType());
  auto func = llvm::Function::Create(funcType, llvm::Function::InternalLinkage, "EFunction", &module);
  auto block = llvm::BasicBlock::Create(module.getContext(), "entry", func);

  // fill in function
  builder.SetInsertPoint(block);
  Visit(expr.body.get());
  builder.CreateRet(result);

  verifyFunction(*func);

  // restore function state
  builder.SetInsertPoint(previousBlock);
  values.swap(previousValues);

  result = func;
}

void Compiler::VisitECall(const ECall& expr)
{
  auto builtin = dyn_cast<VBuiltin>(expr.function->value.get());
  if(builtin)
  {
    assert(expr.argument->kind == Expr::Kind_EList);
    auto& args = static_cast<EList&>(*expr.argument).exprs;
    builtin->Compile(*this, args);
  }
  else
  {
    Visit(expr.function.get());
    auto function = result;

    Visit(expr.argument.get());
    auto argument = result;

    result = builder.CreateCall(function, llvm::ArrayRef<llvm::Value*>(argument));
  }
}

} // namespace xra
