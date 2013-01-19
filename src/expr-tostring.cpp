#include "common.hpp"
#include "visitor.hpp"

namespace xra {

struct ExprToStringVisitor : Visitor<ExprToStringVisitor, const Expr>
{
  ostream& os;
  int level;

  ExprToStringVisitor(ostream& os_) :
    os(os_),
    level(0)
  {}

#define BEGIN(e)                     \
  void Visit##e(const e& expr) {  \
    for(int i = 0; i < level; i++)   \
      os << "  ";                    \
    os << #e;                        \
    if(expr.loc.source)              \
      os << " (" << expr.loc << ")";
#define END(e)                       \
    if(expr.value)                   \
      os << " " << *expr.value;      \
    os << endl;                      \
    level++;                         \
    base::Visit##e(expr);            \
    level--;                         \
  }

  BEGIN(EVariable)
    os << " `" << expr.name << "`";
  END(EVariable)

  BEGIN(EBoolean)
    os << " " << (expr.literal ? "true" : "false");
  END(EBoolean)

  BEGIN(EInteger)
    os << " " << expr.literal;
  END(EInteger)

  BEGIN(EFloat)
    os << " " << expr.literal;
  END(EFloat)

  BEGIN(EString)
    os << " \"";
    EscapeString(expr.literal, os);
    os << "\"";
  END(EString)

  BEGIN(EFunction)
  END(EFunction)

  BEGIN(ECall)
  END(ECall)

  BEGIN(EList)
  END(EList)

  BEGIN(EExtern)
    os << " " << expr.name << " " << *expr.externType;
  END(EExtern)

  BEGIN(ETypeAlias)
    os << " " << expr.name << " " << *expr.aliasedType;
  END(ETypeAlias)

#undef BEGIN
#undef END
};

ostream& operator<<(ostream& os, const Expr& expr)
{
  ExprToStringVisitor visitor(os);
  visitor.Visit(&expr);
  return os;
}

} // namespace xra
