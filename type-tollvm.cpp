#include "common.hpp"
#include "visitor.hpp"

namespace xra {

struct TypeToLLVMVisitor : Visitor<TypeToLLVMVisitor, const Type>
{
  llvm::LLVMContext& ctx;
  llvm::Type* result;

  TypeToLLVMVisitor(llvm::LLVMContext& ctx_) :
    ctx(ctx_),
    result(nullptr)
  {}

  void VisitTBoolean(const TBoolean&)
  {
    result = llvm::Type::getInt1Ty(ctx);
  }

  void VisitTList(const TList& type)
  {
    if(type.types.empty()) {
      result = llvm::Type::getVoidTy(ctx);
      return;
    }

    vector<llvm::Type*> llvmTypes;
    for(auto& t : type.types) {
      Visit(t.get());
      llvmTypes.push_back(result);
    }
    result = llvm::StructType::get(ctx, llvmTypes, false);
  }

  void VisitTFunction(const TFunction& type)
  {
    Visit(type.parameter.get());
    auto funcParameter = result;

    // void is not a valid function argument!
    llvm::ArrayRef<llvm::Type*> funcParameterArr;
    if(!funcParameter->isVoidTy())
      funcParameterArr = llvm::ArrayRef<llvm::Type*>(funcParameter);
    
    Visit(type.result.get());
    auto funcResult = result;

    result = llvm::FunctionType::get(funcResult, funcParameterArr, false);
    result = result->getPointerTo();
  }
};

llvm::Type* ToLLVM(const Type& type, llvm::LLVMContext& ctx)
{
  TypeToLLVMVisitor visitor(ctx);
  visitor.Visit(&type);
  return visitor.result;
}

} // namespace xra
