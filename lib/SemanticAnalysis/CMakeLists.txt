add_library(locic-semanalysis
	AliasTypeResolver.cpp
	Analysis.cpp
	CallValue.cpp
	Cast.cpp
	Context.cpp
	ConvertException.cpp
	ConvertForLoop.cpp
	ConvertFunctionDecl.cpp
	ConvertFunctionDef.cpp
	ConvertModuleScope.cpp
	ConvertNamespace.cpp
	ConvertPredicate.cpp
	ConvertScope.cpp
	ConvertStatement.cpp
	ConvertType.cpp
	ConvertTypeInstance.cpp
	ConvertValue.cpp
	ConvertVar.cpp
	DefaultMethods.cpp
	GetMethod.cpp
	GetMethodSet.cpp
	Literal.cpp
	Lval.cpp
	MethodSet.cpp
	MethodSetElement.cpp
	MethodSetSatisfies.cpp
	NameSearch.cpp
	Passes/AddAliasValuesPass.cpp
	Passes/AddFunctionDeclsPass.cpp
	Passes/AddGlobalStructuresPass.cpp
	Passes/AddTemplateVariableTypesPass.cpp
	Passes/AddTypeMemberVariablesPass.cpp
	Passes/CheckTemplateInstantiationsPass.cpp
	Passes/CompleteFunctionTemplateVariableRequirementsPass.cpp
	Passes/CompleteTypeTemplateVariableRequirementsPass.cpp
	Passes/GenerateDefaultMethodsPass.cpp
	Ref.cpp
	ScopeElement.cpp
	ScopeStack.cpp
	SearchResult.cpp
	Template.cpp
	TypeCapabilities.cpp
	VarArgCast.cpp
)
