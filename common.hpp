#ifndef XRA_COMMON_HPP
#define XRA_COMMON_HPP

#include <deque>
#include <map>
#include <memory>
#include <set>
#include <sstream>
#include <stack>
#include <vector>
#include <llvm/Support/Casting.h>
#include <boost/intrusive_ptr.hpp>

namespace xra {

using namespace std;

using llvm::isa;
using llvm::dyn_cast;

template<typename T, typename... Args>
unique_ptr<T> make_unique(Args&&... args)
{
  return unique_ptr<T>(new T(forward<Args>(args)...));
}

template<typename Source, typename Target>
struct CopyConst { typedef Target type; };

template<typename Source, typename Target>
struct CopyConst<const Source, Target> { typedef const Target type; };

class Token;
ostream& operator<<(ostream&, const Token&);

class Expr;
typedef boost::intrusive_ptr<Expr> ExprPtr;
ostream& operator<<(ostream&, const Expr&);

class Value;
typedef boost::intrusive_ptr<Value> ValuePtr;
ostream& operator<<(ostream&, const Value&);

class Type;
typedef boost::intrusive_ptr<Type> TypePtr;
ostream& operator<<(ostream&, const Type&);

typedef map<string, TypePtr> TypeSubst;
ostream& operator<<(ostream&, const TypeSubst&);

class Env;
ostream& operator<<(ostream&, const Env&);

struct SourceLoc
{
  shared_ptr<string> source;
  int line;
  int column;
};

class Base
{
public:
  Base() :
    refcount(0)
  {}

  virtual ~Base() {}

  Base(const Base&) = delete;
  Base& operator=(const Base&) = delete;

private:
  friend void intrusive_ptr_add_ref(Base* base);
  friend void intrusive_ptr_release(Base* base);
  int refcount;
};

inline void intrusive_ptr_add_ref(Base* base)
{
  base->refcount++;
}

inline void intrusive_ptr_release(Base* base)
{
  if(--base->refcount == 0)
    delete base;
}

class Error
{
public:
  Error()
  {}

  ~Error()
  {
    ss << endl;
  }

  template<class T>
  Error& operator<<(T&& value)
  {
    ss << forward<T>(value);
    return *this;
  }

  static string Get()
  {
    return ss.str();
  }

  Error(const Error&) = delete;
  Error& operator=(const Error&) = delete;

private:
  static stringstream ss;
};

void EscapeString(const string&, ostream&);

} // namespace xra

#endif // XRA_COMMON_HPP
