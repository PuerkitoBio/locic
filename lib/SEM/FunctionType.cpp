#include <string>

#include <locic/SEM/Context.hpp>
#include <locic/SEM/FunctionType.hpp>
#include <locic/SEM/Predicate.hpp>
#include <locic/SEM/TypeArray.hpp>
#include <locic/SEM/Type.hpp>

#include <locic/Support/Hasher.hpp>
#include <locic/Support/MakeString.hpp>

namespace locic {
	
	namespace SEM {
		
		class Type;
		
		FunctionAttributes::FunctionAttributes(const bool argIsVarArg,
		                                       const bool argIsMethod,
		                                       const bool argIsTemplated,
		                                       Predicate argNoExceptPredicate)
		: isVarArg_(argIsVarArg),
		isMethod_(argIsMethod),
		isTemplated_(argIsTemplated),
		noExceptPredicate_(std::move(argNoExceptPredicate)) { }
		
		FunctionAttributes FunctionAttributes::copy() const {
			return FunctionAttributes(isVarArg(),
			                          isMethod(),
			                          isTemplated(),
			                          noExceptPredicate().copy());
		}
		
		bool FunctionAttributes::isVarArg() const {
			return isVarArg_;
		}
		
		bool FunctionAttributes::isMethod() const {
			return isMethod_;
		}
		
		bool FunctionAttributes::isTemplated() const {
			return isTemplated_;
		}
		
		const Predicate& FunctionAttributes::noExceptPredicate() const {
			return noExceptPredicate_;
		}
		
		std::string FunctionAttributes::toString() const {
			return makeString("FunctionAttributes("
			                  "isVarArg: %s, "
			                  "isMethod: %s, "
			                  "isTemplated: %s, "
			                  "noExceptPredicate: %s)",
			                  isVarArg() ? "true" : "false",
			                  isMethod() ? "true" : "false",
			                  isTemplated() ? "true" : "false",
			                  noExceptPredicate().toString().c_str());
		}
		
		std::size_t FunctionAttributes::hash() const {
			Hasher hasher;
			hasher.add(isVarArg());
			hasher.add(isMethod());
			hasher.add(isTemplated());
			hasher.add(noExceptPredicate());
			return hasher.get();
		}
		
		bool FunctionAttributes::operator==(const FunctionAttributes& other) const {
			return isVarArg() == other.isVarArg() &&
			       isMethod() == other.isMethod() &&
			       isTemplated() == other.isTemplated() &&
			       noExceptPredicate() == other.noExceptPredicate();
		}
		
		FunctionTypeData::FunctionTypeData(FunctionAttributes attributes,
		                                   const Type* const returnType,
		                                   TypeArray parameterTypes)
		: attributes_(std::move(attributes)),
		returnType_(returnType),
		parameterTypes_(std::move(parameterTypes)) { }
		
		FunctionTypeData FunctionTypeData::copy() const {
			return FunctionTypeData(attributes().copy(),
			                        returnType(),
			                        parameterTypes().copy());
		}
		
		const Context& FunctionTypeData::context() const {
			return returnType()->context();
		}
		
		const FunctionAttributes& FunctionTypeData::attributes() const {
			return attributes_;
		}
		
		const Type* FunctionTypeData::returnType() const {
			return returnType_;
		}
		
		const TypeArray& FunctionTypeData::parameterTypes() const {
			return parameterTypes_;
		}
		
		std::string FunctionTypeData::toString() const {
			return makeString("FunctionType("
			                  "attributes: %s, "
			                  "returnType: %s, "
			                  "parameterTypes: %s)",
			                  attributes().toString().c_str(),
			                  returnType()->toString().c_str(),
			                  makeArrayPtrString(parameterTypes()).c_str());
		}
		
		std::size_t FunctionTypeData::hash() const {
			Hasher hasher;
			hasher.add(attributes());
			hasher.add(returnType());
			hasher.add(parameterTypes());
			return hasher.get();
		}
		
		bool FunctionTypeData::operator==(const FunctionTypeData& other) const {
			return attributes() == other.attributes() &&
			       returnType() == other.returnType() &&
			       parameterTypes() == other.parameterTypes();
		}
		
		FunctionType::FunctionType(FunctionAttributes attributes, const Type* const returnType, TypeArray parameterTypes)
		: data_(nullptr) {
			FunctionTypeData functionTypeData(std::move(attributes),
			                                  returnType,
			                                  std::move(parameterTypes));
			*this = returnType->context().getFunctionType(std::move(functionTypeData));
		}
		
		FunctionType FunctionType::substitute(const TemplateVarMap& templateVarMap) const {
			const auto substitutedReturnType = returnType()->substitute(templateVarMap);
			
			bool changed = (substitutedReturnType != returnType());
			
			TypeArray substitutedParameterTypes;
			
			for (const auto parameterType: parameterTypes()) {
				const auto substitutedParameterType = parameterType->substitute(templateVarMap);
				changed |= (substitutedParameterType != parameterType);
				substitutedParameterTypes.push_back(substitutedParameterType);
			}
			
			auto noExceptPredicate = attributes().noExceptPredicate().substitute(templateVarMap);
			changed |= (noExceptPredicate != attributes().noExceptPredicate());
			
			if (changed) {
				FunctionAttributes newAttributes(attributes().isVarArg(),
				                                 attributes().isMethod(),
				                                 attributes().isTemplated(),
				                                 std::move(noExceptPredicate));
				return FunctionType(std::move(newAttributes), substitutedReturnType, std::move(substitutedParameterTypes));
			} else {
				return *this;
			}
		}
		
		std::size_t FunctionType::hash() const {
			Hasher hasher;
			hasher.add(data_);
			return hasher.get();
		}
		
	}
	
}
