#include "common.hpp"
#include "expr-parser.hpp"
#include "expr.hpp"
#include "type.hpp"

#define TOKEN(t) (tokens().type == Token::t)
#define ERROR(what) \
  { \
    Error() << what << " near " << tokens() << " at expr-parser.cpp:" << __LINE__; \
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

ExprPtr ExprParser::FlatBlock()
{
  auto list = make_unique<EList>();

  while(true) {
    list->exprs.push_back(Expr());

    if(!TOKEN(Nodent))
      break;
    tokens.Consume();
  }

  return new ECall(new EVariable(";"), list.release());
}

ExprPtr ExprParser::Block() // prefix: indent
{
  ExprPtr expr = FlatBlock();

  if(!TOKEN(Dedent))
    EXPECTED(Dedent)
  tokens.Consume();

  return expr;
}

ExprPtr ExprParser::Clause()
{
  if(TOKEN(Indent)) {
    tokens.Consume();
    return Block();
  }

  if(!TOKEN(Colon))
    EXPECTED(Colon)
  tokens.Consume();

  return Expr();
}

ExprPtr ExprParser::Name()
{
  string name;

  if(TOKEN(Operator) && tokens().strValue == ".") {
    name += '.';
    tokens.Consume();
  }

  while(true) {
    if(!TOKEN(Identifier))
      EXPECTED(Identifier)
    name += tokens().strValue;
    tokens.Consume();

    if(!TOKEN(Operator) || tokens().strValue != ".")
      break;
    name += '.';
    tokens.Consume();
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
  tokens.Consume();

  list->exprs.push_back(FlatBlock());

  if(indented) {
    if(!TOKEN(Dedent))
      EXPECTED(Dedent)
    tokens.Consume();
  }

  return new ECall(new EVariable("#module"), list.release());
}

ExprPtr ExprParser::Using() // prefix: using
{
  auto list = make_unique<EList>();
  list->exprs.push_back(Name());
  if(!TOKEN(Nodent))
    EXPECTED(Nodent)
  tokens.Consume();
  list->exprs.push_back(FlatBlock());
  return new ECall(new EVariable("#using"), list.release());
}

ExprPtr ExprParser::Fn() // prefix: fn
{
  return new EFunction(ParseTypeList(tokens), Clause());
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
      tokens.Consume();
    else if(TOKEN(Nodent) && tokens(1).type == Token::Elsif)
      tokens.Consume(2);
    else
      more = false;
  }

  more = true;
  if(TOKEN(Else))
    tokens.Consume();
  else if(TOKEN(Nodent) && tokens(1).type == Token::Else)
    tokens.Consume(2);
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
  auto name = tokens().strValue;
  tokens.Consume();

  if(!TOKEN(Operator) || tokens().strValue != "=")
    EXPECTED(Equals)
  tokens.Consume();

  auto type = ParseType(tokens);
  if(!type)
    EXPECTED(Type)

  return new ETypeAlias(name, type);
}

ExprPtr ExprParser::Extern() // prefix: extern
{
  if(!TOKEN(Identifier))
    EXPECTED(Identifier)
  string name = tokens().strValue;
  tokens.Consume();

  TypePtr type = ParseType(tokens);
  if(!type)
    EXPECTED(Type)

  return new EExtern(name, type);
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

    if(TOKEN(Backtick) && tokens(1).type == Token::Identifier) {
      op = "#infix";
    }
    else if(TOKEN(If)) {
      op = "#if";
    }
    else if(TOKEN(While)) {
      op = "#while";
    }
    else if(TOKEN(Operator)) {
      op = tokens().strValue;
    }

    auto binaryOp = binaryOperators.find(op);
    if(binaryOp == binaryOperators.end())
      binaryOp = binaryOperators.find("#custom");

    int prec = binaryOp->second.first;
    bool rightAssoc = binaryOp->second.second;
    if(prec < precedence)
      break;
    
    if(op != "#call")
      tokens.Consume();

    if(op == "#infix") {
      op = tokens().strValue;
      tokens.Consume();
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
    expr = new EInteger(tokens().intValue);
    tokens.Consume();
  }
  else if(TOKEN(Float)) {
    expr = new EFloat(tokens().floatValue);
    tokens.Consume();
  }
  else if(TOKEN(String)) {
    expr = new EString(tokens().strValue);
    tokens.Consume();
  }
  else if(TOKEN(True)) {
    expr = new EBoolean(true);
    tokens.Consume();
  }
  else if(TOKEN(False)) {
    expr = new EBoolean(false);
    tokens.Consume();
  }
  else if(TOKEN(Module)) {
    tokens.Consume();
    expr = Module();
  }
  else if(TOKEN(Using)) {
    tokens.Consume();
    expr = Using();
  }
  else if(TOKEN(Fn)) {
    tokens.Consume();
    expr = Fn();
  }
  else if(TOKEN(If)) {
    tokens.Consume();
    expr = If();
  }
  else if(TOKEN(While)) {
    tokens.Consume();
    expr = While();
  }
  else if(TOKEN(Break)) {
    tokens.Consume();
    expr = Break();
  }
  else if(TOKEN(Return)) {
    tokens.Consume();
    expr = Return();
  }
  else if(TOKEN(TypeAlias)) {
    tokens.Consume();
    expr = TypeAlias();
  }
  else if(TOKEN(Extern)) {
    tokens.Consume();
    expr = Extern();
  }
  else if(TOKEN(OpenParen))
  {
    tokens.Consume();

    expr = Expr(false, 0);
    if(!expr)
      expr = new EList;

    if(!TOKEN(CloseParen))
      EXPECTED(CloseParen)
    tokens.Consume();
  }
  else if(TOKEN(Backtick))
  {
    tokens.Consume();

    if(!TOKEN(Operator))
      EXPECTED(Operator)
    expr = new EVariable(tokens().strValue);
    tokens.Consume();
  }
  else if(TOKEN(Identifier))
  {
    expr = new EVariable(tokens().strValue);
    tokens.Consume();
  }
  else if(TOKEN(Operator))
  {
    auto unaryOp = unaryOperators.find(tokens().strValue);
    tokens.Consume();

    if(unaryOp == unaryOperators.end())
      ERROR("unknown unary operator: " << tokens().strValue)

    expr = Expr(true, unaryOp->second);
    expr = new ECall(new EVariable(unaryOp->first), expr);
  }

  if(!expr)
  {
    if(required) {
      Error() << "unexpected token " << tokens() << " parsing expression";
      tokens.Consume();
    }
    return expr;
  }

  if(TOKEN(Slash))
  {
    tokens.Consume();
    expr->type = ParseType(tokens);
  }

  return expr;
}

ExprPtr ExprParser::TopLevel()
{
  // tokens may generate a leading nodent, eat it
  if(TOKEN(Nodent))
    tokens.Consume();

  auto list = make_unique<EList>();

  while(true) {
    list->exprs.push_back(Expr());

    if(!TOKEN(Nodent))
      break;
    tokens.Consume();
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
