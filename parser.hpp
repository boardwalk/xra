#ifndef XRA_PARSER_HPP
#define XRA_PARSER_HPP

#include "buffered-lexer.hpp"
#include "expr.hpp"

namespace xra {

ExprPtr Parse(BufferedLexer&);

} // namespace xra

#endif // XRA_PARSER_HPP
