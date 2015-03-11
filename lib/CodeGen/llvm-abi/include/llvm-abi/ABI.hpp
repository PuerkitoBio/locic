#ifndef LLVMABI_ABI_HPP
#define LLVMABI_ABI_HPP

#include <memory>
#include <string>
#include <vector>

#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Type.h>
#include <llvm/IR/Value.h>

#include <llvm-abi/Type.hpp>

namespace llvm_abi {

	struct FunctionType {
		Type* returnType;
		std::vector<Type*> argTypes;
	};
	
	typedef llvm::IRBuilder<> IRBuilder;
	
	class Builder {
	public:
		virtual IRBuilder& getEntryBuilder() = 0;
		
		virtual IRBuilder& getBuilder() = 0;
		
	protected:
		// Prevent destructor calls via this class.
		~Builder() { }
		
	};
	
	class ABI {
		public:
			inline virtual ~ABI() { }
			
			virtual std::string name() const = 0;
			
			virtual const llvm::DataLayout& dataLayout() const = 0;
			
			virtual size_t typeSize(Type* type) const = 0;
			
			virtual size_t typeAlign(Type* type) const = 0;
			
			virtual std::vector<size_t> calculateStructOffsets(llvm::ArrayRef<StructMember> structMembers) const = 0;
			
			virtual llvm::Type* longDoubleType() const = 0;
			
			virtual void encodeValues(Builder& builder, std::vector<llvm::Value*>& argValues, llvm::ArrayRef<Type*> argTypes) = 0;
			
			virtual void decodeValues(Builder& builder, std::vector<llvm::Value*>& argValues, llvm::ArrayRef<Type*> argTypes, llvm::ArrayRef<llvm::Type*> llvmArgTypes) = 0;
			
			virtual llvm::FunctionType* rewriteFunctionType(llvm::FunctionType* llvmFunctionType, const FunctionType& functionType) = 0;
		
	};
	
	std::unique_ptr<ABI> createABI(llvm::Module* module, const std::string& targetTriple);

}

#endif
