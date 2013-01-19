#include "common.hpp"
#include "type.hpp"
#include "buffered-lexer.hpp"

#define TOKEN(t) (lexer().type == Token::t)
#define NEXT_TOKEN(t) (lexer(1).type == Token::t)
#define ERROR(what) \
  { \
    Error() << what << " near " << lexer() << " at type-parser.cpp:" << __LINE__; \
    return TypePtr(); \
  }
#define EXPECTED(t) \
  ERROR("expected " #t)

namespace xra {

TypePtr ParseTypeList(BufferedLexer& lexer) // prefix: "("
{
  auto list = make_unique<TList>();

  while(true) {
    string field;
    if(TOKEN(Identifier) && NEXT_TOKEN(Slash)) {
      field = lexer().strValue;
      lexer.Consume(2);
    }

    auto type = ParseType(lexer);
    if(!type)
      EXPECTED(Type)

    list->fields.push_back({move(field), type});

    if(!TOKEN(Operator) || lexer().strValue  != ",")
      break;
    lexer.Consume();
  }

  return list.release();
}

TypePtr ParseType(BufferedLexer& lexer) // prefix: "\"
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
      type = ParseTypeList(lexer);
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

  if(TOKEN(Operator) && lexer().strValue == "->")
  {
    lexer.Consume();

    // the parameter to a function is always a list
    if(!isa<TList>(*type)) {
      auto list = make_unique<TList>();
      list->fields.push_back({string(), type});
      type = list.release();
    }

    TypePtr typeRight = ParseType(lexer);
    type = new TFunction(type, typeRight);
  }

  return type;
}

} // namespace xra
