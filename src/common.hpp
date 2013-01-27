#ifndef XRA_COMMON_HPP
#define XRA_COMMON_HPP

#include "common_system.hpp"

namespace xra {

using namespace std;

using llvm::isa;
using llvm::dyn_cast;

template<typename T, typename... Args>
unique_ptr<T> make_unique(Args&&... args)
{
  return unique_ptr<T>(new T(forward<Args>(args)...));
}

struct SourceLoc
{
  shared_ptr<string> source;
  int line;
  int column;
};

class Expr;
typedef boost::intrusive_ptr<Expr> ExprPtr;

class Value;
typedef boost::intrusive_ptr<Value> ValuePtr;

class Type;
typedef boost::intrusive_ptr<Type> TypePtr;

typedef map<string, TypePtr> TypeSubst;

template<class TokenSource>
class TokenBuffer;

class Lexer;
class ExprParser;

class TypeChecker;
class Compiler;
class Env;

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

template<class C>
class ReverseWrapper
{
  C& c;
public:
  ReverseWrapper(C& c_) : c(c_) {}
  typename C::const_reverse_iterator begin() const { return c.crbegin(); }
  typename C::const_reverse_iterator end() const { return c.crend(); }
};

template<class C>
ReverseWrapper<C> Reverse(C& c) { return ReverseWrapper<C>(c); }

// common.cpp
ostream& operator<<(ostream&, const SourceLoc&);
void EscapeString(const string&, ostream&);

// builtins.cpp
void AddBuiltins(Env&);

} // namespace xra

#endif // XRA_COMMON_HPP
