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
    EndOfFile,
    // constants
    Integer, // intValue
    Float, // floatValue
    String, // strValue
    // special identifiers
    BooleanType,
    IntegerType,
    FloatType,
    StringType,
    Signed,
    Unsigned,
    True,
    False,
    Module,
    Using,
    Fn,
    If,
    Elsif,
    Else,
    While,
    Break,
    Return,
    TypeAlias,
    Extern,
    Macro,
    // special operators
    Dollar,
    OpenParen,
    CloseParen,
    Colon,
    Slash,
    Backtick,
    // other identifiers
    Identifier, // strValue
    // other operators
    Operator // strValue
  };

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
  deque<Token> tokens;
  size_t lastConsumed;
  int parenLevel;

  Lexer(const Lexer&);
  Lexer& operator=(const Lexer&);

  char GetChar();
  void UngetStr(const string& str);
  void MakeToken(Token::Type type);
  void MakeError(string s);
  void MakeIdentifier(string s);
  void MakeOperator(string s);

  void NextToken();
  bool NestableComment();
  void String();
  void Number();

public:
  Lexer(istream& inputStream_, const string& source) :
    inputStream(inputStream_),
    lastChar(' '),
    lastConsumed(0),
    parenLevel(0)
  {
    loc.source = make_shared<string>(source);
    loc.line = 1;
    loc.column = 0;
  }

  Token& operator()(size_t i = 0);
  void Consume(size_t n = 1);
};

} // namespace xra

#endif // XRA_LEXER_HPP
