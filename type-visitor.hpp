#ifndef XRA_TYPE_VISITOR_HPP
#define XRA_TYPE_VISITOR_HPP

namespace xra {

#define VISIT(c) \
  case Type::Kind_##c: \
  { \
    typedef typename CopyConst<Ty, c>::type SubTy; \
    visitor.Visit(static_cast<SubTy&>(type)); \
    break; \
  }

template<class Ty, class ClassTy>
void VisitType(Ty& type, ClassTy& visitor)
{
  switch(type.kind) {
    VISIT(TError)
    VISIT(TVoid)
    VISIT(TVariable)
    VISIT(TList)
    VISIT(TFunction)
  }
}

#undef VISIT

} // namespace xra

#endif // XRA_TYPE_VISITOR_HPP
