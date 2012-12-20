#ifndef XRA_EXPR_VISITOR_HPP
#define XRA_EXPR_VISITOR_HPP

namespace xra {

#define VISIT(c) \
  case Expr::Kind_##c: \
  { \
    typedef typename CopyConst<Ty, c>::type SubTy; \
    visitor.Visit(static_cast<SubTy&>(expr)); \
    break; \
  }

template<class Ty, class ClassTy>
void VisitExpr(Ty& expr, ClassTy& visitor)
{
  switch(expr.kind) {
    VISIT(EVoid)
    VISIT(EVariable)
    VISIT(EBoolean)
    VISIT(EInteger)
    VISIT(EFloat)
    VISIT(EString)
    VISIT(EBlock)
    VISIT(ETuple)
    VISIT(EIf)
    VISIT(EFunction)
    VISIT(EUnaryOp)
    VISIT(EBinaryOp)
    VISIT(EExtern)
  }
}

#undef VISIT

} // namespace xra

#endif // XRA_EXPR_VISITOR_HPP
