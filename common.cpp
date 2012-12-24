#include "common.hpp"
#include <iomanip>

namespace xra {

void EscapeString(const string& str, stringstream& ss)
{
  for(size_t i = 0; i < str.size(); i++) {
    if(str[i] == '\"')
      ss << "\\\"";
    else if(!isprint(str[i]))
      ss << "\\x" << hex << setw(2) << setfill('0') << (int)str[i];
    else
      ss << str[i];
  }
}

} // namespace xra
