#ifndef XRA_EXPR_HPP
#define XRA_EXPR_HPP

#include "type.hpp"

namespace xra {

class BufferedLexer;

/*
 * Base expression
 */

class Expr;
typedef unique_ptr<Expr> ExprPtr;

class Expr
{
public:
  enum Kind {
    Kind_EError,
    Kind_EVoid,
    Kind_EVariable,
    Kind_EBoolean,
    Kind_EInteger,
    Kind_EFloat,
    Kind_EString,
    Kind_EIf,
    Kind_EFunction,
    Kind_ECall,
    Kind_EReturn,
    Kind_EList,
    Kind_EExtern
  };

  virtual ~Expr() {}

  // expr-parser.cpp
  static ExprPtr Parse(BufferedLexer&);

  // expr-tostring.cpp
  string ToString() const;

  // expr-geterrors.cpp
  string GetErrors() const;

  // expr-infer.cpp
  void Infer();

  Expr(const Expr&) = delete;
  Expr& operator=(const Expr&) = delete;

  const Kind kind;
  TypePtr type;
  TypePtr finalType;

protected:
  Expr(Kind kind_) :
    kind(kind_)
  {}
};

template<class T>
T& operator<<(T& stream, const Expr& expr)
{
  return stream << expr.ToString();
}

/*
 * Subexpressions
 */
class EError : public Expr
{
public:
  EError(string what_) :
    Expr(Kind_EError),
    what(move(what_))
  {}

  static bool classof(const Expr* expr)
  {
    return expr->kind == Kind_EError;
  }

  const string what;
};
class EVoid : public Expr
{
public:
  EVoid() :
    Expr(Kind_EVoid)
  {}

  static bool classof(const Expr* expr)
  {
    return expr->kind == Kind_EVoid;
  }
};

class EVariable : public Expr
{
public:
  EVariable(string name_) :
    Expr(Kind_EVariable),
    name(name_)
  {}

  static bool classof(const Expr* expr)
  {
    return expr->kind == Kind_EVariable;
  }

  const string name;
};


class EBoolean : public Expr
{
public:
  EBoolean(bool value_) :
    Expr(Kind_EBoolean),
    value(value_)
  {}

  static bool classof(const Expr* expr)
  {
    return expr->kind == Kind_EBoolean;
  }

  const bool value;
};

class EInteger : public Expr
{
public:
  EInteger(long value_) :
    Expr(Kind_EInteger),
    value(value_)
  {}

  static bool classof(const Expr* expr)
  {
    return expr->kind == Kind_EInteger;
  }

  const long value;
};

class EFloat : public Expr
{
public:
  EFloat(double value_) :
    Expr(Kind_EFloat),
    value(value_)
  {}

  static bool classof(const Expr* expr)
  {
    return expr->kind == Kind_EFloat;
  }

  const double value;
};

class EString : public Expr
{
public:
  EString(string value_) :
    Expr(Kind_EString),
    value(value_)
  {}

  static bool classof(const Expr* expr)
  {
    return expr->kind == Kind_EString;
  }

  const string value;
};

class EIf : public Expr
{
public:
  EIf() :
    Expr(Kind_EIf)
  {}

  static bool classof(const Expr* expr)
  {
    return expr->kind == Kind_EIf;
  }

  vector<pair<ExprPtr, ExprPtr> > condClauses;
  ExprPtr elseClause;
};

class EFunction : public Expr
{
public:
  EFunction(ExprPtr param_, ExprPtr body_) :
    Expr(Kind_EFunction),
    param(move(param_)),
    body(move(body_))
  {}

  static bool classof(const Expr* expr)
  {
    return expr->kind == Kind_EFunction;
  }

  ExprPtr param;
  ExprPtr body;
};

class ECall : public Expr
{
public:
  ECall(ExprPtr function_, ExprPtr argument_) :
    Expr(Kind_ECall),
    function(move(function_)),
    argument(move(argument_))
  {}

  static bool classof(const Expr* expr)
  {
    return expr->kind == Kind_ECall;
  }

  ExprPtr function;
  ExprPtr argument;
};

class EReturn : public Expr
{
public:
  EReturn(ExprPtr expr_) :
    Expr(Kind_EReturn),
    expr(move(expr_))
  {}

  static bool classof(const Expr* expr)
  {
    return expr->kind == Kind_EReturn;
  }

  ExprPtr expr;
};

class EList : public Expr
{
public:
  EList() :
    Expr(Kind_EList)
  {}

  static bool classof(const Expr* expr)
  {
    return expr->kind == Kind_EList;
  }

  vector<ExprPtr> exprs;
};

class EExtern : public Expr
{
public:
  EExtern(string name_, TypePtr externType_) :
    Expr(Kind_EExtern),
    name(move(name_)),
    externType(move(externType_))
  {}

  static bool classof(const Expr* expr)
  {
    return expr->kind == Kind_EExtern;
  }

  const string name;
  const TypePtr externType;
};

} // namespace xra

#endif // XRA_EXPR_HPP
