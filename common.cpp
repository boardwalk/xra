#include "common.hpp"
#include <iomanip>

namespace xra {

stringstream Error::ss;

void EscapeString(const string& str, ostream& os)
{
  for(size_t i = 0; i < str.size(); i++) {
    if(str[i] == '\"')
      os << "\\\"";
    else if(!isprint(str[i]))
      os << "\\x" << hex << setw(2) << setfill('0') << (int)str[i];
    else
      os << str[i];
  }
}

} // namespace xra
