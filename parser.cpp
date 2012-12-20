#include "common.hpp"
#include "parser.hpp"
#include "lexer.hpp"
#include <iostream>

#define TOKEN(t) (lexer.Get().type == Token::t)
#define ERROR(t) \
  { \
    cerr << "expected " #t << " near " << lexer.Get() << endl; \
    failure = true; \
    return {}; \
  }

namespace xra {

static const map<string, pair<int, bool> > binaryOperators {
  {"||", {0, false}},
  {"&&", {1, false}},
  {"=", {2, false}},
  {"+", {3, false}},
  {"-", {3, false}},
  {"*", {5, false}},
  {"/", {5, false}},
  {"^", {6, true}}
};

static const map<string, int> unaryOperators {
  {"-", 4}
};

static bool failure = false; // TODO not reentrant

TypePtr ParseType(BufferedLexer& lexer)
{
  if(!TOKEN(Colon))
    return {};
  lexer.Next();

  if(!TOKEN(Identifier))
    ERROR(Identifier)
  auto type = make_unique<TVariable>(lexer.Get().strValue);
  lexer.Next();

  return TypePtr(type.release());
}

ExprPtr ParseExpr(BufferedLexer&);

ExprPtr ParseBlock(BufferedLexer& lexer)
{
  auto block = make_unique<EBlock>();

  if(!TOKEN(Indent))
    return {};
  lexer.Next();

  while(true) {
    ExprPtr expr = ParseExpr(lexer);
    if(!expr)
      ERROR(Expr)

    block->Push(move(expr));

    if(!TOKEN(Nodent))
      break;
    lexer.Next();
  }

  if(!TOKEN(Dedent))
    ERROR(Dedent)
  lexer.Next();

  return ExprPtr(block.release());
}

ExprPtr ParseExtern(BufferedLexer& lexer)
{
  if(!TOKEN(Extern))
    return {};
  lexer.Next();

  if(!TOKEN(Identifier))
    ERROR(Identifier)
  string name = lexer.Get().strValue;
  lexer.Next();

  TypePtr type = ParseType(lexer);
  if(!type)
    ERROR(Type)

  return ExprPtr(new EExtern(name, type));
}

ExprPtr ParseIf(BufferedLexer& lexer)
{
  if(!TOKEN(If))
    return {};
  lexer.Next();

  ExprPtr cond = ParseExpr(lexer);
  if(!cond)
    ERROR(Expr)

  ExprPtr then;
  if(TOKEN(Then)) {
    lexer.Next();
    then = ParseExpr(lexer);
    if(!then)
      ERROR(Expr)
  }
  else {
    then = ParseBlock(lexer);
    if(!then)
      ERROR(Block)
  }

  ExprPtr _else;
  if(TOKEN(Else)) {
    lexer.Next();
    if(!TOKEN(Indent)) {
      _else = ParseExpr(lexer);
      if(!_else)
        ERROR(Expr)
    }
    else {
      _else = ParseBlock(lexer);
      if(!_else)
        ERROR(Block)
    }
  }

  return ExprPtr(new EIf(move(cond), move(then), move(_else)));
}

ExprPtr ParseFn(BufferedLexer& lexer)
{
  if(!TOKEN(Fn))
    return {};
  lexer.Next();

  ExprPtr param = ParseExpr(lexer);
  if(!param)
    ERROR(Expr)

  if(!TOKEN(Operator) || lexer.Get().strValue != "->")
    ERROR(Operator)
  lexer.Next();

  ExprPtr body = ParseExpr(lexer);
  if(!body)
    ERROR(Expr)

  return ExprPtr(new EFunction(move(param), move(body)));
}

ExprPtr ParseExpr_P(BufferedLexer& lexer);

ExprPtr ParseExpr_Exp(BufferedLexer& lexer, int p)
{
  ExprPtr expr = ParseExpr_P(lexer);

  while(TOKEN(Operator))
  {
    auto binaryOp = binaryOperators.find(lexer.Get().strValue);
    if(binaryOp == binaryOperators.end())
      break;
    int prec = binaryOp->second.first;
    bool rightAssoc = binaryOp->second.second;
    if(prec < p)
      break;
    lexer.Next();

    int q = rightAssoc ? prec : (1 + prec);

    auto exprRight = ParseExpr_Exp(lexer, q);
    if(!exprRight)
      ERROR(Expr)

    expr.reset(new EBinaryOp(binaryOp->first, move(expr), move(exprRight)));
  }

  return expr;
}

ExprPtr ParseExpr_P(BufferedLexer& lexer)
{
  ExprPtr expr;

  if(TOKEN(Operator))
  {
    auto unaryOp = unaryOperators.find(lexer.Get().strValue);
    if(unaryOp == unaryOperators.end()) {
      cerr << "unknown unary operator " << lexer.Get().strValue << endl;
      return {};
    }
    lexer.Next();

    expr = ParseExpr_Exp(lexer, unaryOp->second);
    if(!expr)
      ERROR(Expr)

    expr.reset(new EUnaryOp(unaryOp->first, move(expr)));
  }
  else if(TOKEN(OpenParen))
  {
    lexer.Next();

    auto tuple = make_unique<ETuple>();

    while(true) {
      auto e = ParseExpr_Exp(lexer, 0);
      if(!e)
        break;
      tuple->Push(move(e));

      if(!TOKEN(Comma))
        break;
      lexer.Next();
    }

    if(!TOKEN(CloseParen))
      ERROR(CloseParen)
    lexer.Next();

    if(tuple->exprs.empty()) {
      expr.reset(new EVoid);
    }
    else if(tuple->exprs.size() == 1) {
      expr = move(tuple->exprs.front());
    }
    else {
      expr.reset(tuple.release());
    }
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
  failure = false;
  ExprPtr expr = ParseExpr(lexer);
  if(!TOKEN(EndOfFile))
    ERROR(EndOfFile)
  if(failure)
    expr.reset();
  return expr;
}

} // namespace xra
