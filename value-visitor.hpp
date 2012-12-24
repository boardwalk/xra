#ifndef XRA_VALUE_VISITOR_HPP
#define XRA_VALUE_VISITOR_HPP

namespace xra {

template<class VisitorTy, class NodeTy>
struct ValueVisitor
{
  typedef ValueVisitor<VisitorTy, NodeTy> base;

#define SUBCLASS \
  static_cast<VisitorTy&>(*this)

#define VISIT(c) \
  void Visit##c(typename CopyConst<NodeTy, V##c>::type& type)

#define CASE(c) \
  case Value::Kind_V##c: \
    return SUBCLASS.Visit##c(static_cast<typename CopyConst<NodeTy, V##c>::type&>(node));

  VISIT(Error) {}

  VISIT(Builtin) {}

  VISIT(Temporary) {}

  VISIT(Constant) {}

  VISIT(Local) {}

  VISIT(Extern) {}

  void Visit(NodeTy& node)
  {
    switch(node.kind) {
      CASE(Error)
      CASE(Builtin)
      CASE(Temporary)
      CASE(Constant)
      CASE(Local)
      CASE(Extern)
    }
  }

#undef SUBCLASS
#undef VISIT
#undef CASE
};

} // namespace xra

#endif // XRA_VALUE_VISITOR_HPP
