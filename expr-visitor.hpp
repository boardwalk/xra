#ifndef XRA_EXPR_VISITOR_HPP
#define XRA_EXPR_VISITOR_HPP

namespace xra {

template<typename Source, typename Target>
struct CopyConst { typedef Target type; };

template<typename Source, typename Target>
struct CopyConst<const Source, Target> { typedef const Target type; };

#define VISIT(c) \
  case Expr::K##c: \
  { \
    typedef typename CopyConst<ExprTy, c>::type SubExprTy; \
    return static_cast<ClassTy*>(this)->Visit(static_cast<SubExprTy&>(expr)); \
  }

template<typename ClassTy, typename ResultTy=void>
class ExprVisitor
{
public:
  template<class ExprTy>
  ResultTy DoVisit(ExprTy& expr)
  {
    switch(expr.kind) {
      VISIT(Extern)
    }
  }
};

#undef VISIT

} // namespace xra

#endif // XRA_EXPR_VISITOR_HPP
