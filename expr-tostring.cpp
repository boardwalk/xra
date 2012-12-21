#include "common.hpp"
#include "expr.hpp"
#include "expr-visitor.hpp"
#include <sstream>

namespace xra {

struct ExprToStringVisitor
{
  stringstream ss;

  void Visit(const EError& expr)
  {
    ss << "<" << expr.what << ">";
  }

  void Visit(const EVoid&)
  {
    ss << "()";
  }

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

  void Visit(const EIf& expr)
  {
    ss << "(if ";
    VisitExpr(*expr.cond, *this);
    ss << " ";
    VisitExpr(*expr.then, *this);
    if(expr._else) {
      ss << " ";
      VisitExpr(*expr._else, *this);
    }
    ss << ")";
  }

  void Visit(const EFunction& expr)
  {
    ss << "(fn ";
    VisitExpr(*expr.param, *this);
    ss << " ";
    VisitExpr(*expr.body, *this);
    ss << ")";
  }

  void Visit(const ECall& expr)
  {
    ss << "(go ";
    VisitExpr(*expr.function, *this);
    ss << " ";
    VisitExpr(*expr.argument, *this);
    ss << ")";
  }

  void Visit(const EList& expr)
  {
    ss << "{";
    int i = 0;
    for(auto& e : expr.exprs) {
      if(i++ != 0) ss << " ";
      VisitExpr(*e, *this);
    }
    ss << "}";
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
  VisitExpr(*this, visitor);
  return visitor.ss.str();
}
  
} // namespace xra
