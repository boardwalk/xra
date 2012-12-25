#include "common.hpp"
#include "lexer.hpp"

namespace xra {

static bool IsSpace(char c)
{
  return c == ' ' || c == '\t';
}

static bool IdentifierInitial(char c)
{
  return isalpha(c) || c == '_';
}

static bool IdentifierSubsequent(char c)
{
  return IdentifierInitial(c) || isdigit(c);
}

static bool Operator(char c)
{
  switch(c) {
    case '!': case '$': case '%': case '&':
    case '*': case '+': case ',': case '-':
    case '.': case '/': case ';': case '<':
    case '=': case '>': case '?': case '@':
    case '\\': case '^': case '|': case '~':
      return true;
  }
  return false;
}

void ToString(const Token& token, stringstream& ss)
{
  switch(token.type) {
    case Token::Error:
      ss << "<error " << token.strValue << ">";
      break;
    case Token::Indent:
      ss << "<indent>";
      break;
    case Token::Nodent:
      ss << "<nodent>";
      break;
    case Token::Dedent:
      ss << "<dedent>";
      break;
    case Token::Fn:
      ss << "fn";
      break;
    case Token::Else:
      ss << "else";
      break;
    case Token::Elsif:
      ss << "elsif";
      break;
    case Token::If:
      ss << "if";
      break;
    case Token::Then:
      ss << "then";
      break;
    case Token::Return:
      ss << "return";
      break;
    case Token::BooleanType:
      ss << "bool";
      break;
    case Token::IntegerType:
      ss << "int";
      break;
    case Token::FloatType:
      ss << "float";
      break;
    case Token::StringType:
      ss << "str";
      break;
    case Token::True:
      ss << "true";
      break;
    case Token::False:
      ss << "false";
      break;
    case Token::Extern:
      ss << "extern";
      break;
    case Token::Identifier:
      ss << "<identifier " << token.strValue << ">";
      break;
    case Token::OpenParen:
      ss << '(';
      break;
    case Token::CloseParen:
      ss << ')';
      break;
    case Token::Colon:
      ss << ':';
      break;
    case Token::Backtick:
      ss << '`';
      break;
    case Token::Integer:
      ss << token.intValue;
      break;
    case Token::Float:
      ss << token.floatValue;
      break;
    case Token::String:
      ss << "\"";
      EscapeString(token.strValue, ss);
      ss << "\"";
      break;
    case Token::Operator:
      ss << "<operator " << token.strValue << ">";
      break;
    case Token::EndOfFile:
      ss << "<eof>";
      break;
    default:
      ss << "<unknown>";
      break;
  }
  ss << " L" << token.line << "C" << token.column;
}

char Lexer::GetChar()
{
  if(lastChar == '\n') {
    line++;
    column = 1;
  }
  else {
    column++;
  }
  lastChar = inputStream.get();    
  return lastChar;
}

void Lexer::UngetStr(const string& str)
{
  if(str.empty())
    return;

  inputStream.putback(lastChar);
  lastChar = str[0];

  for(ssize_t i = str.size() - 1; i > 0; i--)
    inputStream.putback(str[i]);

  column -= str.size();
}

Token Lexer::MakeToken(Token::Type type)
{
  Token token;
  token.type = type;
  token.line = line;
  token.column = column;
  return token;
}

Token Lexer::MakeError(const char* err)
{
  Token token = MakeToken(Token::Error);
  token.strValue = err;
  return token;
}

Token Lexer::operator()()
{
  if(dedentCount > 0) {
    dedentCount--;
    return MakeToken(Token::Dedent);
  }

  if(dedentCount == 0) {
    dedentCount--;
    return MakeToken(Token::Nodent);
  }

  while(IsSpace(lastChar))
    GetChar();

  if(lastChar == '\r' || lastChar == '\n')
  {
    bool isCR = (lastChar == '\r');
    GetChar();
    if(isCR && lastChar == '\n')
      GetChar();

    int indentSize = 0;
    while(IsSpace(lastChar)) {
      indentSize++;
      GetChar();
    }

    if(lastChar == '\r' || lastChar == '\n' || lastChar == '#' || lastChar == EOF)
      return (*this)();

    if(indentSize > indents.top()) {
      indents.push(indentSize);
      return MakeToken(Token::Indent);
    }

    if(indentSize == indents.top())
      return MakeToken(Token::Nodent);

    dedentCount = 0;
    do {
      indents.pop();
      dedentCount++;
    } while(indentSize < indents.top());

    if(indentSize != indents.top())
      return MakeError("invalid indentation");

    dedentCount--;
    return MakeToken(Token::Dedent);
  }

  if(lastChar == '#') {
    GetChar();
    if(lastChar == '|') {
      GetChar();
      if(!NestableComment())
        return MakeError("missing end of nestable comment");
    }
    else {
      while(lastChar != '\r' && lastChar != '\n' && lastChar != EOF)
        GetChar();
    }
    return (*this)();
  }

  if(IdentifierInitial(lastChar))
  {
    string str(1, lastChar);
    while(IdentifierSubsequent(GetChar()))
      str += lastChar;
    if(str == "fn") return MakeToken(Token::Fn);
    if(str == "if") return MakeToken(Token::If);
    if(str == "then") return MakeToken(Token::Then);
    if(str == "elsif") return MakeToken(Token::Elsif);
    if(str == "else") return MakeToken(Token::Else);
    if(str == "return") return MakeToken(Token::Return);
    if(str == "bool") return MakeToken(Token::BooleanType);
    if(str == "int") return MakeToken(Token::IntegerType);
    if(str == "float") return MakeToken(Token::FloatType);
    if(str == "str") return MakeToken(Token::StringType);
    if(str == "true") return MakeToken(Token::True);
    if(str == "false") return MakeToken(Token::False);
    if(str == "extern") return MakeToken(Token::Extern);
    Token token = MakeToken(Token::Identifier);
    token.strValue = move(str);
    return token;
  }

  if(lastChar == '(') {
    GetChar();
    return MakeToken(Token::OpenParen);
  }

  if(lastChar == ')') {
    GetChar();
    return MakeToken(Token::CloseParen);
  }

  if(lastChar == ':') {
    GetChar();
    return MakeToken(Token::Colon);
  }

  if(lastChar == '`') {
    GetChar();
    return MakeToken(Token::Backtick);
  }

  if(Operator(lastChar))
  {
    string str(1, lastChar);
    while(Operator(GetChar()))
      str += lastChar;

    Token token = MakeToken(Token::Operator);
    token.strValue = str;
    return token;
  }

  if(isdigit(lastChar))
  {
    Token token;
    if(Number(token))
      return token;
  }

  if(lastChar == '"') {
    Token token;
    if(String(token))
      return token;
  }

  if(lastChar == EOF) {
    if(indents.size() > 1) {
      indents.pop();
      return MakeToken(Token::Dedent);
    }
    return MakeToken(Token::EndOfFile);
  }

  return MakeError("invalid character");
}

bool Lexer::NestableComment()
{
  while(lastChar != EOF)
  {
    if(lastChar == '#')
    {
      if(GetChar() == '|') {
        GetChar();
        if(!NestableComment())
          return false;
      }
    }
    else if(lastChar == '|')
    {
      if(GetChar() == '#') {
        GetChar();
        return true;
      }
    }
    else
    {
      GetChar();
    }
  }
  return false;
}

bool Lexer::Number(Token& token)
{
  string s(1, lastChar);
  while(isdigit(GetChar()))
    s += lastChar;

  if(lastChar != '.') {
    token = MakeToken(Token::Integer);
    token.intValue = strtol(s.c_str(), nullptr, 10);
    return true;
  }

  s += lastChar;
  GetChar();

  if(!isdigit(lastChar)) {
    UngetStr(s);
    return false;
  }
  s += lastChar;

  while(isdigit(GetChar()))
    s += lastChar;

  token = MakeToken(Token::Float);
  token.floatValue = strtod(s.c_str(), nullptr);
  return true;
}

bool Lexer::String(Token& token)
{
  string str;
  while(true)
  {
    GetChar();
    if(lastChar == '\\') {
      GetChar();
      char c;
      switch(lastChar) {
        case '\'': c = '\''; break;
        case '\"': c = '\"'; break;
        case '\\': c = '\\'; break;
        case '0':  c = '\0'; break;
        case 'a':  c = '\a'; break;
        case 'b':  c = '\b'; break;
        case 'f':  c = '\f'; break;
        case 'n':  c = '\n'; break;
        case 'r':  c = '\r'; break;
        case 't':  c = '\t'; break;
        case 'v':  c = '\v'; break;
        default:
          token = MakeError("invalid escape sequence");
          return true;
      }
      str += c;
    }
    else if(lastChar == '"') {
      GetChar();
      token = MakeToken(Token::String);
      token.strValue = move(str);
      return true;
    }
    else if(lastChar == '\r' || lastChar == '\n' || lastChar == EOF) {
      token = MakeError("unterminated string literal");
      return true;
    }
    else {
      str += lastChar;
    }
  }
}

} // namespace xra
