#include "common.hpp"
#include "type.hpp"
#include "buffered-lexer.hpp"
#include <sstream>

#define TOKEN(t) (lexer.Get().type == Token::t)
#define ERROR(what) \
  { \
    stringstream ss; \
    ss << what << " near " << lexer.Get() << " at type.parser.cpp:" << __LINE__; \
    return TypePtr(new TError(ss.str())); \
  }
#define EXPECTED(t) \
  ERROR("expected " #t)

namespace xra {

TypePtr ParseType(BufferedLexer& lexer) // prefix: :
{
  TypePtr type;

  if(TOKEN(Identifier)) {
    type.reset(new TVariable(lexer.Get().strValue));
    lexer.Consume();
  }
  else if(TOKEN(OpenParen)) {
    lexer.Consume();
    if(TOKEN(CloseParen)) {
      type = VoidType;
    }
    else {
      type = ParseType(lexer);
      if(!TOKEN(CloseParen))
        EXPECTED(CloseParen);
    }
    lexer.Consume();
  }

  if(!type) {
    stringstream ss;
    ss << "unexpected token " << lexer.Get();
    type.reset(new TError(ss.str()));
    lexer.Consume();
    return type;
  }

  if(TOKEN(Operator))
  {
    if(lexer.Get().strValue == ",")
    {
      lexer.Consume();
      TypePtr typeRight = ParseType(lexer);

      TList* list = dyn_cast<TList>(typeRight.get());
      if(list) {
        list->types.insert(list->types.begin(), type);
        type = typeRight;
      }
      else {
        list = new TList();
        list->types.push_back(type);
        list->types.push_back(typeRight);
        type.reset(list);
      }
    }
    else if(lexer.Get().strValue == "->")
    {
      lexer.Consume();
      TypePtr typeRight = ParseType(lexer);
      type.reset(new TFunction(type, typeRight));
    }
  }

  return type;
}

} // namespace xra
