#include "common.hpp"
#include "compiler.hpp"
#include "env.hpp"
#include <iostream>

namespace xra {

class BSequence : public VBuiltin
{
public:
  ValuePtr Infer(Env&, TypeSubst&, const vector<ExprPtr>&);
  void Compile(Compiler&, const vector<ExprPtr>&);
};

class BAssign : public VBuiltin
{
public:
  ValuePtr Infer(Env&, TypeSubst&, const vector<ExprPtr>&);
  void Compile(Compiler&, const vector<ExprPtr>&);
};

class BIf : public VBuiltin
{
public:
  ValuePtr Infer(Env&, TypeSubst&, const vector<ExprPtr>&);
  void Compile(Compiler&, const vector<ExprPtr>&);
};

void AddBuiltins(Env& env)
{
  env.AddValue(";", new BSequence);
  env.AddValue("=", new BAssign);
  env.AddValue("#if", new BIf);
}

ValuePtr BSequence::Infer(Env& env, TypeSubst& subst, const vector<ExprPtr>& args)
{
  for(auto& e : args)
  {
    TypeSubst lastSubst;
    subst.swap(lastSubst);

    e->Infer(env, subst);

    if(!e->value)
      return {};

    Compose(lastSubst, subst);
  }

  return args.back()->value;
}

ValuePtr BAssign::Infer(Env& env, TypeSubst& subst, const vector<ExprPtr>& args)
{
  assert(args.size() == 2);

  auto& left = args[0];
  auto& right = args[1];

  right->Infer(env, subst);

  if(!right->value)
    return {};

  // if the left side is a plain variable not in the environment, create a fresh local
  if(isa<EVariable>(left.get()))
  {
    auto& name = static_cast<EVariable&>(*left).name;
    if(!env[name]) {
      left->value = new VLocal;
      left->value->type = MakeTypeVar();
      env.AddValue(name, left->value);
    }
  }

  if(!left->value)
  {
    TypeSubst leftSubst;
    left->Infer(env, leftSubst);
    if(!left->value)
      return {};
    Compose(leftSubst, subst);
  }

  auto unifySubst = Unify(*left->value->type, *right->value->type);
  left->value->type = xra::Apply(unifySubst, *left->value->type);
  Compose(unifySubst, subst);

  return left->value;
}

ValuePtr BIf::Infer(Env& env, TypeSubst& subst, const vector<ExprPtr>& args)
{
  // conditions
  for(size_t i = 0; i < args.size(); i += 2)
  {
    TypeSubst condSubst;
    args[i]->Infer(env, condSubst);
    if(!args[i]->value)
      return {};

    Compose(condSubst, subst);
    Compose(Unify(*args[i]->value->type, *BooleanType), subst);
  }

  // clauses
  for(size_t i = 1; i < args.size(); i += 2)
  {
    TypeSubst clauseSubst;
    args[i]->Infer(env, subst);
    if(!args[i]->value)
      return {};

    Compose(clauseSubst, subst);
    if(i != 1)
      Compose(Unify(*args[i]->value->type, *args[i - 2]->value->type), subst);
  }

  for(auto& e : args)
    e->value->type = xra::Apply(subst, *e->value->type);

  ValuePtr value = new VTemporary;
  value->type = args[1]->value->type;
  return value;
}

void BSequence::Compile(Compiler& compiler, const vector<ExprPtr>& args)
{
  for(auto& e : args) {
    compiler.result = nullptr;
    compiler.Visit(e.get());
  }
}

void BAssign::Compile(Compiler& compiler, const vector<ExprPtr>& args)
{
  compiler.Visit(args[1].get());
  if(!compiler.result)
    return;

  auto rightResult = compiler.result;
  compiler.result = nullptr;

  compiler.Visit(args[0].get());
  if(!compiler.result)
    return;

  compiler.builder.CreateStore(rightResult, compiler.result);
}

void BIf::Compile(Compiler& compiler, const vector<ExprPtr>& args)
{
  const int nclauses = args.size() / 2;
  auto& builder = compiler.builder;
  auto& ctx = compiler.module.getContext();
  auto func = builder.GetInsertBlock()->getParent();

  auto alloc = builder.CreateAlloca(ToLLVM(*args[1]->value->type, ctx), nullptr, "iftmp");
  auto endifBlock = llvm::BasicBlock::Create(ctx, "endif");

  for(int i = 0; i < nclauses; i++)
  {
    auto thenBlock = llvm::BasicBlock::Create(ctx, "then", func);
    auto elseBlock = (i < nclauses - 1) ? llvm::BasicBlock::Create(ctx, "else", func) : endifBlock;

    compiler.Visit(args[i * 2].get());
    builder.CreateCondBr(compiler.result, thenBlock, elseBlock);
    compiler.result = nullptr;

    builder.SetInsertPoint(thenBlock);

    compiler.Visit(args[i * 2 + 1].get());
    builder.CreateStore(compiler.result, alloc);
    compiler.result = nullptr;
    builder.CreateBr(endifBlock);

    builder.SetInsertPoint(elseBlock);
  }

  func->getBasicBlockList().push_back(endifBlock);

  compiler.result = builder.CreateLoad(alloc);
}

} // namespace xra
