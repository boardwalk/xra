#ifndef XRA_VALUE_HPP
#define XRA_VALUE_HPP

#include "type.hpp"

namespace xra {

/*
 * Base value
 */

class Value
{
public:
  enum Kind {
    Kind_VError,
    Kind_VBuiltin,
    Kind_VTemporary,
    Kind_VConstant,
    Kind_VLocal,
    Kind_VExtern
  };

  virtual ~Value() {}

  void Apply(const TypeSubst& subst)
  {
    if(type) {
      auto localSubst = subst;
      for(auto& var : freeVars)
        localSubst.erase(var);
      type = xra::Apply(localSubst, *type);
    }
  }

  const Kind kind;
  TypePtr type;
  set<string> freeVars;

protected:
  Value(Kind kind_) :
    kind(kind_),
    refcount(0)
  {}

private:
  friend void intrusive_ptr_add_ref(Value* value);
  friend void intrusive_ptr_release(Value* value);
  int refcount;
};

inline void intrusive_ptr_add_ref(Value* value)
{
  value->refcount++;
}

inline void intrusive_ptr_release(Value* value)
{
  if(--value->refcount == 0)
    delete value;
}

// value-tostring.cpp
void ToString(const Value&, stringstream&);

/*
 * Subvalues
 */

#define CLASSOF(c) \
 static bool classof(const Value* value) { return value->kind == Kind_##c; }

class VError : public Value
{
public:
  VError(const string& what) :
    Value(Kind_VError)
  {
    type = new TError(what);
  }

  CLASSOF(VError)
};

class VBuiltin : public Value
{
public:
  VBuiltin() :
    Value(Kind_VBuiltin)
  {
    type = new TError("Builtin");
  }

  CLASSOF(VBuiltin)

  virtual ValuePtr Infer(Env& env, TypeSubst&, Expr&) = 0;
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
  {
    type = VoidType;
  }

  VConstant(bool value) :
    Value(Kind_VConstant),
    boolValue(value)
  {
    type = BooleanType;
  }

  VConstant(long value) :
    Value(Kind_VConstant),
    intValue(value)
  {
    type = IntegerType;
  }

  VConstant(double value) :
    Value(Kind_VConstant),
    floatValue(value)
  {
    type = FloatType;
  }

  VConstant(const string& value) :
    Value(Kind_VConstant),
    strValue(value)
  {
    type = StringType;
  }

  CLASSOF(VConstant)

  const union {
    bool boolValue;
    long intValue;
    double floatValue;
  };
  const string strValue;
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

#undef CLASSOF

} // namespace xra

#endif