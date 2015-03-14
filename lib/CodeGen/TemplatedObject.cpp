#include <assert.h>

#include <locic/SEM.hpp>

#include <locic/CodeGen/LLVMIncludes.hpp>
#include <locic/CodeGen/Support.hpp>
#include <locic/CodeGen/TemplatedObject.hpp>

namespace locic {

	namespace CodeGen {
		
		TemplatedObject TemplatedObject::TypeInstance(const SEM::TypeInstance* const typeInstance) {
			assert(typeInstance != nullptr);
			TemplatedObject object(TYPEINSTANCE);
			object.data_.typeInstance = typeInstance;
			return object;
		}
		
		TemplatedObject TemplatedObject::Function(const SEM::TypeInstance* const parentTypeInstance, SEM::Function* const function) {
			TemplatedObject object(FUNCTION);
			object.data_.functionPair.parentTypeInstance = parentTypeInstance;
			object.data_.functionPair.function = function;
			return object;
		}
		
		TemplatedObject::Kind TemplatedObject::kind() const {
			return kind_;
		}
		
		bool TemplatedObject::isTypeInstance() const {
			return kind_ == TYPEINSTANCE;
		}
		
		bool TemplatedObject::isFunction() const {
			return kind_ == FUNCTION;
		}
		
		const SEM::TypeInstance* TemplatedObject::typeInstance() const {
			assert(isTypeInstance());
			return data_.typeInstance;
		}
		
		const SEM::TypeInstance* TemplatedObject::parentTypeInstance() const {
			assert(isFunction());
			return data_.functionPair.parentTypeInstance;
		}
		
		SEM::Function* TemplatedObject::function() const {
			assert(isFunction());
			return data_.functionPair.function;
		}
		
		bool TemplatedObject::operator==(const TemplatedObject& other) const {
			if (kind() != other.kind()) {
				return false;
			}
			
			switch (kind()) {
				case TYPEINSTANCE:
					return typeInstance() == other.typeInstance();
				case FUNCTION:
					return parentTypeInstance() == other.parentTypeInstance() &&
						function() == other.function();
				default:
					llvm_unreachable("Unknown templated object kind.");
			}
		}
		
		bool TemplatedObject::operator!=(const TemplatedObject& other) const {
			return !(*this == other);
		}
		
		bool TemplatedObject::operator<(const TemplatedObject& other) const {
			if (kind() != other.kind()) {
				return kind() < other.kind();
			}
			
			switch (kind()) {
				case TYPEINSTANCE:
					return typeInstance() < other.typeInstance();
				case FUNCTION:
					if (parentTypeInstance() != other.parentTypeInstance()) {
						return parentTypeInstance() < other.parentTypeInstance();
					}
					return function() < other.function();
				default:
					llvm_unreachable("Unknown templated object kind.");
			}
		}
				
		TemplatedObject::TemplatedObject(Kind pKind)
			: kind_(pKind) { }
		
		TemplateInst TemplateInst::Type(const SEM::Type* const rawType) {
			const auto type = rawType->resolveAliases();
			assert(type->isObject());
			return TemplateInst(TemplatedObject::TypeInstance(type->getObjectType()), arrayRef(type->templateArguments()));
		}
		
		TemplateInst TemplateInst::Function(const SEM::Type* parentType, SEM::Function* function, llvm::ArrayRef<SEM::Value> functionArgs) {
			if (parentType != nullptr) {
				assert(parentType->isObject());
				llvm::SmallVector<SEM::Value, 10> args;
				for (const auto& arg: parentType->templateArguments()) {
					args.push_back(arg.copy());
				}
				for (size_t i = 0; i < functionArgs.size(); i++) {
					args.push_back(functionArgs[i].copy());
				}
				return TemplateInst(TemplatedObject::Function(parentType->getObjectType(), function), args);
			} else {
				return TemplateInst(TemplatedObject::Function(nullptr, function), functionArgs);
			}
		}
		
		TemplateInst::TemplateInst(TemplatedObject pObject, llvm::ArrayRef<SEM::Value> pArguments)
			: object_(pObject) {
				for (size_t i = 0; i < pArguments.size(); i++) {
					arguments_.push_back(pArguments[i].copy());
				}
			}
		
		TemplateInst TemplateInst::copy() const {
			return TemplateInst(object(), arguments());
		}
		
		TemplatedObject TemplateInst::object() const {
			return object_;
		}
		
		llvm::ArrayRef<SEM::Value> TemplateInst::arguments() const {
			return arrayRef(arguments_);
		}
		
		bool TemplateInst::operator<(const TemplateInst& other) const {
			if (object() != other.object()) {
				return object() < other.object();
			}
			
			if (arguments().size() != other.arguments().size()) {
				return arguments().size() < other.arguments().size();
			}
			
			for (size_t i = 0; i < arguments().size(); i++) {
				if (arguments()[i] != other.arguments()[i]) {
					return arguments()[i].hash() < other.arguments()[i].hash();
				}
			}
			
			return false;
		}
		
	}
	
}

