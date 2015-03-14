#ifndef LOCIC_CODEGEN_METHODINFO_HPP
#define LOCIC_CODEGEN_METHODINFO_HPP

#include <locic/SEM.hpp>

#include <locic/Support/String.hpp>

namespace locic {
	
	namespace CodeGen {
		
		struct MethodInfo {
			const SEM::Type* parentType;
			String name;
			const SEM::Type* functionType;
			HeapArray<SEM::Value> templateArgs;
			
			MethodInfo(const SEM::Type* const argParentType, const String& argName,
				const SEM::Type* const argFunctionType, HeapArray<SEM::Value> argTemplateArgs)
			: parentType(argParentType), name(argName), functionType(argFunctionType), templateArgs(std::move(argTemplateArgs)) { }
		};
		
		struct TypeInfoComponents {
			llvm::Value* vtablePointer;
			llvm::Value* templateGenerator;
			
			TypeInfoComponents(llvm::Value* const argVtablePointer, llvm::Value* const argTemplateGenerator)
			: vtablePointer(argVtablePointer), templateGenerator(argTemplateGenerator) { }
		};
		
		TypeInfoComponents getTypeInfoComponents(Function& function, llvm::Value* typeInfoValue);
		
		struct VirtualObjectComponents {
			TypeInfoComponents typeInfo;
			llvm::Value* objectPointer;
			
			VirtualObjectComponents(
				const TypeInfoComponents argTypeInfo,
				llvm::Value* const argObjectPointer)
			: typeInfo(argTypeInfo),
			objectPointer(argObjectPointer) { }
		};
		
		VirtualObjectComponents getVirtualObjectComponents(Function& function, llvm::Value* interfaceStructValue);
		
		struct VirtualMethodComponents {
			VirtualObjectComponents object;
			llvm::Value* hashValue;
			
			VirtualMethodComponents(
				const VirtualObjectComponents argObject,
				llvm::Value* const argHashValue)
			: object(argObject),
			hashValue(argHashValue) { }
		};
		
		VirtualMethodComponents getVirtualMethodComponents(Function& function, bool isStatic, llvm::Value* interfaceMethodValue);
		
	}
	
}

#endif
