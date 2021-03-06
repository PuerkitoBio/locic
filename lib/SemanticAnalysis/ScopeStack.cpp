#include <string>
#include <vector>

#include <locic/Debug.hpp>
#include <locic/Support/Name.hpp>
#include <locic/SEM.hpp>

#include <locic/SemanticAnalysis/Context.hpp>
#include <locic/SemanticAnalysis/Exception.hpp>
#include <locic/SemanticAnalysis/ScopeStack.hpp>

namespace locic {

	namespace SemanticAnalysis {
	
		Name getCurrentName(const ScopeStack& scopeStack) {
			Name name = Name::Absolute();
			
			for (size_t i = 0; i < scopeStack.size(); i++) {
				const auto& element = scopeStack[i];
				
				if (element.hasName()) {
					name = name + element.name();
				}
			}
			
			return name;
		}
		
		SEM::TypeInstance* lookupParentType(const ScopeStack& scopeStack) {
			for (size_t i = 0; i < scopeStack.size(); i++) {
				const auto pos = scopeStack.size() - i - 1;
				const auto& element = scopeStack[pos];
				
				if (element.isTypeInstance()) {
					return element.typeInstance();
				}
			}
			
			return nullptr;
		}
		
		SEM::Function* lookupParentFunction(const ScopeStack& scopeStack) {
			for (size_t i = 0; i < scopeStack.size(); i++) {
				const auto pos = scopeStack.size() - i - 1;
				const auto& element = scopeStack[pos];
				
				if (element.isFunction()) {
					return element.function();
				}
			}
			
			return nullptr;
		}
		
		const SEM::TemplatedObject& lookupTemplatedObject(const ScopeStack& scopeStack) {
			for (size_t i = 0; i < scopeStack.size(); i++) {
				const auto pos = scopeStack.size() - i - 1;
				const auto& element = scopeStack[pos];
				
				if (element.isFunction()) {
					return *(element.function());
				} else if (element.isAlias()) {
					return *(element.alias());
				} else if (element.isTypeInstance()) {
					return *(element.typeInstance());
				}
			}
			
			throw std::runtime_error("Failed to find templated object.");
		}
		
		const SEM::Type* getParentFunctionReturnType(const ScopeStack& scopeStack) {
			const auto function = lookupParentFunction(scopeStack);
			assert(function != nullptr);
			return function->type().returnType();
		}
		
		const SEM::Type* getBuiltInType(Context& context, const String& typeName, SEM::TypeArray templateArgs) {
			const auto& scopeStack = context.scopeStack();
			const auto rootElement = scopeStack[0];
			assert(rootElement.isNamespace());
			
			const auto iterator = rootElement.nameSpace()->items().find(typeName);
			assert(iterator != rootElement.nameSpace()->items().end() && "Failed to find built-in type!");
			
			const auto& value = iterator->second;
			assert(value.isTypeInstance() || value.isAlias());
			
			SEM::ValueArray templateArgValues;
			templateArgValues.reserve(templateArgs.size());
			
			const auto& templateVariables = (value.isTypeInstance() ? value.typeInstance().templateVariables() : value.alias().templateVariables());
			
			for (size_t i = 0; i < templateArgs.size(); i++) {
				const auto& argVar = templateVariables[i];
				const auto& argType = templateArgs[i];
				templateArgValues.push_back(SEM::Value::TypeRef(argType, argVar->type()->createStaticRefType(argType)));
			}
			
			if (value.isTypeInstance()) {
				assert(templateArgs.size() == value.typeInstance().templateVariables().size());
				return SEM::Type::Object(&(value.typeInstance()), std::move(templateArgValues));
			} else {
				assert(templateArgs.size() == value.alias().templateVariables().size());
				return SEM::Type::Alias(value.alias(), std::move(templateArgValues));
			}
		}
		
		const SEM::Type* getBuiltInTypeWithValueArgs(Context& context, const String& typeName, SEM::ValueArray templateArgValues) {
			const auto& scopeStack = context.scopeStack();
			const auto rootElement = scopeStack[0];
			assert(rootElement.isNamespace());
			
			const auto iterator = rootElement.nameSpace()->items().find(typeName);
			if (iterator == rootElement.nameSpace()->items().end()) {
				throw std::runtime_error(makeString("Failed to find built-in type '%s'.", typeName.c_str()));
			}
			
			const auto& value = iterator->second;
			assert(value.isTypeInstance() || value.isAlias());
			
			if (value.isTypeInstance()) {
				assert(templateArgValues.size() == value.typeInstance().templateVariables().size());
				return SEM::Type::Object(&(value.typeInstance()), std::move(templateArgValues));
			} else {
				assert(templateArgValues.size() == value.alias().templateVariables().size());
				return SEM::Type::Alias(value.alias(), std::move(templateArgValues));
			}
		}
		
	}
	
}

