#ifndef XRA_TYPE_HPP
#define XRA_TYPE_HPP

namespace xra {

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

  const Kind kind;

  // type-tostring.cpp
  string ToString() const;

  // type-geterrors.cpp
  string GetErrors() const;

protected:
  Type(Kind kind_) :
    kind(kind_)
  {}

  Type(const Type&);
  Type& operator=(const Type&);
};

typedef shared_ptr<Type> TypePtr;

template<class T>
T& operator<<(T& stream, const Type& type)
{
  stream << type.ToString();
  return stream;
}

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
  TFunction(TypePtr argument_, TypePtr result_) :
    Type(Kind_TFunction),
    argument(move(argument_)),
    result(move(result_))
  {}

  static bool classof(const Type* type)
  {
    return type->kind == Kind_TFunction;
  }

  TypePtr argument;
  TypePtr result;
};

} // namespace xra

#endif // XRA_TYPE_HPP
