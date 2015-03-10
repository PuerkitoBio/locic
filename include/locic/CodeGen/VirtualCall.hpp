#ifndef LOCIC_CODEGEN_VIRTUALCALL_HPP
#define LOCIC_CODEGEN_VIRTUALCALL_HPP

#include <vector>

#include <locic/CodeGen/LLVMIncludes.hpp>

#include <locic/SEM.hpp>
#include <locic/CodeGen/Module.hpp>

namespace locic {

	namespace CodeGen {
		
		namespace VirtualCall {
			
			llvm::Constant* generateVTableSlot(Module& module, const SEM::TypeInstance* typeInstance, llvm::ArrayRef<SEM::Function*> methods);
			
			llvm::Value* generateCall(Function& function, const SEM::Type* functionType, llvm::Value* interfaceMethodValue,
				llvm::ArrayRef<llvm::Value*> args, Optional<llvm::DebugLoc> debugLoc = None, llvm::Value* hintResultValue = nullptr);
			
			enum CountFnKind {
				ALIGNOF,
				SIZEOF
			};
			
			llvm::Value* generateCountFnCall(Function& function, llvm::Value* typeInfoValue, CountFnKind kind);
			
			void generateMoveCall(Function& function, llvm::Value* typeInfoValue, llvm::Value* sourceValue,
				llvm::Value* destValue, llvm::Value* positionValue);
			
			void generateDestructorCall(Function& function, llvm::Value* typeInfoValue, llvm::Value* objectValue);
			
		}
	}
	
}

#endif
