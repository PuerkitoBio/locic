#ifndef LOCIC_SEM_TEMPLATEVAR_HPP
#define LOCIC_SEM_TEMPLATEVAR_HPP

#include <string>

namespace locic {

	namespace SEM {
	
		class Type;
		class TypeInstance;
		
		enum TemplateVarType {
			TEMPLATEVAR_TYPENAME,
			TEMPLATEVAR_POLYMORPHIC
		};
		
		class TemplateVar {
			public:
				TemplateVar(TemplateVarType t);
				
				TemplateVarType type() const;
				
				void setSpecType(Type* spec);
				
				Type* specType() const;
				
				void setSpecTypeInstance(TypeInstance* spec);
				
				TypeInstance* specTypeInstance() const;
				
				std::string toString() const;
				
			private:
				TemplateVarType type_;
				Type* specType_;
				TypeInstance* specTypeInstance_;
				
		};
		
	}
	
}

#endif
