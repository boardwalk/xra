#ifndef XRA_EXPR_PARSER_HPP
#define XRA_EXPR_PARSER_HPP

#include "token-buffer.hpp"

namespace xra {

class ExprParser
{
  TokenBuffer<Lexer> tokens;

  ExprPtr FlatBlock();
  ExprPtr Block();
  ExprPtr Clause();
  ExprPtr Name();

  ExprPtr Module();
  ExprPtr Using();
  ExprPtr Fn();
  ExprPtr If();
  ExprPtr While();
  ExprPtr Break();
  ExprPtr Return();
  ExprPtr TypeAlias();
  ExprPtr Extern();
  ExprPtr Expr(bool required = true, int precedence = 0);
  ExprPtr Expr_P(bool required);

public:
  ExprParser(Lexer& lexer) :
    tokens(lexer)
  {}

  ExprPtr TopLevel();
};

}

#endif // XRA_EXPR_PARSER_HPP
