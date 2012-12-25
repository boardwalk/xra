#ifndef XRA_EXPR_VISITOR_HPP
#define XRA_EXPR_VISITOR_HPP

namespace xra {

template<class VisitorTy, class NodeTy>
struct ExprVisitor
{
  typedef ExprVisitor<VisitorTy, NodeTy> base;

#define SUBCLASS \
  static_cast<VisitorTy&>(*this)

#define VISIT(c) \
  void Visit##c(typename CopyConst<NodeTy, E##c>::type& expr)

#define CASE(c) \
  case Expr::Kind_E##c: \
    return SUBCLASS.Visit##c(static_cast<typename CopyConst<NodeTy, E##c>::type&>(*node));

  VISIT(Void) {}
  
  VISIT(Variable) {}
  
  VISIT(Boolean) {}
  
  VISIT(Integer) {}
  
  VISIT(Float) {}
  
  VISIT(String) {}

  VISIT(Function) {
    SUBCLASS.Visit(expr.param.get());
    SUBCLASS.Visit(expr.body.get());
  }
  
  VISIT(Call) {
    SUBCLASS.Visit(expr.function.get());
    SUBCLASS.Visit(expr.argument.get());
  }

  VISIT(List) {
    for(auto& e : expr.exprs)
      SUBCLASS.Visit(e.get());
  }

  VISIT(Extern) {}

  void Visit(NodeTy* node)
  {
    assert(node);
    switch(node->kind) {
      CASE(Void)
      CASE(Variable)
      CASE(Boolean)
      CASE(Integer)
      CASE(Float)
      CASE(String)
      CASE(Function)
      CASE(Call)
      CASE(List)
      CASE(Extern)
    }
  }

#undef SUBCLASS
#undef VISIT
#undef CASE
};

} // namespace xra

#endif // XRA_EXPR_VISITOR_HPP
