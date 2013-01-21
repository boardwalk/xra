#ifndef XRA_MACRO_PARSER_HPP
#define XRA_MACRO_PARSER_HPP

#include "token-buffer.hpp"
#include "scoped-map.hpp"

namespace xra {

class MacroParser
{
  struct MacroDef {
    vector<string> params;
    vector<Token> body;
  };

  TokenBuffer<Lexer> tokens;
  ScopedMap<string, MacroDef> macros;
  set<string> activeMacros;
  SourceLoc macroCallLoc;

  void IncMacro();
  void FileMacro();
  void LineMacro();
  void ShellMacro();
  void CatMacro(bool asIdentifier);

  void Macro();
  void MacroCall();
  void UserMacroCall(const MacroDef& macro);

public:
  MacroParser(Lexer& lexer) :
    tokens(lexer)
  {}

  Token operator()();
};

} // namespace xra

#endif // XRA_MACRO_PARSER_HPP
