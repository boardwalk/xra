#include "common.hpp"
#include "expr.hpp"
#include "buffered-lexer.hpp"
#include <sstream>

#define TOKEN(t) (lexer.Get().type == Token::t)
#define ERROR(what) \
  { \
    stringstream ss; \
    ss << what << " near " << lexer.Get() << " at parser.cpp:" << __LINE__;  \
    return ExprPtr(new EError(ss.str())); \
  }
#define EXPECTED(t) \
  ERROR("expected " #t)

namespace xra {

static const map<string, pair<int, bool> > binaryOperators {
  {"\\", {19, false}},
  {".", {18, false}},
  {"$", {16, false}},
  {"*", {15, false}},
  {"/", {15, false}},
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
  {"+", 17},
  {"-", 17},
  {"!", 17},
  {"~", 17},
  {"*", 17},
  {"&", 17}
};

static ExprPtr ParseExpr(BufferedLexer&);

static ExprPtr ParseFlatBlock(BufferedLexer& lexer)
{
  auto list = make_unique<EList>();

  while(true) {
    list->exprs.push_back(ParseExpr(lexer));

    if(!TOKEN(Nodent))
      break;
    lexer.Consume();
  }

  return ExprPtr(new ECall(
    ExprPtr(new EVariable(";")),
    ExprPtr(list.release())));
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
  string name = lexer.Get().strValue;
  lexer.Consume();

  TypePtr type = Type::Parse(lexer);
  if(!type)
    EXPECTED(Type)

  return ExprPtr(new EExtern(name, type));
}

static ExprPtr ParseIfClause(BufferedLexer& lexer, bool needThen)
{
  if(TOKEN(Indent)) {
    lexer.Consume();
    return ParseBlock(lexer);
  }

  if(needThen) {
    if(!TOKEN(Then))
      EXPECTED(Then)
    lexer.Consume();
  }

  return ParseExpr(lexer);
}

static ExprPtr ParseIf(BufferedLexer& lexer) // prefix: if
{
  auto list = make_unique<EList>();

  bool more = true;
  while(more)
  {
    list->exprs.push_back(ParseExpr(lexer));
    list->exprs.push_back(ParseIfClause(lexer, true));

    if(TOKEN(Elsif))
      lexer.Consume();
    else if(TOKEN(Nodent) && lexer.Get(1).type == Token::Elsif)
      lexer.Consume(2);
    else
      more = false;
  }

  more = true;
  if(TOKEN(Else))
    lexer.Consume();
  else if(TOKEN(Nodent) && lexer.Get(1).type == Token::Else)
    lexer.Consume(2);
  else
    more = false;

  if(more)
    list->exprs.push_back(ParseIfClause(lexer, false));

  return ExprPtr(new ECall(ExprPtr(new EVariable("#if")), ExprPtr(list.release())));
}

static ExprPtr ParseReturn(BufferedLexer& lexer) // prefix: return
{
  return ExprPtr(new EReturn(ParseExpr(lexer)));
}

static ExprPtr ParseFn(BufferedLexer& lexer) // prefix: fn
{
  ExprPtr param = ParseExpr(lexer);

  if(!TOKEN(Operator) || lexer.Get().strValue != "->")
    EXPECTED(Operator)
  lexer.Consume();

  ExprPtr body;
  if(TOKEN(Indent)) {
    lexer.Consume();
    body = ParseBlock(lexer);
  }
  else {
    body = ParseExpr(lexer);
  }

  return ExprPtr(new EFunction(move(param), move(body)));
}

static ExprPtr ParseExpr_P(BufferedLexer& lexer, bool required);

ExprPtr ParseExpr_Exp(BufferedLexer& lexer, int p, bool required)
{
  ExprPtr expr = ParseExpr_P(lexer, required);
  if(!expr)
    return {};

  string lastOp;

  while(!TOKEN(EndOfFile))
  {
    string op = TOKEN(Operator) ? lexer.Get().strValue : "$";

    auto binaryOp = binaryOperators.find(op);
    if(binaryOp == binaryOperators.end())
      break;
    int prec = binaryOp->second.first;
    bool rightAssoc = binaryOp->second.second;
    if(prec < p)
      break;
    if(op != "$")
      lexer.Consume();

    int q = rightAssoc ? prec : (1 + prec);

    auto exprRight = ParseExpr_Exp(lexer, q, false);
    if(!exprRight) {
      if(op == "$")
        break;
      EXPECTED(Expr)
    }

    if(op == "$") {
      expr.reset(new ECall(
        move(expr),
        move(exprRight)));
    }
    else if(op == "," && lastOp == ",") {
      auto list = static_cast<EList*>(expr.get());
      list->exprs.push_back(move(exprRight));
    }
    else {
      auto list = make_unique<EList>();
      list->exprs.push_back(move(expr));
      list->exprs.push_back(move(exprRight));

      if(op == ",") {
        expr.reset(list.release());
      }
      else {
        expr.reset(new ECall(ExprPtr(new EVariable(op)), ExprPtr(list.release())));
      }
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
    auto unaryOp = unaryOperators.find(lexer.Get().strValue);
    lexer.Consume();

    if(unaryOp == unaryOperators.end())
      ERROR("unknown unary operator")

    expr = ParseExpr_Exp(lexer, unaryOp->second, true);

    expr.reset(new ECall(
      ExprPtr(new EVariable(unaryOp->first)),
      move(expr)));
  }
  else if(TOKEN(OpenParen))
  {
    lexer.Consume();

    expr = ParseExpr_Exp(lexer, 0, false);
    if(!expr)
      expr.reset(new EVoid);

    if(!TOKEN(CloseParen))
      EXPECTED(CloseParen)
    lexer.Consume();
  }
  else if(TOKEN(Backtick))
  {
    lexer.Consume();

    if(!TOKEN(Operator))
      EXPECTED(Operator)
    expr.reset(new EVariable(lexer.Get().strValue));
    lexer.Consume();

    if(!TOKEN(Backtick))
      EXPECTED(Backtick)
    lexer.Consume();
  }
  else if(TOKEN(Identifier)) {
    expr.reset(new EVariable(lexer.Get().strValue));
    lexer.Consume();
  }
  else if(TOKEN(True)) {
    expr.reset(new EBoolean(true));
    lexer.Consume();
  }
  else if(TOKEN(False)) {
    expr.reset(new EBoolean(false));
    lexer.Consume();
  }
  else if(TOKEN(Integer)) {
    expr.reset(new EInteger(lexer.Get().intValue));
    lexer.Consume();
  }
  else if(TOKEN(Float)) {
    expr.reset(new EFloat(lexer.Get().floatValue));
    lexer.Consume();
  }
  else if(TOKEN(String)) {
    expr.reset(new EString(lexer.Get().strValue));
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
  else if(TOKEN(Return)) {
    lexer.Consume();
    expr = ParseReturn(lexer);
  }
  else if(TOKEN(Fn)) {
    lexer.Consume();
    expr = ParseFn(lexer);
  }

  if(!expr) {
    if(required) {
      stringstream ss;
      ss << "unexpected token " << lexer.Get();
      expr.reset(new EError(ss.str()));
      lexer.Consume();
    }
    return expr;
  }

  if(TOKEN(Colon)) {
    lexer.Consume();
    expr->type = Type::Parse(lexer);
  }

  return expr;
}

static ExprPtr ParseExpr(BufferedLexer& lexer)
{
  return ParseExpr_Exp(lexer, 0, true);
}

ExprPtr Expr::Parse(BufferedLexer& lexer)
{
  ExprPtr expr = ParseFlatBlock(lexer);
  if(!TOKEN(EndOfFile)) // TODO this is masking better errors!
    EXPECTED(EndOfFile)
  return expr;
}

} // namespace xra
