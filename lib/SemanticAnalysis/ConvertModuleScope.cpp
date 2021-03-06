#include <locic/AST/ModuleScope.hpp>
#include <locic/AST/Node.hpp>
#include <locic/SEM/ModuleScope.hpp>
#include <locic/Support/Name.hpp>

namespace locic {

	namespace SemanticAnalysis {
		
		Name stringListToName(const AST::Node<AST::StringList>& astStringListNode) {
			Name name = Name::Absolute();
			for (const auto& stringNode: *astStringListNode) {
				name = name + stringNode;
			}
			return name;
		}
		
		SEM::ModuleScope ConvertModuleScope(const AST::Node<AST::ModuleScope>& astModuleScopeNode) {
			if (astModuleScopeNode->kind == AST::ModuleScope::IMPORT) {
				if (astModuleScopeNode->isNamed) {
					return SEM::ModuleScope::Import(stringListToName(astModuleScopeNode->moduleName), *(astModuleScopeNode->version));
				} else {
					return SEM::ModuleScope::Import(Name::Absolute(), Version(0, 0, 0));
				}
			} else {
				if (astModuleScopeNode->isNamed) {
					return SEM::ModuleScope::Export(stringListToName(astModuleScopeNode->moduleName), *(astModuleScopeNode->version));
				} else {
					return SEM::ModuleScope::Export(Name::Absolute(), Version(0, 0, 0));
				}
			}
		}
		
	}
	
}
