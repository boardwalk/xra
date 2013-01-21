#ifndef XRA_TOKEN_BUFFER_HPP
#define XRA_TOKEN_BUFFER_HPP

#include "lexer.hpp"

namespace xra {

template<class TokenSource>
class TokenBuffer
{
  TokenSource& source;
  deque<Token> buffer;

public:
  TokenBuffer(TokenSource& source_) :
    source(source_)
  {}

  void Consume(size_t n = 1)
  {
    while(n--)
      buffer.pop_front();
  }

  Token& operator()(size_t i = 0)
  {
    while(i >= buffer.size())
      buffer.push_back(source());
    return buffer[i];
  }

  void Unget(const Token& token)
  {
    buffer.push_front(token);
  }
};

} // namespace xra

#endif // XRA_TOKEN_BUFFER_HPP
