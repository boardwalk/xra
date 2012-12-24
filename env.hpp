#ifndef XRA_ENV_HPP
#define XRA_ENV_HPP

#include "value.hpp"

namespace xra {

class Env
{
  friend class Scope;

public:
  ValuePtr operator[](const string& name) const
  {
    auto it = values.find(name);
    if(it != values.end())
      return it->second;
    return nullptr;
  }

  void AddValue(const string& name, ValuePtr value)
  {
    auto it = values.find(name);
    if(it != values.end()) {
      shadowedValues.push(*it);
      undos.push(&Env::UnshadowValue);
      it->second = value;
    }
    else {
      createdValues.push(name);
      undos.push(&Env::UncreateValue);
      values[name] = value;
    }
  }

  void Apply(const TypeSubst& subst)
  {
    for(auto& pair : values)
      xra::Apply(subst, pair.second->typeScheme);
  }

private:  
  typedef void (Env::*Undo)();
  stack<Undo> undos;

  typedef map<string, ValuePtr> ValueMap;
  ValueMap values;
  
  stack<string> createdValues;
  stack<ValueMap::value_type> shadowedValues;

  void UncreateValue()
  {
    values.erase(createdValues.top());
    createdValues.pop();
  }

  void UnshadowValue()
  {
    values[shadowedValues.top().first] = shadowedValues.top().second;
    shadowedValues.pop();
  }
};

class Scope
{
public:
  Scope(Env& env_) :
    env(env_),
    pos(env_.undos.size())
  {}

  ~Scope()
  {
    assert(env.undos.size() >= pos);
    while(env.undos.size() > pos) {
      (env.*env.undos.top())();
      env.undos.pop();
    }
  }

  // noncopyable
  Scope(const Scope&) = delete;
  Scope& operator=(const Scope&) = delete;

private:
  Env& env;
  size_t pos;
};

} // namespace xra

#endif // XRA_ENV_HPP
