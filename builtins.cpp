#include "common.hpp"
#include "type.hpp"

namespace xra {

class BScope : public TBuiltin {};
class BSelect : public TBuiltin {};
class BMultiply : public TBuiltin {};
class BDivide : public TBuiltin {};
class BPlus : public TBuiltin {};
class BSubtract : public TBuiltin {};
class BLeftShift : public TBuiltin {};
class BRightShift : public TBuiltin {};
class BLessThan : public TBuiltin {};
class BLessEqual : public TBuiltin {};
class BGreaterThan : public TBuiltin {};
class BGreaterEqual : public TBuiltin {};
class BEqual : public TBuiltin {};
class BNotEqual : public TBuiltin {};
class BBitwiseAnd : public TBuiltin {};
class BBitwiseXor : public TBuiltin {};
class BBitwiseOr : public TBuiltin {};
class BLogicalAnd : public TBuiltin {};
class BLogicalOr : public TBuiltin {};
class BAssign : public TBuiltin {};
class BSequence : public TBuiltin {};

template<class T>
void Set(TypeEnv& env, const char* name)
{
  env[name].type.reset(new T);
}

void SetBuiltins(TypeEnv& env)
{
  Set<BScope>(env, "\\");
  Set<BSelect>(env, ".");
  Set<BMultiply>(env, "*");
  Set<BDivide>(env, "/");
  Set<BPlus>(env, "+");
  Set<BSubtract>(env, "-");
  Set<BLeftShift>(env, "<<");
  Set<BRightShift>(env, ">>");
  Set<BLessThan>(env, "<");
  Set<BLessEqual>(env, "<=");
  Set<BGreaterThan>(env, ">");
  Set<BGreaterEqual>(env, ">=");
  Set<BEqual>(env, "==");
  Set<BNotEqual>(env, "!=");
  Set<BBitwiseAnd>(env, "&");
  Set<BBitwiseXor>(env, "^");
  Set<BBitwiseOr>(env, "|");
  Set<BLogicalAnd>(env, "&&");
  Set<BLogicalOr>(env, "||");
  Set<BAssign>(env, "=");
  Set<BSequence>(env, ";");
}

} // namespace xra
