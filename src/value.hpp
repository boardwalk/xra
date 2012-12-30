#ifndef XRA_VALUE_HPP
#define XRA_VALUE_HPP

#include "base.hpp"

namespace xra {

/*
 * Base value
 */

class Value : public Base
{
public:
  void Apply(const TypeSubst& subst);

  TypePtr type;
  set<string> freeVars;

protected:
  Value(Kind kind_) :
    Base(kind_)
  {}
};

// value-tostring.cpp
ostream& operator<<(ostream&, const Value&);

/*
 * Subvalues
 */

class VBuiltin : public Value
{
public:
  VBuiltin() :
    Value(Kind_VBuiltin)
  {}

  CLASSOF(VBuiltin)

  virtual ValuePtr Infer(TypeChecker&, const vector<ExprPtr>&) = 0;
  virtual void Compile(Compiler&, const vector<ExprPtr>&) = 0;
};

class VTemporary : public Value
{
public:
  VTemporary() :
    Value(Kind_VTemporary)
  {}

  CLASSOF(VTemporary)
};

class VConstant : public Value
{
public:
  VConstant() :
    Value(Kind_VConstant)
  {}

  CLASSOF(VConstant)
};

class VLocal : public Value
{
public:
  VLocal() :
    Value(Kind_VLocal)
  {}

  CLASSOF(VLocal)
};

class VExtern : public Value
{
public:
  VExtern() :
    Value(Kind_VExtern)
  {}

  CLASSOF(VExtern)
};

} // namespace xra

#endif
