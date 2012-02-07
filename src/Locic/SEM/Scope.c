#include <stdlib.h>
#include <Locic/Array.h>
#include <Locic/List.h>
#include <Locic/SEM/Scope.h>

SEM_Scope * SEM_MakeScope(){
	SEM_Scope * scope = malloc(sizeof(SEM_Scope));
	scope->localVariables = Locic_Array_Alloc();
	scope->statementList = Locic_List_Alloc();
	return scope;
}
