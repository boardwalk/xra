#include "common.hpp"
#include "expr.hpp"
#include "expr-visitor.hpp"

namespace xra {

struct ExprGetErrorsVisitor : ExprVisitor<ExprGetErrorsVisitor, const Expr>
{
  string str;

  void VisitError(const EError& expr)
  {
    if(!str.empty())
      str += '\n';
    str += expr.what;
    base::VisitError(expr);
  }

  void VisitAny(const Expr& expr)
  {
    if(expr.type)
      str += expr.type->GetErrors();
    base::VisitAny(expr);
  }
};

string Expr::GetErrors() const
{
  ExprGetErrorsVisitor visitor;
  visitor.VisitAny(*this);
  return move(visitor.str);
}
  
} // namespace xra
