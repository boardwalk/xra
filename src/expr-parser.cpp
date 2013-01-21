#include "common.hpp"
#include "expr-parser.hpp"
#include "expr.hpp"
#include "type.hpp"

#define TOKEN(t) (lexer().type == Token::t)
#define ERROR(what) \
  { \
    Error() << what << " near " << lexer() << " at expr-parser.cpp:" << __LINE__; \
    return {}; \
  }
#define EXPECTED(t) \
  ERROR("expected " #t)

namespace xra {

static const map<string, pair<int, bool> > binaryOperators {
  {".", {19, false}},
  {"#infix", {18, false}}, // a call to a regular function using backticks (x `f y)
  {"#call", {17, true}}, // a call to a function using a space (f x)
  {"*", {16, false}},
  {"/", {16, false}},
  {"%", {16, false}},
  {"#custom", {15, false}}, // default settings for custom operators (any not listed)
  {"+", {14, false}},
  {"-", {14, false}},
  {"<<", {13, false}},
  {">>", {13, false}},
  {"<", {12, false}},
  {"<=", {12, false}},
  {">", {12, false}},
  {">=", {12, false}},
  {"==", {11, false}},
  {"!=", {11, false}},
  {"&", {10, false}},
  {"^", {9, false}},
  {"|", {8, false}},
  {"&&", {7, false}},
  {"||", {6, false}},
  {",", {4, false}},
  {"=", {3, true}},
  {"#if", {2, false}},
  {"#while", {2, false}},
  {";", {1, false}}
};

static const map<string, int> unaryOperators {
  {"+", 18},
  {"-", 18},
  {"!", 18},
  {"~", 18},
  {"*", 18},
  {"&", 18}
};

static string MakeIdentifier()
{
  static int count = 0;
  ++count;

  string name(1, '$');
  for(int i = count; i > 0; i /= 26)
    name += ('a' + (i % 26) - 1);

  return name;
}

ExprPtr ExprParser::FileMacro()
{
  return new EString(*macroCallLoc.source);
}

ExprPtr ExprParser::LineMacro()
{
  return new EInteger((unsigned long)macroCallLoc.line);
}

ExprPtr ExprParser::ShellMacro()
{
  if(!TOKEN(String))
    EXPECTED(String)
  auto cmd = lexer().strValue;
  lexer.Consume();

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

  return new EString(move(output));
}

ExprPtr ExprParser::FlatBlock()
{
  auto list = make_unique<EList>();

  while(true) {
    list->exprs.push_back(Expr());

    if(!TOKEN(Nodent))
      break;
    lexer.Consume();
  }

  return new ECall(new EVariable(";"), list.release());
}

ExprPtr ExprParser::Block() // prefix: indent
{
  ExprPtr expr = FlatBlock();

  if(!TOKEN(Dedent))
    EXPECTED(Dedent)
  lexer.Consume();

  return expr;
}

ExprPtr ExprParser::Clause()
{
  if(TOKEN(Indent)) {
    lexer.Consume();
    return Block();
  }

  if(!TOKEN(Colon))
    EXPECTED(Colon)
  lexer.Consume();

  return Expr();
}

ExprPtr ExprParser::Name()
{
  string name;

  if(TOKEN(Operator) && lexer().strValue == ".") {
    name += '.';
    lexer.Consume();
  }

  while(true) {
    if(!TOKEN(Identifier))
      EXPECTED(Identifier)
    name += lexer().strValue;
    lexer.Consume();

    if(!TOKEN(Operator) || lexer().strValue != ".")
      break;
    name += '.';
    lexer.Consume();
  }

  return new EVariable(move(name));
}

ExprPtr ExprParser::Module() // prefix: module
{
  auto list = make_unique<EList>();
  list->exprs.push_back(Name());

  if(!TOKEN(Indent) && !TOKEN(Nodent))
    EXPECTED(IndentOrNodent)
  bool indented = TOKEN(Indent);
  lexer.Consume();

  list->exprs.push_back(FlatBlock());

  if(indented) {
    if(!TOKEN(Dedent))
      EXPECTED(Dedent)
    lexer.Consume();
  }

  return new ECall(new EVariable("#module"), list.release());
}

ExprPtr ExprParser::Using() // prefix: using
{
  auto list = make_unique<EList>();
  list->exprs.push_back(Name());
  if(!TOKEN(Nodent))
    EXPECTED(Nodent)
  lexer.Consume();
  list->exprs.push_back(FlatBlock());
  return new ECall(new EVariable("#using"), list.release());
}

ExprPtr ExprParser::Fn() // prefix: fn
{
  return new EFunction(ParseTypeList(lexer), Clause());
}

ExprPtr ExprParser::If() // prefix: if
{
  auto list = make_unique<EList>();

  bool more = true;
  while(more)
  {
    list->exprs.push_back(Expr());
    list->exprs.push_back(Clause());

    if(TOKEN(Elsif))
      lexer.Consume();
    else if(TOKEN(Nodent) && lexer(1).type == Token::Elsif)
      lexer.Consume(2);
    else
      more = false;
  }

  more = true;
  if(TOKEN(Else))
    lexer.Consume();
  else if(TOKEN(Nodent) && lexer(1).type == Token::Else)
    lexer.Consume(2);
  else
    more = false;

  if(more) {
    list->exprs.push_back(new EBoolean(true));
    list->exprs.push_back(Clause());
  }

  return new ECall(new EVariable("#if"), list.release());
}

ExprPtr ExprParser::While() // prefix: while
{
  auto list = make_unique<EList>();
  list->exprs.push_back(Expr());
  list->exprs.push_back(Clause());

  return new ECall(new EVariable("#while"), list.release());
}

ExprPtr ExprParser::Break() // prefix: break
{
  return new ECall(new EVariable("#break"), new EList);
}

ExprPtr ExprParser::Return() // prefix: return
{
  auto list = make_unique<EList>();
  ExprPtr expr = Expr(false, 0);
  if(expr)
    list->exprs.push_back(expr);
  return new ECall(new EVariable("#return"), list.release());
}

ExprPtr ExprParser::TypeAlias()
{
  if(!TOKEN(Identifier))
    EXPECTED(Identifier);
  auto name = lexer().strValue;
  lexer.Consume();

  if(!TOKEN(Operator) || lexer().strValue != "=")
    EXPECTED(Equals)
  lexer.Consume();

  auto type = ParseType(lexer);
  if(!type)
    EXPECTED(Type)

  return new ETypeAlias(name, type);
}

ExprPtr ExprParser::Extern() // prefix: extern
{
  if(!TOKEN(Identifier))
    EXPECTED(Identifier)
  string name = lexer().strValue;
  lexer.Consume();

  TypePtr type = ParseType(lexer);
  if(!type)
    EXPECTED(Type)

  return new EExtern(name, type);
}

ExprPtr ExprParser::Macro() // prefix: macro
{
  if(!TOKEN(Identifier))
    EXPECTED(Identifier)
  auto name = lexer().strValue;
  lexer.Consume();

  MacroDef macro;

  /*
   * parse parameters
   */
  if(TOKEN(OpenParen))
  {
    lexer.Consume();

    set<string> paramSet;
    while(true)
    {
      if(!TOKEN(Identifier))
        EXPECTED(Identifier)
      auto param = lexer().strValue;
      lexer.Consume();

      if(!paramSet.insert(param).second)
        ERROR("duplicate macro parameter: " << param);
      macro.params.push_back(param);

      if(!TOKEN(Operator) || lexer().strValue != ",")
        break;
      lexer.Consume();
    }

    if(!TOKEN(CloseParen))
      EXPECTED(CloseParen)
    lexer.Consume();
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
  lexer.Consume();

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

    if(TOKEN(Operator) && lexer().strValue == "$" &&
       lexer(1).type == Token::Identifier &&
       macros.Find(lexer(1).strValue) == macros.End() &&
       lexer(1).strValue != "file" &&
       lexer(1).strValue != "line" &&
       lexer(1).strValue != "shell") // TODO maybe not hardcode this?
    {
      token = lexer(1);
      lexer.Consume(2);

      auto& mangledIden = localVars[token.strValue];
      if(mangledIden.empty())
        mangledIden = MakeIdentifier();

      token.strValue = mangledIden;
    }
    else
    {
      token = lexer();
      lexer.Consume();
    }

    macro.body.push_back(move(token));
  }

  if(blockMacro)
    lexer.Consume();

  macros.AddValue(name, move(macro));
  return new EList;
}

ExprPtr ExprParser::MacroCall() // prefix: $
{
  macroCallLoc = lexer().loc;

  if(!TOKEN(Identifier))
    EXPECTED(Identifier)
  auto name = lexer().strValue;
  lexer.Consume();

  auto macro = macros.Find(name);
  if(macro != macros.End())
  {
    if(activeMacros.find(name) != activeMacros.end())
      ERROR("recursive call of macro " << name);

    activeMacros.insert(name);
    auto expr = UserMacroCall(macro->second);
    activeMacros.erase(name);

    return expr;
  }

  if(name == "file")
    return FileMacro();
  if(name == "line")
    return LineMacro();
  if(name == "shell")
    return ShellMacro();

  ERROR("undefined macro " << name)
}

ExprPtr ExprParser::UserMacroCall(const MacroDef& macro)
{
 map<string, vector<Token> > args;

  if(!macro.params.empty())
  {
    if(!TOKEN(OpenParen))
      EXPECTED(OpenParen)
    lexer.Consume();

    for(size_t i = 0; i < macro.params.size(); i++)
    {
      vector<Token> arg;

      while(true)
      {
        if((i + 1) < macro.params.size() && TOKEN(Operator) && lexer().strValue == ",")
          break;
        if((i + 1) == macro.params.size() && TOKEN(CloseParen))
          break;
        if(TOKEN(EndOfFile))
          ERROR("unterminated macro arguments")
        arg.push_back(lexer());
        lexer.Consume();
      }
      lexer.Consume();

      args[macro.params[i]] = move(arg);
    }
  }

  for(auto token = macro.body.rbegin(); token != macro.body.rend(); ++token)
  {
    if(token->type == Token::Identifier)
    {
      auto arg = args.find(token->strValue);
      if(arg != args.end())
      {
        for(auto argToken = arg->second.rbegin(); argToken != arg->second.rend(); ++argToken)
          lexer.Unget(*argToken);
      }
      else {
        auto tokenCopy = *token;
        tokenCopy.loc = macroCallLoc;
        lexer.Unget(tokenCopy);
      }
    }
    else {
      auto tokenCopy = *token;
      tokenCopy.loc = macroCallLoc;
      lexer.Unget(tokenCopy);
    }
  }

  return Expr();
}

ExprPtr ExprParser::Expr(bool required, int precedence)
{
  ExprPtr expr = Expr_P(required);
  if(!expr)
    return ExprPtr();

  string lastOp;

  while(!TOKEN(EndOfFile))
  {
    string op = "#call";

    if(TOKEN(Backtick) && lexer(1).type == Token::Identifier) {
      op = "#infix";
    }
    else if(TOKEN(If)) {
      op = "#if";
    }
    else if(TOKEN(While)) {
      op = "#while";
    }
    else if(TOKEN(Operator)) {
      op = lexer().strValue;
    }

    auto binaryOp = binaryOperators.find(op);
    if(binaryOp == binaryOperators.end())
      binaryOp = binaryOperators.find("#custom");

    int prec = binaryOp->second.first;
    bool rightAssoc = binaryOp->second.second;
    if(prec < precedence)
      break;
    
    if(op != "#call")
      lexer.Consume();

    if(op == "#infix") {
      op = lexer().strValue;
      lexer.Consume();
    }

    int q = rightAssoc ? prec : (1 + prec);

    auto exprRight = Expr(false, q);
    if(!exprRight) {
      if(op == "#call")
        break;
      EXPECTED(Expr)
    }

    if(op == "#call") {
      if(!isa<EList>(exprRight.get())) {
        auto list = make_unique<EList>();
        list->exprs.push_back(move(exprRight));
        exprRight = list.release();
      }
      expr = new ECall(expr, exprRight);
    }
    else if(op == "," && lastOp == ",") {
      auto list = static_cast<EList*>(expr.get());
      list->exprs.push_back(move(exprRight));
    }
    else {
      if(op == "#if" || op == "#while")
        expr.swap(exprRight);

      auto list = make_unique<EList>();
      list->exprs.push_back(move(expr));
      list->exprs.push_back(move(exprRight));

      if(op == ",")
        expr = list.release();
      else
        expr = new ECall(new EVariable(op), list.release());
    }

    lastOp = move(op);
  }

  return expr;
}

ExprPtr ExprParser::Expr_P(bool required)
{
  ExprPtr expr;

  if(TOKEN(Integer)) {
    expr = new EInteger(lexer().intValue);
    lexer.Consume();
  }
  else if(TOKEN(Float)) {
    expr = new EFloat(lexer().floatValue);
    lexer.Consume();
  }
  else if(TOKEN(String)) {
    expr = new EString(lexer().strValue);
    lexer.Consume();
  }
  else if(TOKEN(True)) {
    expr = new EBoolean(true);
    lexer.Consume();
  }
  else if(TOKEN(False)) {
    expr = new EBoolean(false);
    lexer.Consume();
  }
  else if(TOKEN(Module)) {
    lexer.Consume();
    expr = Module();
  }
  else if(TOKEN(Using)) {
    lexer.Consume();
    expr = Using();
  }
  else if(TOKEN(Fn)) {
    lexer.Consume();
    expr = Fn();
  }
  else if(TOKEN(If)) {
    lexer.Consume();
    expr = If();
  }
  else if(TOKEN(While)) {
    lexer.Consume();
    expr = While();
  }
  else if(TOKEN(Break)) {
    lexer.Consume();
    expr = Break();
  }
  else if(TOKEN(Return)) {
    lexer.Consume();
    expr = Return();
  }
  else if(TOKEN(TypeAlias)) {
    lexer.Consume();
    expr = TypeAlias();
  }
  else if(TOKEN(Extern)) {
    lexer.Consume();
    expr = Extern();
  }
  else if(TOKEN(Macro)) {
    lexer.Consume();
    expr = Macro();
  }
  else if(TOKEN(OpenParen))
  {
    lexer.Consume();

    expr = Expr(false, 0);
    if(!expr)
      expr = new EList;

    if(!TOKEN(CloseParen))
      EXPECTED(CloseParen)
    lexer.Consume();
  }
  else if(TOKEN(Backtick))
  {
    lexer.Consume();

    if(!TOKEN(Operator))
      EXPECTED(Operator)
    expr = new EVariable(lexer().strValue);
    lexer.Consume();
  }
  else if(TOKEN(Identifier))
  {
    expr = new EVariable(lexer().strValue);
    lexer.Consume();
  }
  else if(TOKEN(Operator))
  {
    if(lexer().strValue == "$")
    {
      lexer.Consume();
      expr = MacroCall();
    }
    else
    {
      auto unaryOp = unaryOperators.find(lexer().strValue);
      lexer.Consume();

      if(unaryOp == unaryOperators.end())
        ERROR("unknown unary operator: " << lexer().strValue)

      expr = Expr(true, unaryOp->second);
      expr = new ECall(new EVariable(unaryOp->first), expr);
    }
  }

  if(!expr)
  {
    if(required) {
      Error() << "unexpected token " << lexer() << " parsing expression";
      lexer.Consume();
    }
    return expr;
  }

  if(TOKEN(Slash))
  {
    lexer.Consume();
    expr->type = ParseType(lexer);
  }

  return expr;
}

ExprPtr ExprParser::TopLevel()
{
  // lexer may generate a leading nodent, eat it
  if(TOKEN(Nodent))
    lexer.Consume();

  auto list = make_unique<EList>();

  while(true) {
    list->exprs.push_back(Expr());

    if(!TOKEN(Nodent))
      break;
    lexer.Consume();
  }

  if(!TOKEN(EndOfFile))
    EXPECTED(EndOfFile)

  list->exprs.push_back(new EList);

  return new EFunction(
    new TList,
    new ECall(
      new EVariable(";"),
      list.release()));
}

} // namespace xra
