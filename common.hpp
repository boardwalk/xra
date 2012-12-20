#ifndef XRA_COMMON_HPP
#define XRA_COMMON_HPP

#include <memory>
#include <string>
#include <stack>

namespace xra {

using namespace std;

template<typename T, typename... Args>
unique_ptr<T> make_unique(Args&&... args)
{
  return unique_ptr<T>(new T(forward<Args>(args)...));
}

} // namespace xra

#endif // XRA_COMMON_HPP
