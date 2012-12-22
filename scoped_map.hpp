#ifndef XRA_SCOPED_MAP_HPP
#define XRA_SCOPED_MAP_HPP

#include <boost/iterator/transform_iterator.hpp>

namespace xra {

/*
 * scoped_map is a versioned-variant of std::map
 * use scoped_map::lock to create and destroy scopes
 * note: iterates over pair<const K&, V&> values, not pair<const K, V> references
 */
template<class Key, class Value>
class scoped_map
{
  typedef vector<Value> value_vector;
  typedef map<Key, value_vector> base_map;
  typedef typename base_map::iterator base_iterator;
  typedef typename base_map::const_iterator base_const_iterator;
  typedef typename base_map::value_type base_value_type;
  typedef pair<const Key&, Value&> value_type;
  typedef pair<const Key&, const Value&> const_value_type;

  static value_type get_value(base_value_type& p)
  {
    return {p.first, p.second.back()};
  }
  static const_value_type get_const_value(const base_value_type& p)
  {
    return {p.first, p.second.back()};
  }

  typedef value_type (*get_value_type)(base_value_type&);
  typedef const_value_type (*get_const_value_type)(const base_value_type&);

  void push()
  {
    scopes.emplace_back();
  }

  void pop()
  {
    for(auto& key : scopes.back())
    {
      auto it = data.find(key);
      it->second.pop_back();
      if(it->second.empty())
        data.erase(it);
    }
    scopes.pop_back();
  }

  base_map data;
  vector<set<string> > scopes;

public:
  typedef boost::transform_iterator<get_value_type, base_iterator> iterator;
  typedef boost::transform_iterator<get_const_value_type, base_const_iterator> const_iterator;

  class lock
  {
    scoped_map& map;
    unsigned int level;

  public:
    lock(scoped_map& map_) : map(map_)
    {
      map.push();
      level = map.scopes.size();
    }

    ~lock()
    {
      if(level != map.scopes.size())
        abort();
      map.pop();
    }

    // noncopyable
    lock(const lock&) = delete;
    lock& operator=(const lock&) = delete;
  };

  scoped_map()
  {
    scopes.emplace_back();
  }

  ~scoped_map()
  {
    if(scopes.size() != 1)
      abort();
  }

  iterator begin()
  {
    return iterator(data.begin(), &get_value);
  }

  iterator end()
  {
    return iterator(data.end(), &get_value);
  }

  iterator find(const Key& key)
  {
    return iterator(data.find(key), &get_value);
  }

  const_iterator begin() const
  {
    return const_iterator(data.begin(), &get_const_value);
  }

  const_iterator end() const
  {
    return const_iterator(data.end(), &get_const_value);
  }

  const_iterator find(const Key& key) const
  {
    return const_iterator(data.find(key), &get_const_value);
  }

  Value& operator[](const Key& key)
  {
    auto& vec = data[key];

    if(!in_top_scope(key))
    {
      if(vec.empty()) {
        vec.emplace_back();
      }
      else {
        Value v( vec.back() );
        vec.push_back(v);
      }
      scopes.back().insert(key);
    }

    return vec.back();
  }

  bool in_top_scope(const Key& key) const
  {
    return scopes.back().find(key) != scopes.back().end();
  }

  // noncopyable
  scoped_map(const scoped_map&) = delete;
  scoped_map& operator=(const scoped_map&) = delete;
};

} // namespace xra

#endif // XRA_SCOPED_MAP_HPP
