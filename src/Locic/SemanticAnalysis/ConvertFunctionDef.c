#include <stdio.h>
#include <Locic/AST.h>
#include <Locic/SEM.h>
#include <Locic/StringMap.h>
#include <Locic/SemanticAnalysis/Context.h>
#include <Locic/SemanticAnalysis/ConvertScope.h>

SEM_FunctionDef * Locic_SemanticAnalysis_ConvertFunctionDef(Locic_SemanticContext * context, AST_FunctionDef * functionDef){
	// Find the corresponding semantic function declaration.
	SEM_FunctionDecl * semFunctionDecl = Locic_StringMap_Find(context->functionDeclarations, functionDef->declaration->name);
	if(semFunctionDecl == NULL){
		printf("Internal compiler error: semantic function declaration not found for definition '%s'.\n", functionDef->declaration->name);
		return NULL;
	}
	
	Locic_SemanticContext_StartFunction(context, semFunctionDecl);
	
	// AST information gives parameter names; SEM information gives parameter variable information.
	Locic_List * synParameters = functionDef->declaration->parameters;
	Locic_List * semParameters = semFunctionDecl->parameterVars;
	
	Locic_ListElement * synIterator, * semIterator;
	for(synIterator = Locic_List_Begin(synParameters), semIterator = Locic_List_Begin(semParameters);
		synIterator != Locic_List_End(synParameters);
		synIterator = synIterator->next, semIterator = semIterator->next){
		
		AST_TypeVar * typeVar = synIterator->data;
		SEM_Var * paramVar = semIterator->data;
		
		// Create a mapping from the parameter's name to its variable information.
		if(Locic_StringMap_Insert(context->functionContext.parameters, typeVar->name, paramVar) != NULL){
			printf("Semantic Analysis Error: cannot share names between function parameters.\n");
			return NULL;
		}
	}
	
	// Generate the outer function scope.
	// (which will then generate its contents etc.)
	SEM_Scope * scope = Locic_SemanticAnalysis_ConvertScope(context, functionDef->scope);
	
	if(scope == NULL){
		return NULL;
	}
	
	Locic_SemanticContext_EndFunction(context);
	
	// Build and return the function definition.
	return SEM_MakeFunctionDef(semFunctionDecl, scope);
}

