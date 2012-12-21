#include "common.hpp"
#include "expr.hpp"
#include "expr-visitor.hpp"

namespace xra {

struct ExprGetErrorsVisitor
{
  string str;

  void GetTypeErrors(const Expr& expr)
  {
    if(expr.type)
      str += expr.type->GetErrors();
  }

  void Visit(const Expr& expr)
  {
    GetTypeErrors(expr);
  }

  void Visit(const EError& expr)
  {
    if(!str.empty())
      str += '\n';
    str += expr.what;
    GetTypeErrors(expr);
  }

  void Visit(const EIf& expr)
  {
    for(auto& c : expr.condClauses) {
      VisitExpr(*c.first, *this);
      VisitExpr(*c.second, *this);
    }
    if(expr.elseClause)
      VisitExpr(*expr.elseClause, *this);
    GetTypeErrors(expr);
  }

  void Visit(const EFunction& expr)
  {
    VisitExpr(*expr.param, *this);
    VisitExpr(*expr.body, *this);
    GetTypeErrors(expr);
  }

  void Visit(const ECall& expr)
  {
    VisitExpr(*expr.function, *this);
    VisitExpr(*expr.argument, *this);
    GetTypeErrors(expr);
  }

  void Visit(const EList& expr)
  {
    for(auto& e : expr.exprs)
      VisitExpr(*e, *this);
    GetTypeErrors(expr);
  }
};

string Expr::GetErrors() const
{
  ExprGetErrorsVisitor visitor;
  VisitExpr(*this, visitor);
  return move(visitor.str);
}
  
} // namespace xra
