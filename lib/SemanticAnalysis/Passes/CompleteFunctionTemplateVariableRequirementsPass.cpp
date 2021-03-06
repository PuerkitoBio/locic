#include <locic/AST.hpp>
#include <locic/SemanticAnalysis/Context.hpp>
#include <locic/SemanticAnalysis/ConvertNamespace.hpp>
#include <locic/SemanticAnalysis/ConvertPredicate.hpp>
#include <locic/SemanticAnalysis/ConvertType.hpp>
#include <locic/SemanticAnalysis/NameSearch.hpp>
#include <locic/SemanticAnalysis/ScopeStack.hpp>
#include <locic/SemanticAnalysis/SearchResult.hpp>

namespace locic {
	
	namespace SemanticAnalysis {
		
		void CompleteFunctionTemplateVariableRequirements(Context& context, const AST::Node<AST::Function>& astFunctionNode, const SEM::Predicate& parentRequiresPredicate) {
			const auto function = context.scopeStack().back().function();
			
			// Add any requirements specified by parent.
			auto predicate = parentRequiresPredicate.copy();
			
			// Add any requirements in require() specifier.
			if (!astFunctionNode->requireSpecifier().isNull()) {
				predicate = SEM::Predicate::And(std::move(predicate), ConvertRequireSpecifier(context, astFunctionNode->requireSpecifier()));
			}
			
			// Add requirements specified inline for template variables.
			for (auto astTemplateVarNode: *(astFunctionNode->templateVariables())) {
				const auto& templateVarName = astTemplateVarNode->name;
				const auto semTemplateVar = function->namedTemplateVariables().at(templateVarName);
				
				const auto& astSpecType = astTemplateVarNode->specType;
				
				if (astSpecType->isVoid()) {
					// No requirement specified.
					continue;
				}
			 	
			 	const auto semSpecType = ConvertType(context, astSpecType);
			 	
			 	// Add the satisfies requirement to the predicate.
				auto inlinePredicate = SEM::Predicate::Satisfies(semTemplateVar->selfRefType(), semSpecType);
				predicate = SEM::Predicate::And(std::move(predicate), std::move(inlinePredicate));
			}
			
			function->setRequiresPredicate(std::move(predicate));
		}
		
		void CompleteNamespaceDataFunctionTemplateVariableRequirements(Context& context, const AST::Node<AST::NamespaceData>& astNamespaceDataNode) {
			const auto semNamespace = context.scopeStack().back().nameSpace();
			
			for (auto astFunctionNode: astNamespaceDataNode->functions) {
				auto& semChildFunction = findNamespaceFunction(context, *(astFunctionNode->name()));
				
				const auto& name = astFunctionNode->name();
				assert(!name->empty());
				
				if (name->size() == 1) {
					PushScopeElement pushFunction(context.scopeStack(), ScopeElement::Function(&semChildFunction));
					CompleteFunctionTemplateVariableRequirements(context, astFunctionNode, SEM::Predicate::True());
				} else {
					const auto searchResult = performSearch(context, name->getPrefix());
					const auto parentTypeInstance = searchResult.typeInstance();
					
					// Push the type instance on the scope stack, since the extension method is
					// effectively within the scope of the type instance.
					PushScopeElement pushTypeInstance(context.scopeStack(), ScopeElement::TypeInstance(parentTypeInstance));
					PushScopeElement pushFunction(context.scopeStack(), ScopeElement::Function(&semChildFunction));
					
					CompleteFunctionTemplateVariableRequirements(context, astFunctionNode, parentTypeInstance->requiresPredicate());
				}
			}
			
			for (auto astModuleScopeNode: astNamespaceDataNode->moduleScopes) {
				CompleteNamespaceDataFunctionTemplateVariableRequirements(context, astModuleScopeNode->data);
			}
			
			for (auto astNamespaceNode: astNamespaceDataNode->namespaces) {
				auto& semChildNamespace = semNamespace->items().at(astNamespaceNode->name).nameSpace();
				
				PushScopeElement pushNamespace(context.scopeStack(), ScopeElement::Namespace(&semChildNamespace));
				CompleteNamespaceDataFunctionTemplateVariableRequirements(context, astNamespaceNode->data);
			}
			
			for (auto astTypeInstanceNode: astNamespaceDataNode->typeInstances) {
				auto& semChildTypeInstance = semNamespace->items().at(astTypeInstanceNode->name).typeInstance();
				
				PushScopeElement pushTypeInstance(context.scopeStack(), ScopeElement::TypeInstance(&semChildTypeInstance));
				for (const auto& astFunctionNode: *(astTypeInstanceNode->functions)) {
					const auto methodName = CanonicalizeMethodName(astFunctionNode->name()->last());
					auto& semChildFunction = semChildTypeInstance.functions().at(methodName);
					
					PushScopeElement pushFunction(context.scopeStack(), ScopeElement::Function(semChildFunction.get()));
					CompleteFunctionTemplateVariableRequirements(context, astFunctionNode, semChildTypeInstance.requiresPredicate());
				}
			}
		}
		
		void CompleteFunctionTemplateVariableRequirementsPass(Context& context, const AST::NamespaceList& rootASTNamespaces) {
			for (auto astNamespaceNode: rootASTNamespaces) {
				CompleteNamespaceDataFunctionTemplateVariableRequirements(context, astNamespaceNode->data);
			}
		}
		
	}
	
}
