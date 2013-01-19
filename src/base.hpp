#ifndef XRA_BASE_HPP
#define XRA_BASE_HPP

namespace xra {

class Base
{
public:
  enum Kind {
    // Expressions
    Kind_EVariable,
    Kind_EBoolean,
    Kind_EInteger,
    Kind_EFloat,
    Kind_EString,
    Kind_EFunction,
    Kind_ECall,
    Kind_EList,
    Kind_EExtern,
    Kind_ETypeAlias,
    // Values
    Kind_VBuiltin,
    Kind_VTemporary,
    Kind_VConstant,
    Kind_VLocal,
    Kind_VExtern,
    // Types
    Kind_TBoolean,
    Kind_TInteger,
    Kind_TFloat,
    Kind_TString,
    Kind_TVariable,
    Kind_TList,
    Kind_TFunction
  };

  virtual ~Base() {}

  const Kind kind;

  Base(const Base&) = delete;
  Base& operator=(const Base&) = delete;

protected:
  Base(Kind kind_) :
    kind(kind_),
    refcount(0)
  {}

private:
  friend void intrusive_ptr_add_ref(Base* base);
  friend void intrusive_ptr_release(Base* base);
  int refcount;
};

inline void intrusive_ptr_add_ref(Base* base)
{
  base->refcount++;
}

inline void intrusive_ptr_release(Base* base)
{
  if(--base->refcount == 0)
    delete base;
}

} // namespace xra

#define CLASSOF(c) \
  static bool classof(const Base* base) { return base->kind == Kind_##c; }

#endif // XRA_BASE_HPP
