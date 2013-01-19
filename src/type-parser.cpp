#include "common.hpp"
#include "type.hpp"
#include "buffered-lexer.hpp"

#define TOKEN(t) (lexer().type == Token::t)
#define ERROR(what) \
  { \
    Error() << what << " near " << lexer() << " at type-parser.cpp:" << __LINE__; \
    return TypePtr(); \
  }
#define EXPECTED(t) \
  ERROR("expected " #t)

namespace xra {

static TypePtr ParseType(BufferedLexer& lexer, int level)
{
  TypePtr type;

  if(TOKEN(Identifier)) {
    type = new TVariable(lexer().strValue);
    lexer.Consume();
  }
  else if(TOKEN(BooleanType)) {
    type = BooleanType;
    lexer.Consume();
  }
  else if(TOKEN(IntegerType)) {
    lexer.Consume();
    bool signed_ = true;
    if(TOKEN(Unsigned)) {
      signed_ = false;
      lexer.Consume();
    }
    else if(TOKEN(Signed)) {
      signed_ = true;
      lexer.Consume();
    }
    unsigned int width = sizeof(int) * CHAR_BIT;
    if(TOKEN(Integer)) {
      width = (unsigned int)lexer().intValue;
      lexer.Consume();
    }
    if(width != 8 && width != 16 && width != 32 &&
       width != 64 && width != 128)
      ERROR("Invalid integer width")
    type = new TInteger(signed_, width);
  }
  else if(TOKEN(FloatType)) {
    lexer.Consume();
    unsigned int width = sizeof(float) * CHAR_BIT;
    if(TOKEN(Integer)) {
      width = (unsigned int)lexer().intValue;
      lexer.Consume();
    }
    if(width != 16 && width != 32 && width != 64 &&
       width != 80 && width != 128)
      ERROR("Invalid float width")
    type = new TFloat(width);
  }
  else if(TOKEN(StringType)) {
    type = StringType;
    lexer.Consume();
  }
  else if(TOKEN(OpenParen)) {
    lexer.Consume();
    if(TOKEN(CloseParen)) {
      type = VoidType;
    }
    else {
      type = ParseType(lexer, level + 1);
      if(!TOKEN(CloseParen))
        EXPECTED(CloseParen);
    }
    lexer.Consume();
  }

  if(!type) {
    Error() << "unexpected token " << lexer() << " while parsing type";
    lexer.Consume();
    return type;
  }

  if(TOKEN(Operator))
  {
    if(level != 0 && lexer().strValue == ",")
    {
      lexer.Consume();
      TypePtr typeRight = ParseType(lexer, level);

      TList* list = dyn_cast<TList>(typeRight.get());
      if(list) {
        list->types.insert(list->types.begin(), type);
        type = typeRight;
      }
      else {
        list = new TList();
        list->types.push_back(type);
        list->types.push_back(typeRight);
        type = list;
      }
    }
    else if(lexer().strValue == "->")
    {
      // the parameter to a function is always a list
      if(!isa<TList>(*type)) {
        TList* list = new TList();
        list->types.push_back(type);
        type = list;
      }

      lexer.Consume();
      TypePtr typeRight = ParseType(lexer, level);
      type = new TFunction(type, typeRight);
    }
  }

  return type;
}

TypePtr ParseType(BufferedLexer& lexer) // prefix: "\"
{
  return ParseType(lexer, 0);
}

} // namespace xra
