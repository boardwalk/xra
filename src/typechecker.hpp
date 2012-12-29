#ifndef XRA_TYPECHECKER_HPP
#define XRA_TYPECHECKER_HPP

#include "visitor.hpp"
#include "env.hpp"

namespace xra {

class TypeChecker : public Visitor<TypeChecker, Expr>
{
public:
  Env env;
  TypeSubst subst;

  void VisitEVariable(EVariable&);
  void VisitEBoolean(EBoolean&);
  void VisitEInteger(EInteger&);
  void VisitEFloat(EFloat&);
  void VisitEString(EString&);
  void VisitEFunction(EFunction&);
  void VisitECall(ECall&);
  void VisitEList(EList&);
  void VisitEExtern(EExtern&);
  void Visit(Base*);
};

} // namespace xra

#endif // XRA_TYPECHECKER_HPP
