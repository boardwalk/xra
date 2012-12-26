#include "common.hpp"
#include "expr.hpp"
#include "value.hpp"
#include "type.hpp"

namespace xra {

/*
 * these are here so we don't have to include value.hpp & type.hpp in the header
 */
Expr::Expr(Kind kind_) :
  Base(kind_)
{}

Expr::~Expr()
{}

} // namespace xra
