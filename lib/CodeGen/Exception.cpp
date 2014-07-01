#include <vector>

#include <locic/CodeGen/ConstantGenerator.hpp>
#include <locic/CodeGen/Exception.hpp>
#include <locic/CodeGen/Function.hpp>
#include <locic/CodeGen/LLVMIncludes.hpp>
#include <locic/CodeGen/Module.hpp>
#include <locic/CodeGen/ScopeExitActions.hpp>
#include <locic/CodeGen/Support.hpp>
#include <locic/CodeGen/TypeGenerator.hpp>

namespace locic {

	namespace CodeGen {
	
		llvm::Function* getExceptionAllocateFunction(Module& module) {
			const std::string functionName = "__loci_allocate_exception";
			const auto result = module.getFunctionMap().tryGet(functionName);
			
			if (result.hasValue()) {
				return result.getValue();
			}
			
			TypeGenerator typeGen(module);
			const auto functionType = typeGen.getFunctionType(typeGen.getI8PtrType(), std::vector<llvm::Type*> { getSizeType(module.getTargetInfo()) });
			
			const auto function = createLLVMFunction(module, functionType, llvm::Function::ExternalLinkage, functionName);
			function->addFnAttr(llvm::Attribute::NoUnwind);
			
			module.getFunctionMap().insert(functionName, function);
			
			return function;
		}
		
		llvm::Function* getExceptionFreeFunction(Module& module) {
			const std::string functionName = "__loci_free_exception";
			const auto result = module.getFunctionMap().tryGet(functionName);
			
			if (result.hasValue()) {
				return result.getValue();
			}
			
			TypeGenerator typeGen(module);
			llvm::Type* const argTypes[] = { typeGen.getI8PtrType() };
			const auto functionType = typeGen.getFunctionType(typeGen.getVoidType(), argTypes);
			
			const auto function = createLLVMFunction(module, functionType, llvm::Function::ExternalLinkage, functionName);
			function->addFnAttr(llvm::Attribute::NoUnwind);
			
			module.getFunctionMap().insert(functionName, function);
			
			return function;
		}
		
		llvm::Function* getExceptionThrowFunction(Module& module) {
			const std::string functionName = "__loci_throw";
			const auto result = module.getFunctionMap().tryGet(functionName);
			
			if (result.hasValue()) {
				return result.getValue();
			}
			
			TypeGenerator typeGen(module);
			llvm::Type* const argTypes[] = { typeGen.getI8PtrType(), typeGen.getI8PtrType(), typeGen.getI8PtrType() };
			const auto functionType = typeGen.getVoidFunctionType(argTypes);
			
			const auto function = createLLVMFunction(module, functionType, llvm::Function::ExternalLinkage, functionName);
			
			module.getFunctionMap().insert(functionName, function);
			
			return function;
		}
		
		llvm::Function* getExceptionRethrowFunction(Module& module) {
			const std::string functionName = "__loci_rethrow";
			const auto result = module.getFunctionMap().tryGet(functionName);
			
			if (result.hasValue()) {
				return result.getValue();
			}
			
			TypeGenerator typeGen(module);
			llvm::Type* const argTypes[] = { typeGen.getI8PtrType() };
			const auto functionType = typeGen.getVoidFunctionType(argTypes);
			
			const auto function = createLLVMFunction(module, functionType, llvm::Function::ExternalLinkage, functionName);
			
			module.getFunctionMap().insert(functionName, function);
			
			return function;
		}
		
		llvm::Function* getExceptionPersonalityFunction(Module& module) {
			const std::string functionName = "__loci_personality_v0";
			const auto result = module.getFunctionMap().tryGet(functionName);
			
			if (result.hasValue()) {
				return result.getValue();
			}
			
			TypeGenerator typeGen(module);
			const auto functionType = typeGen.getVarArgsFunctionType(typeGen.getI32Type(), std::vector<llvm::Type*> {});
			
			const auto function = createLLVMFunction(module, functionType, llvm::Function::ExternalLinkage, functionName);
			function->addFnAttr(llvm::Attribute::NoUnwind);
			
			module.getFunctionMap().insert(functionName, function);
			
			return function;
		}
		
		llvm::Function* getExceptionPtrFunction(Module& module) {
			const std::string functionName = "__loci_get_exception";
			const auto result = module.getFunctionMap().tryGet(functionName);
			
			if (result.hasValue()) {
				return result.getValue();
			}
			
			TypeGenerator typeGen(module);
			const auto functionType = typeGen.getFunctionType(typeGen.getI8PtrType(), std::vector<llvm::Type*> {typeGen.getI8PtrType()});
			
			const auto function = createLLVMFunction(module, functionType, llvm::Function::ExternalLinkage, functionName);
			function->addFnAttr(llvm::Attribute::NoUnwind);
			
			module.getFunctionMap().insert(functionName, function);
			
			return function;
		}
		
		bool anyExceptionActions(Function& function, bool isRethrow) {
			const auto& unwindStack = function.unwindStack();
			
			// Look for any actions to be performed
			// if an exception is thrown.
			for (size_t i = 0; i < unwindStack.size(); i++) {
				const size_t pos = unwindStack.size() - i - 1;
				const auto& action = unwindStack.at(pos);
				
				const bool isExceptionState = true;
				if (action.isCatch() || isActiveAction(action, isExceptionState, isRethrow)) {
					return true;
				}
			}
			
			return false;
		}
		
		bool anyExceptionCleanupActions(Function& function, bool isRethrow) {
			const auto& unwindStack = function.unwindStack();
			
			// Look for any actions to be performed
			// if an exception is thrown.
			for (size_t i = 0; i < unwindStack.size(); i++) {
				const size_t pos = unwindStack.size() - i - 1;
				const auto& action = unwindStack.at(pos);
				
				const bool isExceptionState = true;
				if (isActiveAction(action, isExceptionState, isRethrow)) {
					return true;
				}
			}
			
			return false;
		}
		
		llvm::BasicBlock* genLandingPad(Function& function, bool isRethrow) {
			assert(anyExceptionActions(function, isRethrow));
			
			const auto latestLandingPad = function.latestLandingPad();
			// TODO: also support the rethrow case.
			if (latestLandingPad != nullptr && !isRethrow) {
				return latestLandingPad->getParent();
			}
			
			auto& module = function.module();
			const auto& unwindStack = function.unwindStack();
			
			TypeGenerator typeGen(module);
			const auto landingPadType = typeGen.getStructType(std::vector<llvm::Type*> {typeGen.getI8PtrType(), typeGen.getI32Type()});
			const auto personalityFunction = getExceptionPersonalityFunction(module);
			
			// Find all catch types on the stack.
			llvm::SmallVector<llvm::Constant*, 5> catchTypes;
			
			for (size_t i = 0; i < unwindStack.size(); i++) {
				const size_t pos = unwindStack.size() - i - 1;
				const auto& action = unwindStack.at(pos);
				
				if (action.isCatch()) {
					catchTypes.push_back(action.catchTypeInfo());
				}
			}
			
			const auto currentBB = function.getBuilder().GetInsertBlock();
			
			const auto landingPadBB = function.createBasicBlock("");
			
			function.selectBasicBlock(landingPadBB);
			
			const auto landingPad = function.getBuilder().CreateLandingPad(landingPadType, personalityFunction, catchTypes.size());
			landingPad->setCleanup(anyExceptionCleanupActions(function, isRethrow));
			
			for (size_t i = 0; i < catchTypes.size(); i++) {
				landingPad->addClause(ConstantGenerator(module).getPointerCast(catchTypes[i], typeGen.getI8PtrType()));
			}
			
			// Unwind stack due to exception.
			genExceptionUnwind(function, landingPad, isRethrow);
			
			// TODO: also support the rethrow case.
			if (!isRethrow) {
				function.setLatestLandingPad(landingPad);
			}
			
			function.selectBasicBlock(currentBB);
			
			return landingPadBB;
		}
		
		void genExceptionUnwind(Function& function, llvm::Value* exceptionInfo, bool isRethrow) {
			const auto& unwindStack = function.unwindStack();
			llvm::BasicBlock* catchBlock = nullptr;
			
			// Perform all scope exit actions until the next catch block.
			for (size_t i = 0; i < unwindStack.size(); i++) {
				const size_t pos = unwindStack.size() - i - 1;
				const auto& action = unwindStack.at(pos);
				
				if (action.isCatch()) {
					catchBlock = action.catchBlock();
					break;
				}
				
				const bool isExceptionState = true;
				performScopeExitAction(function, pos, isExceptionState, isRethrow);
			}
			
			if (catchBlock != nullptr) {
				// Jump to the next catch block and store the exception
				// information so it can be used by the catch block.
				function.getBuilder().CreateStore(exceptionInfo, function.exceptionInfo());
				function.getBuilder().CreateBr(catchBlock);
			} else {
				// There isn't a catch block; resume unwinding.
				function.getBuilder().CreateResume(exceptionInfo);
			}
		}
		
		void scheduleExceptionDestroy(Function& function, llvm::Value* exceptionPtrValue) {
			function.pushUnwindAction(UnwindAction::DestroyException(exceptionPtrValue));
		}
		
		llvm::Constant* getTypeNameGlobal(Module& module, const std::string& typeName) {
			ConstantGenerator constGen(module);
			TypeGenerator typeGen(module);
			
			// Generate the name of the exception type as a constant C string.
			const auto typeNameConstant = constGen.getString(typeName.c_str());
			const auto typeNameType = typeGen.getArrayType(typeGen.getI8Type(), typeName.size() + 1);
			const auto typeNameGlobal = module.createConstGlobal("type_name", typeNameType, llvm::GlobalValue::PrivateLinkage, typeNameConstant);
			typeNameGlobal->setAlignment(1);
			
			// Convert array to a pointer.
			return constGen.getGetElementPtr(typeNameGlobal, std::vector<llvm::Constant*> {constGen.getI32(0), constGen.getI32(0)});
		}
		
		llvm::Constant* genCatchInfo(Module& module, SEM::TypeInstance* catchTypeInstance) {
			assert(catchTypeInstance->isException());
			
			const auto typeName = catchTypeInstance->name().toString();
			const auto typeNameGlobalPtr = getTypeNameGlobal(module, typeName);
			
			ConstantGenerator constGen(module);
			TypeGenerator typeGen(module);
			
			// Create a constant struct {i32, i8*} for the catch type info.
			const auto typeInfoType = typeGen.getStructType(std::vector<llvm::Type*> {typeGen.getI32Type(), typeGen.getI8PtrType()});
			
			// Calculate offset to check based on number of parents.
			size_t offset = 0;
			auto currentInstance = catchTypeInstance;
			
			while (currentInstance->parent() != NULL) {
				offset++;
				currentInstance = currentInstance->parent();
			}
			
			const auto castedTypeNamePtr = constGen.getPointerCast(typeNameGlobalPtr, typeGen.getI8PtrType());
			const auto typeInfoValue = constGen.getStruct(typeInfoType, std::vector<llvm::Constant*> {constGen.getI32(offset), castedTypeNamePtr});
			
			const auto typeInfoGlobal = module.createConstGlobal("catch_type_info", typeInfoType, llvm::GlobalValue::PrivateLinkage, typeInfoValue);
			
			return constGen.getGetElementPtr(typeInfoGlobal, std::vector<llvm::Constant*> {constGen.getI32(0), constGen.getI32(0)});
		}
		
		llvm::Constant* genThrowInfo(Module& module, SEM::TypeInstance* throwTypeInstance) {
			assert(throwTypeInstance->isException());
			
			std::vector<std::string> typeNames;
			
			// Add type names in REVERSE order.
			auto currentInstance = throwTypeInstance;
			
			while (currentInstance != NULL) {
				typeNames.push_back(currentInstance->name().toString());
				currentInstance = currentInstance->parent();
			}
			
			assert(!typeNames.empty());
			
			// Since type names were added in reverse
			// order, fix this by reversing the array.
			std::reverse(std::begin(typeNames), std::end(typeNames));
			
			ConstantGenerator constGen(module);
			TypeGenerator typeGen(module);
			
			// Create a constant struct {i32, i8*} for the throw type info.
			const auto typeNameArrayType = typeGen.getArrayType(typeGen.getI8PtrType(), typeNames.size());
			const auto typeInfoType = typeGen.getStructType(std::vector<llvm::Type*> {typeGen.getI32Type(), typeNameArrayType});
			
			std::vector<llvm::Constant*> typeNameConstants;
			
			for (const auto& typeName : typeNames) {
				const auto typeNameGlobalPtr = getTypeNameGlobal(module, typeName);
				const auto castedTypeNamePtr = constGen.getPointerCast(typeNameGlobalPtr, typeGen.getI8PtrType());
				typeNameConstants.push_back(castedTypeNamePtr);
			}
			
			const auto typeNameArray = constGen.getArray(typeNameArrayType, typeNameConstants);
			const auto typeInfoValue = constGen.getStruct(typeInfoType, std::vector<llvm::Constant*> {constGen.getI32(typeNames.size()), typeNameArray});
			
			const auto typeInfoGlobal = module.createConstGlobal("throw_type_info", typeInfoType, llvm::GlobalValue::PrivateLinkage, typeInfoValue);
			
			return constGen.getGetElementPtr(typeInfoGlobal, std::vector<llvm::Constant*> {constGen.getI32(0), constGen.getI32(0)});
		}
		
		TryScope::TryScope(Function& function, llvm::BasicBlock* catchBlock, const std::vector<llvm::Constant*>& catchTypeList)
			: function_(function), catchCount_(catchTypeList.size()) {
			assert(!catchTypeList.empty());
			for (size_t i = 0; i < catchTypeList.size(); i++) {
				// Push in reverse order.
				const auto catchType = catchTypeList.at(catchTypeList.size() - i - 1);
				function_.pushUnwindAction(UnwindAction::CatchException(catchBlock, catchType));
			}
		}
		
		TryScope::~TryScope() {
			for (size_t i = 0; i < catchCount_; i++) {
				assert(function_.unwindStack().back().isCatch());
				function_.popUnwindAction();
			}
		}
		
	}
	
}

