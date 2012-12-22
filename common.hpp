#ifndef XRA_COMMON_HPP
#define XRA_COMMON_HPP

#include <deque>
#include <map>
#include <memory>
#include <set>
#include <stack>
#include <string>
#include <vector>
#include <llvm/Support/Casting.h>

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

string EscapeString(const string& str);

} // namespace xra

#endif // XRA_COMMON_HPP
