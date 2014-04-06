#include <assert.h>

#include <stdexcept>

#include <locic/SEM.hpp>

#include <locic/CodeGen/Debug.hpp>
#include <locic/CodeGen/Destructor.hpp>
#include <locic/CodeGen/Function.hpp>
#include <locic/CodeGen/GenVar.hpp>
#include <locic/CodeGen/LLVMIncludes.hpp>
#include <locic/CodeGen/Memory.hpp>
#include <locic/CodeGen/Module.hpp>

namespace locic {

	namespace CodeGen {
	
		void genVarAlloca(Function& function, SEM::Var* var) {
			if (var->isAny()) {
				return;
			}
			
			if (var->isBasic()) {
				auto& module = function.module();
				auto& varMap = module.debugModule().varMap;
				
				// Create an alloca for this variable.
				const auto stackObject = genAlloca(function, var->type());
				
				// Generate debug information for the variable
				// if any is available.
				const auto iterator = varMap.find(var);
				
				if (iterator != varMap.end()) {
					const auto& varInfo = iterator->second;
					const auto debugDeclare = genDebugVar(function, varInfo, genDebugType(module, var->constructType()), stackObject);
					
					const auto varDeclStart = varInfo.declLocation.range().start();
					debugDeclare->setDebugLoc(llvm::DebugLoc::get(varDeclStart.lineNumber(), varDeclStart.column(), function.debugInfo()));
				}
				
				// Add this to the local variable map, so that
				// any SEM vars can be mapped to the actual value.
				function.getLocalVarMap().insert(var, stackObject);
			} else if (var->isComposite()) {
				// Generate child vars.
				for (const auto childVar: var->children()) {
					genVarAlloca(function, childVar);
				}
			} else {
				throw std::runtime_error("Unknown variable kind.");
			}
		}
		
		void genVarInitialise(Function& function, SEM::Var* var, llvm::Value* initialiseValue) {
			if (var->isAny()) {
				// Casting to 'any', which means the destructor
				// should be called for the value.
				genDestructorCall(function, var->constructType(), initialiseValue);
			} else if (var->isBasic()) {
				const auto varValue = function.getLocalVarMap().get(var);
				genStoreVar(function, initialiseValue, varValue, var);
				
				// Add this to the list of variables to be
				// destroyed at the end of the function.
				function.unwindStack().push_back(UnwindAction::Destroy(var->type(), varValue));
			} else if (var->isComposite()) {
				// For composite variables, extract each member of
				// the type and assign it to its variable.
				for (size_t i = 0; i < var->children().size(); i++) {
					const auto childVar = var->children().at(i);
					const auto childInitialiseValue = function.getBuilder().CreateConstInBoundsGEP2_32(initialiseValue, 0, i);
					const auto loadedChildInitialiseValue = genLoad(function, childInitialiseValue, childVar->constructType());
					genVarInitialise(function, childVar, loadedChildInitialiseValue);
				}
			} else {
				throw std::runtime_error("Unknown var kind.");
			}
		}
		
	}
	
}
