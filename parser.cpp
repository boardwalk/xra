#include "common.hpp"
#include "parser.hpp"
#include "lexer.hpp"
#include <iostream>

#define TOKEN(t) (lexer.Get().type == Token::t)
#define ERROR(t) \
  { \
    cerr << "expected " #t << " near " << lexer.Get() << endl; \
    return {}; \
  }

namespace xra {

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

ExprPtr ParseExpr(BufferedLexer& lexer)
{
  if(TOKEN(Identifier)) {
    ExprPtr expr(new EVariable(lexer.Get().strValue));
    lexer.Next();
    return expr;
  }
  if(TOKEN(True)) {
    lexer.Next();
    return ExprPtr(new EBoolean(true));
  }
  else if(TOKEN(False)) {
    lexer.Next();
    return ExprPtr(new EBoolean(false));
  }
  else if(TOKEN(Integer)) {
    ExprPtr expr(new EInteger(lexer.Get().intValue));
    lexer.Next();
    return expr;
  }
  else if(TOKEN(Float)) {
    ExprPtr expr(new EFloat(lexer.Get().floatValue));
    lexer.Next();
    return expr;
  }
  else if(TOKEN(String)) {
    ExprPtr expr(new EString(lexer.Get().strValue));
    lexer.Next();
    return expr;
  }
  if(TOKEN(Extern)) {
    return ParseExtern(lexer);
  }
  else if(TOKEN(If)) {
    return ParseIf(lexer);
  }
  return {};
}

ExprPtr Parse(BufferedLexer& lexer)
{
  lexer.Next();
  return ParseExpr(lexer);
}

} // namespace xra
