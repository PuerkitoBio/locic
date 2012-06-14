#include <stdio.h>
#include <Locic/SEM.h>
#include <Locic/SemanticAnalysis/CanCast.hpp>
#include <Locic/SemanticAnalysis/Context.hpp>

namespace Locic{

	namespace SemanticAnalysis{

SEM::Value * CastValueToType(Locic_SemanticContext * context, SEM::Value * value, SEM::Type * type){
	// Try a plain implicit cast.
	if(CanDoImplicitCast(context, value->type, type) == NULL){
		return new SEM::Cast(type, value);
	}
			
	// Can't just cast from one type to the other =>
	// must attempt copying (to remove lvalue/const).
	if(CanDoImplicitCopy(context, value->type)){
		// If possible, create a copy.
		SEM::Value * copiedValue = new SEM::Value(*value);
		const char * err = CanDoImplicitCast(context, copiedValue->type, type);
		if(err == NULL){
			// Copying worked.
			return SEM::Value::Cast(type, copiedValue);
		}
		printf("%s", err);
	}
	
	// Copying also failed.
	return NULL;
}

const char * CanDoImplicitCast(Locic_SemanticContext * context, SEM::Type * sourceType, SEM::Type * destType){
	if(destType->typeEnum == SEM::Type::VOID){
		// Everything can be cast to void.
		return NULL;
	}
	
	if(sourceType->typeEnum != destType->typeEnum && sourceType->typeEnum != SEM::Type::NULLT){
		return "Semantic Analysis Error: Types don't match.\n";
	}
	
	// Check for const-correctness.
	if(sourceType->isMutable == false && destType->isMutable == true){
		return "Semantic Analysis Error: Const-correctness violation.\n";
	}
	
	if(sourceType->isLValue == true && destType->isLValue == false){
		return "Semantic Analysis Error: Cannot cast from lvalue to rvalue.\n";
	}
	
	switch(sourceType->typeEnum){
		case SEM::Type::NULLT:
		{
			if(destType->typeEnum == SEM::Type::BASIC){
				return "Semantic Analysis Error: Cannot cast null to basic type.";
			}
			
			if(destType->typeEnum == SEM::Type::NAMED){
				if(destType->namedType.typeInstance->typeEnum == SEM::TYPEINST_STRUCT){
					return "Semantic Analysis Error: Cannot cast null to struct type.";
				}
			}
			
			return NULL;
		}
		case SEM::Type::BASIC:
		{
			if(sourceType->basicType.typeEnum != destType->basicType.typeEnum){
				return "Semantic Analysis Error: Cannot implicitly convert between different basic types.\n";
			}
			return NULL;
		}
		case SEM::Type::NAMED:
		{
			if(sourceType->namedType.typeInstance != destType->namedType.typeInstance){
				return "Semantic Analysis Error: Cannot convert between incompatible named types.\n";
			}
			return NULL;
		}
		case SEM::Type::PTR:
		{
			SEM::Type * sourcePtr = sourceType->ptrType.ptrType;
			SEM::Type * destPtr = destType->ptrType.ptrType;
			
			// Check for const-correctness inside pointers (e.g. to prevent T** being cast to const T**).
			if(sourcePtr->typeEnum == SEM::Type::PTR && destPtr->typeEnum == SEM::Type::PTR){
				SEM::Type * sourcePtrType = sourcePtr->ptrType.ptrType;
				SEM::Type * destPtrType = destPtr->ptrType.ptrType;
				if(sourcePtrType->isMutable == true && destPtrType->isMutable == false){
					if(sourcePtr->isMutable == true && destPtr->isMutable == true){
						return "Semantic Analysis Error: Const-correctness violation on pointer type.\n";
					}
				}
			}
			
			return CanDoImplicitCast(context, sourcePtr, destPtr);
		}
		case SEM::Type::FUNC:
		{
			const char * err = CanDoImplicitCast(context, sourceType->funcType.returnType, destType->funcType.returnType);
			if(err != NULL) return err;
			
			const std::list<SEM::Type *>& sourceList = sourceType->funcType.parameterTypes;
			const std::list<SEM::Type *>& destList = destType->funcType.parameterTypes;
			
			if(sourceList.size() != destList.size()){
				return "Semantic Analysis Error: Number of parameters doesn't match in function type.\n";
			}
			
			std::list<SEM::Type *>::const_iterator sourceIt = sourceList.begin(),
				destIt = destList.begin();
			
			while(sourceIt != sourceList.end()){
				if(CanDoImplicitCast(context, *sourceIt, *destIt) != NULL){
					return "Semantic Analysis Error: Cannot cast parameter type in function type.\n";
				}
				++sourceIt;
				++destIt;
			}
			
			return NULL;
		}
		default:
			return "Internal Compiler Error: Unknown type enum value.";
	}
}

bool CanDoImplicitCopy(SemanticContext& context, SEM::Type * type){
	switch(type->typeEnum){
		case SEM::Type::BASIC:
		case SEM::Type::PTR:
		case SEM::Type::FUNC:
			// Basic, pointer and function types can be copied implicitly.
			return true;
		default:
			return false;
	}
}

bool CanDoExplicitCast(SemanticContext& context, SEM::Type * sourceType, SEM::Type * destType){
	const char * err = CanDoImplicitCast(context, sourceType, destType);
	if(err == NULL){
		return true;
	}
	
	if(sourceType->typeEnum != destType->typeEnum){
		return false;
	}
	
	switch(sourceType->typeEnum){
		case SEM::Type::BASIC:
		{
			if(sourceType->basicType.typeEnum == destType->basicType.typeEnum){
				return true;
			}
			
			// Int -> Float.
			if(sourceType->basicType.typeEnum == SEM::Type::BasicType::INT && destType->basicType.typeEnum == SEM::Type::BasicType::FLOAT){
				return true;
			}
			
			// Float -> Int.
			if(sourceType->basicType.typeEnum == SEM::Type::BasicType::FLOAT && destType->basicType.typeEnum == SEM::Type::BasicType::INT){
				return true;
			}
			
			return false;
		}
		case SEM::Type::NAMED:
		case SEM::Type::PTR:
		case SEM::Type::FUNC:
		{
			printf("%s", err);
			return false;
		}
		default:
			return false;
	}
}

}

}




