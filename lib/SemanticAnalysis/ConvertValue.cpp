#include <assert.h>
#include <stdio.h>

#include <limits>
#include <list>
#include <map>
#include <stdexcept>
#include <string>

#include <locic/AST.hpp>
#include <locic/Debug.hpp>
#include <locic/SEM.hpp>

#include <locic/SemanticAnalysis/CanCast.hpp>
#include <locic/SemanticAnalysis/Context.hpp>
#include <locic/SemanticAnalysis/ConvertType.hpp>
#include <locic/SemanticAnalysis/ConvertValue.hpp>
#include <locic/SemanticAnalysis/Literal.hpp>
#include <locic/SemanticAnalysis/Lval.hpp>
#include <locic/SemanticAnalysis/NameSearch.hpp>
#include <locic/SemanticAnalysis/Ref.hpp>
#include <locic/SemanticAnalysis/TypeProperties.hpp>

namespace locic {

	namespace SemanticAnalysis {
		
		SEM::Type* getFunctionType(SEM::Type* type) {
			if (type->isInterfaceMethod()) {
				return type->getInterfaceMethodFunctionType();
			}
			
			if (type->isMethod()) {
				return type->getMethodFunctionType();
			}
			
			return type;
		}
		
		bool CanValueThrow(SEM::Value* value) {
			switch (value->kind()) {
				case SEM::Value::REINTERPRET: {
					return CanValueThrow(value->reinterpretValue.value);
				}
				case SEM::Value::DEREF_REFERENCE: {
					return CanValueThrow(value->derefReference.value);
				}
				case SEM::Value::TERNARY: {
					return CanValueThrow(value->ternary.condition) ||
						CanValueThrow(value->ternary.ifTrue) ||
						CanValueThrow(value->ternary.ifFalse);
				}
				case SEM::Value::CAST: {
					return CanValueThrow(value->cast.value);
				}
				case SEM::Value::POLYCAST: {
					return CanValueThrow(value->polyCast.value);
				}
				case SEM::Value::LVAL: {
					return CanValueThrow(value->makeLval.value);
				}
				case SEM::Value::NOLVAL: {
					return CanValueThrow(value->makeNoLval.value);
				}
				case SEM::Value::REF: {
					return CanValueThrow(value->makeRef.value);
				}
				case SEM::Value::NOREF: {
					return CanValueThrow(value->makeNoRef.value);
				}
				case SEM::Value::INTERNALCONSTRUCT: {
					for (const auto parameter: value->internalConstruct.parameters) {
						if (CanValueThrow(parameter)) {
							return true;
						}
					}
					return false;
				}
				case SEM::Value::MEMBERACCESS: {
					return CanValueThrow(value->memberAccess.object);
				}
				case SEM::Value::REFVALUE: {
					return CanValueThrow(value->refValue.value);
				}
				case SEM::Value::FUNCTIONCALL: {
					for (const auto parameter: value->functionCall.parameters) {
						if (CanValueThrow(parameter)) {
							return true;
						}
					}
					
					const auto functionValue = value->functionCall.functionValue;
					const auto functionType = getFunctionType(functionValue->type());
					assert(functionType->isFunction());
					return CanValueThrow(functionValue) || !functionType->isFunctionNoExcept();
				}
				default:
					return false;
			}
		}
		
		std::string binaryOpToString(AST::BinaryOpKind kind) {
			switch (kind) {
				case AST::OP_ISEQUAL:
					return "==";
				case AST::OP_NOTEQUAL:
					return "!=";
				case AST::OP_LESSTHAN:
					return "<";
				case AST::OP_LESSTHANOREQUAL:
					return "<=";
				case AST::OP_GREATERTHAN:
					return ">";
				case AST::OP_GREATERTHANOREQUAL:
					return ">=";
				default:
					throw std::runtime_error("Unknown binary op.");
			}
		}
		
		std::string binaryOpName(AST::BinaryOpKind kind) {
			switch (kind) {
				case AST::OP_ISEQUAL:
					return "equal";
				case AST::OP_NOTEQUAL:
					return "not_equal";
				case AST::OP_LESSTHAN:
					return "less_than";
				case AST::OP_LESSTHANOREQUAL:
					return "less_than_or_equal";
				case AST::OP_GREATERTHAN:
					return "greater_than";
				case AST::OP_GREATERTHANOREQUAL:
					return "greater_than_or_equal";
				default:
					throw std::runtime_error("Unknown binary op.");
			}
		}
		
		SEM::Value* GetBinaryOp(Context& context, SEM::Value* value, AST::BinaryOpKind opKind, const Debug::SourceLocation& location) {
			const auto derefType = getDerefType(value->type());
			
			if (!derefType->isObjectOrTemplateVar()) {
				throw ErrorException(makeString("Can't perform binary operator '%s' for non-object type '%s' at position %s.",
					binaryOpToString(opKind).c_str(), derefType->toString().c_str(), location.toString().c_str()));
			}
			
			const auto typeInstance = derefType->getObjectOrSpecType();
			
			const auto methodName = binaryOpName(opKind);
			
			// Look for the 
			if (typeInstance->functions().find(CanonicalizeMethodName(methodName)) != typeInstance->functions().end()) {
				return GetMethod(context, value, methodName, location);
			} else {
				return nullptr;
			}
		}
		
		SEM::Value* MakeMemberAccess(Context& context, SEM::Value* value, const std::string& memberName, const Debug::SourceLocation& location) {
			const auto derefType = getDerefType(value->type());
			
			if (!derefType->isObjectOrTemplateVar()) {
				throw ErrorException(makeString("Can't access member '%s' of type '%s' at position %s.",
					memberName.c_str(), derefType->toString().c_str(), location.toString().c_str()));
			}
			
			const auto typeInstance = derefType->getObjectOrSpecType();
			
			// Look for methods.
			if (typeInstance->functions().find(CanonicalizeMethodName(memberName)) != typeInstance->functions().end()) {
				return GetMethod(context, value, memberName, location);
			}
			
			// TODO: this should be replaced by falling back on 'property' methods.
			// Look for variables.
			if (typeInstance->isDatatype() || typeInstance->isException() || typeInstance->isStruct()) {
				const auto variableIterator = typeInstance->namedVariables().find(memberName);
				if (variableIterator != typeInstance->namedVariables().end()) {
					return createMemberVarRef(context, value, variableIterator->second);
				}
			}
			
			throw ErrorException(makeString("Can't access member '%s' in type '%s' at position %s.",
				memberName.c_str(), typeInstance->name().toString().c_str(), location.toString().c_str()));
		}
		
		static Name getCanonicalName(const Name& name) {
			return name.getPrefix() + CanonicalizeMethodName(name.last());
		}
		
		static SearchResult performSymbolLookup(Context& context, const Name& name) {
			const auto searchResult = performSearch(context, name);
			if (!searchResult.isNone()) return searchResult;
			
			// Fall back on looking for canonicalized static method names.
			const auto functionSearchResult = performSearch(context, getCanonicalName(name));
			if (!functionSearchResult.isFunction()) {
				return SearchResult::None();
			}
			
			return functionSearchResult;
		}
		
		SEM::Value* ConvertValueData(Context& context, const AST::Node<AST::Value>& astValueNode) {
			assert(astValueNode.get() != nullptr);
			const auto& location = astValueNode.location();
			
			switch (astValueNode->typeEnum) {
				case AST::Value::BRACKET: {
					return ConvertValue(context, astValueNode->bracket.value);
				}
				case AST::Value::SELF: {
					return getSelfValue(context, location);
				}
				case AST::Value::THIS: {
					const auto thisTypeInstance = lookupParentType(context.scopeStack());
					const auto thisFunction = lookupParentFunction(context.scopeStack());
					
					if (thisTypeInstance == nullptr) {
						throw ErrorException(makeString("Cannot access 'this' in non-method at %s.",
							location.toString().c_str()));
					}
					
					if (thisFunction->isStaticMethod()) {
						throw ErrorException(makeString("Cannot access 'this' in static method at %s.",
							location.toString().c_str()));
					}
					
					const auto selfType = thisTypeInstance->selfType();
					const auto selfConstType = thisFunction->isConstMethod() ? selfType->createConstType() : selfType;
					const auto ptrTypeInstance = getBuiltInType(context.scopeStack(), "__ptr");
					return SEM::Value::This(SEM::Type::Object(ptrTypeInstance, { selfConstType })->createConstType());
				}
				case AST::Value::LITERAL: {
					const auto& specifier = astValueNode->literal.specifier;
					auto& constant = *(astValueNode->literal.constant);
					return getLiteralValue(context, specifier, constant, location);
				}
				case AST::Value::SYMBOLREF: {
					const auto& astSymbolNode = astValueNode->symbolRef.symbol;
					const Name name = astSymbolNode->createName();
					
					const auto searchResult = performSymbolLookup(context, name);
					
					// Get a map from template variables to their values (i.e. types).
					const auto templateVarMap = GenerateTemplateVarMap(context, astSymbolNode);
					
					if (searchResult.isNone()) {
						throw ErrorException(makeString("Couldn't find symbol or value '%s' at %s.",
							name.toString().c_str(), location.toString().c_str()));
					} else if (searchResult.isFunction()) {
						const auto function = searchResult.function();
						assert(function != nullptr && "Function pointer must not be NULL (as indicated by isFunction() being true)");
						
						const auto functionTemplateArguments = GetTemplateValues(templateVarMap, function->templateVariables());
						
						if (function->isMethod()) {
							if (!function->isStaticMethod()) {
								throw ErrorException(makeString("Cannot refer directly to non-static class method '%s' at %s.",
									name.toString().c_str(), location.toString().c_str()));
							}
							
							const auto typeSearchResult = performSearch(context, name.getPrefix());
							assert(typeSearchResult.isTypeInstance());
							
							const auto typeInstance = typeSearchResult.typeInstance();
							
							const auto parentTemplateArguments = GetTemplateValues(templateVarMap, typeInstance->templateVariables());
							const auto parentType = SEM::Type::Object(typeInstance, parentTemplateArguments);
							
							return SEM::Value::FunctionRef(parentType, function, functionTemplateArguments, templateVarMap);
						} else {
							return SEM::Value::FunctionRef(nullptr, function, functionTemplateArguments, templateVarMap);
						}
					} else if (searchResult.isTypeInstance()) {
						const auto typeInstance = searchResult.typeInstance();
						
						if (typeInstance->isInterface()) {
							throw ErrorException(makeString("Can't construct interface type '%s' at %s.",
								name.toString().c_str(), location.toString().c_str()));
						}
						
						const auto parentType = SEM::Type::Object(typeInstance, GetTemplateValues(templateVarMap, typeInstance->templateVariables()));
						return GetStaticMethod(parentType, "create", location);
					} else if (searchResult.isTypeAlias()) {
						const auto typeAlias = searchResult.typeAlias();
						const auto templateArguments = GetTemplateValues(templateVarMap, typeAlias->templateVariables());
						assert(templateArguments.size() == typeAlias->templateVariables().size());
						const auto resolvedType = SEM::Type::Alias(typeAlias, templateArguments)->resolveAliases();
						return GetStaticMethod(resolvedType, "create", location);
					} else if (searchResult.isVar()) {
						// Variables must just be a single plain string,
						// and be a relative name (so no precending '::').
						assert(astSymbolNode->size() == 1);
						assert(astSymbolNode->isRelative());
						assert(astSymbolNode->first()->templateArguments()->empty());
						const auto referenceTypeInst = getBuiltInType(context.scopeStack(), "__ref");
						const auto var = searchResult.var();
						return SEM::Value::LocalVar(var, SEM::Type::Object(referenceTypeInst, { var->type() })->createRefType(var->type()));
					} else if (searchResult.isTemplateVar()) {
						assert(templateVarMap.empty() && "Template vars cannot have template arguments.");
						const auto templateVar = searchResult.templateVar();
						return GetStaticMethod(SEM::Type::TemplateVarRef(templateVar), "create", location);
					} else {
						throw std::runtime_error("Unknown search result for name reference.");
					}
					
					throw std::runtime_error("Invalid if-statement fallthrough in ConvertValue for name reference.");
				}
				case AST::Value::MEMBERREF: {
					const auto& memberName = astValueNode->memberRef.name;
					const auto selfValue = getSelfValue(context, location);
					
					const auto derefType = getDerefType(selfValue->type());
					assert(derefType->isObject());
					
					const auto typeInstance = derefType->getObjectType();
					const auto variableIterator = typeInstance->namedVariables().find(memberName);
					
					if (variableIterator == typeInstance->namedVariables().end()) {
						throw ErrorException(makeString("Member variable '@%s' not found at position %s.",
							memberName.c_str(), location.toString().c_str()));
					}
					
					return createMemberVarRef(context, selfValue, variableIterator->second);
				}
				case AST::Value::SIZEOF: {
					return SEM::Value::SizeOf(ConvertType(context, astValueNode->sizeOf.type), getBuiltInType(context.scopeStack(), "size_t")->selfType());
				}
				case AST::Value::BINARYOP: {
					const auto binaryOp = astValueNode->binaryOp.kind;
					const auto leftOperand = ConvertValue(context, astValueNode->binaryOp.leftOperand);
					const auto rightOperand = ConvertValue(context, astValueNode->binaryOp.rightOperand);
					
					const auto objectValue = tryDissolveValue(context, leftOperand, location);
					
					switch (binaryOp) {
						case AST::OP_ISEQUAL: {
							const auto opMethod = GetBinaryOp(context, objectValue, binaryOp, location);
							if (opMethod != nullptr) {
								return CallValue(context, opMethod, { rightOperand }, location);
							} else {
								// Fall back on 'compare' method.
								const auto compareMethod = GetMethod(context, objectValue, "compare", location);
								const auto compareResult = CallValue(context, compareMethod, { rightOperand }, location);
								const auto isEqualMethod = GetMethod(context, compareResult, "isEqual", location);
								return CallValue(context, isEqualMethod, {}, location);
							}
						}
						case AST::OP_NOTEQUAL: {
							const auto opMethod = GetBinaryOp(context, objectValue, binaryOp, location);
							if (opMethod != nullptr) {
								return CallValue(context, opMethod, { rightOperand }, location);
							} else {
								// Fall back on 'compare' method.
								const auto compareMethod = GetMethod(context, objectValue, "compare", location);
								const auto compareResult = CallValue(context, compareMethod, { rightOperand }, location);
								const auto isNotEqualMethod = GetMethod(context, compareResult, "isNotEqual", location);
								return CallValue(context, isNotEqualMethod, {}, location);
							}
						}
						case AST::OP_LESSTHAN: {
							const auto opMethod = GetBinaryOp(context, objectValue, binaryOp, location);
							if (opMethod != nullptr) {
								return CallValue(context, opMethod, { rightOperand }, location);
							} else {
								// Fall back on 'compare' method.
								const auto compareMethod = GetMethod(context, objectValue, "compare", location);
								const auto compareResult = CallValue(context, compareMethod, { rightOperand }, location);
								const auto isNotEqualMethod = GetMethod(context, compareResult, "isLessThan", location);
								return CallValue(context, isNotEqualMethod, {}, location);
							}
						}
						case AST::OP_LESSTHANOREQUAL: {
							const auto opMethod = GetBinaryOp(context, objectValue, binaryOp, location);
							if (opMethod != nullptr) {
								return CallValue(context, opMethod, { rightOperand }, location);
							} else {
								// Fall back on 'compare' method.
								const auto compareMethod = GetMethod(context, objectValue, "compare", location);
								const auto compareResult = CallValue(context, compareMethod, { rightOperand }, location);
								const auto isNotEqualMethod = GetMethod(context, compareResult, "isLessThanOrEqual", location);
								return CallValue(context, isNotEqualMethod, {}, location);
							}
						}
						case AST::OP_GREATERTHAN: {
							const auto opMethod = GetBinaryOp(context, objectValue, binaryOp, location);
							if (opMethod != nullptr) {
								return CallValue(context, opMethod, { rightOperand }, location);
							} else {
								// Fall back on 'compare' method.
								const auto compareMethod = GetMethod(context, objectValue, "compare", location);
								const auto compareResult = CallValue(context, compareMethod, { rightOperand }, location);
								const auto isNotEqualMethod = GetMethod(context, compareResult, "isGreaterThan", location);
								return CallValue(context, isNotEqualMethod, {}, location);
							}
						}
						case AST::OP_GREATERTHANOREQUAL: {
							const auto opMethod = GetBinaryOp(context, objectValue, binaryOp, location);
							if (opMethod != nullptr) {
								return CallValue(context, opMethod, { rightOperand }, location);
							} else {
								// Fall back on 'compare' method.
								const auto compareMethod = GetMethod(context, objectValue, "compare", location);
								const auto compareResult = CallValue(context, compareMethod, { rightOperand }, location);
								const auto isNotEqualMethod = GetMethod(context, compareResult, "isGreaterThanOrEqual", location);
								return CallValue(context, isNotEqualMethod, {}, location);
							}
						}
						case AST::OP_LOGICALAND: {
							const auto boolType = getBuiltInType(context.scopeStack(), "bool");
							const auto boolValue = ImplicitCast(context, leftOperand, boolType->selfType(), location);
							
							// Logical AND only evaluates the right operand if the left
							// operand is TRUE, otherwise it returns FALSE.
							return SEM::Value::Ternary(boolValue, rightOperand, SEM::Value::Constant(Constant::False(), boolType->selfType()));
						}
						case AST::OP_LOGICALOR: {
							const auto boolType = getBuiltInType(context.scopeStack(), "bool");
							const auto boolValue = ImplicitCast(context, leftOperand, boolType->selfType(), location);
							
							// Logical OR only evaluates the right operand if the left
							// operand is FALSE, otherwise it returns TRUE.
							return SEM::Value::Ternary(boolValue, SEM::Value::Constant(Constant::True(), boolType->selfType()), rightOperand);
						}
						default:
							throw std::runtime_error("Unknown binary op kind.");
					}
				}
				case AST::Value::TERNARY: {
					const auto cond = ConvertValue(context, astValueNode->ternary.condition);
					
					const auto boolType = getBuiltInType(context.scopeStack(), "bool");
					const auto boolValue = ImplicitCast(context, cond, boolType->selfType(), location);
					
					const auto ifTrue = ConvertValue(context, astValueNode->ternary.ifTrue);
					const auto ifFalse = ConvertValue(context, astValueNode->ternary.ifFalse);
					
					const auto targetType = UnifyTypes(context, ifTrue->type(), ifFalse->type(), location);
					
					const auto castIfTrue = ImplicitCast(context, ifTrue, targetType, location);
					const auto castIfFalse = ImplicitCast(context, ifFalse, targetType, location);
					
					return SEM::Value::Ternary(boolValue, castIfTrue, castIfFalse);
				}
				case AST::Value::CAST: {
					const auto sourceValue = ConvertValue(context, astValueNode->cast.value);
					const auto sourceType = ConvertType(context, astValueNode->cast.sourceType);
					const auto targetType = ConvertType(context, astValueNode->cast.targetType);
					
					switch(astValueNode->cast.castKind) {
						case AST::Value::CAST_STATIC: {
							throw ErrorException("static_cast not yet implemented.");
						}
						case AST::Value::CAST_CONST:
							throw ErrorException("const_cast not yet implemented.");
						case AST::Value::CAST_DYNAMIC:
							throw ErrorException("dynamic_cast not yet implemented.");
						case AST::Value::CAST_REINTERPRET:
							if (!sourceType->isPrimitive() || sourceType->getObjectType()->name().last() != "__ptr"
								|| !targetType->isPrimitive() || targetType->getObjectType()->name().last() != "__ptr") {
								throw ErrorException(makeString("reinterpret_cast currently only supports ptr<T>, "
									"in cast from value %s of type %s to type %s at position %s.",
									sourceValue->toString().c_str(), sourceType->toString().c_str(),
									targetType->toString().c_str(), location.toString().c_str()));
							}
							return SEM::Value::Reinterpret(ImplicitCast(context, sourceValue, sourceType, location), targetType);
						default:
							throw std::runtime_error("Unknown cast kind.");
					}
				}
				case AST::Value::LVAL: {
					const auto sourceValue = ConvertValue(context, astValueNode->makeLval.value);
					
					if (sourceValue->type()->isLval()) {
						throw ErrorException(makeString("Can't create lval of value that is already a lval, for value '%s' at position %s.",
							sourceValue->toString().c_str(), location.toString().c_str()));
					}
					
					if (sourceValue->type()->isRef()) {
						throw ErrorException(makeString("Can't create value that is both an lval and a ref, for value '%s' at position %s.",
							sourceValue->toString().c_str(), location.toString().c_str()));
					}
					
					const auto targetType = ConvertType(context, astValueNode->makeLval.targetType);
					return SEM::Value::Lval(targetType, sourceValue);
				}
				case AST::Value::NOLVAL: {
					const auto sourceValue = ConvertValue(context, astValueNode->makeNoLval.value);
					
					if (!sourceValue->type()->isLval()) {
						throw ErrorException(makeString("Can't use 'nolval' operator on non-lval value '%s' at position '%s'.",
							sourceValue->toString().c_str(), location.toString().c_str()));
					}
					
					return SEM::Value::NoLval(sourceValue);
				}
				case AST::Value::REF: {
					const auto sourceValue = ConvertValue(context, astValueNode->makeRef.value);
					
					if (sourceValue->type()->isLval()) {
						throw ErrorException(makeString("Can't create value that is both an lval and a ref, for value '%s' at position %s.",
							sourceValue->toString().c_str(), location.toString().c_str()));
					}
					
					if (sourceValue->type()->isRef()) {
						throw ErrorException(makeString("Can't create ref of value that is already a ref, for value '%s' at position %s.",
							sourceValue->toString().c_str(), location.toString().c_str()));
					}
					
					const auto targetType = ConvertType(context, astValueNode->makeRef.targetType);
					return SEM::Value::Ref(targetType, sourceValue);
				}
				case AST::Value::NOREF: {
					const auto sourceValue = ConvertValue(context, astValueNode->makeNoRef.value);
					
					if (!sourceValue->type()->isRef()) {
						throw ErrorException(makeString("Can't use 'noref' operator on non-ref value '%s' at position '%s'.",
							sourceValue->toString().c_str(), location.toString().c_str()));
					}
					
					return SEM::Value::NoRef(sourceValue);
				}
				case AST::Value::INTERNALCONSTRUCT: {
					const auto& astParameterValueNodes = astValueNode->internalConstruct.parameters;
					
					const auto thisTypeInstance = lookupParentType(context.scopeStack());
					
					if (thisTypeInstance == nullptr) {
						throw ErrorException(makeString("Cannot call internal constructor in non-method at position %s.",
							location.toString().c_str()));
					}
					
					if (astParameterValueNodes->size() != thisTypeInstance->variables().size()) {
						throw ErrorException(makeString("Internal constructor called "
							   "with wrong number of arguments; received %llu, expected %llu at position %s.",
							(unsigned long long) astParameterValueNodes->size(),
							(unsigned long long) thisTypeInstance->variables().size(),
							location.toString().c_str()));
					}
					
					std::vector<SEM::Value*> semValues;
					
					for(size_t i = 0; i < thisTypeInstance->variables().size(); i++){
						const auto semVar = thisTypeInstance->variables().at(i);
						const auto semValue = ConvertValue(context, astParameterValueNodes->at(i));
						const auto semParam = ImplicitCast(context, semValue, semVar->constructType(), location);
						semValues.push_back(semParam);
					}
					
					return SEM::Value::InternalConstruct(thisTypeInstance, semValues);
				}
				case AST::Value::MEMBERACCESS: {
					const auto& memberName = astValueNode->memberAccess.memberName;
					
					auto object = ConvertValue(context, astValueNode->memberAccess.object);
					
					if (memberName != "address" && memberName != "assign" && memberName != "dissolve" && memberName != "move") {
						object = tryDissolveValue(context, object, location);
					}
					
					return MakeMemberAccess(context, object, memberName, astValueNode.location());
				}
				case AST::Value::TEMPLATEDMEMBERACCESS: {
					const auto& memberName = astValueNode->templatedMemberAccess.memberName;
					const auto object = ConvertValue(context, astValueNode->templatedMemberAccess.object);
					
					std::vector<SEM::Type*> templateArguments;
					templateArguments.reserve(astValueNode->templatedMemberAccess.typeList->size());
					
					for (const auto typeArg: *(astValueNode->templatedMemberAccess.typeList)) {
						templateArguments.push_back(ConvertType(context, typeArg));
					}
					
					return GetTemplatedMethod(context, object, memberName, templateArguments, astValueNode.location());
				}
				case AST::Value::FUNCTIONCALL: {
					const auto functionValue = ConvertValue(context, astValueNode->functionCall.functionValue);
					
					std::vector<SEM::Value*> argumentValues;
					
					for (const auto& astArgumentValueNode: *(astValueNode->functionCall.parameters)) {
						argumentValues.push_back(ConvertValue(context, astArgumentValueNode));
					}
					
					return CallValue(context, functionValue, argumentValues, location);
				}
				default:
					throw std::runtime_error("Unknown AST::Value kind.");
			}
		}
		
		Debug::ValueInfo makeValueInfo(const AST::Node<AST::Value>& astValueNode) {
			Debug::ValueInfo valueInfo;
			valueInfo.location = astValueNode.location();
			return valueInfo;
		}
		
		SEM::Value* ConvertValue(Context& context, const AST::Node<AST::Value>& astValueNode) {
			const auto semValue = ConvertValueData(context, astValueNode);
			context.debugModule().valueMap.insert(std::make_pair(semValue, makeValueInfo(astValueNode)));
			return semValue;
		}
		
	}
	
}


