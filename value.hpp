#ifndef XRA_VALUE_HPP
#define XRA_VALUE_HPP

#include "expr.hpp"

namespace xra {

/*
 * Base value
 */

class Value
{
public:
  enum Kind {
    Kind_VBuiltin,
    Kind_VLocal,
    Kind_VExtern
  };

  virtual ~Value() {}

  TypeScheme typeScheme;

  const Kind kind;

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

typedef boost::intrusive_ptr<Value> ValuePtr;

/*
 * Subvalues
 */

class VBuiltin : public Value
{
public:
  VBuiltin() :
    Value(Kind_VBuiltin)
  {}

  static bool classof(const Value* value)
  {
    return value->kind == Kind_VBuiltin;
  }

  virtual TypePtr Infer(Expr& argument, TypeSubst& subst)
  { return VoidType; }
};

class VLocal : public Value
{
public:
  VLocal() :
    Value(Kind_VLocal)
  {}

  static bool classof(const Value* value)
  {
    return value->kind == Kind_VLocal;
  }
};

class VExtern : public Value
{
public:
  VExtern() :
    Value(Kind_VExtern)
  {}

  static bool classof(const Value* value)
  {
    return value->kind == Kind_VExtern;
  }
};

} // namespace xra

#endif
