#include <cassert>

#include <locic/CodeGen/ArgInfo.hpp>
#include <locic/CodeGen/ConstantGenerator.hpp>
#include <locic/CodeGen/Function.hpp>
#include <locic/CodeGen/FunctionTranslationStub.hpp>
#include <locic/CodeGen/GenFunctionCall.hpp>
#include <locic/CodeGen/GenType.hpp>
#include <locic/CodeGen/Memory.hpp>
#include <locic/CodeGen/Module.hpp>
#include <locic/CodeGen/TypeGenerator.hpp>
#include <locic/CodeGen/UnwindAction.hpp>
#include <locic/SEM/FunctionType.hpp>
#include <locic/Support/Utils.hpp>

namespace locic {

	namespace CodeGen {
		
		bool areTypesFunctionallyEquivalent(Module& module,
		                                    const SEM::Type* const firstType,
		                                    const SEM::Type* const secondType) {
			return firstType == secondType ||
				genArgType(module, firstType) == genArgType(module, secondType);
		}
		
		bool doFunctionTypesMatch(Module& module,
		                          SEM::FunctionType firstType,
		                          SEM::FunctionType secondType) {
			assert(firstType.attributes().isTemplated() == secondType.attributes().isTemplated());
			
			const auto& firstParameterTypes = firstType.parameterTypes();
			const auto& secondParameterTypes = secondType.parameterTypes();
			assert(firstParameterTypes.size() == secondParameterTypes.size());
			
			for (size_t i = 0; i < firstParameterTypes.size(); i++) {
				const auto& firstParameterType = firstParameterTypes[i];
				const auto& secondParameterType = secondParameterTypes[i];
				if (!areTypesFunctionallyEquivalent(module, firstParameterType, secondParameterType)) {
					return false;
				}
			}
			
			return areTypesFunctionallyEquivalent(module, firstType.returnType(), secondType.returnType());
		}
		
		llvm::Value* genTranslatedFunctionPointer(Function& functionGenerator,
		                                          llvm::Function* function,
		                                          SEM::FunctionType functionType,
		                                          SEM::FunctionType translatedFunctionType) {
			assert(functionType.attributes().isTemplated() == translatedFunctionType.attributes().isTemplated());
			
			auto& module = functionGenerator.module();
			
			if (doFunctionTypesMatch(module, functionType, translatedFunctionType)) {
				const auto functionPtrType = genFunctionType(module, translatedFunctionType)->getPointerTo();
				return functionGenerator.getBuilder().CreatePointerCast(function, functionPtrType);
			} else {
				return genFunctionTranslationStub(module, function, functionType, translatedFunctionType);
			}
		}
		
		using TranslatedArguments = llvm::SmallVector<llvm::Value*, 10>;
		
		llvm::Value* getSingleTranslatedArgument(Function& functionGenerator,
		                                         llvm::Value* const argValue,
		                                         const SEM::Type* const parameterType,
		                                         const SEM::Type* const translatedParameterType) {
			auto& builder = functionGenerator.getBuilder();
			auto& module = functionGenerator.module();
			
			const auto llvmParameterType = genArgType(module, parameterType);
			const auto llvmTranslatedParameterType = genArgType(module, translatedParameterType);
			
			// Being able to pass the inner parameter type by value must imply
			// that the outer parameter type can be passed by value.
			assert(checkImplies(canPassByValue(module, parameterType), canPassByValue(module, translatedParameterType)));
			
			if (!canPassByValue(module, parameterType) && canPassByValue(module, translatedParameterType)) {
				// Create an alloca to hinner the parameter so it can be passed by pointer
				// into the target function.
				const auto argAlloca = genAlloca(functionGenerator, translatedParameterType);
				genStore(functionGenerator, argValue, argAlloca, translatedParameterType);
				return builder.CreatePointerCast(argAlloca, llvmParameterType);
			} else if (llvmParameterType->isPointerTy() && llvmTranslatedParameterType->isPointerTy()) {
				// Make sure our pointers have the right type.
				return builder.CreatePointerCast(argValue, llvmParameterType);
			} else {
				return argValue;
			}
		}
		
		TranslatedArguments getTranslatedArguments(Function& functionGenerator,
		                                           SEM::FunctionType functionType,
		                                           SEM::FunctionType translatedFunctionType,
		                                           llvm::Value* const returnVar,
		                                           const ArgInfo& argInfo,
		                                           const ArgInfo& translatedArgInfo) {
			auto& module = functionGenerator.module();
			auto& builder = functionGenerator.getBuilder();
			
			TranslatedArguments args;
			
			if (argInfo.hasReturnVarArgument()) {
				const auto llvmReturnType = genArgType(module, functionType.returnType());
				args.push_back(builder.CreatePointerCast(returnVar, llvmReturnType));
			}
			
			if (translatedArgInfo.hasTemplateGeneratorArgument()) {
				args.push_back(functionGenerator.getTemplateGenerator());
			}
			
			if (translatedArgInfo.hasContextArgument()) {
				args.push_back(functionGenerator.getRawContextValue());
			}
			
			const auto& parameterTypes = functionType.parameterTypes();
			const auto& translatedParameterTypes = translatedFunctionType.parameterTypes();
			assert(parameterTypes.size() == translatedParameterTypes.size());
			
			for (size_t i = 0; i < parameterTypes.size(); i++) {
				const auto argValue = functionGenerator.getArg(i);
				const auto& parameterType = parameterTypes[i];
				const auto& translatedParameterType = translatedParameterTypes[i];
				
				args.push_back(getSingleTranslatedArgument(functionGenerator, argValue, parameterType, translatedParameterType));
			}
			
			return args;
		}
		
		llvm::Function* createTranslationStubFunction(Module& module,
		                                              llvm::Function* function,
		                                              const ArgInfo& translatedArgInfo) {
			const auto functionName = module.getCString("translateStub_") + function->getName();
			const auto linkage = llvm::Function::InternalLinkage;
			const auto llvmFunction = createLLVMFunction(module, translatedArgInfo, linkage, functionName);
			
			// Always inline if possible.
			llvmFunction->addFnAttr(llvm::Attribute::AlwaysInline);
			
			return llvmFunction;
		}
		
		llvm::Function* genFunctionTranslationStub(Module& module,
		                                           llvm::Function* function,
		                                           SEM::FunctionType functionType,
		                                           SEM::FunctionType translatedFunctionType) {
			const auto llvmTranslatedFunctionType = genFunctionType(module, translatedFunctionType);
			
			const auto stubIdPair = std::make_pair(function, llvmTranslatedFunctionType);
			const auto iterator = module.functionPtrStubMap().find(stubIdPair);
			if (iterator != module.functionPtrStubMap().end()) {
				return iterator->second;
			}
			
			const auto argInfo = getFunctionArgInfo(module, functionType);
			const auto translatedArgInfo = getFunctionArgInfo(module, translatedFunctionType);
			
			const auto llvmFunction = createTranslationStubFunction(module, function, translatedArgInfo);
			
			module.functionPtrStubMap().insert(std::make_pair(stubIdPair, llvmFunction));
			
			Function functionGenerator(module, *llvmFunction, translatedArgInfo);
			auto& builder = functionGenerator.getBuilder();
			
			const auto returnVar =
				argInfo.hasReturnVarArgument() ?
					translatedArgInfo.hasReturnVarArgument() ?
						functionGenerator.getReturnVar() :
						genAlloca(functionGenerator, translatedFunctionType.returnType())
					: nullptr;
			
			TranslatedArguments arguments = getTranslatedArguments(functionGenerator,
			                                                       functionType,
			                                                       translatedFunctionType,
			                                                       returnVar,
			                                                       argInfo,
			                                                       translatedArgInfo);
			
			const auto result = genRawFunctionCall(functionGenerator, argInfo, function, arguments);
			
			if (argInfo.hasReturnVarArgument() && !translatedArgInfo.hasReturnVarArgument()) {
				builder.CreateRet(builder.CreateLoad(returnVar));
			} else {
				if (llvmTranslatedFunctionType->getReturnType()->isVoidTy()) {
					builder.CreateRetVoid();
				} else {
					if (result->getType()->isPointerTy()) {
						builder.CreateRet(builder.CreatePointerCast(result, llvmTranslatedFunctionType->getReturnType()));
					} else {
						builder.CreateRet(result);
					}
				}
			}
			
			return llvmFunction;
		}
		
	}
	
}

