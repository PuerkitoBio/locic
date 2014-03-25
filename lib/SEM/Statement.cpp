#include <assert.h>

#include <string>
#include <vector>

#include <locic/String.hpp>

#include <locic/SEM/CatchClause.hpp>
#include <locic/SEM/Scope.hpp>
#include <locic/SEM/Statement.hpp>
#include <locic/SEM/SwitchCase.hpp>
#include <locic/SEM/Type.hpp>
#include <locic/SEM/Value.hpp>
#include <locic/SEM/Var.hpp>

namespace locic {

	namespace SEM {
	
		Statement* Statement::ValueStmt(Value* value) {
			Statement* statement = new Statement(VALUE);
			statement->valueStmt_.value = value;
			return statement;
		}
		
		Statement* Statement::ScopeStmt(Scope* scope) {
			Statement* statement = new Statement(SCOPE);
			statement->scopeStmt_.scope = scope;
			return statement;
		}
		
		Statement* Statement::InitialiseStmt(Var* var, Value* value) {
			Statement* statement = new Statement(INITIALISE);
			statement->initialiseStmt_.var = var;
			statement->initialiseStmt_.value = value;
			return statement;
		}
		
		Statement* Statement::If(Value* condition, Scope* ifTrue, Scope* ifFalse) {
			Statement* statement = new Statement(IF);
			statement->ifStmt_.condition = condition;
			statement->ifStmt_.ifTrue = ifTrue;
			statement->ifStmt_.ifFalse = ifFalse;
			return statement;
		}
		
		Statement* Statement::Switch(Value* value, const std::vector<SwitchCase*>& caseList) {
			Statement* statement = new Statement(SWITCH);
			statement->switchStmt_.value = value;
			statement->switchStmt_.caseList = caseList;
			return statement;
		}
		
		Statement* Statement::While(Value* condition, Scope* whileTrue) {
			Statement* statement = new Statement(WHILE);
			statement->whileStmt_.condition = condition;
			statement->whileStmt_.whileTrue = whileTrue;
			return statement;
		}
		
		Statement* Statement::Try(Scope* scope, const std::vector<CatchClause*>& catchList) {
			Statement* statement = new Statement(TRY);
			statement->tryStmt_.scope = scope;
			statement->tryStmt_.catchList = catchList;
			return statement;
		}
		
		Statement* Statement::ReturnVoid() {
			Statement* statement = new Statement(RETURN);
			statement->returnStmt_.value = NULL;
			return statement;
		}
		
		Statement* Statement::Return(Value* value) {
			Statement* statement = new Statement(RETURN);
			statement->returnStmt_.value = value;
			return statement;
		}
		
		Statement* Statement::Throw(Value* value) {
			Statement* statement = new Statement(THROW);
			statement->throwStmt_.value = value;
			return statement;
		}
		
		Statement* Statement::Break() {
			return new Statement(BREAK);
		}
		
		Statement* Statement::Continue() {
			return new Statement(CONTINUE);
		}
		
		Statement::Statement(Kind k)
			: kind_(k) { }
			
		Statement::Kind Statement::kind() const {
			return kind_;
		}
		
		bool Statement::isValueStatement() const {
			return kind() == VALUE;
		}
		
		Value* Statement::getValue() const {
			assert(isValueStatement());
			return valueStmt_.value;
		}
		
		bool Statement::isScope() const {
			return kind() == SCOPE;
		}
		
		Scope& Statement::getScope() const {
			assert(isScope());
			return *(scopeStmt_.scope);
		}
		
		bool Statement::isInitialiseStatement() const {
			return kind() == INITIALISE;
		}
		
		Var* Statement::getInitialiseVar() const {
			assert(isInitialiseStatement());
			return initialiseStmt_.var;
		}
		
		Value* Statement::getInitialiseValue() const {
			assert(isInitialiseStatement());
			return initialiseStmt_.value;
		}
		
		bool Statement::isIfStatement() const {
			return kind() == IF;
		}
		
		Value* Statement::getIfCondition() const {
			assert(isIfStatement());
			return ifStmt_.condition;
		}
		
		Scope& Statement::getIfTrueScope() const {
			assert(isIfStatement());
			assert(ifStmt_.ifTrue != NULL);
			return *(ifStmt_.ifTrue);
		}
		
		bool Statement::hasIfFalseScope() const {
			assert(isIfStatement());
			return ifStmt_.ifFalse != NULL;
		}
		
		Scope& Statement::getIfFalseScope() const {
			assert(isIfStatement());
			assert(hasIfFalseScope());
			return *(ifStmt_.ifFalse);
		}
		
		bool Statement::isSwitchStatement() const {
			return kind() == SWITCH;
		}
		
		Value* Statement::getSwitchValue() const {
			assert(isSwitchStatement());
			return switchStmt_.value;
		}
		
		const std::vector<SwitchCase*>& Statement::getSwitchCaseList() const {
			assert(isSwitchStatement());
			return switchStmt_.caseList;
		}
		
		bool Statement::isWhileStatement() const {
			return kind() == WHILE;
		}
		
		Value* Statement::getWhileCondition() const {
			assert(isWhileStatement());
			return whileStmt_.condition;
		}
		
		Scope& Statement::getWhileScope() const {
			assert(isWhileStatement());
			return *(whileStmt_.whileTrue);
		}
		
		bool Statement::isTryStatement() const {
			return kind() == TRY;
		}
		
		Scope& Statement::getTryScope() const {
			assert(isTryStatement());
			return *(tryStmt_.scope);
		}
		
		const std::vector<CatchClause*>& Statement::getTryCatchList() const {
			assert(isTryStatement());
			return tryStmt_.catchList;
		}
		
		bool Statement::isReturnStatement() const {
			return kind() == RETURN;
		}
		
		Value* Statement::getReturnValue() const {
			assert(isReturnStatement());
			return returnStmt_.value;
		}
		
		bool Statement::isThrowStatement() const {
			return kind() == THROW;
		}
		
		Value* Statement::getThrowValue() const {
			assert(isThrowStatement());
			return throwStmt_.value;
		}
		
		bool Statement::isBreakStatement() const {
			return kind() == BREAK;
		}
		
		bool Statement::isContinueStatement() const {
			return kind() == CONTINUE;
		}
		
		std::string Statement::toString() const {
			switch (kind_) {
				case VALUE: {
					return makeString("ValueStatement(value: %s)",
									  valueStmt_.value->toString().c_str());
				}
				
				case SCOPE: {
					return makeString("ScopeStatement(scope: %s)",
									  scopeStmt_.scope->toString().c_str());
				}
				
				case INITIALISE: {
					return makeString("InitialiseStatement(var: %s, value: %s)",
									  initialiseStmt_.var->toString().c_str(),
									  initialiseStmt_.value->toString().c_str());
				}
				
				case IF: {
					return makeString("IfStatement(condition: %s, ifTrue: %s, ifFalse: %s)",
									  ifStmt_.condition->toString().c_str(),
									  ifStmt_.ifTrue->toString().c_str(),
									  ifStmt_.ifFalse->toString().c_str());
				}
				
				case SWITCH: {
					return makeString("SwitchStatement(value: %s, caseList: %s)",
									  switchStmt_.value->toString().c_str(),
									  makeArrayString(switchStmt_.caseList).c_str());
				}
				
				case WHILE: {
					return makeString("WhileStatement(condition: %s, whileTrue: %s)",
									  whileStmt_.condition->toString().c_str(),
									  whileStmt_.whileTrue->toString().c_str());
				}
				
				case TRY: {
					return makeString("TryStatement(scope: %s, catchList: %s)",
									  tryStmt_.scope->toString().c_str(),
									  makeArrayString(tryStmt_.catchList).c_str());
				}
				
				case RETURN: {
					return makeString("ReturnStatement(value: %s)",
									  returnStmt_.value == NULL ? "[VOID]" :
									  returnStmt_.value->toString().c_str());
				}
				
				case THROW: {
					return makeString("ThrowStatement(value: %s)",
									  throwStmt_.value->toString().c_str());
				}
				
				case BREAK: {
					return "BreakStatement";
				}
				
				case CONTINUE: {
					return "ContinueStatement";
				}
				
				default:
					assert(false && "Unknown SEM::Statement kind.");
					return "Statement([INVALID])";
			}
		}
		
	}
	
}

