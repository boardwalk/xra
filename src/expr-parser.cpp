#include "common.hpp"
#include "expr.hpp"
#include "type.hpp"
#include "buffered-lexer.hpp"

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
  {"#infix", {18, false}}, // a call to a regular function using backticks (x `f` y)
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
  {"=", {2, true}},
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

static ExprPtr ParseExpr(BufferedLexer&); // true, 0
static ExprPtr ParseExpr_Exp(BufferedLexer&, bool, int);

static ExprPtr ParseFlatBlock(BufferedLexer& lexer)
{
  auto list = make_unique<EList>();

  while(true) {
    list->exprs.push_back(ParseExpr(lexer));

    if(!TOKEN(Nodent))
      break;
    lexer.Consume();
  }

  return new ECall(new EVariable(";"), list.release());
}

static ExprPtr ParseBlock(BufferedLexer& lexer) // prefix: indent
{
  ExprPtr expr = ParseFlatBlock(lexer);

  if(!TOKEN(Dedent))
    EXPECTED(Dedent)
  lexer.Consume();

  return expr;
}

static ExprPtr ParseExtern(BufferedLexer& lexer) // prefix: extern
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

static ExprPtr ParseClause(BufferedLexer& lexer)
{
  if(TOKEN(Indent)) {
    lexer.Consume();
    return ParseBlock(lexer);
  }

  if(!TOKEN(Colon))
    EXPECTED(Colon)
  lexer.Consume();

  return ParseExpr(lexer);
}

static ExprPtr ParseIf(BufferedLexer& lexer) // prefix: if
{
  auto list = make_unique<EList>();

  bool more = true;
  while(more)
  {
    list->exprs.push_back(ParseExpr(lexer));
    list->exprs.push_back(ParseClause(lexer));

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
    list->exprs.push_back(ParseClause(lexer));
  }

  return new ECall(new EVariable("#if"), list.release());
}

static ExprPtr ParseWhile(BufferedLexer& lexer) // prefix: while
{
  auto list = make_unique<EList>();
  list->exprs.push_back(ParseExpr(lexer));
  list->exprs.push_back(ParseClause(lexer));

  return new ECall(new EVariable("#while"), list.release());
}

static ExprPtr ParseReturn(BufferedLexer& lexer) // prefix: return
{
  auto list = make_unique<EList>();
  ExprPtr expr = ParseExpr_Exp(lexer, false, 0);
  if(expr)
    list->exprs.push_back(expr);
  return new ECall(new EVariable("#return"), list.release());
}

static ExprPtr ParseBreak(BufferedLexer&) // prefix: break
{
  return new ECall(new EVariable("#break"), new EList);
}

static ExprPtr ParseFn(BufferedLexer& lexer) // prefix: fn
{
  return new EFunction(ParseExpr(lexer), ParseClause(lexer));
}

static ExprPtr ParseExpr_P(BufferedLexer& lexer, bool required);

static ExprPtr ParseExpr_Exp(BufferedLexer& lexer, bool required, int p)
{
  ExprPtr expr = ParseExpr_P(lexer, required);
  if(!expr)
    return ExprPtr();

  string lastOp;

  while(!TOKEN(EndOfFile))
  {
    string op = "#call";

    if(TOKEN(Backtick) && lexer(1).type == Token::Identifier) {
      op = "#infix";
    }
    else if(TOKEN(Operator)) {
      op = lexer().strValue;
    }

    auto binaryOp = binaryOperators.find(op);
    if(binaryOp == binaryOperators.end())
      binaryOp = binaryOperators.find("#custom");

    int prec = binaryOp->second.first;
    bool rightAssoc = binaryOp->second.second;
    if(prec < p)
      break;
    
    if(op != "#call")
      lexer.Consume();

    if(op == "#infix") {
      op = lexer().strValue;
      lexer.Consume();
      if(!TOKEN(Backtick))
        EXPECTED(Backtick);
      lexer.Consume();
    }

    int q = rightAssoc ? prec : (1 + prec);

    auto exprRight = ParseExpr_Exp(lexer, false, q);
    if(!exprRight) {
      if(op == "#call")
        break;
      EXPECTED(Expr)
    }

    if(op == "#call") {
      expr = new ECall(expr, exprRight);
    }
    else if(op == "," && lastOp == ",") {
      auto list = static_cast<EList*>(expr.get());
      list->exprs.push_back(move(exprRight));
    }
    else {
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

static ExprPtr ParseExpr_P(BufferedLexer& lexer, bool required)
{
  ExprPtr expr;

  if(TOKEN(Operator))
  {
    auto unaryOp = unaryOperators.find(lexer().strValue);
    lexer.Consume();

    if(unaryOp == unaryOperators.end())
      ERROR("unknown unary operator")

    expr = ParseExpr_Exp(lexer, true, unaryOp->second);
    expr = new ECall(new EVariable(unaryOp->first), expr);
  }
  else if(TOKEN(OpenParen))
  {
    lexer.Consume();

    expr = ParseExpr_Exp(lexer, false, 0);
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

    if(!TOKEN(Backtick))
      EXPECTED(Backtick)
    lexer.Consume();
  }
  else if(TOKEN(Identifier)) {
    expr = new EVariable(lexer().strValue);
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
  else if(TOKEN(Integer)) {
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
  else if(TOKEN(Extern)) {
    lexer.Consume();
    expr = ParseExtern(lexer);
  }
  else if(TOKEN(If)) {
    lexer.Consume();
    expr = ParseIf(lexer);
  }
  else if(TOKEN(While)) {
    lexer.Consume();
    expr = ParseWhile(lexer);
  }
  else if(TOKEN(Return)) {
    lexer.Consume();
    expr = ParseReturn(lexer);
  }
  else if(TOKEN(Break)) {
    lexer.Consume();
    expr = ParseBreak(lexer);
  }
  else if(TOKEN(Fn)) {
    lexer.Consume();
    expr = ParseFn(lexer);
  }

  if(!expr) {
    if(required) {
      Error() << "unexpected token " << lexer() << " parsing expression";
      lexer.Consume();
    }
    return expr;
  }

  if(TOKEN(Slash)) {
    lexer.Consume();
    expr->type = ParseType(lexer);
  }

  return expr;
}

static ExprPtr ParseExpr(BufferedLexer& lexer)
{
  return ParseExpr_Exp(lexer, true, 0);
}

ExprPtr ParseTopLevel(BufferedLexer& lexer)
{
  // lexer may generate a leading nodent, eat it
  if(TOKEN(Nodent))
    lexer.Consume();

  auto list = make_unique<EList>();

  while(true) {
    list->exprs.push_back(ParseExpr(lexer));

    if(!TOKEN(Nodent))
      break;
    lexer.Consume();
  }

  if(!TOKEN(EndOfFile))
    EXPECTED(EndOfFile)

  list->exprs.push_back(new EList);

  return new EFunction(
    new EList,
    new ECall(
      new EVariable(";"),
      list.release()));
}

} // namespace xra
