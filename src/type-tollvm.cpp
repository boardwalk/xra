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

  void VisitTInteger(const TInteger& type)
  {
    result = llvm::Type::getIntNTy(ctx, type.width);
  }

  void VisitTFloat(const TFloat& type)
  {
    if(type.width == 16)
      result = llvm::Type::getHalfTy(ctx);
    else if(type.width == 32)
      result = llvm::Type::getFloatTy(ctx);
    else if(type.width == 64)
      result = llvm::Type::getDoubleTy(ctx);
    else if(type.width == 80)
      result = llvm::Type::getX86_FP80Ty(ctx);
    else if(type.width == 128)
      result = llvm::Type::getFP128Ty(ctx);
  }

  void VisitTString(const TString&)
  {
    result = llvm::Type::getInt8PtrTy(ctx);
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
    vector<llvm::Type*> llvmParams;
    for(auto& param : static_cast<TList&>(*type.parameter).types) {
      Visit(param.get());
      llvmParams.push_back(result);
    }

    Visit(type.result.get());
    auto llvmResult = result;

    result = llvm::FunctionType::get(llvmResult, llvmParams, false);
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
