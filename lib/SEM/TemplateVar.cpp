#include <locic/MakeString.hpp>
#include <locic/Support/String.hpp>

#include <locic/SEM/TemplateVar.hpp>
#include <locic/SEM/Type.hpp>
#include <locic/SEM/TypeInstance.hpp>

namespace locic {

	namespace SEM {
	
		TemplateVar::TemplateVar(Context& argContext, Name argName, size_t i)
			: context_(argContext), name_(std::move(argName)), index_(i) { }
		
		Context& TemplateVar::context() const {
			return context_;
		}
		
		const Name& TemplateVar::name() const {
			return name_;
		}
		
		size_t TemplateVar::index() const {
			return index_;
		}
		
		void TemplateVar::setDebugInfo(const Debug::TemplateVarInfo newDebugInfo) {
			debugInfo_ = make_optional(newDebugInfo);
		}
		
		Optional<Debug::TemplateVarInfo> TemplateVar::debugInfo() const {
			return debugInfo_;
		}
		
		std::string TemplateVar::toString() const {
			return makeString("TemplateVar(name = %s, index = %llu)",
				name().toString().c_str(),
				(unsigned long long) index());
		}
		
	}
	
}

