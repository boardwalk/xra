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
    return SUBCLASS.Visit##c(static_cast<typename CopyConst<NodeTy, E##c>::type&>(expr));

  VISIT(Error) {}

  VISIT(Void) {}
  
  VISIT(Variable) {}
  
  VISIT(Boolean) {}
  
  VISIT(Integer) {}
  
  VISIT(Float) {}
  
  VISIT(String) {}
  
  VISIT(If) {
    for(auto& p : expr.condClauses) {
      SUBCLASS.VisitAny(*p.first);
      SUBCLASS.VisitAny(*p.second);
    }
    if(expr.elseClause)
      SUBCLASS.VisitAny(*expr.elseClause);
  }
  
  VISIT(Function) {
    SUBCLASS.VisitAny(*expr.param);
    SUBCLASS.VisitAny(*expr.body);
  }
  
  VISIT(Call) {
    SUBCLASS.VisitAny(*expr.function);
    SUBCLASS.VisitAny(*expr.argument);
  }

  VISIT(Return) {
    SUBCLASS.VisitAny(*expr.expr);
  }

  VISIT(List) {
    for(auto& e : expr.exprs)
      SUBCLASS.VisitAny(*e);
  }

  VISIT(Extern) {}

  void VisitAny(NodeTy& expr)
  {
    switch(expr.kind) {
      CASE(Error)
      CASE(Void)
      CASE(Variable)
      CASE(Boolean)
      CASE(Integer)
      CASE(Float)
      CASE(String)
      CASE(If)
      CASE(Function)
      CASE(Call)
      CASE(Return)
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
