#include "common.hpp"
#include "expr.hpp"
#include "expr-visitor.hpp"

namespace xra {

struct ExprAnalyzeVisitor : ExprVisitor<ExprAnalyzeVisitor, Expr>
{
  void VisitError(EError& expr)
  {
    expr.finalType.reset(new TError("expression error"));
  }

  void VisitVoid(EVoid& expr)
  {
    expr.finalType.reset(new TVoid);
  }

  void VisitVariable(EVariable& expr)
  {
    // TODO
    base::VisitVariable(expr);
  }

  void VisitBoolean(EBoolean& expr)
  {
    expr.finalType.reset(new TVariable("bool"));
  }

  void VisitInteger(EInteger& expr)
  {
    expr.finalType.reset(new TVariable("int"));
  }

  void VisitFloat(EFloat& expr)
  {
    expr.finalType.reset(new TVariable("float"));
  }

  void VisitString(EString& expr)
  {
    expr.finalType.reset(new TVariable("str"));
  }

  void VisitIf(EIf& expr)
  {
    // TODO
    base::VisitIf(expr);
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
    // TODO
    base::VisitReturn(expr);
  }

  void VisitList(EList& expr)
  {
    // TODO
    base::VisitList(expr);
  }

  void VisitExtern(EExtern& expr)
  {
    expr.finalType.reset(new TVoid);
  }

  void VisitAny(Expr& expr)
  {
    // TODO
    base::VisitAny(expr);
  }
};

void Expr::Analyze()
{
  ExprAnalyzeVisitor visitor;
  visitor.VisitAny(*this);
}

} // namespace xra
