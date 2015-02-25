#ifndef LOCIC_SEMANTICANALYSIS_EXCEPTION_HPP
#define LOCIC_SEMANTICANALYSIS_EXCEPTION_HPP

#include <cassert>
#include <cstddef>
#include <cstdio>
#include <string>
#include <vector>
#include <locic/Exception.hpp>
#include <locic/Name.hpp>
#include <locic/String.hpp>

namespace locic {

	namespace SemanticAnalysis {
	
		class Exception: public locic::Exception { };
		
		class CastException: public Exception { };
		
		class NameClashException: public Exception {
			public:
				enum Kind {
					FUNCTION_WITH_FUNCTION,
					FUNCTION_WITH_NAMESPACE,
					FUNCTION_WITH_TYPE,
					TYPE_WITH_NAMESPACE,
					TYPE_WITH_TYPE
				};
				
				NameClashException(Kind kind, Name name)
					: kind_(kind), name_(std::move(name)) { }
					
				Kind getKind() const {
					return kind_;
				}
				
				const Name& getName() const {
					return name_;
				}
				
				std::string toString() const {
					switch(kind_) {
						case FUNCTION_WITH_FUNCTION: {
							return makeString("Function name '%s' clashes with existing function.", name_.toString().c_str());
						}
						case FUNCTION_WITH_NAMESPACE: {
							return makeString("Function name '%s' clashes with existing namespace.", name_.toString().c_str());
						}
						case FUNCTION_WITH_TYPE: {
							return makeString("Function name '%s' clashes with existing type.", name_.toString().c_str());
						}
						case TYPE_WITH_NAMESPACE: {
							return makeString("Type name '%s' clashes with existing namespace.", name_.toString().c_str());
						}
						case TYPE_WITH_TYPE: {
							return makeString("Type name '%s' clashes with existing type.", name_.toString().c_str());
						}
						default: {
							assert(false);
							return "";
						}
					}
				}
				
			private:
				Kind kind_;
				Name name_;
				
		};
		
		class TemplateVariableClashException: public Exception {
			public:
				TemplateVariableClashException(Name typeName, const std::string& varName)
					: typeName_(std::move(typeName)), varName_(varName) { }
					
				std::string toString() const {
					return makeString("More than one template variable shares name '%s' in type '%s'.", varName_.c_str(), typeName_.toString().c_str());
				}
				
			private:
				Name typeName_;
				std::string varName_;
				
		};
		
		class MemberVariableClashException: public Exception {
			public:
				MemberVariableClashException(Name typeName, const std::string& varName)
					: typeName_(std::move(typeName)), varName_(varName) { }
					
				std::string toString() const {
					return makeString("More than one member variable shares name '%s' in type '%s'.", varName_.c_str(), typeName_.toString().c_str());
				}
				
			private:
				Name typeName_;
				std::string varName_;
				
		};
		
		class NonUnifiableTypeClashException: public Exception {
			public:
				NonUnifiableTypeClashException(Name name)
					: name_(std::move(name)) { }
					
				std::string toString() const {
					return makeString("Non-unifiable types share name '%s'.", name_.toString().c_str());
				}
				
			private:
				Name name_;
				
		};
		
		class NonUnifiableFunctionsException: public Exception {
			public:
				NonUnifiableFunctionsException(Name name, const std::string& newType, const std::string& existingType)
					: name_(std::move(name)), newType_(newType),
					  existingType_(existingType) { }
					  
				std::string toString() const {
					return makeString("Declarations of function '%s' don't match: %s and %s.",
							name_.toString().c_str(),
							newType_.c_str(),
							existingType_.c_str());
				}
				
			private:
				Name name_;
				std::string newType_;
				std::string existingType_;
				
		};
		
		class MultipleFunctionDefinitionsException: public Exception {
			public:
				MultipleFunctionDefinitionsException(Name name)
					: name_(std::move(name)) { }
					
				std::string toString() const {
					return makeString("Function '%s' is defined more than once.", name_.toString().c_str());
				}
				
			private:
				Name name_;
				
		};
		
		class ParamVariableClashException: public Exception {
			public:
				ParamVariableClashException(Name functionName, const std::string& paramName)
					: functionName_(std::move(functionName)), paramName_(paramName) { }
					
				std::string toString() const {
					return makeString("Parameter variable '%s' is defined more than once, in function '%s'.", paramName_.c_str(), functionName_.toString().c_str());
				}
				
			private:
				Name functionName_;
				std::string paramName_;
				
		};
		
		class LocalVariableClashException: public Exception {
			public:
				LocalVariableClashException(Name functionName, const std::string& paramName)
					: functionName_(std::move(functionName)), paramName_(paramName) { }
					
				std::string toString() const {
					return makeString("Local variable '%s' is defined more than once, in function '%s'.", paramName_.c_str(), functionName_.toString().c_str());
				}
				
			private:
				Name functionName_;
				std::string paramName_;
				
		};
		
		class MissingReturnStatementException: public Exception {
			public:
				MissingReturnStatementException(Name functionName)
					: functionName_(std::move(functionName)) { }
					
				std::string toString() const {
					return makeString("Control reaches end of function '%s' with non-void return type; it needs a return statement.", functionName_.toString().c_str());
				}
				
			private:
				Name functionName_;
				
		};
		
		class ParamVoidTypeException: public Exception {
			public:
				ParamVoidTypeException(Name functionName, const std::string& paramName)
					: functionName_(std::move(functionName)), paramName_(paramName) { }
					
				std::string toString() const {
					return makeString("Parameter variable '%s' cannot have void type, in function '%s'.", paramName_.c_str(), functionName_.toString().c_str());
				}
				
			private:
				Name functionName_;
				std::string paramName_;
				
		};
		
		class LocalVariableShadowingException: public Exception {
			public:
				LocalVariableShadowingException(const std::string& varName)
					: varName_(varName) { }
					
				std::string toString() const {
					return makeString("Local variable '%s' shadows existing local variable of the same name.", varName_.c_str());
				}
				
			private:
				std::string varName_;
				
		};
		
		class ErrorException: public Exception {
			public:
				ErrorException(const std::string& message)
					: message_(message) { }
					
				std::string toString() const {
					return message_;
				}
				
			private:
				std::string message_;
				
		};
		
	}
	
}

#endif
