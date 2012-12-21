#include "common.hpp"
#include "expr.hpp"
#include "expr-visitor.hpp"

namespace xra {

struct ExprGetErrorsVisitor
{
  string str;

  void Visit(const Expr&)
  {
    // nothing
  }

  void Visit(const EError& expr)
  {
    if(!str.empty())
      str += '\n';
    str += expr.what;
  }

  void Visit(const EIf& expr)
  {
    for(auto& c : expr.condClauses) {
      VisitExpr(*c.first, *this);
      VisitExpr(*c.second, *this);
    }
    if(expr.elseClause)
      VisitExpr(*expr.elseClause, *this);
  }

  void Visit(const EFunction& expr)
  {
    VisitExpr(*expr.param, *this);
    VisitExpr(*expr.body, *this);
  }

  void Visit(const ECall& expr)
  {
    VisitExpr(*expr.function, *this);
    VisitExpr(*expr.argument, *this);
  }

  void Visit(const EList& expr)
  {
    for(auto& e : expr.exprs)
      VisitExpr(*e, *this);
  }
};

string Expr::GetErrors() const
{
  ExprGetErrorsVisitor visitor;
  VisitExpr(*this, visitor);
  return move(visitor.str);
}
  
} // namespace xra
