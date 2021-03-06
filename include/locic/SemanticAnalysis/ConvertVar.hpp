#ifndef LOCIC_SEMANTICANALYSIS_CONVERTVAR_HPP
#define LOCIC_SEMANTICANALYSIS_CONVERTVAR_HPP

#include <locic/AST.hpp>
#include <locic/Debug/VarInfo.hpp>

namespace locic {
	
	namespace SEM {
		
		class Type;
		class Var;
		
	}
	
	namespace SemanticAnalysis {
		
		class Context;
		
		Debug::VarInfo makeVarInfo(Debug::VarInfo::Kind kind, const AST::Node<AST::TypeVar>& astTypeVarNode);
		
		void attachVar(Context& context, const String& name, const AST::Node<AST::TypeVar>& astTypeVarNode, SEM::Var* var, Debug::VarInfo::Kind varKind);
		
		SEM::Var* ConvertVar(Context& context, Debug::VarInfo::Kind varKind, const AST::Node<AST::TypeVar>& typeVar);
		
		// Note that this function assumes that the variable is a local variable.
		SEM::Var* ConvertInitialisedVar(Context& context, const AST::Node<AST::TypeVar>& typeVar, const SEM::Type* initialiseType);
		
	}
	
}

#endif
