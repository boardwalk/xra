#ifndef XRA_TYPE_VISITOR_HPP
#define XRA_TYPE_VISITOR_HPP

namespace xra {

template<class VisitorTy, class NodeTy>
struct TypeVisitor
{
  typedef TypeVisitor<VisitorTy, NodeTy> base;

#define SUBCLASS \
  static_cast<VisitorTy&>(*this)

#define VISIT(c) \
  void Visit##c(typename CopyConst<NodeTy, T##c>::type& type)

#define CASE(c) \
  case Type::Kind_T##c: \
    return SUBCLASS.Visit##c(static_cast<typename CopyConst<NodeTy, T##c>::type&>(type));

  VISIT(Error) {}

  VISIT(Void) {}

  VISIT(Variable) {}

  VISIT(List) {
    for(auto& t : type.types)
      SUBCLASS.Visit(*t);
  }

  VISIT(Function) {
    SUBCLASS.Visit(*type.parameter);
    SUBCLASS.Visit(*type.result);
  }

  VISIT(Builtin) {}

  void Visit(NodeTy& type)
  {
    switch(type.kind) {
      CASE(Error)
      CASE(Void)
      CASE(Variable)
      CASE(List)
      CASE(Function)
      CASE(Builtin)
    }
  }

#undef SUBCLASS
#undef VISIT
#undef CASE
};

} // namespace xra

#endif // XRA_TYPE_VISITOR_HPP
