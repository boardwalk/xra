#ifndef XRA_TYPE_HPP
#define XRA_TYPE_HPP

#include "base.hpp"

namespace xra {

/*
 * Base type
 */

class Type : public Base
{
public:
  // type-parser.cpp
  static TypePtr Parse(BufferedLexer&);

protected:
  Type(Kind kind_) :
    Base(kind_)
  {}
};

extern const TypePtr VoidType;
extern const TypePtr BooleanType;
extern const TypePtr IntegerType;
extern const TypePtr FloatType;
extern const TypePtr StringType;

// type.cpp
TypePtr MakeTypeVar();
void Compose(const TypeSubst&, TypeSubst&);
ostream& operator<<(ostream&, const TypeSubst&);

// type-tostring.cpp
ostream& operator<<(ostream&, const Type&);

// type-apply.cpp
TypePtr Apply(const TypeSubst&, Type&);

// type-unify.cpp
TypeSubst Unify(Type&, Type&);

// type-getvariables.cpp
void GetVariables(const Type&, set<string>&);

// type-tollvm.cpp
llvm::Type* ToLLVM(const Type&, llvm::LLVMContext&);

/*
 * Subtypes
 */

class TBoolean : public Type
{
public:
  TBoolean() :
    Type(Kind_TBoolean)
  {}

  CLASSOF(TBoolean)
};

class TInteger : public Type
{
public:
  TInteger() :
    Type(Kind_TInteger)
  {}

  CLASSOF(TInteger)
};

class TFloat : public Type
{
public:
  TFloat() :
    Type(Kind_TFloat)
  {}

  CLASSOF(TFloat)
};

class TString : public Type
{
public:
  TString() :
    Type(Kind_TString)
  {}

  CLASSOF(TString)
};

class TVariable : public Type
{
public:
  TVariable(string name_) :
    Type(Kind_TVariable),
    name(move(name_))
  {}

  CLASSOF(TVariable)

  const string name;
};

class TList : public Type
{
public:
  TList() :
    Type(Kind_TList)
  {}

  CLASSOF(TList)

  vector<TypePtr> types;
};


class TFunction : public Type
{
public:
  TFunction() :
    Type(Kind_TFunction)
  {}

  TFunction(TypePtr parameter_, TypePtr result_) :
    Type(Kind_TFunction),
    parameter(move(parameter_)),
    result(move(result_))
  {}

  CLASSOF(TFunction)

  TypePtr parameter;
  TypePtr result;
};

} // namespace xra

#endif // XRA_TYPE_HPP
