#include <stdexcept>
#include <vector>

#include <locic/Constant.hpp>
#include <locic/Support/MakeArray.hpp>
#include <locic/Support/Name.hpp>
#include <locic/SEM.hpp>

#include <locic/SemanticAnalysis/Context.hpp>
#include <locic/SemanticAnalysis/DefaultMethods.hpp>
#include <locic/SemanticAnalysis/Exception.hpp>
#include <locic/SemanticAnalysis/Lval.hpp>
#include <locic/SemanticAnalysis/MethodSet.hpp>
#include <locic/SemanticAnalysis/Ref.hpp>
#include <locic/SemanticAnalysis/ScopeElement.hpp>
#include <locic/SemanticAnalysis/ScopeStack.hpp>
#include <locic/SemanticAnalysis/TypeProperties.hpp>

namespace locic {

	namespace SemanticAnalysis {
		
		Debug::FunctionInfo makeDefaultFunctionInfo(const SEM::TypeInstance& parentType, const SEM::Function& function) {
			Debug::FunctionInfo functionInfo;
			functionInfo.isDefinition = !parentType.isClassDecl();
			functionInfo.name = function.name().copy();
			functionInfo.declLocation = parentType.debugInfo()->location;
			functionInfo.scopeLocation = parentType.debugInfo()->location;
			return functionInfo;
		}
		
		SEM::Predicate getAutoDefaultMovePredicate(Context& context, const SEM::TypeInstance* const typeInstance) {
			auto requirePredicate = SEM::Predicate::True();
			
			const auto movableType = getBuiltInType(context, context.getCString("movable"), {});
			
			// All member variables need to be movable.
			for (const auto& var: typeInstance->variables()) {
				const auto varType = var->constructType();
				requirePredicate = SEM::Predicate::And(std::move(requirePredicate), SEM::Predicate::Satisfies(varType, movableType));
			}
			
			// All variants need to be movable.
			for (const auto& variantTypeInstance: typeInstance->variants()) {
				const auto varType = variantTypeInstance->selfType();
				requirePredicate = SEM::Predicate::And(std::move(requirePredicate), SEM::Predicate::Satisfies(varType, movableType));
			}
			
			return requirePredicate;
		}
		
		SEM::Predicate getDefaultMovePredicate(Context& context, const SEM::TypeInstance* const typeInstance) {
			if (typeInstance->movePredicate()) {
				// Use a user-provided move predicate if available.
				return typeInstance->movePredicate()->copy();
			} else {
				// Otherwise automatically generate a move predicate.
				return getAutoDefaultMovePredicate(context, typeInstance);
			}
		}
		
		// A similar version of the move predicate but uses notag() to get non-const type.
		SEM::Predicate getDefaultCopyMovePredicate(Context& context, const SEM::TypeInstance* const typeInstance) {
			auto requirePredicate = SEM::Predicate::True();
			
			const auto movableType = getBuiltInType(context, context.getCString("movable"), {});
			
			// All member variables need to be movable.
			for (const auto& var: typeInstance->variables()) {
				const auto varType = var->constructType()->createNoTagType();
				requirePredicate = SEM::Predicate::And(std::move(requirePredicate), SEM::Predicate::Satisfies(varType, movableType));
			}
			
			// All variants need to be movable.
			for (const auto& variantTypeInstance: typeInstance->variants()) {
				const auto varType = variantTypeInstance->selfType()->createNoTagType();
				requirePredicate = SEM::Predicate::And(std::move(requirePredicate), SEM::Predicate::Satisfies(varType, movableType));
			}
			
			return requirePredicate;
		}
		
		SEM::Predicate getDefaultCopyPredicate(Context& context, const SEM::TypeInstance* const typeInstance, const String& propertyName) {
			// Types must be movable to be copyable.
			auto predicate = getDefaultCopyMovePredicate(context, typeInstance);
			
			// All member variables need to be copyable.
			for (const auto& var: typeInstance->variables()) {
				const auto varType = var->constructType()->withoutTags();
				const auto copyableType = getBuiltInType(context, propertyName, { varType });
				predicate = SEM::Predicate::And(std::move(predicate), SEM::Predicate::Satisfies(varType, copyableType));
			}
			
			// All variants need to be copyable.
			for (const auto& variantTypeInstance: typeInstance->variants()) {
				const auto varType = variantTypeInstance->selfType();
				const auto copyableType = getBuiltInType(context, propertyName, { varType });
				predicate = SEM::Predicate::And(std::move(predicate), SEM::Predicate::Satisfies(varType, copyableType));
			}
			
			return predicate;
		}
		
		SEM::Predicate getDefaultImplicitCopyNoExceptPredicate(Context& context, const SEM::TypeInstance* const typeInstance) {
			return getDefaultCopyPredicate(context, typeInstance, context.getCString("noexcept_implicit_copyable"));
		}
		
		SEM::Predicate getDefaultImplicitCopyRequirePredicate(Context& context, const SEM::TypeInstance* const typeInstance) {
			return getDefaultCopyPredicate(context, typeInstance, context.getCString("implicit_copyable"));
		}
		
		SEM::Predicate getDefaultExplicitCopyNoExceptPredicate(Context& context, const SEM::TypeInstance* const typeInstance) {
			return getDefaultCopyPredicate(context, typeInstance, context.getCString("noexcept_copyable"));
		}
		
		SEM::Predicate getDefaultExplicitCopyRequirePredicate(Context& context, const SEM::TypeInstance* const typeInstance) {
			return getDefaultCopyPredicate(context, typeInstance, context.getCString("copyable"));
		}
		
		SEM::Predicate getDefaultComparePredicate(Context& context, const SEM::TypeInstance* const typeInstance, const String& propertyName) {
			auto predicate = SEM::Predicate::True();
			
			// All member variables need to be copyable.
			for (const auto& var: typeInstance->variables()) {
				const auto varType = var->constructType()->withoutTags();
				const auto copyableType = getBuiltInType(context, propertyName, { varType });
				predicate = SEM::Predicate::And(std::move(predicate), SEM::Predicate::Satisfies(varType, copyableType));
			}
			
			// All variants need to be copyable.
			for (const auto& variantTypeInstance: typeInstance->variants()) {
				const auto varType = variantTypeInstance->selfType();
				const auto copyableType = getBuiltInType(context, propertyName, { varType });
				predicate = SEM::Predicate::And(std::move(predicate), SEM::Predicate::Satisfies(varType, copyableType));
			}
			
			return predicate;
		}
		
		SEM::Predicate getDefaultCompareNoExceptPredicate(Context& context, const SEM::TypeInstance* const typeInstance) {
			const auto propertyName = context.getCString("noexcept_comparable");
			return getDefaultComparePredicate(context, typeInstance, propertyName);
		}
		
		SEM::Predicate getDefaultCompareRequirePredicate(Context& context, const SEM::TypeInstance* const typeInstance) {
			const auto propertyName = context.getCString("comparable");
			return getDefaultComparePredicate(context, typeInstance, propertyName);
		}
		
		std::unique_ptr<SEM::Function> CreateDefaultConstructorDecl(Context& context, SEM::TypeInstance* const typeInstance, const Name& name) {
			std::unique_ptr<SEM::Function> semFunction( new SEM::Function(name.copy(), typeInstance->moduleScope().copy()));
			semFunction->setDefault(true);
			
			semFunction->setDebugInfo(makeDefaultFunctionInfo(*typeInstance, *semFunction));
			
			// This method requires move, so add the move predicate.
			semFunction->setRequiresPredicate(SEM::Predicate::And(typeInstance->requiresPredicate().copy(), getDefaultMovePredicate(context, typeInstance)));
			
			semFunction->setMethod(true);
			semFunction->setStaticMethod(true);
			
			const bool isVarArg = false;
			const bool isDynamicMethod = false;
			const bool isTemplatedMethod = !typeInstance->templateVariables().empty();
			
			// Default constructor only moves, and since moves never
			// throw the constructor never throws.
			auto noExceptPredicate = SEM::Predicate::True();
			
			auto constructTypes = typeInstance->constructTypes();
			
			std::vector<SEM::Var*> argVars;
			argVars.reserve(constructTypes.size());
			for (const auto constructType: constructTypes) {
				const auto lvalType = makeValueLvalType(context, constructType);
				argVars.push_back(SEM::Var::Basic(constructType, lvalType));
			}
			
			semFunction->setParameters(std::move(argVars));
			semFunction->setType(SEM::FunctionType(SEM::FunctionAttributes(isVarArg, isDynamicMethod, isTemplatedMethod, std::move(noExceptPredicate)), typeInstance->selfType(), std::move(constructTypes)));
			return semFunction;
		}
		
		std::unique_ptr<SEM::Function> CreateDefaultMoveDecl(Context& context, SEM::TypeInstance* typeInstance, const Name& name) {
			std::unique_ptr<SEM::Function> semFunction(new SEM::Function(name.copy(), typeInstance->moduleScope().copy()));
			semFunction->setDefault(true);
			
			semFunction->setDebugInfo(makeDefaultFunctionInfo(*typeInstance, *semFunction));
			
			semFunction->setRequiresPredicate(SEM::Predicate::And(typeInstance->requiresPredicate().copy(), getDefaultMovePredicate(context, typeInstance)));
			
			semFunction->setMethod(true);
			
			const bool isVarArg = false;
			const bool isDynamicMethod = true;
			const bool isTemplatedMethod = !typeInstance->templateVariables().empty();
			
			// Move never throws.
			auto noExceptPredicate = SEM::Predicate::True();
			
			const auto voidType = getBuiltInType(context, context.getCString("void_t"), {});
			const auto voidPtrType = getBuiltInType(context, context.getCString("__ptr"), { voidType });
			
			const auto sizeType = getBuiltInType(context, context.getCString("size_t"), {});
			
			SEM::TypeArray argTypes;
			argTypes.reserve(2);
			argTypes.push_back(voidPtrType);
			argTypes.push_back(sizeType);
			
			std::vector<SEM::Var*> argVars;
			argVars.reserve(2);
			
			{
				const auto lvalType = makeValueLvalType(context, voidPtrType);
				argVars.push_back(SEM::Var::Basic(voidPtrType, lvalType));
			}
			
			{
				const auto lvalType = makeValueLvalType(context, sizeType);
				argVars.push_back(SEM::Var::Basic(sizeType, lvalType));
			}
			
			semFunction->setParameters(std::move(argVars));
			semFunction->setType(SEM::FunctionType(SEM::FunctionAttributes(isVarArg, isDynamicMethod, isTemplatedMethod, std::move(noExceptPredicate)), voidType, std::move(argTypes)));
			return semFunction;
		}
		
		std::unique_ptr<SEM::Function> CreateDefaultImplicitCopyDecl(Context& context, SEM::TypeInstance* typeInstance, const Name& name) {
			std::unique_ptr<SEM::Function> semFunction(new SEM::Function(name.copy(), typeInstance->moduleScope().copy()));
			semFunction->setDefault(true);
			
			semFunction->setDebugInfo(makeDefaultFunctionInfo(*typeInstance, *semFunction));
			
			semFunction->setRequiresPredicate(SEM::Predicate::And(typeInstance->requiresPredicate().copy(), getDefaultImplicitCopyRequirePredicate(context, typeInstance)));
			
			semFunction->setMethod(true);
			semFunction->setConstPredicate(SEM::Predicate::True());
			
			const bool isVarArg = false;
			const bool isDynamicMethod = true;
			const bool isTemplatedMethod = !typeInstance->templateVariables().empty();
			
			// Default copy method may throw since it
			// may call child copy methods that throw.
			auto noExceptPredicate = getDefaultImplicitCopyNoExceptPredicate(context, typeInstance);
			
			// Implicit copy should return a notag() type.
			const auto returnType = typeInstance->selfType()->createNoTagType();
			
			semFunction->setType(SEM::FunctionType(SEM::FunctionAttributes(isVarArg, isDynamicMethod, isTemplatedMethod, std::move(noExceptPredicate)), returnType, {}));
			return semFunction;
		}
		
		std::unique_ptr<SEM::Function> CreateDefaultExplicitCopyDecl(Context& context, SEM::TypeInstance* typeInstance, const Name& name) {
			std::unique_ptr<SEM::Function> semFunction(new SEM::Function(name.copy(), typeInstance->moduleScope().copy()));
			semFunction->setDefault(true);
			
			semFunction->setDebugInfo(makeDefaultFunctionInfo(*typeInstance, *semFunction));
			
			semFunction->setRequiresPredicate(SEM::Predicate::And(typeInstance->requiresPredicate().copy(), getDefaultExplicitCopyRequirePredicate(context, typeInstance)));
			
			semFunction->setMethod(true);
			semFunction->setConstPredicate(SEM::Predicate::True());
			
			const bool isVarArg = false;
			const bool isDynamicMethod = true;
			const bool isTemplatedMethod = !typeInstance->templateVariables().empty();
			
			// Default copy method may throw since it
			// may call child copy methods that throw.
			auto noExceptPredicate = getDefaultExplicitCopyNoExceptPredicate(context, typeInstance);
			
			// Implicit copy should return a notag() type.
			const auto returnType = typeInstance->selfType()->createNoTagType();
			
			semFunction->setType(SEM::FunctionType(SEM::FunctionAttributes(isVarArg, isDynamicMethod, isTemplatedMethod, std::move(noExceptPredicate)), returnType, {}));
			return semFunction;
		}
		
		std::unique_ptr<SEM::Function> CreateDefaultCompareDecl(Context& context, SEM::TypeInstance* typeInstance, const Name& name) {
			std::unique_ptr<SEM::Function> semFunction(new SEM::Function(name.copy(), typeInstance->moduleScope().copy()));
			semFunction->setDefault(true);
			
			semFunction->setDebugInfo(makeDefaultFunctionInfo(*typeInstance, *semFunction));
			
			semFunction->setRequiresPredicate(SEM::Predicate::And(typeInstance->requiresPredicate().copy(), getDefaultCompareRequirePredicate(context, typeInstance)));
			
			semFunction->setMethod(true);
			semFunction->setConstPredicate(SEM::Predicate::True());
			
			const bool isVarArg = false;
			const bool isDynamicMethod =true;
			const bool isTemplatedMethod = !typeInstance->templateVariables().empty();
			
			// Default compare method may throw since it
			// may call child compare methods that throw.
			auto noExceptPredicate = getDefaultCompareNoExceptPredicate(context, typeInstance);
			
			const auto selfType = typeInstance->selfType();
			const auto argType = createReferenceType(context, selfType->createTransitiveConstType(SEM::Predicate::True()));
			const auto compareResultType = getBuiltInType(context, context.getCString("compare_result_t"), {});
			
			SEM::TypeArray argTypes;
			argTypes.reserve(1);
			argTypes.push_back(argType);
			
			semFunction->setType(SEM::FunctionType(SEM::FunctionAttributes(isVarArg, isDynamicMethod, isTemplatedMethod, std::move(noExceptPredicate)), compareResultType, std::move(argTypes)));
			semFunction->setParameters({ SEM::Var::Basic(argType, argType) });
			return semFunction;
		}
		
		std::unique_ptr<SEM::Function> CreateDefaultSetDeadDecl(Context& context, SEM::TypeInstance* typeInstance, const Name& name) {
			std::unique_ptr<SEM::Function> semFunction(new SEM::Function(name.copy(), typeInstance->moduleScope().copy()));
			semFunction->setDefault(true);
			
			semFunction->setDebugInfo(makeDefaultFunctionInfo(*typeInstance, *semFunction));
			
			semFunction->setMethod(true);
			
			const bool isVarArg = false;
			const bool isDynamicMethod = true;
			const bool isTemplatedMethod = !typeInstance->templateVariables().empty();
			
			// Never throws.
			auto noExceptPredicate = SEM::Predicate::True();
			
			const auto voidType = getBuiltInType(context, context.getCString("void_t"), {});
			
			semFunction->setType(SEM::FunctionType(SEM::FunctionAttributes(isVarArg, isDynamicMethod, isTemplatedMethod, std::move(noExceptPredicate)), voidType, {}));
			return semFunction;
		}
		
		std::unique_ptr<SEM::Function> CreateDefaultIsLiveDecl(Context& context, SEM::TypeInstance* typeInstance, const Name& name) {
			std::unique_ptr<SEM::Function> semFunction(new SEM::Function(name.copy(), typeInstance->moduleScope().copy()));
			semFunction->setDefault(true);
			
			semFunction->setDebugInfo(makeDefaultFunctionInfo(*typeInstance, *semFunction));
			
			semFunction->setMethod(true);
			semFunction->setConstPredicate(SEM::Predicate::True());
			
			const bool isVarArg = false;
			const bool isDynamicMethod =true;
			const bool isTemplatedMethod = !typeInstance->templateVariables().empty();
			
			// Never throws.
			auto noExceptPredicate = SEM::Predicate::True();
			
			const auto boolType = getBuiltInType(context, context.getCString("bool"), {});
			
			semFunction->setType(SEM::FunctionType(SEM::FunctionAttributes(isVarArg, isDynamicMethod, isTemplatedMethod, std::move(noExceptPredicate)), boolType, {}));
			return semFunction;
		}
		
		std::unique_ptr<SEM::Function> CreateDefaultMethodDecl(Context& context, SEM::TypeInstance* typeInstance, bool isStatic, const Name& name, const Debug::SourceLocation& location) {
			assert(!typeInstance->isClassDecl());
			assert(!typeInstance->isInterface());
			
			const auto canonicalName = CanonicalizeMethodName(name.last());
			if (canonicalName == "create") {
				if (!isStatic) {
					throw ErrorException(makeString("Default method '%s' must be static at position %s.",
						name.toString().c_str(), location.toString().c_str()));
				}
				
				return CreateDefaultConstructorDecl(context, typeInstance, name);
			} else if (canonicalName == "__moveto") {
				if (isStatic) {
					throw ErrorException(makeString("Default method '%s' must be non-static at position %s.",
						name.toString().c_str(), location.toString().c_str()));
				}
				
				return CreateDefaultMoveDecl(context, typeInstance, name);
			} else if (canonicalName == "implicitcopy") {
				if (isStatic) {
					throw ErrorException(makeString("Default method '%s' must be non-static at position %s.",
						name.toString().c_str(), location.toString().c_str()));
				}
				
				return CreateDefaultImplicitCopyDecl(context, typeInstance, name);
			} else if (canonicalName == "copy") {
				if (isStatic) {
					throw ErrorException(makeString("Default method '%s' must be non-static at position %s.",
						name.toString().c_str(), location.toString().c_str()));
				}
				
				return CreateDefaultExplicitCopyDecl(context, typeInstance, name);
			} else if (canonicalName == "compare") {
				if (isStatic) {
					throw ErrorException(makeString("Default method '%s' must be non-static at position %s.",
						name.toString().c_str(), location.toString().c_str()));
				}
				
				return CreateDefaultCompareDecl(context, typeInstance, name);
			} else if (canonicalName == "__setdead") {
				if (isStatic) {
					throw ErrorException(makeString("Default method '%s' must be non-static at position %s.",
						name.toString().c_str(), location.toString().c_str()));
				}
				
				return CreateDefaultSetDeadDecl(context, typeInstance, name);
			} else if (canonicalName == "__islive") {
				if (isStatic) {
					throw ErrorException(makeString("Default method '%s' must be non-static at position %s.",
						name.toString().c_str(), location.toString().c_str()));
				}
				
				return CreateDefaultIsLiveDecl(context, typeInstance, name);
			}
			
			throw ErrorException(makeString("Unknown default method '%s' at position %s.",
				name.toString().c_str(), location.toString().c_str()));
		}
		
		bool HasDefaultConstructor(Context& /*context*/, SEM::TypeInstance* const typeInstance) {
			return typeInstance->isDatatype() ||
				typeInstance->isException() ||
				typeInstance->isStruct() ||
				typeInstance->isUnion();
		}
		
		bool HasDefaultMove(Context& context, SEM::TypeInstance* const typeInstance) {
			if (typeInstance->isInterface() || typeInstance->isPrimitive()) {
				return false;
			}
			
			// There's only a default move method if the user
			// hasn't specified a custom move method.
			return typeInstance->functions().find(context.getCString("__moveto")) == typeInstance->functions().end();
		}
		
		bool HasDefaultImplicitCopy(Context& /*context*/, SEM::TypeInstance* const typeInstance) {
			return typeInstance->isClassDef() ||
				typeInstance->isDatatype() ||
				typeInstance->isEnum() ||
				typeInstance->isException() ||
				typeInstance->isStruct() ||
				typeInstance->isUnion() ||
				typeInstance->isUnionDatatype();
		}
		
		bool HasDefaultExplicitCopy(Context& /*context*/, SEM::TypeInstance* const typeInstance) {
			return typeInstance->isClassDef() ||
				typeInstance->isDatatype() ||
				typeInstance->isEnum() ||
				typeInstance->isException() ||
				typeInstance->isStruct() ||
				typeInstance->isUnion() ||
				typeInstance->isUnionDatatype();
		}
		
		bool HasDefaultCompare(Context& /*context*/, SEM::TypeInstance* const typeInstance) {
			return typeInstance->isClassDef() ||
				typeInstance->isDatatype() ||
				typeInstance->isEnum() ||
				typeInstance->isException() ||
				typeInstance->isStruct() ||
				typeInstance->isUnionDatatype();
		}
		
		bool HasDefaultSetDead(Context& context, SEM::TypeInstance* const typeInstance) {
			if (typeInstance->isInterface() || typeInstance->isPrimitive()) {
				return false;
			}
			
			// There's only a default __setdead method if the user
			// hasn't specified a custom __setdead method.
			return typeInstance->functions().find(context.getCString("__setdead")) == typeInstance->functions().end();
		}
		
		bool HasDefaultIsLive(Context& context, SEM::TypeInstance* const typeInstance) {
			if (typeInstance->isInterface() || typeInstance->isPrimitive()) {
				return false;
			}
			
			// There's only a default islive method if the user
			// hasn't specified a custom islive method.
			return typeInstance->functions().find(context.getCString("__islive")) == typeInstance->functions().end();
		}
		
		void CreateDefaultConstructor(Context& context, SEM::TypeInstance* const typeInstance, SEM::Function* const function, const Debug::SourceLocation& location) {
			assert(!typeInstance->isUnionDatatype());
			
			auto functionScope = SEM::Scope::Create();
			
			if (typeInstance->isUnion()) {
				functionScope->statements().push_back(SEM::Statement::Return(SEM::Value::ZeroInitialise(typeInstance->selfType())));
				function->setScope(std::move(functionScope));
			} else {
				HeapArray<SEM::Value> constructValues;
				constructValues.reserve(function->parameters().size());
				
				for (const auto& argVar: function->parameters()) {
					auto argVarValue = createLocalVarRef(context, *argVar);
					constructValues.push_back(CallValue(context, GetSpecialMethod(context, std::move(argVarValue), context.getCString("move"), location), {}, location));
				}
				
				auto internalConstructedValue = SEM::Value::InternalConstruct(typeInstance->selfType(), std::move(constructValues));
				functionScope->statements().push_back(SEM::Statement::Return(std::move(internalConstructedValue)));
				
				function->setScope(std::move(functionScope));
			}
		}
		
		void CreateDefaultCopy(Context& context, const String& functionName, SEM::TypeInstance* typeInstance, SEM::Function* function, const Debug::SourceLocation& location) {
			const auto selfType = typeInstance->selfType();
			const auto selfValue = createSelfRef(context, selfType);
			
			auto functionScope = SEM::Scope::Create();
			
			if (typeInstance->isUnion()) {
				functionScope->statements().push_back(SEM::Statement::Return(SEM::Value::MemCopy(selfValue.copy(), selfType)));
			} else if (typeInstance->isUnionDatatype()) {
				std::vector<SEM::SwitchCase*> switchCases;
				for (const auto variantTypeInstance: typeInstance->variants()) {
					const auto variantType = variantTypeInstance->selfType();
					const auto caseVar = SEM::Var::Basic(variantType, variantType);
					auto caseVarValue = createLocalVarRef(context, *caseVar);
					
					auto caseScope = SEM::Scope::Create();
					auto copyResult = CallValue(context, GetSpecialMethod(context, std::move(caseVarValue), functionName, location), {}, location);
					auto copyResultCast = SEM::Value::Cast(selfType, std::move(copyResult));
					caseScope->statements().push_back(SEM::Statement::Return(std::move(copyResultCast)));
					
					switchCases.push_back(new SEM::SwitchCase(caseVar, std::move(caseScope)));
				}
				functionScope->statements().push_back(SEM::Statement::Switch(selfValue.copy(), std::move(switchCases), nullptr));
			} else {
				HeapArray<SEM::Value> copyValues;
				copyValues.reserve(typeInstance->variables().size());
				
				for (const auto memberVar: typeInstance->variables()) {
					auto selfMember = tryDissolveValue(context, createMemberVarRef(context, selfValue.copy(), *memberVar), location);
					auto copyResult = CallValue(context, GetSpecialMethod(context, std::move(selfMember), functionName, location), {}, location);
					copyValues.push_back(std::move(copyResult));
				}
				
				auto internalConstructedValue = SEM::Value::InternalConstruct(typeInstance->selfType(), std::move(copyValues));
				functionScope->statements().push_back(SEM::Statement::Return(std::move(internalConstructedValue)));
			}
			
			function->setScope(std::move(functionScope));
		}
		
		bool CreateDefaultImplicitCopy(Context& context, SEM::TypeInstance* typeInstance, SEM::Function* function, const Debug::SourceLocation& location) {
			if (!typeInstance->isClassDef() && !supportsImplicitCopy(context, typeInstance->selfType())) {
				// Can't actually do implicit copy (presumably member types don't support it).
				// We still try to generate the method for class definitions since the user
				// requested these to be generated and hence will want to see the error.
				return false;
			}
			CreateDefaultCopy(context, context.getCString("implicitcopy"), typeInstance, function, location);
			return true;
		}
		
		bool CreateDefaultExplicitCopy(Context& context, SEM::TypeInstance* typeInstance, SEM::Function* function, const Debug::SourceLocation& location) {
			if (!typeInstance->isClassDef() && !supportsExplicitCopy(context, typeInstance->selfType())) {
				// Can't actually do copy (presumably member types don't support it).
				// We still try to generate the method for class definitions since the user
				// requested these to be generated and hence will want to see the error.
				return false;
			}
			CreateDefaultCopy(context, context.getCString("copy"), typeInstance, function, location);
			return true;
		}
		
		bool CreateDefaultCompare(Context& context, SEM::TypeInstance* typeInstance, SEM::Function* function, const Debug::SourceLocation& location) {
			assert(!typeInstance->isUnion());
			if (!typeInstance->isClassDef() && !supportsCompare(context, typeInstance->selfType())) {
				// Can't actually do compare (presumably member types don't support it).
				// We still try to generate the method for class definitions since the user
				// requested these to be generated and hence will want to see the error.
				return false;
			}
			
			const auto selfValue = createSelfRef(context, typeInstance->selfType());
			
			const auto compareResultType = getBuiltInType(context, context.getCString("compare_result_t"), {});
			
			const auto operandVar = function->parameters().at(0);
			const auto operandValue = createLocalVarRef(context, *operandVar);
			
			auto functionScope = SEM::Scope::Create();
			
			if (typeInstance->isUnionDatatype()) {
				std::vector<SEM::SwitchCase*> switchCases;
				for (size_t i = 0; i < typeInstance->variants().size(); i++) {
					const auto variantTypeInstance = typeInstance->variants().at(i);
					const auto variantType = variantTypeInstance->selfType();
					const auto caseVar = SEM::Var::Basic(variantType, variantType);
					const auto caseVarValue = createLocalVarRef(context, *caseVar);
					
					auto caseScope = SEM::Scope::Create();
					
					std::vector<SEM::SwitchCase*> subSwitchCases;
					for (size_t j = 0; j < typeInstance->variants().size(); j++) {
						const auto subVariantTypeInstance = typeInstance->variants().at(j);
						const auto subVariantType = subVariantTypeInstance->selfType();
						const auto subCaseVar = SEM::Var::Basic(subVariantType, subVariantType);
						auto subCaseVarValue = createLocalVarRef(context, *subCaseVar);
						
						auto subCaseScope = SEM::Scope::Create();
						auto minusOneConstant = SEM::Value::Constant(Constant::Integer(-1), compareResultType);
						auto plusOneConstant = SEM::Value::Constant(Constant::Integer(1), compareResultType);
						if (i < j) {
							subCaseScope->statements().push_back(SEM::Statement::Return(std::move(minusOneConstant)));
						} else if (i > j) {
							subCaseScope->statements().push_back(SEM::Statement::Return(std::move(plusOneConstant)));
						} else {
							auto compareResult = CallValue(context, GetMethod(context, caseVarValue.copy(), context.getCString("compare"), location), makeHeapArray( std::move(subCaseVarValue) ), location);
							subCaseScope->statements().push_back(SEM::Statement::Return(std::move(compareResult)));
						}
						
						subSwitchCases.push_back(new SEM::SwitchCase(subCaseVar, std::move(subCaseScope)));
					}
					
					caseScope->statements().push_back(SEM::Statement::Switch(derefValue(operandValue.copy()), subSwitchCases, nullptr));
					
					switchCases.push_back(new SEM::SwitchCase(caseVar, std::move(caseScope)));
				}
				
				functionScope->statements().push_back(SEM::Statement::Switch(selfValue.copy(), switchCases, nullptr));
			} else {
				auto currentScope = functionScope.get();
				
				for (const auto memberVar: typeInstance->variables()) {
					auto selfMember = createMemberVarRef(context, selfValue.copy(), *memberVar);
					auto operandMember = createMemberVarRef(context, operandValue.copy(), *memberVar);
					
					auto compareResult = CallValue(context, GetMethod(context, std::move(selfMember), context.getCString("compare"), location), makeHeapArray( std::move(operandMember) ), location);
					auto isEqual = CallValue(context, GetMethod(context, compareResult.copy(), context.getCString("isEqual"), location), {}, location);
					
					auto ifTrueScope = SEM::Scope::Create();
					auto ifFalseScope = SEM::Scope::Create();
					
					const auto nextScope = ifTrueScope.get();
					
					ifFalseScope->statements().push_back(SEM::Statement::Return(std::move(compareResult)));
					
					auto ifStatement = SEM::Statement::If({ new SEM::IfClause(std::move(isEqual), std::move(ifTrueScope)) }, std::move(ifFalseScope));
					currentScope->statements().push_back(std::move(ifStatement));
					
					currentScope = nextScope;
				}
				
				auto zeroConstant = SEM::Value::Constant(Constant::Integer(0), compareResultType);
				currentScope->statements().push_back(SEM::Statement::Return(std::move(zeroConstant)));
			}
			
			function->setScope(std::move(functionScope));
			return true;
		}
		
		bool CreateDefaultMethod(Context& context, SEM::TypeInstance* const typeInstance, SEM::Function* const function, const Debug::SourceLocation& location) {
			assert(!typeInstance->isClassDecl() && !typeInstance->isInterface());
			assert(function->isDeclaration());
			
			PushScopeElement pushFunction(context.scopeStack(), ScopeElement::Function(function));
			
			const auto& name = function->name();
			const auto canonicalName = CanonicalizeMethodName(name.last());
			if (canonicalName == "__moveto" || canonicalName == "__dead" || canonicalName == "__islive") {
				// Generate by CodeGen.
				return true;
			} else if (canonicalName == "create") {
				assert(!typeInstance->isException());
				CreateDefaultConstructor(context, typeInstance, function, location);
				return true;
			} else if (canonicalName == "implicitcopy") {
				if (!HasDefaultImplicitCopy(context, typeInstance)) {
					throw ErrorException(makeString("Default method '%s' is not supported for this type kind, at position %s.",
						name.toString().c_str(), location.toString().c_str()));
				}
				
				return CreateDefaultImplicitCopy(context, typeInstance, function, location);
			} else if (canonicalName == "copy") {
				if (!HasDefaultExplicitCopy(context, typeInstance)) {
					throw ErrorException(makeString("Default method '%s' is not supported for this type kind, at position %s.",
						name.toString().c_str(), location.toString().c_str()));
				}
				
				return CreateDefaultExplicitCopy(context, typeInstance, function, location);
			} else if (canonicalName == "compare") {
				if (!HasDefaultCompare(context, typeInstance)) {
					throw ErrorException(makeString("Default method '%s' is not supported for this type kind, at position %s.",
						name.toString().c_str(), location.toString().c_str()));
				}
				
				return CreateDefaultCompare(context, typeInstance, function, location);
			} else {
				throw std::runtime_error(makeString("Unknown default method '%s'.", name.toString().c_str()));
			}
		}
		
	}
	
}

