#ifndef XRA_EXPR_HPP
#define XRA_EXPR_HPP

#include "base.hpp"

namespace xra {

/*
 * Base expression
 */

class Expr : public Base
{
public:
  SourceLoc loc;
  TypePtr type; // from annotation
  ValuePtr value; // filled by Infer

protected:
  Expr(Kind kind_);
  ~Expr();
};

// expr-tostring.cpp
ostream& operator<<(ostream&, const Expr&);

// expr-compact-tostring.cpp
void ToStringCompact(ostream&, const Expr&);

/*
 * Subexpressions
 */

class EVariable : public Expr
{
public:
  EVariable(string name_) :
    Expr(Kind_EVariable),
    name(name_)
  {}

  CLASSOF(EVariable)

  const string name;
};

class EBoolean : public Expr
{
public:
  EBoolean(bool literal_) :
    Expr(Kind_EBoolean),
    literal(literal_)
  {}

  CLASSOF(EBoolean)

  const bool literal;
};

class EInteger : public Expr
{
public:
  EInteger(unsigned long literal_) :
    Expr(Kind_EInteger),
    literal(literal_)
  {}

  CLASSOF(EInteger)

  const unsigned long literal;
};

class EFloat : public Expr
{
public:
  EFloat(double literal_) :
    Expr(Kind_EFloat),
    literal(literal_)
  {}

  CLASSOF(EFloat)

  const double literal;
};

class EString : public Expr
{
public:
  EString(string literal_) :
    Expr(Kind_EString),
    literal(literal_)
  {}

  CLASSOF(EString)

  const string literal;
};

class EFunction : public Expr
{
public:
  EFunction(TypePtr param_, ExprPtr body_) :
    Expr(Kind_EFunction),
    param(move(param_)),
    body(move(body_))
  {}

  CLASSOF(EFunction)

  TypePtr param;
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

  CLASSOF(ECall)

  ExprPtr function;
  ExprPtr argument;
};

class EList : public Expr
{
public:
  EList() :
    Expr(Kind_EList)
  {}

  CLASSOF(EList)

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

  CLASSOF(EExtern)

  const string name;
  const TypePtr externType;
};

class ETypeAlias : public Expr
{
public:
  ETypeAlias(string name_, TypePtr aliasedType_) :
    Expr(Kind_ETypeAlias),
    name(move(name_)),
    aliasedType(move(aliasedType_))
  {}

  CLASSOF(ETypeAlias)

  const string name;
  const TypePtr aliasedType;
};

} // namespace xra

#endif // XRA_EXPR_HPP
