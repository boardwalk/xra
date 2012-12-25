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

class Expr;
typedef unique_ptr<Expr> ExprPtr;

class Value;
typedef boost::intrusive_ptr<Value> ValuePtr;

class Type;
typedef boost::intrusive_ptr<Type> TypePtr;

typedef map<string, TypePtr> TypeSubst;

class Env;

template<class T>
struct has_tostring : false_type {};

template<>
struct has_tostring<Token> : true_type {};

template<>
struct has_tostring<Expr> : true_type {};

template<>
struct has_tostring<Value> : true_type {};

template<>
struct has_tostring<Type> : true_type {};

template<>
struct has_tostring<TypeSubst> : true_type {};

template<>
struct has_tostring<Env> : true_type {};

template<class StmTy, class ValTy>
typename enable_if<has_tostring<ValTy>::value, StmTy>::type&
operator<<(StmTy& stm, const ValTy& val)
{
  stringstream ss;
  ToString(val, ss);
  stm << ss.str();
  return stm;
}

template<class StmTy, class ValTy>
typename enable_if<has_tostring<ValTy>::value, StmTy>::type&
operator<<(StmTy& stm, const ValTy* val)
{
  if(!val) {
    stm << "(null)";
    return stm;
  }
  return stm << *val;
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

void EscapeString(const string&, stringstream&);

} // namespace xra

#endif // XRA_COMMON_HPP
