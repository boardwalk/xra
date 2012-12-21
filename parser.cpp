#include "common.hpp"
#include "parser.hpp"
#include "lexer.hpp"
#include <iostream>
#include <sstream>

#define TOKEN(t) (lexer.Get().type == Token::t)
#define ERROR(what) \
  { \
    stringstream ss; \
    ss << what << " near " << lexer.Get(); \
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

TypePtr ParseType(BufferedLexer& lexer)
{
  if(!TOKEN(Colon))
    return {};
  lexer.Next();

  if(!TOKEN(Identifier)) {
    cerr << "expected identifier after colon" << endl;
    return {};
  }
  auto type = make_unique<TVariable>(lexer.Get().strValue);
  lexer.Next();

  return TypePtr(type.release());
}

ExprPtr ParseExpr(BufferedLexer&);

ExprPtr ParseBlock(BufferedLexer& lexer)
{
  if(!TOKEN(Indent))
    return {};
  lexer.Next();

  auto list = make_unique<EList>();

  while(true) {
    ExprPtr expr = ParseExpr(lexer);
    if(!expr)
      EXPECTED(Expr);
    list->exprs.push_back(move(expr));

    if(!TOKEN(Nodent))
      break;
    lexer.Next();
  }

  if(!TOKEN(Dedent))
    EXPECTED(Dedent)
  lexer.Next();

  return ExprPtr(new ECall(
    ExprPtr(new EVariable(";")),
    ExprPtr(list.release())));
}

ExprPtr ParseExtern(BufferedLexer& lexer)
{
  if(!TOKEN(Extern))
    return {};
  lexer.Next();

  if(!TOKEN(Identifier))
    EXPECTED(Identifier)
  string name = lexer.Get().strValue;
  lexer.Next();

  TypePtr type = ParseType(lexer);
  if(!type)
    EXPECTED(Type)

  return ExprPtr(new EExtern(name, type));
}

ExprPtr ParseIf(BufferedLexer& lexer)
{
  if(!TOKEN(If))
    return {};
  lexer.Next();

  ExprPtr cond = ParseExpr(lexer);
  if(!cond)
    EXPECTED(Expr)

  ExprPtr then;
  if(TOKEN(Then)) {
    lexer.Next();
    then = ParseExpr(lexer);
    if(!then)
      EXPECTED(Expr)
  }
  else {
    then = ParseBlock(lexer);
    if(!then)
      EXPECTED(Block)
  }

  ExprPtr _else;
  if(TOKEN(Else)) {
    lexer.Next();
    if(!TOKEN(Indent)) {
      _else = ParseExpr(lexer);
      if(!_else)
        EXPECTED(Expr)
    }
    else {
      _else = ParseBlock(lexer);
      if(!_else)
        EXPECTED(Block)
    }
  }

  return ExprPtr(new EIf(move(cond), move(then), move(_else)));
}

ExprPtr ParseReturn(BufferedLexer& lexer)
{
  if(!TOKEN(Return))
    return {};
  lexer.Next();

  ExprPtr expr = ParseExpr(lexer);
  if(!expr)
    EXPECTED(Expr)

  return ExprPtr(new EReturn(move(expr)));
}

ExprPtr ParseFn(BufferedLexer& lexer)
{
  if(!TOKEN(Fn))
    return {};
  lexer.Next();

  ExprPtr param = ParseExpr(lexer);
  if(!param)
    EXPECTED(Expr)

  if(!TOKEN(Operator) || lexer.Get().strValue != "->")
    EXPECTED(Operator)
  lexer.Next();

  ExprPtr body;
  if(TOKEN(Indent)) {
    body = ParseBlock(lexer);
    if(!body)
      EXPECTED(Block)
  }
  else {
    body = ParseExpr(lexer);
    if(!body)
      EXPECTED(Expr)
  }

  return ExprPtr(new EFunction(move(param), move(body)));
}

ExprPtr ParseExpr_P(BufferedLexer& lexer);

ExprPtr ParseExpr_Exp(BufferedLexer& lexer, int p)
{
  ExprPtr expr = ParseExpr_P(lexer);
  if(!expr)
    return {};

  string lastOp;

  while(true)
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
      lexer.Next();

    int q = rightAssoc ? prec : (1 + prec);

    auto exprRight = ParseExpr_Exp(lexer, q);
    if(!exprRight) {
      if(op != "$")
        EXPECTED(Expr)
      break;
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
        expr.reset(new ECall(
          ExprPtr(new EVariable(op)),
          ExprPtr(list.release())));
      }
    }

    lastOp = move(op);
  }

  return expr;
}

ExprPtr ParseExpr_P(BufferedLexer& lexer)
{
  ExprPtr expr;

  if(TOKEN(Operator))
  {
    auto unaryOp = unaryOperators.find(lexer.Get().strValue);
    lexer.Next();

    if(unaryOp == unaryOperators.end())
      ERROR("unknown unary operator")

    expr = ParseExpr_Exp(lexer, unaryOp->second);
    if(!expr)
      EXPECTED(Expr)

    expr.reset(new ECall(
      ExprPtr(new EVariable(unaryOp->first)),
      move(expr)));
  }
  else if(TOKEN(OpenParen))
  {
    lexer.Next();

    expr = ParseExpr_Exp(lexer, 0);

    if(!TOKEN(CloseParen))
      EXPECTED(CloseParen)
    lexer.Next();

    if(!expr)
      expr.reset(new EVoid);
  }
  else if(TOKEN(Backtick))
  {
    lexer.Next();

    if(!TOKEN(Operator))
      EXPECTED(Operator);
    expr.reset(new EVariable(lexer.Get().strValue));
    lexer.Next();

    if(!TOKEN(Backtick))
      EXPECTED(Backtick)
    lexer.Next();
  }
  else if(TOKEN(Identifier)) {
    expr.reset(new EVariable(lexer.Get().strValue));
    lexer.Next();
  }
  else if(TOKEN(True)) {
    expr.reset(new EBoolean(true));
    lexer.Next();
  }
  else if(TOKEN(False)) {
    expr.reset(new EBoolean(false));
    lexer.Next();
  }
  else if(TOKEN(Integer)) {
    expr.reset(new EInteger(lexer.Get().intValue));
    lexer.Next();
  }
  else if(TOKEN(Float)) {
    expr.reset(new EFloat(lexer.Get().floatValue));
    lexer.Next();
  }
  else if(TOKEN(String)) {
    expr.reset(new EString(lexer.Get().strValue));
    lexer.Next();
  }
  else if(TOKEN(Extern)) {
    expr = ParseExtern(lexer);
  }
  else if(TOKEN(If)) {
    expr = ParseIf(lexer);
  }
  else if(TOKEN(Return)) {
    expr = ParseReturn(lexer);
  }
  else if(TOKEN(Fn)) {
    expr = ParseFn(lexer);
  }

  return expr;
}

ExprPtr ParseExpr(BufferedLexer& lexer)
{
  return ParseExpr_Exp(lexer, 0);
}

ExprPtr Parse(BufferedLexer& lexer)
{
  lexer.Next();
  ExprPtr expr = ParseExpr(lexer);
  if(!expr)
    EXPECTED(Expr)
  if(!TOKEN(EndOfFile)) // TODO this is masking better errors!
    EXPECTED(EndOfFile)
  return expr;
}

} // namespace xra
