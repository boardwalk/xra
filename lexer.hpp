#ifndef XRA_LEXER_HPP
#define XRA_LEXER_HPP

#include <istream>

namespace xra {

using namespace std;

struct Token
{
  enum Type {
    Error, // strValue
    Indent,
    Nodent,
    Dedent,
    Fn,
    If,
    Then,
    Else,
    True,
    False,
    Extern,
    Identifier, // strValue
    OpenParen,
    CloseParen,
    Comma,
    Colon,
    Semicolon,
    Integer, // intValue
    Float, // floatValue
    String, // strValue
    Operator, // strValue
    EndOfFile
  };

  Type type;

  int line;
  int column;

  string strValue;
  union {
    long intValue;
    double floatValue;
  };

  string ToString() const;
};

template<class T>
T& operator<<(T& stm, const Token& token)
{
  return stm << token.ToString();
}

class Lexer
{
  istream& inputStream;
  char lastChar;

  int line;
  int column;

  stack<int> indents;
  int dedentCount;

  Lexer(const Lexer&);
  Lexer& operator=(const Lexer&);

  char GetChar();
  void UngetStr(const string& str);
  Token MakeToken(Token::Type type);
  Token MakeError(const char* err);

  bool NestableComment();
  bool Number(Token& token);
  bool String(Token& token);

public:
  Lexer(istream& inputStream_) :
    inputStream(inputStream_),
    lastChar(' '),
    line(1),
    column(0),
    dedentCount(0)
  {
    indents.push(0);
  }

  Token Get();
};

} // namespace xra

#endif // XRA_LEXER_HPP
