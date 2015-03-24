#include <assert.h>

#include <string>

#include <locic/CodeGen/ArgInfo.hpp>
#include <locic/CodeGen/Function.hpp>
#include <locic/CodeGen/GenType.hpp>
#include <locic/CodeGen/Module.hpp>
#include <locic/CodeGen/Move.hpp>
#include <locic/CodeGen/Primitives.hpp>
#include <locic/CodeGen/SizeOf.hpp>
#include <locic/CodeGen/TypeGenerator.hpp>
#include <locic/CodeGen/TypeSizeKnowledge.hpp>
#include <locic/CodeGen/UnwindAction.hpp>

namespace locic {

	namespace CodeGen {
	
		void genPrimitiveMoveCall(Function& function, const SEM::Type* type, llvm::Value* sourceValue, llvm::Value* destValue, llvm::Value* positionValue) {
			assert(sourceValue->getType()->isPointerTy());
			assert(destValue->getType()->isPointerTy());
			
			auto& module = function.module();
			
			/*
			auto& builder = function.getBuilder();
			const auto& typeName = type->getObjectType()->name().last();
			
			if (typeName == "value_lval") {
				const auto targetType = type->templateArguments().front().typeRefType();
				const bool typeSizeIsKnown = isTypeSizeKnownInThisModule(module, targetType);
				
				const auto castType = typeSizeIsKnown ? genPointerType(module, type) : TypeGenerator(module).getI8PtrType();
				const auto sourceObjectPointer = builder.CreatePointerCast(sourceValue, castType);
				const auto destObjectPointer = builder.CreatePointerCast(destValue, castType);
				
				// Check the 'liveness indicator' which indicates whether
				// child value's move method should be run.
				const auto livenessIndicatorPtr = typeSizeIsKnown ?
					builder.CreateConstInBoundsGEP2_32(sourceObjectPointer, 0, 1) :
					builder.CreateInBoundsGEP(sourceObjectPointer, genSizeOf(function, targetType));
				const auto castLivenessIndicatorPtr = builder.CreatePointerCast(livenessIndicatorPtr, TypeGenerator(module).getI1Type()->getPointerTo());
				const auto isLive = builder.CreateLoad(castLivenessIndicatorPtr);
				
				const auto isLiveBB = function.createBasicBlock("is_live");
				const auto afterBB = function.createBasicBlock("");
				
				builder.CreateCondBr(isLive, isLiveBB, afterBB);
				
				// If it is live, run the child value's move method.
				function.selectBasicBlock(isLiveBB);
				genMoveCall(function, targetType, sourceObjectPointer, destObjectPointer, positionValue);
				builder.CreateBr(afterBB);
				
				function.selectBasicBlock(afterBB);
			} else if (typeName == "final_lval" || typeName == "member_lval") {
				const auto targetType = type->templateArguments().front().typeRefType();
				genMoveCall(function, targetType, sourceValue, destValue, positionValue);
			}*/
			
			assert(type->isPrimitive());
			const auto canonicalMethodName = module.getCString("__moveto");
			const auto functionType = type->getObjectType()->functions().at(canonicalMethodName)->type();
			
			const auto methodName = module.getCString("__move_to");
			MethodInfo methodInfo(type, methodName, functionType, {});
			
			PendingResultArray arguments;
			arguments.push_back(sourceValue);
			arguments.push_back(destValue);
			arguments.push_back(positionValue);
			
			llvm::Value* const hintResultValue = nullptr;
			
			(void) genTrivialPrimitiveFunctionCall(function, std::move(methodInfo), std::move(arguments), hintResultValue);
		}
		
		bool primitiveTypeHasCustomMove(Module& module, const SEM::Type* type) {
			assert(type->isPrimitive());
			const auto& name = type->getObjectType()->name().first();
			const auto kind = module.primitiveKind(name);
			return (kind == PrimitiveMemberLval || kind == PrimitiveValueLval) && typeHasCustomMove(module, type->templateArguments().front().typeRefType());
		}
		
		bool primitiveTypeInstanceHasCustomMove(Module& module, const SEM::TypeInstance* typeInstance) {
			assert(typeInstance->isPrimitive());
			const auto& name = typeInstance->name().first();
			const auto kind = module.primitiveKind(name);
			return (kind == PrimitiveMemberLval || kind == PrimitiveValueLval);
		}
		
	}
	
}

