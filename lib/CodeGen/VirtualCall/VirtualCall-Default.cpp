#include <vector>

#include <llvm-abi/Type.hpp>

#include <locic/SEM.hpp>

#include <locic/CodeGen/ArgInfo.hpp>
#include <locic/CodeGen/ConstantGenerator.hpp>
#include <locic/CodeGen/Function.hpp>
#include <locic/CodeGen/GenFunction.hpp>
#include <locic/CodeGen/GenValue.hpp>
#include <locic/CodeGen/Memory.hpp>
#include <locic/CodeGen/Module.hpp>
#include <locic/CodeGen/TypeGenerator.hpp>
#include <locic/CodeGen/VirtualCall.hpp>
#include <locic/CodeGen/VTable.hpp>

namespace locic {

	namespace CodeGen {
	
		namespace VirtualCall {
		
			llvm::FunctionType* getStubFunctionType(Module& module) {
				TypeGenerator typeGen(module);
				
				std::vector<llvm::Type*> argTypes;
				
				// Return value pointer type (to handle
				// any possible return type).
				argTypes.push_back(typeGen.getI8PtrType());
				
				// Class pointer type.
				argTypes.push_back(typeGen.getI8PtrType());
				
				// Hash value type.
				argTypes.push_back(typeGen.getI64Type());
				
				// Arguments struct pointer type.
				argTypes.push_back(typeGen.getI8PtrType());
				
				return typeGen.getVoidFunctionType(argTypes);
			}
			
			ArgInfo getStubArgInfo() {
				const bool hasReturnVarArgument = true;
				const bool hasContextArgument = true;
				
				std::vector<llvm_abi::Type> standardArguments;
				standardArguments.push_back(llvm_abi::Type::Integer(llvm_abi::Int64));
				standardArguments.push_back(llvm_abi::Type::Pointer());
				
				return ArgInfo(hasReturnVarArgument, hasContextArgument, std::move(standardArguments), {nullptr, nullptr});
			}
			
			void setStubAttributes(llvm::Function* llvmFunction) {
				{
					// Return value pointer attributes.
					llvmFunction->addAttribute(1, llvm::Attribute::StructRet);
					llvmFunction->addAttribute(1, llvm::Attribute::NoAlias);
					llvmFunction->addAttribute(1, llvm::Attribute::NoCapture);
				}
				
				{
					// Arguments struct pointer attributes.
					llvmFunction->addAttribute(4, llvm::Attribute::NoAlias);
					llvmFunction->addAttribute(4, llvm::Attribute::NoCapture);
				}
			}
			
			llvm::Value* makeArgsStruct(Function& function, const std::vector<SEM::Value*>& args) {
				if (args.empty()) {
					// Don't allocate struct when it's not needed.
					return ConstantGenerator(function.module()).getNullPointer(TypeGenerator(function.module()).getI8PtrType());
				}
				
				std::vector<llvm::Value*> llvmArgs;
				for (auto arg: args) {
					llvmArgs.push_back(genValue(function, arg));
				}
				
				std::vector<llvm::Type*> llvmArgsTypes;
				for (auto arg: llvmArgs) {
					llvmArgsTypes.push_back(arg->getType());
				}
				
				llvm::Type* llvmArgsStructType = TypeGenerator(function.module()).getStructType(llvmArgsTypes);
				
				llvm::Value* llvmArgsStructPtr = function.getEntryBuilder().CreateAlloca(llvmArgsStructType);
				for (size_t offset = 0; offset < args.size(); offset++) {
					llvm::Value* argPtr = function.getBuilder().CreateConstInBoundsGEP2_32(
						llvmArgsStructPtr, 0, offset);
					function.getBuilder().CreateStore(llvmArgs.at(offset), argPtr);
				}
				
				return llvmArgsStructPtr;
			}
			
			llvm::Value* generateCall(Function& function, llvm::Value* objectPtr, llvm::Value* vtablePtr, llvm::Value* hashValue, const std::vector<llvm::Value*>& args) {
				ConstantGenerator constantGen(function.module());
				
				// Calculate the slot for the virtual call.
				const auto vtableSizeValue = constantGen.getI64(VTABLE_SIZE);
				
				const auto vtableOffsetValue = function.getBuilder().CreateURem(methodHashValue, vtableSizeValue, "vtableOffset");
				
				// Get a pointer to the slot.
				std::vector<llvm::Value*> vtableEntryGEP;
				vtableEntryGEP.push_back(constantGen.getI32(0));
				vtableEntryGEP.push_back(constantGen.getI32(2));
				vtableEntryGEP.push_back(llvmVtableOffsetValue);
				
				const auto vtableEntryPointer = function.getBuilder().CreateInBoundsGEP(vtablePointer, vtableEntryGEP, "vtableEntryPointer");
				
				// Load the slot.
				const auto methodFunctionPointer =
					function.getBuilder().CreateLoad(vtableEntryPointer, "methodFunctionPointer");
				
				// Cast the loaded pointer to the stub function type.
				const auto stubFunctionPtrType = getStubFunctionType(function.module())->getPointerTo();
						 
				const auto castedMethodFunctionPointer = function.getBuilder().CreatePointerCast(methodFunctionPointer, stubFunctionPtrType, "castedMethodFunctionPointer");
				
				// i8
				const auto i8PtrType = TypeGenerator(function.module()).getI8PtrType();
				
				// Put together the arguments.
				std::vector<llvm::Value*> parameters;
				
				SEM::Type* functionType = methodValue->type()->getInterfaceMethodFunctionType();
				SEM::Type* returnType = functionType->getFunctionReturnType();
				
				// If the return type isn't void, allocate space on the stack for the return value.
				llvm::Value* returnVarValue = nullptr;
				if (!returnType->isVoid()) {
					returnVarValue = genAlloca(function, returnType);
					parameters.push_back(function.getBuilder().CreatePointerCast(returnVarValue, i8PtrType, "castedReturnVarPtr"));
				} else {
					parameters.push_back(constantGen.getNullPointer(i8PtrType));
				}
				
				// Pass in the object pointer.
				parameters.push_back(function.getBuilder().CreatePointerCast(llvmObjectPointer, i8PtrType, "castedObjectPtr"));
				
				// Pass in the method hash value.
				parameters.push_back(methodHashValue);
				
				// Store all the arguments into a struct on the stack,
				// and pass the pointer to the stub.
				const auto argsStructPtr = makeArgsStruct(function, args);
				parameters.push_back(function.getBuilder().CreatePointerCast(argsStructPtr, i8PtrType, "castedArgsStructPtr"));
				
				// Call the stub function.
				const auto callReturnValue = function.getBuilder().CreateCall(castedMethodFunctionPointer, parameters);
				
				// If the return type isn't void, load the return value from the stack.
				if (returnVarValue != NULL) {
					return genLoad(function, llvmReturnVarValue, returnType);
				} else {
					return llvmCallReturnValue;
				}
			}
			
			llvm::Constant* generateVTableSlot(Module& module, SEM::Type* parentType, const std::vector<SEM::Function*>& methods) {
				ConstantGenerator constGen(module);
				
				if (methods.empty()) {
					return constGen.getNullPointer(TypeGenerator(module).getI8PtrType());
				}
				
				TypeGenerator typeGen(module);
				
				const auto linkage = llvm::Function::PrivateLinkage;
				
				llvm::Function* llvmFunction = createLLVMFunction(module, getStubFunctionType(module), linkage, "__slot_conflict_resolution_stub");
				
				setStubAttributes(llvmFunction);
				
				Function function(module, *llvmFunction, getStubArgInfo());
				
				const auto llvmHashValue = function.getArg(0);
				const auto llvmOpaqueArgsStructPtr = function.getArg(1);
				
				for (const auto semMethod : methods) {
					auto callMethodBasicBlock = function.createBasicBlock("callMethod");
					auto tryNextMethodBasicBlock = function.createBasicBlock("tryNextMethod");
					
					const auto methodHash = CreateMethodNameHash(semMethod->name().last());
					
					const auto cmpValue = function.getBuilder().CreateICmpEQ(llvmHashValue, constGen.getI64(methodHash));
					function.getBuilder().CreateCondBr(cmpValue, callMethodBasicBlock, tryNextMethodBasicBlock);
					
					function.selectBasicBlock(callMethodBasicBlock);
					
					const auto llvmMethod = genFunction(module, parentType, semMethod);
					
					SEM::Type* functionType = semMethod->type();
					SEM::Type* returnType = functionType->getFunctionReturnType();
					
					std::vector<llvm::Value*> parameters;
					
					// If the function uses a return value pointer, just pass
					// the pointer we received from our caller.
					if (!isTypeSizeAlwaysKnown(module, returnType)) {
						llvm::Type* returnVarPointerType = llvmMethod->getFunctionType()->getParamType(0);
						llvm::Value* llvmCastReturnVar = function.getBuilder().CreatePointerCast(function.getReturnVar(), returnVarPointerType, "castedReturnVar");
						parameters.push_back(llvmCastReturnVar);
					}
					
					// If this is not a static method, pass the object pointer.
					if (!semMethod->isStaticMethod()) {
						const size_t objectPointerOffset = parameters.size();
						llvm::Type* objectPointerType = llvmMethod->getFunctionType()->getParamType(objectPointerOffset);
						parameters.push_back(function.getBuilder().CreatePointerCast(function.getContextValue(), objectPointerType));
					}
					
					const size_t numArgs = functionType->getFunctionParameterTypes().size();
					const size_t rawArgsOffset = parameters.size();
					
					// Build the args struct type.
					std::vector<llvm::Type*> llvmArgsTypes;
					for (size_t offset = 0; offset < numArgs; offset++) {
						assert(offset < llvmMethod->getFunctionType()->getNumParams());
						const size_t rawOffset = rawArgsOffset + offset;
						llvmArgsTypes.push_back(llvmMethod->getFunctionType()->getParamType(rawOffset));
					}
					
					llvm::Type* llvmArgsStructPtrType = typeGen.getStructType(llvmArgsTypes)->getPointerTo();
					
					// Cast the args struct pointer.
					llvm::Value* llvmArgsStructPtr =
						function.getBuilder().CreatePointerCast(llvmOpaqueArgsStructPtr, llvmArgsStructPtrType, "castedArgsStructPtr");
					
					// Extract the arguments.
					for (size_t offset = 0; offset < numArgs; offset++) {
						llvm::Value* argPtr = function.getBuilder().CreateConstInBoundsGEP2_32(
							llvmArgsStructPtr, 0, offset);
						parameters.push_back(function.getBuilder().CreateLoad(argPtr, "extractedArg"));
					}
					
					// Call the method.
					llvm::Value* llvmCallReturnValue = function.getBuilder().CreateCall(llvmMethod, parameters);
					
					// Store return value.
					if (isTypeSizeAlwaysKnown(module, returnType) && !returnType->isVoid()) {
						const auto returnValuePointerType = llvmMethod->getFunctionType()->getReturnType()->getPointerTo();
						const auto llvmCastReturnVar = function.getBuilder().CreatePointerCast(function.getReturnVar(), returnValuePointerType, "castedReturnVar");
						function.getBuilder().CreateStore(llvmCallReturnValue, llvmCastReturnVar);
					}
					
					function.getBuilder().CreateRetVoid();
					
					function.selectBasicBlock(tryNextMethodBasicBlock);
				}
				
				// Terminate function with unreachable
				// (notifies optimiser that this should
				// never be reached...).
				function.getBuilder().CreateUnreachable();
				
				return llvmFunction;
			}
			
		}
		
	}
}
	
