#include <vector>

#include <locic/CodeGen/Module.hpp>
#include <locic/CodeGen/Primitives.hpp>
#include <locic/CodeGen/TypeSizeKnowledge.hpp>
#include <locic/SEM/TemplateVar.hpp>
#include <locic/SEM/Type.hpp>
#include <locic/SEM/TypeInstance.hpp>
#include <locic/SEM/Var.hpp>

namespace locic {

	namespace CodeGen {
	
		bool isObjectTypeSizeKnownInThisModule(Module& module, const SEM::TypeInstance* const objectType) {
			if (objectType->isEnum() || objectType->isStruct() || objectType->isUnion()) {
				// C types can only contain known size members.
				return true;
			} else if (objectType->isClassDef() || objectType->isDatatype() || objectType->isException()) {
				// All members of the type must have a known size
				// for it to have a known size.
				for (auto var: objectType->variables()) {
					if (!isTypeSizeKnownInThisModule(module, var->type())) {
						return false;
					}
				}
				return true;
			} else if (objectType->isUnionDatatype()) {
				for (auto variantTypeInstance: objectType->variants()) {
					if (!isObjectTypeSizeKnownInThisModule(module, variantTypeInstance)) {
						return false;
					}
				}
				return true;
			} else {
				return false;
			}
		}
		
		bool isTypeSizeKnownInThisModule(Module& module, const SEM::Type* const type) {
			switch (type->kind()) {
				case SEM::Type::OBJECT: {
					const auto objectType = type->getObjectType();
					if (objectType->isPrimitive()) {
						return isPrimitiveTypeSizeKnownInThisModule(module, type);
					} else {
						return isObjectTypeSizeKnownInThisModule(module, objectType);
					}
				}
				case SEM::Type::TEMPLATEVAR:
					return false;
				case SEM::Type::ALIAS:
					return isTypeSizeKnownInThisModule(module, type->resolveAliases());
				default:
					llvm_unreachable("Unknown SEM type kind enum.");
			}
		}
		
		bool isObjectTypeSizeAlwaysKnown(Module& module, const SEM::TypeInstance* const objectType) {
			if (objectType->isEnum() || objectType->isStruct() || objectType->isUnion()) {
				// C types can only contain known size members.
				return true;
			} else if (objectType->isDatatype() || objectType->isException()) {
				// All members of the type must have a known size
				// for it to have a known size.
				for (auto var: objectType->variables()) {
					if (!isTypeSizeAlwaysKnown(module, var->type())) {
						return false;
					}
				}
				return true;
			} else if (objectType->isUnionDatatype()) {
				for (auto variantTypeInstance: objectType->variants()) {
					if (!isObjectTypeSizeAlwaysKnown(module, variantTypeInstance)) {
						return false;
					}
				}
				return true;
			} else {
				return false;
			}
		}
		
		bool isTypeSizeAlwaysKnown(Module& module, const SEM::Type* const type) {
			switch (type->kind()) {
				case SEM::Type::OBJECT:
					if (type->isPrimitive()) {
						return isPrimitiveTypeSizeAlwaysKnown(module, type);
					} else {
						return isObjectTypeSizeAlwaysKnown(module, type->getObjectType());
					}
				case SEM::Type::TEMPLATEVAR:
					return false;
				case SEM::Type::ALIAS:
					return isTypeSizeAlwaysKnown(module, type->resolveAliases());
				default:
					llvm_unreachable("Unknown SEM type kind enum.");
			}
		}
		
	}
	
}

