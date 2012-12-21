#ifndef XRA_BUFFERED_LEXER_HPP
#define XRA_BUFFERED_LEXER_HPP

#include "lexer.hpp"

namespace xra {

class BufferedLexer
{
  Lexer& lexer;
  deque<Token> buffer;

public:
  BufferedLexer(Lexer& lexer_) :
    lexer(lexer_)
  {}

  void Consume(size_t n = 1)
  {
    while(n-- > 0)
      buffer.pop_front();
  }

  Token& Get(size_t i = 0)
  {
    while(i >= buffer.size())
      buffer.push_back(lexer.Get());

    return buffer[i];
  }
};

} // namespace xra

#endif // XRA_BUFFERED_LEXER_HPP
