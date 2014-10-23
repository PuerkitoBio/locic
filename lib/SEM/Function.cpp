#include <locic/String.hpp>

#include <locic/SEM/Function.hpp>
#include <locic/SEM/Scope.hpp>
#include <locic/SEM/Type.hpp>
#include <locic/SEM/Var.hpp>

namespace locic {

	namespace SEM {
	
		Function::Function(Name pName, ModuleScope pModuleScope)
			: isPrimitive_(false),
			  isMethod_(false),
			  isStaticMethod_(false),
			  isConstMethod_(false),
			  type_(nullptr),
			  name_(std::move(pName)),
			  moduleScope_(std::move(pModuleScope)) { }
		
		const Name& Function::name() const {
			return name_;
		}
		
		void Function::setType(const Type* pType) {
			assert(type_ == nullptr);
			type_ = pType;
		}
		
		const Type* Function::type() const {
			return type_;
		}
		
		const ModuleScope& Function::moduleScope() const {
			return moduleScope_;
		}
		
		bool Function::isDeclaration() const {
			return !isDefinition();
		}
		
		bool Function::isDefinition() const {
			return scope_.get() != nullptr;
		}
		
		void Function::setPrimitive(bool pIsPrimitive) {
			isPrimitive_ = pIsPrimitive;
		}
		
		bool Function::isPrimitive() const {
			return isPrimitive_;
		}
		
		void Function::setMethod(bool pIsMethod) {
			isMethod_ = pIsMethod;
		}
		
		bool Function::isMethod() const {
			return isMethod_;
		}
		
		void Function::setStaticMethod(bool pIsStaticMethod) {
			// isStaticMethod() implies isMethod().
			assert(!pIsStaticMethod || isMethod());
			isStaticMethod_ = pIsStaticMethod;
		}
		
		bool Function::isStaticMethod() const {
			return isStaticMethod_;
		}
		
		void Function::setConstMethod(bool pIsConstMethod) {
			// isConstMethod() implies isMethod().
			assert(!pIsConstMethod || isMethod());
			isConstMethod_ = pIsConstMethod;
		}
		
		bool Function::isConstMethod() const {
			return isConstMethod_;
		}
		
		std::vector<TemplateVar*>& Function::templateVariables() {
			return templateVariables_;
		}
		
		const std::vector<TemplateVar*>& Function::templateVariables() const {
			return templateVariables_;
		}
		
		std::map<std::string, TemplateVar*>& Function::namedTemplateVariables() {
			return namedTemplateVariables_;
		}
		
		const std::map<std::string, TemplateVar*>& Function::namedTemplateVariables() const {
			return namedTemplateVariables_;
		}
		
		TemplateRequireMap& Function::typeRequirements() {
			return typeRequirements_;
		}
		
		const TemplateRequireMap& Function::typeRequirements() const {
			return typeRequirements_;
		}
		
		void Function::setParameters(std::vector<Var*> pParameters) {
			parameters_ = std::move(pParameters);
		}
		
		const std::vector<Var*>& Function::parameters() const {
			return parameters_;
		}
		
		std::map<std::string, Var*>& Function::namedVariables() {
			return namedVariables_;
		}
		
		const std::map<std::string, Var*>& Function::namedVariables() const {
			return namedVariables_;
		}
		
		const Scope& Function::scope() const {
			assert(isDefinition());
			return *scope_;
		}
		
		Function* Function::createTemplatedDecl() const {
			assert(templateVariables().empty());
			assert(type() != nullptr);
			
			const auto newFunction = new SEM::Function(name(), moduleScope());
			newFunction->setMethod(isMethod());
			newFunction->setStaticMethod(isStaticMethod());
			newFunction->setConstMethod(isConstMethod());
			newFunction->setType(type()->makeTemplatedFunction());
			newFunction->setParameters(parameters());
			return newFunction;
		}
		
		Function* Function::fullSubstitute(Name declName, const TemplateVarMap& templateVarMap) const {
			assert(isDeclaration());
			assert(templateVariables().empty());
			assert(type() != nullptr);
			
			const auto newFunction = new SEM::Function(std::move(declName), moduleScope());
			newFunction->setMethod(isMethod());
			newFunction->setStaticMethod(isStaticMethod());
			newFunction->setConstMethod(isConstMethod());
			newFunction->setType(type()->substitute(templateVarMap));
			
			// Parameter types need to be substituted.
			std::vector<Var*> substitutedParam;
			substitutedParam.reserve(parameters().size());
			
			for (const auto param: parameters()) {
				substitutedParam.push_back(param->substitute(templateVarMap));
			}
			
			newFunction->setParameters(std::move(substitutedParam));
			
			return newFunction;
		}
		
		void Function::setScope(std::unique_ptr<Scope> newScope) {
			assert(scope_.get() == nullptr);
			assert(newScope.get() != nullptr);
			scope_ = std::move(newScope);
		}
		
		std::string Function::toString() const {
			if (isDeclaration()) {
				return makeString("FunctionDeclaration(name: %s, isMethod: %s, isStatic: %s, isConst: %s, type: %s)",
								  name().toString().c_str(),
								  isMethod() ? "Yes" : "No",
								  isStaticMethod() ? "Yes" : "No",
								  isConstMethod() ? "Yes" : "No",
								  type()->toString().c_str());
			} else {
				return makeString("FunctionDefinition(name: %s, isMethod: %s, isStatic: %s, isConst: %s, type: %s, scope: %s)",
								  name().toString().c_str(),
								  isMethod() ? "Yes" : "No",
								  isStaticMethod() ? "Yes" : "No",
								  isConstMethod() ? "Yes" : "No",
								  type()->toString().c_str(),
								  scope().toString().c_str());
			}
		}
		
	}
	
}

