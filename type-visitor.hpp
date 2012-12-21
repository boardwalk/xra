#ifndef XRA_TYPE_VISITOR_HPP
#define XRA_TYPE_VISITOR_HPP

namespace xra {

#define VISIT(c) \
  case Type::Kind_##c: \
  { \
    typedef typename CopyConst<NodeTy, c>::type SubNodeTy; \
    Visit(static_cast<SubNodeTy&>(type)); \
    break; \
  }

template<class VisitorTy, class NodeTy>
struct TypeVisitor
{
  typedef TypeVisitor<VisitorTy, NodeTy> base;

  void Visit(NodeTy& type)
  {
    // nothing
  }

  void Visit(typename CopyConst<NodeTy, TFunction>::type& type)
  {
    VisitAny(*type.argument);
    VisitAny(*type.result);
  }

  void Visit(typename CopyConst<NodeTy, TList>::type& type)
  {
    for(auto& t : type.types)
      VisitAny(*t);
  }

  void VisitAny(NodeTy& type)
  {
    switch(type.kind) {
      VISIT(TError)
      VISIT(TVoid)
      VISIT(TVariable)
      VISIT(TList)
      VISIT(TFunction)
    }
  }
};

#undef VISIT

} // namespace xra

#endif // XRA_TYPE_VISITOR_HPP
