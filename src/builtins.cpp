#include "common.hpp"
#include "typechecker.hpp"
#include "compiler.hpp"

namespace xra {

/*
 * BSequence
 */

class BSequence : public VBuiltin
{
public:
  ValuePtr Infer(TypeChecker&, const vector<ExprPtr>&);
  void Compile(Compiler&, const vector<ExprPtr>&);
};

ValuePtr BSequence::Infer(TypeChecker& checker, const vector<ExprPtr>& args)
{
  for(auto& e : args)
  {
    TypeSubst lastSubst;
    checker.subst.swap(lastSubst);

    checker.Visit(e.get());

    if(!e->value)
      return {};

    Compose(lastSubst, checker.subst);
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
  ValuePtr Infer(TypeChecker&, const vector<ExprPtr>&);
  void Compile(Compiler&, const vector<ExprPtr>&);
};

ValuePtr BAssign::Infer(TypeChecker& checker, const vector<ExprPtr>& args)
{
  assert(args.size() == 2);

  auto& left = args[0];
  auto& right = args[1];

  checker.Visit(right.get());
  if(!right->value)
    return {};

  TypeSubst rightSubst;
  checker.subst.swap(rightSubst);

  // if the left side is a plain variable not in the environment, create a fresh local
  if(isa<EVariable>(left.get()))
  {
    auto& name = static_cast<EVariable&>(*left).name;
    if(!checker.env[name]) {
      left->value = new VLocal;
      left->value->type = MakeTypeVar();
      checker.env.AddValue(name, left->value);
    }
  }

  if(!left->value)
  {
    checker.Visit(left.get());
    if(!left->value)
      return {};
  }

  Compose(rightSubst, checker.subst);

  auto unifySubst = Unify(*left->value->type, *right->value->type);
  left->value->type = xra::Apply(unifySubst, *left->value->type);
  Compose(unifySubst, checker.subst);

  return left->value;
}

void BAssign::Compile(Compiler& compiler, const vector<ExprPtr>& args)
{
  compiler.Visit(args[1].get());
  if(!compiler.result)
    return;

  auto rightResult = compiler.Read(compiler.result);
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
  ValuePtr Infer(TypeChecker&, const vector<ExprPtr>&);
  void Compile(Compiler&, const vector<ExprPtr>&);
};

ValuePtr BIf::Infer(TypeChecker& checker, const vector<ExprPtr>& args)
{
  // conditions
  for(size_t i = 0; i < args.size(); i += 2)
  {
    TypeSubst lastSubst;
    checker.subst.swap(lastSubst);

    checker.Visit(args[i].get());
    if(!args[i]->value)
      return {};

    Compose(lastSubst, checker.subst);
    Compose(Unify(*args[i]->value->type, *BooleanType), checker.subst);
  }

  // clauses
  for(size_t i = 1; i < args.size(); i += 2)
  {
    TypeSubst lastSubst;
    checker.subst.swap(lastSubst);

    checker.Visit(args[i].get());
    if(!args[i]->value)
      return {};

    Compose(lastSubst, checker.subst);
    if(i != 1)
      Compose(Unify(*args[i]->value->type, *args[i - 2]->value->type), checker.subst);
  }

  for(auto& e : args)
    e->value->type = xra::Apply(checker.subst, *e->value->type);

  ValuePtr value = new VTemporary;
  value->type = (args.size() > 2) ? args[1]->value->type : VoidType;
  return value;
}

void BIf::Compile(Compiler& compiler, const vector<ExprPtr>& args)
{
  assert(args.size() >= 2 && args.size() % 2 == 0);

  const size_t nclauses = args.size() / 2;
  auto& builder = compiler.builder;
  auto& ctx = compiler.module.getContext();
  auto func = builder.GetInsertBlock()->getParent();

  llvm::AllocaInst* alloc = nullptr;
  if(args.size() > 2)
    alloc = builder.CreateAlloca(ToLLVM(*args[1]->value->type, ctx), nullptr, "iftmp");
  auto endifBlock = llvm::BasicBlock::Create(ctx, "endif");

  for(size_t i = 0; i < nclauses; i++)
  {
    auto thenBlock = llvm::BasicBlock::Create(ctx, "then", func);
    auto contBlock = (i < nclauses - 1) ? llvm::BasicBlock::Create(ctx, "else", func) : endifBlock;

    // if
    compiler.Visit(args[i * 2].get());
    compiler.result = compiler.Read(compiler.result);
    builder.CreateCondBr(compiler.result, thenBlock, contBlock);
    compiler.result = nullptr;

    // then
    builder.SetInsertPoint(thenBlock);
    compiler.Visit(args[i * 2 + 1].get());
    if(alloc)
      builder.CreateStore(compiler.Read(compiler.result), alloc);
    compiler.result = nullptr;
    builder.CreateBr(contBlock);

    // else or endif
    builder.SetInsertPoint(contBlock);
  }

  func->getBasicBlockList().push_back(endifBlock);

  if(alloc)
    compiler.result = builder.CreateLoad(alloc);
}

/*
 * BWhile
 */

class BWhile : public VBuiltin
{
  ValuePtr Infer(TypeChecker&, const vector<ExprPtr>&);
  void Compile(Compiler&, const vector<ExprPtr>&);
};

ValuePtr BWhile::Infer(TypeChecker& checker, const vector<ExprPtr>& args)
{
  assert(args.size() == 2);

  auto& cond = args[0];
  auto& body = args[1];

  checker.Visit(cond.get());
  if(!cond->value)
    return {};

  TypeSubst condSubst;
  checker.subst.swap(condSubst);

  bool lastInsideLoop = checker.insideLoop;
  checker.insideLoop = true;
  checker.Visit(body.get());
  checker.insideLoop = lastInsideLoop;
  if(!body->value)
    return {};

  Compose(condSubst, checker.subst);
  Compose(Unify(*cond->value->type, *BooleanType), checker.subst);

  ValuePtr value = new VConstant;
  value->type = VoidType;
  return value; // TODO should a while yield a value?
}

void BWhile::Compile(Compiler& compiler, const vector<ExprPtr>& args)
{
  auto& builder = compiler.builder;
  auto& ctx = compiler.module.getContext();
  auto func = builder.GetInsertBlock()->getParent();

  auto condBlock = llvm::BasicBlock::Create(ctx, "while", func);
  auto doBlock = llvm::BasicBlock::Create(ctx, "do", func);
  auto lastEndLoopBlock = compiler.endLoopBlock;
  compiler.endLoopBlock = llvm::BasicBlock::Create(ctx, "endwhile");

  builder.CreateBr(condBlock);

  // while
  builder.SetInsertPoint(condBlock);
  compiler.Visit(args[0].get());
  builder.CreateCondBr(compiler.result, doBlock, compiler.endLoopBlock);
  compiler.result = nullptr;

  // do
  builder.SetInsertPoint(doBlock);
  compiler.Visit(args[1].get());
  compiler.result = nullptr;
  builder.CreateBr(condBlock);

  // endloop
  func->getBasicBlockList().push_back(compiler.endLoopBlock);
  builder.SetInsertPoint(compiler.endLoopBlock);
  compiler.endLoopBlock = lastEndLoopBlock;
}

/*
 * BBreak
 */

class BBreak : public VBuiltin
{
public:
  ValuePtr Infer(TypeChecker&, const vector<ExprPtr>&);
  void Compile(Compiler&, const vector<ExprPtr>&);
};

ValuePtr BBreak::Infer(TypeChecker& checker, const vector<ExprPtr>&)
{
  if(!checker.insideLoop) {
    Error() << "break used outside loop";
    return {};
  }

  auto value = new VConstant;
  value->type = VoidType;
  return value;
}

void BBreak::Compile(Compiler& compiler, const vector<ExprPtr>&)
{
  auto& builder = compiler.builder;
  auto func = builder.GetInsertBlock()->getParent();
  auto contBlock = llvm::BasicBlock::Create(builder.getContext(), "breakcont", func);

  builder.CreateBr(compiler.endLoopBlock);
  builder.SetInsertPoint(contBlock);
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
TypePtr ResultType<true_type>(TypePtr) { return BooleanType; }

template<class Operation>
class BArithmetic : public VBuiltin
{
public:
  ValuePtr Infer(TypeChecker&, const vector<ExprPtr>&);
  void Compile(Compiler&, const vector<ExprPtr>&);
};

template<class Operation>
ValuePtr BArithmetic<Operation>::Infer(TypeChecker& checker, const vector<ExprPtr>& args)
{
  assert(args.size() == 2);

  auto& left = args[0];
  auto& right = args[1];

  checker.Visit(left.get());
  if(!left->value)
    return {};

  TypeSubst leftSubst;
  checker.subst.swap(leftSubst);

  checker.Visit(right.get());
  if(!right->value)
    return {};

  Compose(leftSubst, checker.subst);
  Compose(Unify(*left->value->type, *right->value->type), checker.subst);

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
  env.AddValue("#while", new BWhile);
  env.AddValue("#break", new BBreak);
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
