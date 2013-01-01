#include "common.hpp"
#include "compiler.hpp"
#include <llvm/Analysis/Verifier.h>

namespace xra {

void Compiler::VisitEVariable(const EVariable& expr)
{
  if(isa<VLocal>(expr.value.get())) {
    auto& alloc = values[expr.name];
    if(!alloc) {
      auto type = ToLLVM(*expr.value->type, module.getContext());
      alloc = builder.CreateAlloca(type, nullptr, expr.name);
    }
    result = alloc;
  }
  else if(isa<VExtern>(expr.value.get())) {
    result = module.getGlobalVariable(expr.name);
    if(!result)
      result = module.getFunction(expr.name);
  }

  assert(result);
}

void Compiler::VisitEBoolean(const EBoolean& expr)
{
  result = expr.literal ? builder.getTrue() : builder.getFalse();
}

void Compiler::VisitEInteger(const EInteger& expr)
{
  result = llvm::ConstantInt::get(ToLLVM(*expr.value->type, module.getContext()), expr.literal);
}

void Compiler::VisitEFloat(const EFloat& expr)
{
  result = llvm::ConstantFP::get(ToLLVM(*expr.value->type, module.getContext()), expr.literal);
}

void Compiler::VisitEString(const EString& expr)
{
  result = builder.CreateGlobalStringPtr(expr.literal, "EString");
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
  builder.CreateRet(Read(result));

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
    assert(isa<EList>(expr.argument.get()));
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

void Compiler::VisitEList(const EList& expr)
{
  if(expr.exprs.empty())
    return;

  // TODO
}

void Compiler::VisitEExtern(const EExtern& expr)
{
  if(isa<TFunction>(expr.externType.get()))
  {
    auto funcType = dyn_cast<llvm::FunctionType>(ToLLVM(*expr.externType, module.getContext())->getPointerElementType());
    llvm::Function::Create(funcType, llvm::Function::ExternalLinkage, expr.name, &module);
  }
  else
  {
    auto varType = ToLLVM(*expr.externType, module.getContext());
    new llvm::GlobalVariable(module, varType, false, llvm::GlobalVariable::ExternalLinkage, nullptr, expr.name);
  }
}

} // namespace xra
