#include "common.hpp"
#include "expr.hpp"
#include "expr-visitor.hpp"
#include <sstream>

namespace xra {

struct ExprToStringVisitor : ExprVisitor<ExprToStringVisitor>
{
  stringstream ss;

  void Visit(const EVariable& expr)
  {
    ss << expr.name;
  }

  void Visit(const EBoolean& expr)
  {
    ss << (expr.value ? "true" : "false");
  }

  void Visit(const EInteger& expr)
  {
    ss << expr.value;
  }

  void Visit(const EFloat& expr)
  {
    ss << expr.value;
  }

  void Visit(const EString& expr)
  {
    ss << "\"" << EscapeString(expr.value) << "\"";
  }

  void Visit(const EBlock& expr)
  {
    ss << "(block";
    for(auto& e : expr.exprs) {
      ss << " ";
      DoVisit(*e);
    }
    ss << ")";
  }

  void Visit(const EIf& expr)
  {
    ss << "(if ";
    DoVisit(*expr.cond);
    ss << " ";
    DoVisit(*expr.then);
    if(expr._else) {
      ss << " ";
      DoVisit(*expr._else);
    }
    ss << ")";
  }

  void Visit(const EExtern& expr)
  {
    ss << "(extern ";
    ss << expr.name;
    ss << " ";
    ss << expr.externType->ToString();
    ss << ")";
  }
};

string Expr::ToString() const
{
  ExprToStringVisitor visitor;
  visitor.DoVisit(*this);
  return visitor.ss.str();
}
  
} // namespace xra
