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
    case '[': case ']': case '^': case '{':
    case '|': case '}': case '~': 
      return true;
  }
  return false;
}

ostream& operator<<(ostream& os, const Token& token)
{
  switch(token.type) {
    case Token::Error:
      os << "<" << token.strValue << ">";
      break;
    case Token::Indent:
      os << "<indent>";
      break;
    case Token::Nodent:
      os << "<nodent>";
      break;
    case Token::Dedent:
      os << "<dedent>";
      break;
    case Token::EndOfFile:
      os << "<eof>";
      break;
    // constants
    case Token::Integer:
      os << "<int " << token.intValue << ">";
      break;
    case Token::Float:
      os << "<float " << token.floatValue << ">";
      break;
    case Token::String:
      os << "\"";
      EscapeString(token.strValue, os);
      os << "\"";
      break;
    // special identifiers
    case Token::BooleanType:
      os << "bool";
      break;
    case Token::IntegerType:
      os << "int";
      break;
    case Token::FloatType:
      os << "float";
      break;
    case Token::StringType:
      os << "str";
      break;
    case Token::Signed:
      os << "signed";
      break;
    case Token::Unsigned:
      os << "unsigned";
      break;
    case Token::True:
      os << "true";
      break;
    case Token::False:
      os << "false";
      break;
    case Token::Fn:
      os << "fn";
      break;
    case Token::If:
      os << "if";
      break;
    case Token::Else:
      os << "else";
      break;
    case Token::Elsif:
      os << "elsif";
      break;
    case Token::While:
      os << "while";
      break;
    case Token::Break:
      os << "break";
      break;
    case Token::Return:
      os << "return";
      break;
    case Token::TypeAlias:
      os << "type";
      break;
    case Token::Extern:
      os << "extern";
      break;
    // special operators
    case Token::OpenParen:
      os << '(';
      break;
    case Token::CloseParen:
      os << ')';
      break;
    case Token::Colon:
      os << ':';
      break;
    case Token::Slash:
      os << '\\';
      break;
    case Token::Backtick:
      os << '`';
      break;
    // other identifiers
    case Token::Identifier:
      os << "<identifier " << token.strValue << ">";
      break;
    // other keywords
    case Token::Operator:
      os << "<operator " << token.strValue << ">";
      break;
  }
  os << " at " << token.loc;
  return os;
}

char Lexer::GetChar()
{
  if(lastChar == '\n') {
    loc.line++;
    loc.column = 1;
  }
  else {
    loc.column++;
  }
  lastChar = (char)inputStream.get();    
  return lastChar;
}

void Lexer::UngetStr(const string& str)
{
  if(str.empty())
    return;

  inputStream.putback(lastChar);
  lastChar = str[0];

  for(size_t i = str.size(); i > 1; i--)
    inputStream.putback(str[i - 1]);

  loc.column -= str.size();
}

Token Lexer::MakeToken(Token::Type type)
{
  Token token;
  token.type = type;
  token.loc = loc;
  return token;
}

Token Lexer::MakeError(string err)
{
  Token token = MakeToken(Token::Error);
  token.strValue = move(err);
  return token;
}

Token Lexer::operator()()
{
  if(!nextTokens.empty()) {
    Token token = move(nextTokens.front());
    nextTokens.pop();
    return token;
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

    if(lastChar == '\r' || lastChar == '\n' || lastChar == EOF ||
       lastChar == '#' || parenLevel > 0)
      return (*this)();

    if(indentSize > indents.top()) {
      indents.push(indentSize);
      return MakeToken(Token::Indent);
    }

    while(indentSize < indents.top()) {
      indents.pop();
      nextTokens.push(MakeToken(Token::Dedent));
    }

    nextTokens.push(MakeToken(Token::Nodent));

    if(indentSize != indents.top())
      return MakeError("invalid indentation");

    return (*this)();
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

  if(lastChar == '\'')
    return String(false);

  if(lastChar == '"')
    return String(true);

  if(isdigit(lastChar))
    return Number();

  if(IdentifierInitial(lastChar))
  {
    string str(1, lastChar);
    while(IdentifierSubsequent(GetChar()))
      str += lastChar;
    if(str == "bool") return MakeToken(Token::BooleanType);
    if(str == "int") return MakeToken(Token::IntegerType);
    if(str == "float") return MakeToken(Token::FloatType);
    if(str == "str") return MakeToken(Token::StringType);
    if(str == "signed") return MakeToken(Token::Signed);
    if(str == "unsigned") return MakeToken(Token::Unsigned);
    if(str == "true") return MakeToken(Token::True);
    if(str == "false") return MakeToken(Token::False);
    if(str == "fn") return MakeToken(Token::Fn);
    if(str == "if") return MakeToken(Token::If);
    if(str == "elsif") return MakeToken(Token::Elsif);
    if(str == "else") return MakeToken(Token::Else);
    if(str == "while") return MakeToken(Token::While);
    if(str == "break") return MakeToken(Token::Break);
    if(str == "return") return MakeToken(Token::Return);
    if(str == "type") return MakeToken(Token::TypeAlias);
    if(str == "extern") return MakeToken(Token::Extern);
    Token token = MakeToken(Token::Identifier);
    token.strValue = move(str);
    return token;
  }

  if(lastChar == '(') {
    GetChar();
    parenLevel++;
    return MakeToken(Token::OpenParen);
  }

  if(lastChar == ')') {
    GetChar();
    parenLevel--;
    return MakeToken(Token::CloseParen);
  }

  if(lastChar == ':') {
    GetChar();
    return MakeToken(Token::Colon);
  }

  if(lastChar == '\\') {
    GetChar();
    return MakeToken(Token::Slash);
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

  if(lastChar == EOF) {
    if(indents.size() > 1) {
      indents.pop();
      return MakeToken(Token::Dedent);
    }
    return MakeToken(Token::EndOfFile);
  }

  return MakeError(string("invalid character ") + lastChar);
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

Token Lexer::Number()
{
  int base = 10;
  if(lastChar == '0') {
    GetChar();
    if(lastChar == 'b') {
      GetChar();
      base = 2;
    }
    else if(lastChar == 'x') {
      GetChar();
      base = 16;
    }
    else if(isalnum(lastChar)) {
      base = 8;
    }
    else {
      UngetStr("0");
    }
  }

  string s(1, lastChar);
  while(isalnum(GetChar()))
    s += lastChar;

  if(lastChar != '.') {
    for(char c : s) {
      if(c >= '0' && c < ('0' + min(base, 10)))
        continue;
      if(base > 10 && tolower(c) >= 'a' && tolower(c) < ('a' + (base - 10)))
        continue;
      return MakeError("invalid character in integer constant");
    }
    Token token = MakeToken(Token::Integer);
    token.intValue = (unsigned long)strtol(s.c_str(), nullptr, base);
    return token;
  }

  if(base != 10)
    return MakeError("non-decimal floating point not allowed");

  s += lastChar;
  while(isalnum(GetChar()))
    s += lastChar;

  for(char c : s) {
    if(c >= '0' && c <= '9')
      continue;
    if(c == '.')
      continue;
    return MakeError("invalid character in float constant");
  }

  Token token = MakeToken(Token::Float);
  token.floatValue = strtod(s.c_str(), nullptr);
  return token;
}

Token Lexer::String(bool interpolate)
{
  char delim = lastChar;

  string str;
  string error;
  vector<pair<size_t,  string> > interpolations;

  while(true)
  {
    GetChar();
    if(lastChar == delim)
    {
      GetChar();
      break;
    }
    else if(lastChar == '#' && interpolate)
    {
      GetChar();
      if(lastChar == '{')
      {
        string interp;
        int delimLevel = 1;
        while(delimLevel > 0) {
          interp.push_back(GetChar());
          if(lastChar == EOF) break;
          if(lastChar == '{') delimLevel++;
          if(lastChar == '}') delimLevel--;
        }
        if(lastChar == EOF) {
          error = "unterminated code interpolation";
          break;
        }
        interp.resize(interp.size() - 1);
        interpolations.push_back({str.size(), move(interp)});
      }
      else
      {
        str += '#';
      }
    }
    else if(lastChar == '\\' && interpolate)
    {
      GetChar();
      char c = lastChar;
      switch(c)
      {
      case '\\': c = '\\'; break;
      case '0':  c = '\0'; break;
      case 'a':  c = '\a'; break;
      case 'b':  c = '\b'; break;
      case 'f':  c = '\f'; break;
      case 'n':  c = '\n'; break;
      case 'r':  c = '\r'; break;
      case 't':  c = '\t'; break;
      case 'v':  c = '\v'; break;
        // TODO handle \x hex scape
      }
      str += c;
    }
    else if(lastChar == EOF)
    {
      error = "unterminated string literal";
      break;
    }
    else
    {
      str += lastChar;
    }
  }

  if(!error.empty())
    return MakeError(error);

  if(interpolations.empty())
    return MakeToken(Token::String).Str(move(str));

  // escape existing braces
  size_t i = 0;
  while(true) {
    i = str.find_first_of("{}", i);
    if(i == string::npos)
      break;
    for(auto& interp : interpolations) {
      if(interp.first > i)
        interp.first++;
    }
    str.insert(i, 1, str[i]);
    i += 2;
  }

  // insert placeholders
  for(i = interpolations.size(); i > 0; i--) {
    stringstream ss;
    ss << "{" << (i - 1) << "}";
    str.insert(interpolations[i - 1].first, ss.str());
  }

  // desugar interpolation
  // "#{a} #{b} #{c}" => (format("{0} {1} {2}", (a), (b), (c)))
  nextTokens.push(MakeToken(Token::Identifier).Str("format"));
  nextTokens.push(MakeToken(Token::OpenParen));
  nextTokens.push(MakeToken(Token::String).Str(move(str)));
  for(auto& interp : interpolations) {
    nextTokens.push(MakeToken(Token::Operator).Str(","));
    nextTokens.push(MakeToken(Token::OpenParen));
    stringstream ss(interp.second);
    Lexer lexer(ss, "interpolation");
    while(true) {
      Token token = lexer();
      if(token.type == Token::EndOfFile)
        break;
      nextTokens.push(move(token));
    }
    nextTokens.push(MakeToken(Token::CloseParen));
  }
  nextTokens.push(MakeToken(Token::CloseParen));
  nextTokens.push(MakeToken(Token::CloseParen));
  return MakeToken(Token::OpenParen);
}

} // namespace xra
