#include "common.hpp"
#include "expr.hpp"
#include "expr-visitor.hpp"

namespace xra {

struct ExprToStringVisitor : ExprVisitor<ExprToStringVisitor, const Expr>
{
  stringstream& ss;
  int level;

  ExprToStringVisitor(stringstream& ss_) :
    ss(ss_),
    level(0)
  {}

#define BEGIN(e)                          \
  void Visit##e(const E##e& expr) {       \
    for(int i = 0; i < level; i++)        \
      ss << "  ";                         \
    ss << #e;
#define END(e)                            \
    if(expr.value) {                      \
      ss << " ";                          \
      ToString(*expr.value, ss);          \
    }                                     \
    ss << endl;                           \
    level++;                              \
    base::Visit##e(expr);                 \
    level--;                              \
  }

  BEGIN(Void)
  END(Void)

  BEGIN(Variable)
    ss << " `" << expr.name << "`";
  END(Variable)

  BEGIN(Boolean)
    ss << " " << (expr.literal ? "true" : "false");
  END(Boolean)

  BEGIN(Integer)
    ss << " " << expr.literal;
  END(Integer)

  BEGIN(Float)
    ss << " " << expr.literal;
  END(Float)

  BEGIN(String)
    ss << " \"";
    EscapeString(expr.literal, ss);
    ss << "\"";
  END(String)

  BEGIN(Function)
  END(Function)

  BEGIN(Call)
  END(Call)

  BEGIN(List)
  END(List)

  BEGIN(Extern)
    ss << " " << expr.name << " " << *expr.externType;
  END(Extern)

#undef BEGIN
#undef END
};

void ToString(const Expr& expr, stringstream& ss)
{
  ExprToStringVisitor visitor(ss);
  visitor.Visit(&expr);
}

} // namespace xra
