#ifndef XRA_LEXER_HPP
#define XRA_LEXER_HPP

#include <istream>

namespace xra {

struct SourceLoc
{
  shared_ptr<string> source;
  int line;
  int column;
};

struct Token
{
  enum Type {
    Error, // strValue
    Indent,
    Nodent,
    Dedent,
    Fn,
    Return,
    BooleanType,
    IntegerType,
    FloatType,
    StringType,
    If,
    Then,
    Elsif,
    Else,
    True,
    False,
    Extern,
    Identifier, // strValue
    OpenParen,
    CloseParen,
    Colon,
    Backtick,
    Integer, // intValue
    Float, // floatValue
    String, // strValue
    Operator, // strValue
    EndOfFile
  };

  Type type;
  SourceLoc loc;

  string strValue;
  union {
    long intValue;
    double floatValue;
  };
};

class Lexer
{
  istream& inputStream;
  SourceLoc loc;
  char lastChar;
  stack<int> indents;
  int dedentCount;

  Lexer(const Lexer&);
  Lexer& operator=(const Lexer&);

  char GetChar();
  void UngetStr(const string& str);
  Token MakeToken(Token::Type type);
  Token MakeError(string err);

  bool NestableComment();
  bool Number(Token& token);
  bool String(Token& token);

public:
  Lexer(istream& inputStream_, const string& source) :
    inputStream(inputStream_),
    lastChar(' '),
    dedentCount(-1)
  {
    loc.source = make_shared<string>(source);
    loc.line = 1;
    loc.column = 0;
    indents.push(0);
  }

  Token operator()();
};

} // namespace xra

#endif // XRA_LEXER_HPP
