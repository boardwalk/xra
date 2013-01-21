#include "common.hpp"
#include "macro-parser.hpp"
#include <fstream>

#define TOKEN(t) (tokens().type == Token::t)
#define ERROR(what) \
  { \
    Error() << what << " near " << tokens() << " at macro-parser.cpp:" << __LINE__; \
    return; \
  }
#define EXPECTED(t) \
  ERROR("expected " #t)

namespace xra {

static string MakeIdentifier()
{
  static int count = 0;
  ++count;

  string name(1, '$');
  for(int i = count; i > 0; i /= 26)
    name += ('a' + (i % 26) - 1);

  return name;
}

void MacroParser::IncMacro()
{
  if(!TOKEN(String))
    EXPECTED(String)
  auto fileName = tokens().strValue;
  tokens.Consume();

  ifstream file(fileName);
  if(!file)
    ERROR("could not open include " << fileName)

  Lexer lexer(file, fileName);
  vector<Token> fileTokens;
  while(true) {
    Token token = lexer();
    if(token.type == Token::EndOfFile)
      break;
    fileTokens.push_back(token);
  }

  for(auto& token : Reverse(fileTokens))
    tokens.Unget(token);
}

void MacroParser::FileMacro()
{
  Token token;
  token.type = Token::String;
  token.loc = macroCallLoc;
  token.strValue = *macroCallLoc.source;
  tokens.Unget(token);
}

void MacroParser::LineMacro()
{
  Token token;
  token.type = Token::Integer;
  token.loc = macroCallLoc;
  token.intValue = (unsigned long)macroCallLoc.line;
  tokens.Unget(token);
}

void MacroParser::ShellMacro()
{
  if(!TOKEN(String))
    EXPECTED(String)
  auto cmd = tokens().strValue;
  tokens.Consume();

  FILE* fp = popen(cmd.c_str(), "r");
  if(fp == nullptr)
    ERROR("shell failed")

  string output;
  while(true) {
    char buf[64];
    size_t bytesRead = fread(buf, 1, sizeof(buf), fp);
    output.append(buf, bytesRead);
    if(bytesRead < sizeof(buf))
      break;
  }

  int end = feof(fp);

  if(pclose(fp) != 0)
    ERROR("shell failed");
  if(end == 0)
    ERROR("shell read failed");

  size_t outputSize = output.size();
  while(outputSize && isspace(output[outputSize - 1]))
    outputSize--;
  output.resize(outputSize);

  Token token;
  token.type = Token::String;
  token.loc = macroCallLoc;
  token.strValue = move(output);
  tokens.Unget(token);
}

void MacroParser::CatMacro(bool asIdentifier)
{
  if(!TOKEN(OpenParen))
    EXPECTED(OpenParen)
  tokens.Consume();

  stringstream ss;
  while(true) {
    if(TOKEN(Operator) && tokens().strValue == "$") {
      tokens.Consume();
      MacroCall();
      continue;
    }

    if(TOKEN(Identifier) || TOKEN(String) || TOKEN(Operator)) {
      ss << tokens().strValue;
    }
    else if(TOKEN(Integer)) {
      ss << tokens().intValue;
    }
    else if(TOKEN(Float)) {
      ss << tokens().floatValue;
    }
    else {
      ERROR("expected token with value")
    }
    tokens.Consume();

    if(TOKEN(CloseParen))
      break;
    if(TOKEN(EndOfFile))
      ERROR("unterminated macro arguments")
    if(!TOKEN(Operator) || tokens().strValue != ",")
      EXPECTED(Comma)

    tokens.Consume();
  }
  tokens.Consume();

  Token token;
  token.type = asIdentifier ? Token::Identifier : Token::String;
  token.loc = macroCallLoc;
  token.strValue = ss.str();

  if(asIdentifier)
  {
    int i = 0;
    for(char c : token.strValue) {
      if(i++ && isdigit(c))
        continue;
      if(isalpha(c) || c == '_')
        continue;
      ERROR("bad identifier: " << token.strValue)
    }
  }

  tokens.Unget(token);
}

void MacroParser::Macro() // prefix: macro
{
  if(!TOKEN(Identifier))
    EXPECTED(Identifier)
  auto name = tokens().strValue;
  tokens.Consume();

  MacroDef macro;

  /*
   * parse parameters
   */
  if(TOKEN(OpenParen))
  {
    tokens.Consume();

    set<string> paramSet;
    while(true)
    {
      if(!TOKEN(Identifier))
        EXPECTED(Identifier)
      auto param = tokens().strValue;
      tokens.Consume();

      if(!paramSet.insert(param).second)
        ERROR("duplicate macro parameter: " << param);
      macro.params.push_back(param);

      if(!TOKEN(Operator) || tokens().strValue != ",")
        break;
      tokens.Consume();
    }

    if(!TOKEN(CloseParen))
      EXPECTED(CloseParen)
    tokens.Consume();
  }

  /*
   * parse body
   */
  map<string, string> localVars;
  bool blockMacro;
  int level = 0;

  if(TOKEN(Colon))
    blockMacro = false;
  else if(TOKEN(Indent))
    blockMacro = true;
  else
    EXPECTED(ColonOrIndent);
  tokens.Consume();

  while(true)
  {
    if(blockMacro)
    {
      if(TOKEN(Indent))
        level++;
      else if(TOKEN(Dedent))
        level--;
      if(level < 0)
        break;
    }
    else
    {
      if(TOKEN(Nodent) || TOKEN(EndOfFile))
        break;
      if(TOKEN(Indent) || TOKEN(Nodent))
        ERROR("unexpected indentation in single line macro")
    }

    Token token;

    if(TOKEN(Operator) && tokens().strValue == "$" &&
       tokens(1).type == Token::Identifier &&
       macros.Find(tokens(1).strValue) == macros.End() &&
       tokens(1).strValue != "inc" &&
       tokens(1).strValue != "file" &&
       tokens(1).strValue != "line" &&
       tokens(1).strValue != "shell" &&
       tokens(1).strValue != "cat" &&
       tokens(1).strValue != "cati") // TODO maybe not hardcode this?
    {
      token = tokens(1);
      tokens.Consume(2);

      auto& mangledIden = localVars[token.strValue];
      if(mangledIden.empty())
        mangledIden = MakeIdentifier();

      token.strValue = mangledIden;
    }
    else
    {
      token = tokens();
      tokens.Consume();
    }

    macro.body.push_back(move(token));
  }

  if(blockMacro)
    tokens.Consume();
  if(TOKEN(Nodent))
    tokens.Consume();

  macros.AddValue(name, move(macro));
}

void MacroParser::MacroCall() // prefix: $
{
  macroCallLoc = tokens().loc;

  if(!TOKEN(Identifier))
    EXPECTED(Identifier)
  auto name = tokens().strValue;
  tokens.Consume();

  auto macro = macros.Find(name);
  if(macro != macros.End())
  {
    if(activeMacros.find(name) != activeMacros.end())
      ERROR("recursive call of macro " << name);

    activeMacros.insert(name);
    UserMacroCall(macro->second);
    activeMacros.erase(name);
  }
  else if(name == "inc")
    IncMacro();
  else if(name == "file")
    FileMacro();
  else if(name == "line")
    LineMacro();
  else if(name == "shell")
    ShellMacro();
  else if(name == "cat")
    CatMacro(false);
  else if(name == "cati")
    CatMacro(true);
  else
    ERROR("undefined macro " << name)
}

void MacroParser::UserMacroCall(const MacroDef& macro)
{
 map<string, vector<Token> > args;

  if(!macro.params.empty())
  {
    if(!TOKEN(OpenParen))
      EXPECTED(OpenParen)
    tokens.Consume();

    for(size_t i = 0; i < macro.params.size(); i++)
    {
      vector<Token> arg;

      while(true)
      {
        if((i + 1) < macro.params.size() && TOKEN(Operator) && tokens().strValue == ",")
          break;
        if((i + 1) == macro.params.size() && TOKEN(CloseParen))
          break;
        if(TOKEN(EndOfFile))
          ERROR("unterminated macro arguments")
        arg.push_back(tokens());
        tokens.Consume();
      }
      tokens.Consume();

      args[macro.params[i]] = move(arg);
    }
  }

  for(auto token : Reverse(macro.body))
  {
    token.loc = macroCallLoc;

    if(token.type != Token::Identifier) {
      tokens.Unget(token);
      continue;
    }

    auto arg = args.find(token.strValue);
    if(arg == args.end()) {
      tokens.Unget(token);
      continue;
    }

    for(auto& argToken : Reverse(arg->second))
      tokens.Unget(argToken);
  }
}

Token MacroParser::operator()()
{
  while(true) {
    if(TOKEN(Operator) && tokens().strValue == "$") {
      tokens.Consume();
      MacroCall();
    }
    else if(TOKEN(Macro)) {
      tokens.Consume();
      Macro();
    }
    else break;
  }

  Token token = tokens();
  tokens.Consume();
  return token;
}

} // namespace xra
