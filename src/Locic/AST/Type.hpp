#ifndef LOCIC_AST_TYPE_HPP
#define LOCIC_AST_TYPE_HPP

#include <string>
#include <vector>
#include <Locic/Name.hpp>

namespace AST {

	struct Type {
		enum TypeEnum {
			UNDEFINED,
			VOID,
			NULLT,
			NAMED,
			POINTER,
			FUNCTION
		} typeEnum;
		
		static const bool MUTABLE = true;
		static const bool CONST = false;
		
		bool isMutable;
		
		struct {
			Locic::Name name;
		} namedType;
		
		struct {
			// Type that is being pointed to.
			Type* targetType;
		} pointerType;
		
		struct {
			bool isVarArg;
			Type* returnType;
			std::vector<Type*> parameterTypes;
		} functionType;
		
		inline Type()
			: typeEnum(VOID),
			  isMutable(MUTABLE) { }
			  
		inline Type(TypeEnum e, bool m)
			: typeEnum(e),
			  isMutable(m) { }
		
		inline static Type* UndefinedType() {
			return new Type(UNDEFINED, MUTABLE);
		}
			  
		inline static Type* VoidType() {
			return new Type(VOID, MUTABLE);
		}
		
		inline static Type* Named(bool isMutable, const Locic::Name& name) {
			Type* type = new Type(NAMED, isMutable);
			type->namedType.name = name;
			return type;
		}
		
		inline static Type* Pointer(Type* targetType) {
			Type* type = new Type(POINTER, MUTABLE);
			type->pointerType.targetType = targetType;
			return type;
		}
		
		inline static Type* Function(bool isMutable, Type* returnType, const std::vector<Type*>& parameterTypes) {
			Type* type = new Type(FUNCTION, isMutable);
			type->functionType.isVarArg = false;
			type->functionType.returnType = returnType;
			type->functionType.parameterTypes = parameterTypes;
			return type;
		}
		
		inline static Type* VarArgFunction(bool isMutable, Type* returnType, const std::vector<Type*>& parameterTypes) {
			Type* type = new Type(FUNCTION, isMutable);
			type->functionType.isVarArg = true;
			type->functionType.returnType = returnType;
			type->functionType.parameterTypes = parameterTypes;
			return type;
		}
		
		inline Type* applyTransitiveConst() {
			Type* t = this;
			
			while(true) {
				t->isMutable = false;
				
				if(t->typeEnum == POINTER) {
					t = t->pointerType.targetType;
				} else {
					break;
				}
			}
			
			return this;
		}
		
		inline std::string toString() const {
			std::string str;
		
			bool bracket = false;
			if(!isMutable){
				str += "const ";
				bracket = true;
			}
			
			if(bracket){
				str += "(";
			}
	
			switch(typeEnum){
				case UNDEFINED:
				{
					str += "[undefined]";
					break;
				}
				case VOID:
				{
					str += "void";
					break;
				}
				case NULLT:
				{
					str += "null";
					break;
				}
				case NAMED:
					str += std::string("[named type: ") + namedType.name.toString() + std::string("]");
					break;
				case POINTER:
					str += pointerType.targetType->toString();
					str += " *";
					break;
				case FUNCTION:
				{
					str += "(";
					str += functionType.returnType->toString();
					str += ")(";
			
					for(std::size_t i = 0; i < functionType.parameterTypes.size(); i++){
						if(i != 0){
							str += ", ";
						}
						str += functionType.parameterTypes.at(i)->toString();
					}
					
					str += ")";
					break;
				}
				default:
					break;
			}
	
			if(bracket) str += ")";
			return str;
		}
		
	};
	
}

#endif
