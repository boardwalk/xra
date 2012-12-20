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
    Kind_TVariable
  };

  const Kind kind;

  // type-tostring.cpp
  string ToString() const;

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
  return stream << type.ToString();
}

/*
 * Subtypes
 */

class TVariable : public Type
{
public:
  TVariable(string name_) :
    Type(Kind_TVariable),
    name(name_)
  {}

  static bool classof(const Type* type)
  {
    return type->kind == Kind_TVariable;
  }

  const string name;
};

} // namespace xra

#endif // XRA_TYPE_HPP
