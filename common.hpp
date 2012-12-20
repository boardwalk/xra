#ifndef XRA_COMMON_HPP
#define XRA_COMMON_HPP

#include <memory>
#include <string>
#include <stack>
#include <vector>

namespace xra {

using namespace std;

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
