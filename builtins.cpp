#include "common.hpp"
#include "compiler.hpp"
#include "env.hpp"
#include <iostream>

namespace xra {

/*
 * BSequence
 */

class BSequence : public VBuiltin
{
public:
  ValuePtr Infer(Env&, TypeSubst&, const vector<ExprPtr>&);
  void Compile(Compiler&, const vector<ExprPtr>&);
};

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

void BSequence::Compile(Compiler& compiler, const vector<ExprPtr>& args)
{
  for(auto& e : args) {
    compiler.result = nullptr;
    compiler.Visit(e.get());
  }
}

/*
 * BAssign
 */

class BAssign : public VBuiltin
{
public:
  ValuePtr Infer(Env&, TypeSubst&, const vector<ExprPtr>&);
  void Compile(Compiler&, const vector<ExprPtr>&);
};

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

/*
 * BIf
 */

class BIf : public VBuiltin
{
public:
  ValuePtr Infer(Env&, TypeSubst&, const vector<ExprPtr>&);
  void Compile(Compiler&, const vector<ExprPtr>&);
};

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

void BIf::Compile(Compiler& compiler, const vector<ExprPtr>& args)
{
  assert(args.size() >= 2 && args.size() % 2 == 0);

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
    compiler.result = compiler.Read(compiler.result);
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

/*
 * BArithmetic
 */

#define ARITHMETIC_OP(c, iop, fop, ic) \
  struct c { \
    static llvm::Value* IntOp(llvm::IRBuilder<>& b, llvm::Value* l, llvm::Value* r) { \
      return b.Create##iop(l, r); \
    } \
    static llvm::Value* FloatOp(llvm::IRBuilder<>& b, llvm::Value* l, llvm::Value* r) { \
      return b.Create##fop(l, r); \
    } \
    typedef ic IsCompare; \
  }

ARITHMETIC_OP(Add, Add, FAdd, false_type);
ARITHMETIC_OP(Sub, Sub, FSub, false_type);
ARITHMETIC_OP(Mul, Mul, FMul, false_type);
ARITHMETIC_OP(Div, UDiv, FDiv, false_type);
ARITHMETIC_OP(Rem, URem, FRem, false_type);
ARITHMETIC_OP(EQ, ICmpEQ, FCmpOEQ, true_type);
ARITHMETIC_OP(NE, ICmpNE, FCmpONE, true_type);
ARITHMETIC_OP(LT, ICmpULT, FCmpOLT, true_type);
ARITHMETIC_OP(LE, ICmpULE, FCmpOLE, true_type);
ARITHMETIC_OP(GT, ICmpUGT, FCmpOGT, true_type);
ARITHMETIC_OP(GE, ICmpUGE, FCmpOGE, true_type);

#undef ARITHMETIC_OP

template<typename IsCompare>
TypePtr ResultType(TypePtr);

template<>
TypePtr ResultType<false_type>(TypePtr type) { return type; }

template<>
TypePtr ResultType<true_type>(TypePtr type) { return BooleanType; }

template<class Operation>
class BArithmetic : public VBuiltin
{
public:
  ValuePtr Infer(Env&, TypeSubst&, const vector<ExprPtr>&);
  void Compile(Compiler&, const vector<ExprPtr>&);
};

template<class Operation>
ValuePtr BArithmetic<Operation>::Infer(Env& env, TypeSubst& subst, const vector<ExprPtr>& args)
{
  assert(args.size() == 2);

  auto& left = args[0];
  auto& right = args[1];

  TypeSubst leftSubst;
  left->Infer(env, leftSubst);
  if(!left->value)
    return {};

  TypeSubst rightSubst;
  right->Infer(env, rightSubst);
  if(!right->value)
    return {};

  Compose(leftSubst, subst);
  Compose(rightSubst, subst);
  Compose(Unify(*left->value->type, *right->value->type), subst);

  auto type = left->value->type.get();
  if(!isa<TInteger>(type) && !isa<TFloat>(type)) {
    Error() << "Arithmetic operation requires float or integer operands";
    return {};
  }

  ValuePtr value = new VTemporary;
  value->type = ResultType<typename Operation::IsCompare>(type);
  return value;
}

template<class Operation>
void BArithmetic<Operation>::Compile(Compiler& compiler, const vector<ExprPtr>& args)
{
  assert(args.size() == 2);

  compiler.Visit(args[0].get());
  auto left = compiler.Read(compiler.result);

  compiler.Visit(args[1].get());
  auto right = compiler.Read(compiler.result);

  if(isa<TInteger>(args[0]->value->type.get()))
    compiler.result = Operation::IntOp(compiler.builder, left, right);
  else
    compiler.result = Operation::FloatOp(compiler.builder, left, right);
}

void AddBuiltins(Env& env)
{
  env.AddValue(";", new BSequence);
  env.AddValue("=", new BAssign);
  env.AddValue("#if", new BIf);
  env.AddValue("+", new BArithmetic<Add>);
  env.AddValue("-", new BArithmetic<Sub>);
  env.AddValue("*", new BArithmetic<Mul>);
  env.AddValue("/", new BArithmetic<Div>);
  env.AddValue("%", new BArithmetic<Rem>);
  env.AddValue("==", new BArithmetic<EQ>);
  env.AddValue("!=", new BArithmetic<NE>);
  env.AddValue("<", new BArithmetic<LT>);
  env.AddValue("<=", new BArithmetic<LE>);
  env.AddValue(">", new BArithmetic<GT>);
  env.AddValue(">=", new BArithmetic<GE>);
}

} // namespace xra
