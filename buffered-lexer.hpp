#ifndef XRA_BUFFERED_LEXER_HPP
#define XRA_BUFFERED_LEXER_HPP

#include "lexer.hpp"

namespace xra {

class BufferedLexer
{
  Lexer& lexer;
  Token token;

public:
  BufferedLexer(Lexer& lexer_) :
    lexer(lexer_)
  {}

  void Next()
  {
    token = lexer.Get();
  }

  const Token& Get() const
  {
    return token;
  }
};

} // namespace xra

#endif // XRA_BUFFERED_LEXER_HPP