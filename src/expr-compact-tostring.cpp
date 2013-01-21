#include "common.hpp"
#include "visitor.hpp"

namespace xra {

struct ExprCompactToStringVisitor : Visitor<ExprCompactToStringVisitor, const Expr>
{
  ostream& os;

  ExprCompactToStringVisitor(ostream& os_) :
    os(os_)
  {}

  void VisitEVariable(const EVariable& expr)
  {
    os << "`" << expr.name << "`";
  }

  void VisitEBoolean(const EBoolean& expr)
  {
    os << (expr.literal ? "true" : "false");
  }

  void VisitEInteger(const EInteger& expr)
  {
    os << expr.literal;
  }

  void VisitEFloat(const EFloat& expr)
  {
    os << expr.literal;
  }

  void VisitEString(const EString& expr)
  {
    os << "\"";
    EscapeString(expr.literal, os);
    os << "\"";
  }

  void VisitEFunction(const EFunction& expr)
  {
    os << "(fn ";
    Visit(expr.param.get());
    os << " ";
    Visit(expr.body.get());
    os << ")";
  }

  void VisitECall(const ECall& expr)
  {
    os << "(call ";
    Visit(expr.function.get());
    os << " ";
    Visit(expr.argument.get());
    os << ")";
  }

  void VisitEList(const EList& expr)
  {
    os << "(";
    int i = 0;
    for(auto& e : expr.exprs) {
      if(i++) os << " ";
      Visit(e.get());
    }
    os << ")";
  }

  void VisitEExtern(const EExtern& expr)
  {
    os << "(extern ";
    os << expr.name;
    os << " ";
    os << *expr.externType;
    os << ")";
  }

  void VisitETypeAlias(const ETypeAlias& expr)
  {
    os << "(type ";
    os << expr.name;
    os << " ";
    os << *expr.aliasedType;
    os << ")";
  }
};

void ToStringCompact(ostream& os, const Expr& expr)
{
  ExprCompactToStringVisitor visitor(os);
  visitor.Visit(&expr);
}

} // namespace xra
