#include "common.hpp"
#include "lexer.hpp"
#include <sstream>
#include <iomanip>

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

static std::string EscapeString(const string& str)
{
  stringstream ss;
  for(size_t i = 0; i < str.size(); i++) {
    if(str[i] == '\"')
      ss << "\\\"";
    else if(!isprint(str[i]))
      ss << "\\x" << hex << setw(2) << setfill('0') << (int)str[i];
    else
      ss << str[i];
  }
  return ss.str();
}

string Token::ToString() const
{
  stringstream ss;
  switch(type) {
    case Error:
      ss << "<error " << strValue << ">";
      break;
    case Indent:
      ss << "<indent>";
      break;
    case Nodent:
      ss << "<nodent>";
      break;
    case Dedent:
      ss << "<dedent>";
      break;
    case Fn:
      ss << "fn";
      break;
    case If:
      ss << "if";
      break;
    case Then:
      ss << "then";
      break;
    case Else:
      ss << "else";
      break;
    case True:
      ss << "true";
      break;
    case False:
      ss << "false";
      break;
    case Extern:
      ss << "extern";
      break;
    case Identifier:
      ss << "<identifier " << strValue << ">";
      break;
    case OpenParen:
      ss << '(';
      break;
    case CloseParen:
      ss << ')';
      break;
    case Comma:
      ss << ',';
      break;
    case Semicolon:
      ss << ';';
      break;
    case Integer:
      ss << intValue;
      break;
    case Float:
      ss << floatValue;
      break;
    case String:
      ss << "\"" << EscapeString(strValue) << "\"";
      break;
    case Operator:
      ss << "<operator " << strValue << ">";
      break;
    case EndOfFile:
      ss << "<eof>";
      break;
    default:
      ss << "<unknown>";
      break;
  }
  ss << " L" << line << "C" << column;
  return ss.str();
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

Token Lexer::Get()
{
  if(dedentCount) {
    dedentCount--;
    return MakeToken(Token::Dedent);
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

    if(lastChar == '\r' || lastChar == '\n')
      return Get();

    if(indentSize > indents.top()) {
      indents.push(indentSize);
      return MakeToken(Token::Indent);
    }

    if(indentSize == indents.top())
      return MakeToken(Token::Nodent);

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
    return Get();
  }

  if(IdentifierInitial(lastChar))
  {
    string str(1, lastChar);
    while(IdentifierSubsequent(GetChar()))
      str += lastChar;
    if(str == "fn") return MakeToken(Token::Fn);
    if(str == "if") return MakeToken(Token::If);
    if(str == "then") return MakeToken(Token::Then);
    if(str == "else") return MakeToken(Token::Else);
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

  if(lastChar == ',') {
    GetChar();
    return MakeToken(Token::Comma);
  }

  if(lastChar == ';') {
    GetChar();
    return MakeToken(Token::Semicolon);
  }

  if(isdigit(lastChar) || lastChar == '-')
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

  if(isprint(lastChar))
  {
    string str(1, lastChar);
    GetChar();
    while(isprint(lastChar) && !IsSpace(lastChar) && lastChar != '#') {
      str += lastChar;
      GetChar();
    }

    Token token = MakeToken(Token::Operator);
    token.strValue = str;
    return token;
  }

  if(lastChar == EOF)
    return MakeToken(Token::EndOfFile);

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
  string s;

  if(lastChar == '-') {
    s += lastChar;
    GetChar();
  }

  if(!isdigit(lastChar)) {
    UngetStr(s);
    return false;
  }
  s += lastChar;

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
