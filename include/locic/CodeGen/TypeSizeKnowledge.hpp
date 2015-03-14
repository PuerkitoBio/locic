#ifndef LOCIC_CODEGEN_TYPESIZEKNOWLEDGE_HPP
#define LOCIC_CODEGEN_TYPESIZEKNOWLEDGE_HPP

#include <locic/SEM.hpp>

#include <locic/CodeGen/Module.hpp>

namespace locic {

	namespace CodeGen {
		
		bool isObjectTypeSizeKnownInThisModule(Module& module, const SEM::TypeInstance* objectType);
		
		bool isTypeSizeKnownInThisModule(Module& module, const SEM::Type* type);
		
		bool isTypeSizeAlwaysKnown(Module& module, const SEM::Type* type);
		
	}
	
}

#endif
