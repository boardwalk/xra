#include "common.hpp"
#include "type.hpp"

namespace xra {

const TypePtr VoidType(new TList);
const TypePtr BooleanType(new TBoolean);
const TypePtr IntegerType(new TInteger(true, sizeof(int) * CHAR_BIT));
const TypePtr FloatType(new TFloat(sizeof(float) * CHAR_BIT));
const TypePtr StringType(new TString);

TypePtr MakeTypeVar()
{
  static int count = 0;
  ++count;

  string name(1, '#');
  for(int i = count; i > 0; i /= 26)
    name += ('a' + (i % 26) - 1);

  return new TVariable(move(name));
}

void Compose(const TypeSubst& a, TypeSubst& b)
{
  for(auto& pair : b)
    pair.second = Apply(a, *pair.second);

  for(auto& pair : a)
    b.insert(pair);
}

ostream& operator<<(ostream& os, const TypeSubst& subst)
{
  os << "{subst";
  for(auto& pair : subst) {
    os << " " << pair.first << "=";
    if(pair.second)
      os << *pair.second;
    else
      os << "(null)";
  }
  os << "}";
  return os;
}

} // namespace xra
