#include "common.hpp"
#include "expr.hpp"
#include "expr-visitor.hpp"

namespace xra {

struct InferVisitor : ExprVisitor<InferVisitor, Expr>
{
  TypeEnv& env;
  TypeSubst subst;

  InferVisitor(TypeEnv& env_) :
    env(env_)
  {}

  void VisitError(EError& expr)
  {
    expr.finalType.reset(new TError("expression error"));
  }

  void VisitVoid(EVoid& expr)
  {
    expr.finalType = VoidType;
  }

  void VisitVariable(EVariable& expr)
  {
    auto it = env.find(expr.name);
    if(it != env.end())
      expr.finalType = it->second.type;
    else
      expr.finalType.reset(new TError("unbound variable"));
  }

  void VisitBoolean(EBoolean& expr)
  {
    expr.finalType = BooleanType;
  }

  void VisitInteger(EInteger& expr)
  {
    expr.finalType = IntegerType;
  }

  void VisitFloat(EFloat& expr)
  {
    expr.finalType = FloatType;
  }

  void VisitString(EString& expr)
  {
    expr.finalType = StringType;
  }

  void VisitIf(EIf& expr)
  {
    for(auto& clause : expr.condClauses)
    {
      InferVisitor condVisitor(env);
      condVisitor.VisitAny(*clause.first);
      Compose(Unify(*clause.first->finalType, *BooleanType), subst);

      //ExprAnalyzeVisitor consVisitor(env);
      //consVisitor.VisitAny(*clause.second);
      //subst = 
      // TODO
    }
  }

  void VisitFunction(EFunction& expr)
  {
    // TODO
    base::VisitFunction(expr);
  }

  void VisitCall(ECall& expr)
  {
    // TODO
    base::VisitCall(expr);
  }

  void VisitReturn(EReturn& expr)
  {
    VisitAny(*expr.expr);
    expr.finalType = VoidType;
  }

  void VisitList(EList& expr)
  {
    auto r = make_unique<TList>();

    for(auto& e : expr.exprs)
    {
      TypeSubst lastSubst;
      subst.swap(lastSubst);

      VisitAny(*e);
      Apply(subst, env);

      Compose(subst, lastSubst);
      subst.swap(lastSubst);

      // TODO children must infer something!
      if(e->finalType)
        r->types.push_back(e->finalType);
      else
        r->types.push_back(VoidType);
    }

    expr.finalType.reset(r.release());
  }

  void VisitExtern(EExtern& expr)
  {
    auto it = env.find(expr.name);
    if(it != env.end()) {
      env[expr.name].type = expr.externType;
      expr.finalType = VoidType;
    }
    else {
      expr.finalType.reset(new TError("redefinition of symbol"));
    }
  }

  void VisitAny(Expr& expr)
  {
    base::VisitAny(expr);
    if(expr.type)
      Compose(Unify(*expr.finalType, *expr.type), subst);
  }
};

void Expr::Infer()
{
  TypeEnv env;
  InferVisitor visitor(env);
  visitor.VisitAny(*this);
}

} // namespace xra
