#ifndef XRA_VISITOR_HPP
#define XRA_VISITOR_HPP

#include "expr.hpp"
#include "value.hpp"
#include "type.hpp"

namespace xra {

template<typename Source, typename Target>
struct CopyConst { typedef Target type; };

template<typename Source, typename Target>
struct CopyConst<const Source, Target> { typedef const Target type; };

template<class VisitorTy, class NodeTy>
struct Visitor
{
  typedef Visitor<VisitorTy, NodeTy> base;

#define SUBCLASS \
  static_cast<VisitorTy&>(*this)

#define VISIT(c) \
  void Visit##c(typename CopyConst<NodeTy, c>::type& node)

#define CASE(c) \
  case Base::Kind_##c: \
    return SUBCLASS.Visit##c(static_cast<typename CopyConst<NodeTy, c>::type&>(*node));

  /*
   * Expressions
   */
  VISIT(EVariable)
  {}
  
  VISIT(EBoolean)
  {}
  
  VISIT(EInteger)
  {}
  
  VISIT(EFloat)
  {}
  
  VISIT(EString)
  {}

  VISIT(EFunction)
  {
    SUBCLASS.Visit(node.param.get());
    SUBCLASS.Visit(node.body.get());
  }
  
  VISIT(ECall)
  {
    SUBCLASS.Visit(node.function.get());
    SUBCLASS.Visit(node.argument.get());
  }

  VISIT(EList)
  {
    for(auto& e : node.exprs)
      SUBCLASS.Visit(e.get());
  }

  VISIT(EExtern)
  {}

  /*
   * Values
   */
  VISIT(VBuiltin)
  {}

  VISIT(VTemporary)
  {}

  VISIT(VConstant)
  {}

  VISIT(VLocal)
  {}

  VISIT(VExtern)
  {}

  /*
   * Types
   */
  VISIT(TBoolean)
  {}

  VISIT(TInteger)
  {}

  VISIT(TFloat)
  {}

  VISIT(TString)
  {}

  VISIT(TVariable)
  {}

  VISIT(TList)
  {
    for(auto& t : node.types)
      SUBCLASS.Visit(t.get());
  }

  VISIT(TFunction)
  {
    SUBCLASS.Visit(node.parameter.get());
    SUBCLASS.Visit(node.result.get());
  }

  void Visit(typename CopyConst<NodeTy, Base>::type* node)
  {
    assert(node);
    switch(node->kind) {
      // Expressions
      CASE(EVariable)
      CASE(EBoolean)
      CASE(EInteger)
      CASE(EFloat)
      CASE(EString)
      CASE(EFunction)
      CASE(ECall)
      CASE(EList)
      CASE(EExtern)
      // Values
      CASE(VBuiltin)
      CASE(VTemporary)
      CASE(VConstant)
      CASE(VLocal)
      CASE(VExtern)
      // Types
      CASE(TBoolean)
      CASE(TInteger)
      CASE(TFloat)
      CASE(TString)
      CASE(TVariable)
      CASE(TList)
      CASE(TFunction)
    }
  }

#undef SUBCLASS
#undef VISIT
#undef CASE
};


} // namespace xra

#endif // XRA_VISITOR_HPP
