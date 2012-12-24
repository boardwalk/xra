#include "common.hpp"
#include "env.hpp"

namespace xra {

class BSequence : public VBuiltin
{
public:
  TypePtr Infer(Expr& argument, TypeSubst& subst)
  {
    printf("boop\n");
    return VoidType;
  }
};

template<class T>
void Add(Env& env, const string& name)
{
  env.AddValue(name, new T());
}

void AddBuiltins(Env& env)
{
  Add<BSequence>(env, ";");
}
  
} // namespace xra
