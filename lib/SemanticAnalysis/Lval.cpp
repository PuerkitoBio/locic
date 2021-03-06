#include <stdexcept>

#include <locic/Debug.hpp>
#include <locic/SEM.hpp>

#include <locic/SemanticAnalysis/Context.hpp>
#include <locic/SemanticAnalysis/Exception.hpp>
#include <locic/SemanticAnalysis/Lval.hpp>
#include <locic/SemanticAnalysis/Ref.hpp>
#include <locic/SemanticAnalysis/ScopeElement.hpp>
#include <locic/SemanticAnalysis/ScopeStack.hpp>
#include <locic/SemanticAnalysis/TypeProperties.hpp>

namespace locic {

	namespace SemanticAnalysis {
	
		const SEM::Type* makeValueLvalType(Context& context, const SEM::Type* const valueType) {
			return getBuiltInType(context, context.getCString("value_lval"), { valueType })->createLvalType(valueType);
		}
		
		const SEM::Type* makeFinalLvalType(Context& context, const SEM::Type* const valueType) {
			return getBuiltInType(context, context.getCString("final_lval"), { valueType })->createLvalType(valueType);
		}
		
		const SEM::Type* makeLvalType(Context& context, const bool isFinal, const SEM::Type* const valueType) {
			if (getDerefType(valueType)->isLval()) {
				return valueType;
			}
			
			if (isFinal) {
				return makeFinalLvalType(context, valueType);
			} else {
				return makeValueLvalType(context, valueType);
			}
		}
		
		size_t getLvalCount(const SEM::Type* type) {
			type = getDerefType(type);
			
			size_t count = 0;
			while (type->isLval()) {
				count++;
				type = getDerefType(type->lvalTarget());
			}
			return count;
		}
		
		bool canDissolveType(Context& context, const SEM::Type* const rawType) {
			const auto type = getSingleDerefType(rawType);
			return type->isLval() && type->isObjectOrTemplateVar() && supportsDissolve(context, type);
		}
		
		bool canDissolveValue(Context& context, const SEM::Value& value) {
			return canDissolveType(context, value.type());
		}
		
		SEM::Value dissolveLval(Context& context, SEM::Value value, const Debug::SourceLocation& location) {
			assert (canDissolveValue(context, value));
			assert(value.type()->isLval() || (value.type()->isRef() && value.type()->isBuiltInReference() && value.type()->refTarget()->isLval()));
			if (!value.type()->isRef()) {
				value = bindReference(context, std::move(value));
			}
			return CallValue(context, GetSpecialMethod(context, std::move(value), context.getCString("dissolve"), location), {}, location);
		}
		
		SEM::Value tryDissolveValue(Context& context, SEM::Value value, const Debug::SourceLocation& location) {
			if (canDissolveValue(context, value)) {
				return dissolveLval(context, std::move(value), location);
			} else {
				return value;
			}
		}
		
	}
	
}


