#ifndef XRA_LEXER_HPP
#define XRA_LEXER_HPP

#include <istream>

namespace xra {

struct Token
{
  enum Type {
    Error, // strValue
    Indent,
    Nodent,
    Dedent,
    Fn,
    If,
    Elsif,
    Else,
    While,
    Break,
    Return,
    BooleanType,
    IntegerType,
    FloatType,
    StringType,
    Unsigned,
    Signed,
    True,
    False,
    Extern,
    Identifier, // strValue
    OpenParen,
    CloseParen,
    Colon,
    Slash,
    Backtick,
    Integer, // intValue
    Float, // floatValue
    String, // strValue
    Operator, // strValue
    EndOfFile
  };

  Token& Str(string str) {
    strValue = move(str);
    return *this;
  }

  Type type;
  SourceLoc loc;

  string strValue;
  union {
    unsigned long intValue;
    double floatValue;
  };
};

ostream& operator<<(ostream&, const Token&);

class Lexer
{
  istream& inputStream;
  SourceLoc loc;
  char lastChar;
  stack<int> indents;
  queue<Token> nextTokens;
  int parenLevel;

  Lexer(const Lexer&);
  Lexer& operator=(const Lexer&);

  char GetChar();
  void UngetStr(const string& str);
  Token MakeToken(Token::Type type);
  Token MakeError(string err);

  bool NestableComment();
  Token String(bool interpolate);
  Token Number();

public:
  Lexer(istream& inputStream_, const string& source) :
    inputStream(inputStream_),
    lastChar(' '),
    parenLevel(0)
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
