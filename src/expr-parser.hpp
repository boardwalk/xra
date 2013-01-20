#ifndef XRA_EXPR_PARSER_HPP
#define XRA_EXPR_PARSER_HPP

#include "buffered-lexer.hpp"

namespace xra {

class ExprParser
{
  struct Macro {
    vector<string> bindings;
    vector<Token> body;
  };

  BufferedLexer& lexer;

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
  ExprPtr Expr(bool requred = true, int precedence = 0);  
  ExprPtr Expr_P(bool required);

public:
  ExprParser(BufferedLexer& lexer_) :
    lexer(lexer_)
  {}

  ExprPtr TopLevel();
};

}

#endif // XRA_EXPR_PARSER_HPP
