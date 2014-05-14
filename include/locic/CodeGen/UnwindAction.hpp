#ifndef LOCIC_CODEGEN_UNWINDACTION_HPP
#define LOCIC_CODEGEN_UNWINDACTION_HPP

#include <vector>

#include <locic/CodeGen/LLVMIncludes.hpp>
#include <locic/SEM.hpp>

namespace locic {

	namespace CodeGen {
	
		enum ScopeExitState {
			SCOPEEXIT_ALWAYS,
			SCOPEEXIT_SUCCESS,
			SCOPEEXIT_FAILURE
		};
		
		class UnwindAction {
			public:
				static UnwindAction Destroy(SEM::Type* type, llvm::Value* value);
				
				static UnwindAction CatchException(llvm::BasicBlock* catchBlock, llvm::Constant* catchTypeInfo);
				
				static UnwindAction ScopeMarker();
				
				static UnwindAction ControlFlow(llvm::BasicBlock* breakBlock, llvm::BasicBlock* continueBlock);
				
				static UnwindAction ScopeExit(ScopeExitState state, SEM::Scope* scope);
				
				enum Kind {
					DESTRUCTOR,
					CATCH,
					SCOPEMARKER,
					CONTROLFLOW,
					SCOPEEXIT
				};
				
				Kind kind() const;
				
				bool isDestructor() const;
				
				bool isCatch() const;
				
				bool isScopeMarker() const;
				
				bool isControlFlow() const;
				
				bool isScopeExit() const;
				
				SEM::Type* destroyType() const;
				
				llvm::Value* destroyValue() const;
				
				llvm::BasicBlock* catchBlock() const;
				
				llvm::Constant* catchTypeInfo() const;
				
				llvm::BasicBlock* breakBlock() const;
				
				llvm::BasicBlock* continueBlock() const;
				
				ScopeExitState scopeExitState() const;
				
				SEM::Scope* scopeExitScope() const;
				
			private:
				inline UnwindAction(Kind pKind)
					: kind_(pKind) { }
				
				Kind kind_;
				
				union Actions {
					struct DestroyAction {
						SEM::Type* type;
						llvm::Value* value;
					} destroyAction;
					
					struct CatchAction {
						llvm::BasicBlock* block;
						llvm::Constant* typeInfo;
					} catchAction;
					
					struct ControlFlowAction {
						llvm::BasicBlock* breakBlock;
						llvm::BasicBlock* continueBlock;
					} controlFlowAction;
					
					struct ScopeExitAction {
						ScopeExitState state;
						SEM::Scope* scope;
					} scopeExitAction;
				} actions_;
				
		};
		
		typedef std::vector<UnwindAction> UnwindStack;
		
	}
	
}

#endif
