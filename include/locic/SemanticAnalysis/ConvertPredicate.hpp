#ifndef LOCIC_SEMANTICANALYSIS_CONVERTPREDICATE_HPP
#define LOCIC_SEMANTICANALYSIS_CONVERTPREDICATE_HPP

#include <locic/AST.hpp>
#include <locic/Support/Optional.hpp>

namespace locic {
	
	namespace SEM {
		
		class Predicate;
		class TemplateVarMap;
		
	}
	
	namespace SemanticAnalysis {
		
		class Context;
		
		SEM::Predicate ConvertPredicate(Context& context, const AST::Node<AST::Predicate>& astPredicateNode);
		
		SEM::Predicate ConvertConstSpecifier(Context& context, const AST::Node<AST::ConstSpecifier>& astConstSpecifierNode);
		
		SEM::Predicate ConvertPredicateSpecifier(Context& context, const AST::Node<AST::RequireSpecifier>& astRequireSpecifierNode,
			bool noneValue, bool noPredicateValue);
		
		inline SEM::Predicate ConvertNoExceptSpecifier(Context& context, const AST::Node<AST::RequireSpecifier>& astRequireSpecifierNode) {
			// Noexcept predicates are 'false' if not specified.
			const bool noneValue = false;
			
			// Noexcept predicates are 'true' if specified without a predicate.
			const bool noPredicateValue = true;
			
			return ConvertPredicateSpecifier(context, astRequireSpecifierNode, noneValue, noPredicateValue);
		}
		
		inline SEM::Predicate ConvertRequireSpecifier(Context& context, const AST::Node<AST::RequireSpecifier>& astRequireSpecifierNode) {
			// Require predicates are 'true' if not specified.
			const bool noneValue = true;
			
			// Not valid...
			const bool noPredicateValue = false;
			
			return ConvertPredicateSpecifier(context, astRequireSpecifierNode, noneValue, noPredicateValue);
		}
		
		Optional<bool> evaluatePredicate(Context& context, const SEM::Predicate& predicate, const SEM::TemplateVarMap& variableAssignments);
		
		bool evaluatePredicateWithDefault(Context& context, const SEM::Predicate& predicate, const SEM::TemplateVarMap& variableAssignments, bool defaultValue);
		
		bool doesPredicateImplyPredicate(Context& context, const SEM::Predicate& firstPredicate, const SEM::Predicate& secondPredicate);
		
		SEM::Predicate reducePredicate(Context& context, SEM::Predicate predicate);
		
	}
	
}

#endif
