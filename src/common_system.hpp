#ifndef XRA_COMMON_SYSTEM_HPP
#define XRA_COMMON_SYSTEM_HPP

#pragma clang system_header

#include <llvm/Config/config.h>
#include <llvm/DerivedTypes.h>
#include <llvm/LLVMContext.h>
#include <llvm/Module.h>
#if LLVM_VERSION_MAJOR == 3 && LLVM_VERSION_MINOR < 2
#include <llvm/Support/IRBuilder.h>
#else
#include <llvm/IRBuilder.h>
#endif
#include <llvm/Support/raw_os_ostream.h>
#include <llvm/Bitcode/ReaderWriter.h>
#include <llvm/Support/TargetSelect.h>
#include <llvm/ExecutionEngine/ExecutionEngine.h>
#include <llvm/ExecutionEngine/JIT.h>
#include <boost/intrusive_ptr.hpp>

#include <map>
#include <memory>
#include <set>
#include <sstream>
#include <stack>
#include <vector>

#endif // XRA_COMMON_SYSTEM_HPP
