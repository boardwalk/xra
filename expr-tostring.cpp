#include "common.hpp"
#include "expr.hpp"
#include "expr-visitor.hpp"

namespace xra {

struct ExprToStringVisitor : ExprVisitor<ExprToStringVisitor>
{
  string str;

  void Visit(const Extern& expr)
  {
    str.append("extern");
  }
};

string Expr::ToString() const
{
  ExprToStringVisitor visitor;
  visitor.DoVisit(*this);
  return visitor.str;
}
  
} // namespace xra
