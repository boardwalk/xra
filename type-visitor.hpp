#ifndef XRA_TYPE_VISITOR_HPP
#define XRA_TYPE_VISITOR_HPP

namespace xra {

#define VISIT(c) \
  case Type::Kind_##c: \
  { \
    typedef typename CopyConst<Ty, c>::type SubTy; \
    return static_cast<ClassTy*>(this)->Visit(static_cast<SubTy&>(type)); \
  }

template<typename ClassTy, typename ResultTy=void>
class TypeVisitor
{
public:
  template<class Ty>
  ResultTy DoVisit(Ty& type)
  {
    switch(type.kind) {
      VISIT(TVariable)
    }
  }
};

#undef VISIT

} // namespace xra

#endif // XRA_TYPE_VISITOR_HPP
