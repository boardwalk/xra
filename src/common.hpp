#ifndef XRA_COMMON_HPP
#define XRA_COMMON_HPP

#include <llvm/Config/config.h>
#include <llvm/DerivedTypes.h>
#include <llvm/LLVMContext.h>
#include <llvm/Module.h>
#if LLVM_VERSION_MAJOR == 3 && LLVM_VERSION_MINOR < 2
#include <llvm/Support/IRBuilder.h>
#else
#include <llvm/IRBuilder.h>
#endif
#include <boost/intrusive_ptr.hpp>

#include <deque>
#include <map>
#include <memory>
#include <set>
#include <sstream>
#include <stack>
#include <vector>

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

class BufferedLexer;
class Env;
class Compiler;

typedef map<string, TypePtr> TypeSubst;

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

ostream& operator<<(ostream&, const SourceLoc&);

void EscapeString(const string&, ostream&);

} // namespace xra

#endif // XRA_COMMON_HPP
