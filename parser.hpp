#ifndef XRA_PARSER_HPP
#define XRA_PARSER_HPP

#include "lexer.hpp"
#include "expr.hpp"

namespace xra {

ExprPtr Parse(Lexer&);

} // namespace xra

#endif // XRA_PARSER_HPP
