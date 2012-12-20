#ifndef XRA_EXPR_HPP
#define XRA_EXPR_HPP

namespace xra {

/*
 * Base expression
 */

struct Expr
{
public:
  enum Kind {
    KExtern
  };

  const Kind kind;

  // expr-tostring.cpp
  string ToString() const;

protected:
  Expr(Kind kind_) :
    kind(kind_)
  {}
};

typedef unique_ptr<Expr> ExprPtr;

template<class T>
T& operator<<(T& stream, const Expr& expr)
{
  return stream << expr.ToString();
}

/*
 * Subexpressions
 */

class Extern : public Expr
{
public:
  Extern() :
    Expr(KExtern)
  {}

  static bool classof(const Expr* expr)
  {
    return expr->kind == KExtern;
  }
};

} // namespace xra

#endif // XRA_EXPR_HPP
