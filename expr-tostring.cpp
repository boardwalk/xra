#include "common.hpp"
#include "expr.hpp"
#include "expr-visitor.hpp"

namespace xra {

struct ExprToStringVisitor : ExprVisitor<ExprToStringVisitor, const Expr>
{
  ostream& os;
  int level;

  ExprToStringVisitor(ostream& os_) :
    os(os_),
    level(0)
  {}

#define BEGIN(e)                     \
  void Visit##e(const E##e& expr) {  \
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

  BEGIN(Void)
  END(Void)

  BEGIN(Variable)
    os << " `" << expr.name << "`";
  END(Variable)

  BEGIN(Boolean)
    os << " " << (expr.literal ? "true" : "false");
  END(Boolean)

  BEGIN(Integer)
    os << " " << expr.literal;
  END(Integer)

  BEGIN(Float)
    os << " " << expr.literal;
  END(Float)

  BEGIN(String)
    os << " \"";
    EscapeString(expr.literal, os);
    os << "\"";
  END(String)

  BEGIN(Function)
  END(Function)

  BEGIN(Call)
  END(Call)

  BEGIN(List)
  END(List)

  BEGIN(Extern)
    os << " " << expr.name << " " << *expr.externType;
  END(Extern)

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
