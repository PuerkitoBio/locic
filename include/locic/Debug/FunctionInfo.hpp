#ifndef LOCIC_DEBUG_FUNCTIONINFO_HPP
#define LOCIC_DEBUG_FUNCTIONINFO_HPP

#include <locic/Debug/SourceLocation.hpp>
#include <locic/Name.hpp>

namespace locic {

	namespace Debug {
		
		struct FunctionInfo {
			bool isDefinition;
			Name name;
			SourceLocation declLocation;
			SourceLocation scopeLocation;
			
			inline FunctionInfo()
				: isDefinition(false), name(), declLocation(SourceLocation::Null()), scopeLocation(SourceLocation::Null()) { }
			
			FunctionInfo(const FunctionInfo& other)
			: isDefinition(other.isDefinition),
			name(other.name.copy()),
			declLocation(other.declLocation),
			scopeLocation(other.scopeLocation) { }
		};
		
	}
	
}

#endif
