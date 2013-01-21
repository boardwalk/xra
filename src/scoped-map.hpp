#ifndef XRA_SCOPED_MAP_HPP
#define XRA_SCOPED_MAP_HPP

namespace xra {

template<class KeyTy, class ValueTy>
class ScopedMap
{
  typedef map<KeyTy, ValueTy> BaseMap;
  BaseMap data;

  enum Operation {
    ValueCreated,
    ValueShadowed
  };

  stack<Operation> operations;
  stack<KeyTy> created;
  stack<typename BaseMap::value_type> shadowed;

  void PopUntil(size_t nops)
  {
    assert(operations.size() >= nops);
    while(operations.size() > nops) {
      if(operations.top() == ValueCreated) {
        data.erase(created.top());
        created.pop();
      }
      else {
        data[shadowed.top().first] = move(shadowed.top().second);
        shadowed.pop();
      }
      operations.pop();
    }
  }

public:
  class Scope
  {
    ScopedMap& map;
    size_t nops;

  public:
    Scope(ScopedMap& map_) :
      map(map_),
      nops(map_.operations.size())
      {}

    ~Scope()
      {
        map.PopUntil(nops);
      }

    // noncopyable
    Scope(const Scope&) = delete;
    Scope& operator=(const Scope&) = delete;
  };

  ScopedMap()
  {}

  const BaseMap& Values() const
  {
    return data;
  }

  ValueTy operator[](const KeyTy& key) const
  {
    auto it = data.find(key);
    if(it != data.end())
      return it->second;
    return {};
  }

  typename BaseMap::const_iterator Find(const KeyTy& key) const
  {
    return data.find(key);
  }

  typename BaseMap::const_iterator End() const
  {
    return data.end();
  }

  void AddValue(const KeyTy& key, ValueTy value)
  {
    auto it = data.lower_bound(key);
    if(it != data.end() && it->first == key) {
      swap(it->second, value);
      shadowed.push({key, move(value)});
      operations.push(ValueShadowed);
    }
    else {
      data.insert(it, {key, move(value)});
      created.push(key);
      operations.push(ValueCreated);
    }
  }

  // noncopyable
  ScopedMap(const ScopedMap&) = delete;
  ScopedMap& operator=(const ScopedMap&) = delete;
};

} // namespace xra

#endif // XRA_SCOPED_MAP_HPP
