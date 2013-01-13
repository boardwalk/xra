#include "common.hpp"
#include <iomanip>

namespace xra {

stringstream Error::ss;

ostream& operator<<(ostream& os, const SourceLoc& loc)
{
  if(loc.source)
    os << *loc.source << ":" << loc.line << ":" << loc.column;
  else
    os << "(unknown)";
  return os;
}

void EscapeString(const string& str, ostream& os)
{
  for(size_t i = 0; i < str.size(); i++) {
    if(str[i] == '\"')
      os << "\\\"";
    else if(str[i] == '\\')
      os << "\\\\";
    else if(!isprint(str[i])) {
      ostream::fmtflags f = os.flags();
      os << "\\x" << hex << setw(2) << setfill('0') << (int)str[i];
      os.flags(f);
    }
    else
      os << str[i];
  }
}

} // namespace xra
