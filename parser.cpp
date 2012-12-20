#include "common.hpp"
#include "parser.hpp"
#include "lexer.hpp"

namespace xra {

ExprPtr Parse(Lexer& lexer)
{
  return make_unique<Extern>();
}

} // namespace xra
