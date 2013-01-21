#ifndef XRA_EXPR_PARSER_HPP
#define XRA_EXPR_PARSER_HPP

#include "buffered-lexer.hpp"
#include "scoped-map.hpp"

namespace xra {

class ExprParser
{
  struct MacroDef {
    vector<string> params;
    vector<Token> body;
  };

  BufferedLexer& lexer;
  ScopedMap<string, MacroDef> macros;
  set<string> activeMacros;
  SourceLoc macroCallLoc;

  ExprPtr FileMacro();
  ExprPtr LineMacro();
  ExprPtr ShellMacro();

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
  ExprPtr Macro();
  ExprPtr MacroCall();
  ExprPtr UserMacroCall(const MacroDef& macro);
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
