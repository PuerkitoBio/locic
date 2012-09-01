#include <cstddef>
#include <cstdio>
#include <list>
#include <Locic/AST.hpp>
#include <Locic/SEM.hpp>
#include <Locic/SemanticAnalysis/Context.hpp>
#include <Locic/SemanticAnalysis/ConvertClassDef.hpp>
#include <Locic/SemanticAnalysis/ConvertFunctionDef.hpp>
#include <Locic/SemanticAnalysis/ConvertModule.hpp>

namespace Locic {

	namespace SemanticAnalysis {
	
		bool ConvertModule(Context& context, AST::Module* module, SEM::Module* semModule) {
			ModuleContext moduleContext(context, semModule);
			
			for(std::size_t i = 0; i < module->functions.size(); i++) {
				AST::Function* astFunction = module->functions.at(i);
				
				if(astFunction->typeEnum == AST::Function::DEFINITION){
					SEM::Function * semFunction = ConvertFunctionDef(moduleContext, astFunction);
					
					if(semFunction == NULL) return false;
					
					semModule->functions.push_back(semFunction);
				}
			}
			
			for(std::size_t i = 0; i < module->typeInstances.size(); i++){
				AST::TypeInstance * astTypeInstance = module->typeInstances.at(i);
				
				if(astTypeInstance->typeEnum == AST::TypeInstance::CLASSDEF){
					SEM::TypeInstance * semTypeInstance = ConvertClassDef(moduleContext, astTypeInstance);
					
					if(semTypeInstance == NULL) return false;
					
					semModule->typeInstances.push_back(semTypeInstance);
				}
			}
			
			return true;
		}
		
	}
	
}

