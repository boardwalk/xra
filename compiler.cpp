#include "common.hpp"
#include "compiler.hpp"

namespace xra {

void Compiler::VisitVoid(const EVoid& expr)
{
}

void Compiler::VisitCall(const ECall& expr)
{
  auto builtin = dyn_cast<VBuiltin>(expr.function->value.get());
  if(builtin) {
    assert(expr.argument->kind == Expr::Kind_EList);
    auto& args = static_cast<EList&>(*expr.argument).exprs;
    builtin->Compile(*this, args);
  }
  else {
    // TODO
  }
}

} // namespace xra
