#include "common.hpp"
#include "token-buffer.hpp"
#include "macro-parser.hpp"
#include "type.hpp"

#define TOKEN(t) (tokens().type == Token::t)
#define NEXT_TOKEN(t) (tokens(1).type == Token::t)
#define ERROR(what) \
  { \
    Error() << what << " near " << tokens() << " at type-parser.cpp:" << __LINE__; \
    return TypePtr(); \
  }
#define EXPECTED(t) \
  ERROR("expected " #t)

namespace xra {

TypePtr ParseTypeList(TokenBuffer<MacroParser>& tokens) // prefix: "("
{
  auto list = make_unique<TList>();

  while(true) {
    string field;
    if(TOKEN(Identifier) && NEXT_TOKEN(Slash)) {
      field = tokens().strValue;
      tokens.Consume(2);
    }

    auto type = ParseType(tokens);
    if(!type)
      EXPECTED(Type)

    list->fields.push_back({move(field), type});

    if(!TOKEN(Operator) || tokens().strValue  != ",")
      break;
    tokens.Consume();
  }

  return list.release();
}

TypePtr ParseType(TokenBuffer<MacroParser>& tokens) // prefix: "\"
{
  TypePtr type;

  if(TOKEN(Identifier)) {
    type = new TVariable(tokens().strValue);
    tokens.Consume();
  }
  else if(TOKEN(BooleanType)) {
    type = BooleanType;
    tokens.Consume();
  }
  else if(TOKEN(IntegerType)) {
    tokens.Consume();
    bool signed_ = true;
    if(TOKEN(Unsigned)) {
      signed_ = false;
      tokens.Consume();
    }
    else if(TOKEN(Signed)) {
      signed_ = true;
      tokens.Consume();
    }
    unsigned int width = sizeof(int) * CHAR_BIT;
    if(TOKEN(Integer)) {
      width = (unsigned int)tokens().intValue;
      tokens.Consume();
    }
    if(width != 8 && width != 16 && width != 32 &&
       width != 64 && width != 128)
      ERROR("Invalid integer width")
    type = new TInteger(signed_, width);
  }
  else if(TOKEN(FloatType)) {
    tokens.Consume();
    unsigned int width = sizeof(float) * CHAR_BIT;
    if(TOKEN(Integer)) {
      width = (unsigned int)tokens().intValue;
      tokens.Consume();
    }
    if(width != 16 && width != 32 && width != 64 &&
       width != 80 && width != 128)
      ERROR("Invalid float width")
    type = new TFloat(width);
  }
  else if(TOKEN(StringType)) {
    type = StringType;
    tokens.Consume();
  }
  else if(TOKEN(OpenParen)) {
    tokens.Consume();
    if(TOKEN(CloseParen)) {
      type = VoidType;
    }
    else {
      type = ParseTypeList(tokens);
      if(!TOKEN(CloseParen))
        EXPECTED(CloseParen);
    }
    tokens.Consume();
  }

  if(!type) {
    Error() << "unexpected token " << tokens() << " while parsing type";
    tokens.Consume();
    return type;
  }

  if(TOKEN(Operator) && tokens().strValue == "->")
  {
    tokens.Consume();

    // the parameter to a function is always a list
    if(!isa<TList>(*type)) {
      auto list = make_unique<TList>();
      list->fields.push_back({string(), type});
      type = list.release();
    }

    TypePtr typeRight = ParseType(tokens);
    type = new TFunction(type, typeRight);
  }

  return type;
}

} // namespace xra
