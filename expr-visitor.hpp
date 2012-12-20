#ifndef XRA_EXPR_VISITOR_HPP
#define XRA_EXPR_VISITOR_HPP

namespace xra {

#define VISIT(c) \
  case Expr::Kind_##c: \
  { \
    typedef typename CopyConst<Ty, c>::type SubTy; \
    return static_cast<ClassTy*>(this)->Visit(static_cast<SubTy&>(expr)); \
  }

template<typename ClassTy, typename ResultTy=void>
class ExprVisitor
{
public:
  template<class Ty>
  ResultTy DoVisit(Ty& expr)
  {
    switch(expr.kind) {
      VISIT(EVariable)
      VISIT(EBoolean)
      VISIT(EInteger)
      VISIT(EFloat)
      VISIT(EString)
      VISIT(EBlock)
      VISIT(EIf)
      VISIT(EExtern)
    }
  }
};

#undef VISIT

} // namespace xra

#endif // XRA_EXPR_VISITOR_HPP
