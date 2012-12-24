#ifndef XRA_EXPR_HPP
#define XRA_EXPR_HPP

#include "value.hpp"

namespace xra {

class BufferedLexer;

/*
 * Base expression
 */

class Env;

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
    Kind_EFunction,
    Kind_ECall,
    Kind_EList,
    Kind_EExtern
  };

  virtual ~Expr() {}

  // expr-parser.cpp
  static ExprPtr Parse(BufferedLexer&);

  // expr-geterrors.cpp
  string GetErrors() const;

  // expr-infer.cpp
  void Infer(Env&, TypeSubst&);

  Expr(const Expr&) = delete;
  Expr& operator=(const Expr&) = delete;

  const Kind kind;
  TypePtr type; // from annotation
  ValuePtr value; // filled by Infer

protected:
  Expr(Kind kind_) :
    kind(kind_)
  {}
};

// expr-tostring.cpp
void ToString(const Expr&, stringstream&);

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
  EBoolean(bool literal_) :
    Expr(Kind_EBoolean),
    literal(literal_)
  {}

  static bool classof(const Expr* expr)
  {
    return expr->kind == Kind_EBoolean;
  }

  const bool literal;
};

class EInteger : public Expr
{
public:
  EInteger(long literal_) :
    Expr(Kind_EInteger),
    literal(literal_)
  {}

  static bool classof(const Expr* expr)
  {
    return expr->kind == Kind_EInteger;
  }

  const long literal;
};

class EFloat : public Expr
{
public:
  EFloat(double literal_) :
    Expr(Kind_EFloat),
    literal(literal_)
  {}

  static bool classof(const Expr* expr)
  {
    return expr->kind == Kind_EFloat;
  }

  const double literal;
};

class EString : public Expr
{
public:
  EString(string literal_) :
    Expr(Kind_EString),
    literal(literal_)
  {}

  static bool classof(const Expr* expr)
  {
    return expr->kind == Kind_EString;
  }

  const string literal;
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
