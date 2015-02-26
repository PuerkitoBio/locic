#ifndef LOCIC_DEBUG_VARINFO_HPP
#define LOCIC_DEBUG_VARINFO_HPP

#include <string>

#include <locic/Debug/SourceLocation.hpp>
#include <locic/String.hpp>

namespace locic {

	namespace Debug {
		
		struct VarInfo {
			enum Kind {
				VAR_AUTO,
				VAR_ARG,
				VAR_MEMBER
			} kind;
			String name;
			SourceLocation declLocation;
			SourceLocation scopeLocation;
			
			inline VarInfo()
			: kind(VAR_AUTO), declLocation(SourceLocation::Null()), scopeLocation(SourceLocation::Null()) { }
		};
		
	}
	
}

#endif
