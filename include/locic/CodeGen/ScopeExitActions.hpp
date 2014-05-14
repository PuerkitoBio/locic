#ifndef LOCIC_CODEGEN_SCOPEEXITACTIONS_HPP
#define LOCIC_CODEGEN_SCOPEEXITACTIONS_HPP

#include <locic/CodeGen/Function.hpp>
#include <locic/CodeGen/UnwindAction.hpp>

namespace locic {

	namespace CodeGen {
	
		void performScopeExitAction(Function& function, const UnwindAction& unwindAction, bool isExceptionState);
		
		void genScopeExitActions(Function& function, bool isExceptionState = false);
		
		void genAllScopeExitActions(Function& function, bool isExceptionState = false);
		
		class LifetimeScope {
			public:
				LifetimeScope(Function& function);
				~LifetimeScope();
				
			private:
				Function& function_;
			
		};
		
	}
	
}

#endif
