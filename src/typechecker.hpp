#ifndef XRA_TYPECHECKER_HPP
#define XRA_TYPECHECKER_HPP

#include "visitor.hpp"
#include "env.hpp"

namespace xra {

class TypeChecker : public Visitor<TypeChecker, Expr>
{
public:
  TypeChecker() :
    insideLoop(false)
  {}

  Env env;
  TypeSubst subst;
  TypePtr returnType;
  bool insideLoop;

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
