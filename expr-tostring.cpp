#include "common.hpp"
#include "expr.hpp"
#include "expr-visitor.hpp"
#include <sstream>

namespace xra {

struct ExprToStringVisitor
{
  stringstream ss;

  void Tail(const Expr& expr)
  {
    if(expr.type)
      ss << " : " << *expr.type;
  }

  void Visit(const EError& expr)
  {
    ss << "<" << expr.what << ">";
    Tail(expr);
  }

  void Visit(const EVoid& expr)
  {
    ss << "()";
    Tail(expr);
  }

  void Visit(const EVariable& expr)
  {
    ss << expr.name;
    Tail(expr);
  }

  void Visit(const EBoolean& expr)
  {
    ss << (expr.value ? "true" : "false");
    Tail(expr);
  }

  void Visit(const EInteger& expr)
  {
    ss << expr.value;
    Tail(expr);
  }

  void Visit(const EFloat& expr)
  {
    ss << expr.value;
    Tail(expr);
  }

  void Visit(const EString& expr)
  {
    ss << "\"" << EscapeString(expr.value) << "\"";
    Tail(expr);
  }

  void Visit(const EIf& expr)
  {
    ss << "(if";
    for(auto& c : expr.condClauses) {
      ss << " (";
      VisitExpr(*c.first, *this);
      ss << " ";
      VisitExpr(*c.second, *this);
      ss << ")";
    }
    if(expr.elseClause) {
      ss << " (else ";
      VisitExpr(*expr.elseClause, *this);
      ss << ")";
    }
    ss << ")";
    Tail(expr);
  }

  void Visit(const EFunction& expr)
  {
    ss << "(fn ";
    VisitExpr(*expr.param, *this);
    ss << " ";
    VisitExpr(*expr.body, *this);
    ss << ")";
    Tail(expr);
  }

  void Visit(const ECall& expr)
  {
    ss << "(go ";
    VisitExpr(*expr.function, *this);
    ss << " ";
    VisitExpr(*expr.argument, *this);
    ss << ")";
    Tail(expr);
  }

  void Visit(const EReturn& expr)
  {
    ss << "(return ";
    VisitExpr(*expr.expr, *this);
    ss << ")";
    Tail(expr);
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
    Tail(expr);
  }

  void Visit(const EExtern& expr)
  {
    ss << "(extern ";
    ss << expr.name;
    ss << " ";
    ss << expr.externType->ToString();
    ss << ")";
    Tail(expr);
  }
};

string Expr::ToString() const
{
  ExprToStringVisitor visitor;
  VisitExpr(*this, visitor);
  return visitor.ss.str();
}

} // namespace xra
