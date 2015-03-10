#include <string>

#include <locic/MakeString.hpp>
#include <locic/Map.hpp>
#include <locic/Support/Name.hpp>
#include <locic/Support/String.hpp>

#include <locic/SEM/Context.hpp>
#include <locic/SEM/Function.hpp>
#include <locic/SEM/ModuleScope.hpp>
#include <locic/SEM/Predicate.hpp>
#include <locic/SEM/TemplateVar.hpp>
#include <locic/SEM/Type.hpp>
#include <locic/SEM/TypeInstance.hpp>
#include <locic/SEM/Var.hpp>

namespace locic {

	namespace SEM {
	
		TypeInstance::TypeInstance(Context& c, Name n, Kind k, ModuleScope m)
			: context_(c),
			name_(std::move(n)),
			kind_(std::move(k)),
			moduleScope_(std::move(m)),
			parent_(nullptr),
			requiresPredicate_(Predicate::True()),
			hasCustomMove_(false) { }
		
		Context& TypeInstance::context() const {
			return context_;
		}
			
		const Name& TypeInstance::name() const {
			return name_;
		}
		
		TypeInstance::Kind TypeInstance::kind() const {
			return kind_;
		}
		
		const SEM::ModuleScope& TypeInstance::moduleScope() const {
			return moduleScope_;
		}
		
		bool TypeInstance::isPrimitive() const {
			return kind() == PRIMITIVE;
		}
		
		bool TypeInstance::isEnum() const {
			return kind() == ENUM;
		}
		
		bool TypeInstance::isStruct() const {
			return kind() == STRUCT;
		}
		
		bool TypeInstance::isUnion() const {
			return kind() == UNION;
		}
		
		bool TypeInstance::isClassDecl() const {
			return kind() == CLASSDECL;
		}
		
		bool TypeInstance::isClassDef() const {
			return kind() == CLASSDEF;
		}
		
		bool TypeInstance::isClass() const {
			return isClassDecl() || isClassDef();
		}
		
		bool TypeInstance::isDatatype() const {
			return kind() == DATATYPE;
		}
		
		bool TypeInstance::isUnionDatatype() const {
			return kind() == UNION_DATATYPE;
		}
		
		bool TypeInstance::isInterface() const {
			return kind() == INTERFACE;
		}
		
		bool TypeInstance::isException() const {
			return kind() == EXCEPTION;
		}
		
		const Type* TypeInstance::selfType() const {
			// TODO: remove const_cast.
			return SEM::Type::Object(const_cast<TypeInstance*>(this), selfTemplateArgs());
		}
		
		TypeArray TypeInstance::selfTemplateArgs() const {
			TypeArray templateArgs;
			templateArgs.reserve(templateVariables().size());
			
			for (const auto templateVar: templateVariables()) {
				// Refer to the template variables of this type instance.
				templateArgs.push_back(SEM::Type::TemplateVarRef(templateVar));
			}
			
			return templateArgs;
		}
		
		std::vector<TemplateVar*>& TypeInstance::templateVariables() {
			return templateVariables_;
		}
		
		const std::vector<TemplateVar*>& TypeInstance::templateVariables() const {
			return templateVariables_;
		}
		
		FastMap<String, TemplateVar*>& TypeInstance::namedTemplateVariables() {
			return namedTemplateVariables_;
		}
		
		const FastMap<String, TemplateVar*>& TypeInstance::namedTemplateVariables() const {
			return namedTemplateVariables_;
		}
		
		const Predicate& TypeInstance::requiresPredicate() const {
			return requiresPredicate_;
		}
		
		void TypeInstance::setRequiresPredicate(Predicate predicate) {
			requiresPredicate_ = std::move(predicate);
		}
		
		std::vector<TypeInstance*>& TypeInstance::variants() {
			return variants_;
		}
		
		const std::vector<TypeInstance*>& TypeInstance::variants() const {
			return variants_;
		}
		
		FastMap<String, Var*>& TypeInstance::namedVariables() {
			return namedVariables_;
		}
		
		const FastMap<String, Var*>& TypeInstance::namedVariables() const {
			return namedVariables_;
		}
		
		std::vector<Var*>& TypeInstance::variables() {
			return variables_;
		}
		
		const std::vector<Var*>& TypeInstance::variables() const {
			return variables_;
		}
		
		FastMap<String, Function*>& TypeInstance::functions() {
			return functions_;
		}
		
		const FastMap<String, Function*>& TypeInstance::functions() const {
			return functions_;
		}
		
		TypeArray TypeInstance::constructTypes() const {
			TypeArray types;
			if (isUnion()) {
				// Unions are constructed with no arguments (zero initialised).
				return types;
			}
			
			types.reserve(variables().size());
			for (const auto var: variables()) {
				types.push_back(var->constructType());
			}
			return types;
		}
		
		void TypeInstance::setHasCustomMove(bool pHasCustomMove) {
			hasCustomMove_ = pHasCustomMove;
		}
		
		bool TypeInstance::hasCustomMove() const {
			return hasCustomMove_;
		}
		
		void TypeInstance::setParent(const Type* pParent) {
			assert(pParent->isObject());
			parent_ = pParent;
		}
		
		const Type* TypeInstance::parent() const {
			return parent_;
		}
		
		std::string TypeInstance::refToString() const {
			switch (kind()) {
				case PRIMITIVE:
					return makeString("Primitive(name: %s)",
									  name().toString().c_str());
									  
				case ENUM:
					return makeString("Enum(name: %s)",
									  name().toString().c_str());
									  
				case STRUCT:
					return makeString("Struct(name: %s)",
									  name().toString().c_str());
									  
				case UNION:
					return makeString("Union(name: %s)",
									  name().toString().c_str());
									  
				case CLASSDECL:
					return makeString("ClassDecl(name: %s)",
									  name().toString().c_str());
									  
				case CLASSDEF:
					return makeString("ClassDef(name: %s)",
									  name().toString().c_str());
									  
				case DATATYPE:
					return makeString("Datatype(name: %s)",
									  name().toString().c_str());
				
				case UNION_DATATYPE:
					return makeString("UnionDatatype(name: %s)",
									  name().toString().c_str());
									  
				case INTERFACE:
					return makeString("Interface(name: %s)",
									  name().toString().c_str());
				
				case EXCEPTION:
					return makeString("Exception(name: %s)",
									  name().toString().c_str());
			}
			
			throw std::logic_error("Unknown SEM::TypeInstance kind.");
		}
		
		std::string TypeInstance::toString() const {
			return makeString("TypeInstance(ref: %s, "
							  "templateVariables: %s, variables: %s, "
							  "functions: %s)",
							  refToString().c_str(),
							  makeArrayPtrString(templateVariables_).c_str(),
							  makeArrayPtrString(variables_).c_str(),
							  makeMapPtrString(functions_).c_str());
		}
		
	}
	
}

