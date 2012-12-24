#ifndef XRA_TYPE_HPP
#define XRA_TYPE_HPP

namespace xra {

class BufferedLexer;

/*
 * Base type
 */

class Type
{
public:
  enum Kind {
    Kind_TError,
    Kind_TVoid,
    Kind_TVariable,
    Kind_TList,
    Kind_TFunction
  };

  virtual ~Type() {}

  // type-parser.cpp
  static TypePtr Parse(BufferedLexer&);

  // type-geterrors.cpp
  string GetErrors() const;

  Type(const Type&) = delete;
  Type& operator=(const Type&) = delete;

  const Kind kind;

protected:
  Type(Kind kind_) :
    kind(kind_),
    refcount(0)
  {}

private:
  friend void intrusive_ptr_add_ref(Type* type);
  friend void intrusive_ptr_release(Type* type);
  int refcount;
};

inline void intrusive_ptr_add_ref(Type* type)
{
  type->refcount++;
}

inline void intrusive_ptr_release(Type* type)
{
  if(--type->refcount == 0)
    delete type;
}

extern const TypePtr VoidType;
extern const TypePtr BooleanType;
extern const TypePtr IntegerType;
extern const TypePtr FloatType;
extern const TypePtr StringType;

// type-tostring.cpp
void ToString(const Type&, stringstream&);

// type-apply.cpp
TypePtr Apply(const TypeSubst&, Type&);

// type-unify.cpp
TypeSubst Unify(Type&, Type&);

// type-getvariables.cpp
void GetVariables(const Type&, set<string>&);

// type.cpp
TypePtr MakeTypeVar();
void Compose(const TypeSubst&, TypeSubst&);
void ToString(const TypeSubst&, stringstream&);

/*
 * Subtypes
 */

class TError : public Type
{
public:
  TError(string what_) :
    Type(Kind_TError),
    what(move(what_))
  {}

  static bool classof(const Type* type)
  {
    return type->kind == Kind_TError;
  }

  const string what;
};

class TVoid : public Type
{
public:
  TVoid() :
    Type(Kind_TVoid)
  {}

  static bool classof(const Type* type)
  {
    return type->kind == Kind_TVoid;
  }
};

class TVariable : public Type
{
public:
  TVariable(string name_) :
    Type(Kind_TVariable),
    name(move(name_))
  {}

  static bool classof(const Type* type)
  {
    return type->kind == Kind_TVariable;
  }

  const string name;
};

class TList : public Type
{
public:
  TList() :
    Type(Kind_TList)
  {}

  static bool classof(const Type* type)
  {
    return type->kind == Kind_TList;
  }

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

  static bool classof(const Type* type)
  {
    return type->kind == Kind_TFunction;
  }

  TypePtr parameter;
  TypePtr result;
};

} // namespace xra

#endif // XRA_TYPE_HPP
