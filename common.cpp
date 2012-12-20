#include "common.hpp"
#include <sstream>
#include <iomanip>

namespace xra {

string EscapeString(const string& str)
{
  stringstream ss;
  for(size_t i = 0; i < str.size(); i++) {
    if(str[i] == '\"')
      ss << "\\\"";
    else if(!isprint(str[i]))
      ss << "\\x" << hex << setw(2) << setfill('0') << (int)str[i];
    else
      ss << str[i];
  }
  return ss.str();
}

} // namespace xra