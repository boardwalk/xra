#ifndef XRA_BUFFERED_LEXER_HPP
#define XRA_BUFFERED_LEXER_HPP

#include "lexer.hpp"

namespace xra {

class BufferedLexer
{
  Lexer& lexer;
  deque<Token> buffer;
  ssize_t pos;

public:
  BufferedLexer(Lexer& lexer_) :
    lexer(lexer_),
    pos(0)
  {}

  void Consume(ssize_t n = 1)
  {
    pos += n;
    while(pos > 1) { // keep one old token
      buffer.pop_front();
      pos--;
    }
  }

  Token& Get(ssize_t i = 0)
  {
    assert(pos + i >= 0);
    while(pos + i >= (ssize_t)buffer.size())
      buffer.push_back(lexer.Get());
    return buffer[pos + i];
  }
};

} // namespace xra

#endif // XRA_BUFFERED_LEXER_HPP
