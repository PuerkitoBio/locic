#include <assert.h>
#include <stdio.h>
#include <Locic/SemanticAnalysis/Context.h>

/* Functions for checking semantic analysis context invariants. */

static void CheckFunctionDataCleared(Locic_SemanticContext * context){
	// Check that the function data is cleared.
	// (i.e. that EndFunction was called).
	assert(context->functionDecl == NULL);
	assert(context->functionContext.nextVarId == 0);
	assert(Locic_StringMap_Size(context->functionContext.parameters) == 0);
	
	// Check that there are no scopes on the stack.
	assert(Locic_Stack_Size(context->scopeStack) == 0);
}

static void CheckClassDataCleared(Locic_SemanticContext * context){
	// Check that the class data is cleared.
	// (i.e. that EndClass was called).
	assert(context->functionParentType == NULL);
	assert(Locic_StringMap_Size(context->classContext.memberVariables) == 0);
}

static void CheckModuleDataCleared(Locic_SemanticContext * context){
	// Check that the module data is cleared.
	// (i.e. that EndModule was called).
	assert(context->module == NULL);
}

/* Semantic Analysis Context implementation. */

Locic_SemanticContext * Locic_SemanticContext_Alloc(){
	Locic_SemanticContext * context = malloc(sizeof(Locic_SemanticContext));
	context->module = NULL;
	context->functionParentType = NULL;
	context->functionDecl = NULL;
	
	context->functionDeclarations = Locic_StringMap_Alloc();
	context->typeInstances = Locic_StringMap_Alloc();
	
	context->classContext.memberVariables = Locic_StringMap_Alloc();
	
	context->functionContext.parameters = Locic_StringMap_Alloc();
	context->functionContext.nextVarId = 0;
	
	context->scopeStack = Locic_Stack_Alloc();
}

void Locic_SemanticContext_Free(Locic_SemanticContext * context){
	// Check function and class data is cleared.
	CheckFunctionDataCleared(context);
	CheckClassDataCleared(context);
	
	Locic_StringMap_Free(context->functionDeclarations);
	Locic_StringMap_Free(context->typeInstances);
	Locic_StringMap_Free(context->classContext.memberVariables);
	Locic_StringMap_Free(context->functionContext.parameters);
	Locic_StringMap_Free(context->scopeStack);
	free(context);
}

void Locic_SemanticContext_StartModule(Locic_SemanticContext * context, SEM_Module * module){
	// Check module, function and class data is cleared.
	CheckFunctionDataCleared(context);
	CheckClassDataCleared(context);
	CheckModuleDataCleared(context);
	
	context->module = module;
}

void Locic_SemanticContext_EndModule(Locic_SemanticContext * context){
	// Clear function data.
	context->module = NULL;
	
	// Check module, function and class data is cleared.
	CheckFunctionDataCleared(context);
	CheckClassDataCleared(context);
	CheckModuleDataCleared(context);
}

void Locic_SemanticContext_StartFunctionParentType(Locic_SemanticContext * context, SEM_TypeInstance * typeInstance){
	// Check function and class data is cleared.
	CheckFunctionDataCleared(context);
	CheckClassDataCleared(context);
	
	context->functionParentType = typeInstance;
}

void Locic_SemanticContext_EndFunctionParentType(Locic_SemanticContext * context){
	// Clear class data.
	Locic_StringMap_Clear(context->classContext.memberVariables);
	context->functionParentType = NULL;
	
	// Check function and class data is cleared.
	CheckFunctionDataCleared(context);
	CheckClassDataCleared(context);
}

void Locic_SemanticContext_StartFunction(Locic_SemanticContext * context, SEM_FunctionDecl * functionDecl){
	// Check function data is cleared.
	CheckFunctionDataCleared(context);
	
	context->functionDecl = functionDecl;
}

void Locic_SemanticContext_EndFunction(Locic_SemanticContext * context){
	// Clear function data.
	context->functionDecl = NULL;
	context->functionContext.nextVarId = 0;
	Locic_StringMap_Clear(context->functionContext.parameters);
	
	// Check function data is cleared.
	CheckFunctionDataCleared(context);
}

void Locic_SemanticContext_PushScope(Locic_SemanticContext * context, SEM_Scope * scope){
	Locic_SemanticContext_Scope * semScope = malloc(sizeof(Locic_SemanticContext_Scope));
	semScope->scope = scope;
	semScope->localVariables = Locic_StringMap_Alloc();
	Locic_Stack_Push(context->scopeStack, semScope);
}

void Locic_SemanticContext_PopScope(Locic_SemanticContext * context){
	Locic_SemanticContext_Scope * semScope = Locic_Stack_Top(context->scopeStack);
	Locic_StringMap_Free(semScope->localVariables);
	free(semScope);
	Locic_Stack_Pop(context->scopeStack);
}

Locic_SemanticContext_Scope * Locic_SemanticContext_TopScope(Locic_SemanticContext * context){
	return (Locic_SemanticContext_Scope *) Locic_Stack_Top(context->scopeStack);
}

SEM_Var * Locic_SemanticContext_DefineLocalVar(Locic_SemanticContext * context, const char * varName, SEM_Type * varType){
	// Look in all current scopes, to prevent local variable shadowing.
	if(Locic_SemanticContext_FindLocalVar(context, varName) != NULL){
		// Variable already exists => define failed.
		return NULL;
	}
	
	Locic_SemanticContext_Scope * currentScope = Locic_SemanticContext_TopScope(context);
	
	SEM_Var * semVar = SEM_MakeVar(SEM_VAR_LOCAL, context->functionContext.nextVarId++, varType);
	
	// Add to local variable name context for this scope.
	SEM_Var * existingVar = Locic_StringMap_Insert(currentScope->localVariables, varName, semVar);
	
	assert(existingVar == NULL);
	
	// Add to SEM tree structure.
	Locic_Array_PushBack(currentScope->scope->localVariables, semVar);
	
	return semVar;
}

SEM_Var * Locic_SemanticContext_FindLocalVar(Locic_SemanticContext * context, const char * varName){
	// Look in the local variables of the current scopes.
	size_t i;
	for(i = Locic_Stack_Size(context->scopeStack); i > 0; i--){
		Locic_SemanticContext_Scope * scope = Locic_Stack_Get(context->scopeStack, i-1);
		SEM_Var * varEntry = Locic_StringMap_Find(scope->localVariables, varName);
		if(varEntry != NULL){
			return varEntry;
		}
	}
	
	// Variable not found in local variables => look in function parameters.
	SEM_Var * varEntry = Locic_StringMap_Find(context->functionContext.parameters, varName);
	if(varEntry != NULL){
		return varEntry;
	}
	
	return NULL;
}
