#include "common.hpp"
#include "expr.hpp"
#include "expr-visitor.hpp"
#include <sstream>

namespace xra {

struct ExprToStringVisitor : ExprVisitor<ExprToStringVisitor, const Expr>
{
  stringstream ss;
  int level;

  ExprToStringVisitor() :
    level(0)
  {}

#define BEGIN(e)                          \
  void Visit##e(const E##e& expr) {       \
    for(int i = 0; i < level; i++)        \
      ss << "  ";                         \
    ss << #e;                             \
    if(expr.finalType)                    \
      ss << " :" << *expr.finalType;
#define END(e)                            \
    ss << endl;                           \
    level++;                              \
    base::Visit##e(expr);                 \
    level--;                              \
  }

  BEGIN(Error)
    ss << " " << expr.what;
  END(Error)

  BEGIN(Void)
  END(Void)

  BEGIN(Variable)
    ss << " " << expr.name;
  END(Variable)

  BEGIN(Boolean)
    ss << " " << (expr.value ? "true" : "false");
  END(Boolean)

  BEGIN(Integer)
    ss << " " << expr.value;
  END(Integer)

  BEGIN(Float)
    ss << " " << expr.value;
  END(Float)

  BEGIN(String)
    ss << " \"" << EscapeString(expr.value) << "\"";
  END(String)

  BEGIN(Function)
  END(Function)

  BEGIN(Call)
  END(Call)

  BEGIN(Return)
  END(Return)

  BEGIN(List)
  END(List)

  BEGIN(Extern)
    ss << " " << expr.name << " " << *expr.externType;
  END(Extern)

#undef BEGIN
#undef END
};

string Expr::ToString() const
{
  ExprToStringVisitor visitor;
  visitor.VisitAny(*this);
  return visitor.ss.str();
}

} // namespace xra
