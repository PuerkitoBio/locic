/* Parser */

%{

#include <cassert>
#include <cstdio>
#include <list>
#include <string>
#include <vector>
#include <Locic/AST.hpp>
#include <Locic/Name.hpp>
#include <Locic/Parser/Context.hpp>
#include <Locic/Parser/Lexer.hpp>
#include <Locic/Parser/Token.hpp>

int Locic_Parser_GeneratedParser_error(void * scanner, Locic::Parser::Context * parserContext, const char *s);
int Locic_Parser_GeneratedParser_lex(Locic::Parser::Token * token, void * lexer, Locic::Parser::Context * parserContext);

%}

// ================ Options ================
%start start

// Parser must be reentrant.
%define api.pure

// Prefix generated symbols.
%define api.prefix Locic_Parser_GeneratedParser_

%glr-parser

%lex-param {void * scanner}
%lex-param {Locic::Parser::Context * parserContext}
%parse-param {void * scanner}
%parse-param {Locic::Parser::Context * parserContext}

%union{
	// Names.
	std::string * str;
	Locic::Name * name;
	
	// Constants.
	bool boolValue;
	int intValue;
	float floatValue;
	
	// Structures.
	AST::Module * module;
	AST::Namespace * nameSpace;
	AST::TypeInstance * typeInstance;
	AST::Function * function;
	std::vector<AST::Function *> * functionArray;
	
	// Type information.
	AST::Type * type;
	std::vector<AST::Type *> * typeArray;
	AST::TypeVar * typeVar;
	std::vector<AST::TypeVar *> * typeVarArray;
	
	// Program code.
	AST::Scope * scope;
	AST::Statement * statement;
	std::vector<AST::Statement *> * statementArray;
	
	// Values.
	AST::Value * value;
	std::vector<AST::Value *> * valueArray;
}

// ================ Terminals ================
%token <str> NAME
%token <boolValue> BOOLCONSTANT
%token <intValue> INTCONSTANT
%token <floatValue> FLOATCONSTANT
%token <str> STRINGCONSTANT
%token <str> CSTRINGCONSTANT

%token UNKNOWN
%token ERROR
%token INTERFACE
%token SEMICOLON
%token NAMESPACE
%token LCURLYBRACKET
%token RCURLYBRACKET
%token AUTO
%token STATIC
%token LROUNDBRACKET
%token RROUNDBRACKET
%token PRIMITIVE
%token STRUCT
%token CLASS
%token COLON
%token VOIDNAME
%token CONST
%token STAR
%token COMMA
%token IF
%token ELSE
%token FOR
%token WHILE
%token SETEQUAL
%token ADDEQUAL
%token SUBEQUAL
%token MULEQUAL
%token DIVEQUAL
%token RETURN
%token AT
%token NULLVAL
%token CAST
%token LTRIBRACKET
%token RTRIBRACKET
%token DOT
%token PTRACCESS
%token PLUS
%token MINUS
%token EXCLAIMMARK
%token AMPERSAND
%token FORWARDSLASH
%token PERCENT
%token ISEQUAL
%token NOTEQUAL
%token GREATEROREQUAL
%token LESSOREQUAL
%token QUESTIONMARK

// ================ Non-Terminals ================
%type <module> module
%type <nameSpace> nameSpace
%type <nameSpace> namedNamespace

%type <typeInstance> typeInstance

%type <function> functionDecl
%type <function> functionDef

%type <function> staticFunctionDecl
%type <function> staticFunctionDef
%type <function> classFunctionDecl
%type <function> classFunctionDef
%type <functionArray> classFunctionDeclList
%type <functionArray> classFunctionDefList

%type <type> typePrecision2
%type <type> typePrecision1
%type <type> typePrecision0
%type <type> type
%type <typeArray> nonEmptyTypeList
%type <typeArray> typeList
%type <typeVar> typeVar
%type <typeVarArray> nonEmptyTypeVarList
%type <typeVarArray> typeVarList
%type <typeVarArray> structVarList

%type <name> fullName

%type <scope> scope
%type <statementArray> statementList
%type <statement> scopedStatement
%type <statement> normalStatement

%type <value> value
%type <valueArray> nonEmptyValueList
%type <valueArray> valueList
%type <value> precision0
%type <value> precision1
%type <value> precision2
%type <value> precision3
%type <value> precision4
%type <value> precision5
%type <value> precision6
%type <value> precision7

// ================ Rules ================
%%
start:
	module
	{
		parserContext->module = $1;
	}
	;

module:
	nameSpace
	{
		$$ = new AST::Module(parserContext->moduleName, $1);
	}
	;

nameSpace:
	// empty
	{
		$$ = new AST::Namespace("");
	}
	| nameSpace functionDecl
	{
		($1)->functions.push_back($2);
		$$ = $1;
	}
	| nameSpace functionDef
	{
		($1)->functions.push_back($2);
		$$ = $1;
	}
	| nameSpace typeInstance
	{
		($1)->typeInstances.push_back($2);
		$$ = $1;
	}
	| nameSpace namedNamespace
	{
		($1)->namespaces.push_back($2);
		$$ = $1;
	}
	| nameSpace SEMICOLON
	{
		$$ = $1;
	}
	| nameSpace error
	{
		parserContext->error("Invalid struct, class, function or other.");
		$$ = $1;
	}
	;

namedNamespace:
	NAMESPACE NAME LCURLYBRACKET nameSpace RCURLYBRACKET
	{
		($4)->name = *($2);
		$$ = $4;
	}
	;

structVarList:
	// empty
	{
		$$ = new std::vector<AST::TypeVar *>();
	}
	;

structVarList:
	structVarList typeVar SEMICOLON
	{
		($1)->push_back($2);
		$$ = $1;
	}
	| structVarList SEMICOLON
	{
		$$ = $1;
	}
	;

staticFunctionDecl:
	STATIC AUTO NAME LROUNDBRACKET typeVarList RROUNDBRACKET SEMICOLON
	{
		$$ = AST::Function::Decl(AST::Type::UndefinedType(), *($3), *($5));
	}
	| STATIC NAME LROUNDBRACKET typeVarList RROUNDBRACKET SEMICOLON
	{
		$$ = AST::Function::Decl(AST::Type::UndefinedType(), *($2), *($4));
	}
	| STATIC type NAME LROUNDBRACKET typeVarList RROUNDBRACKET SEMICOLON
	{
		$$ = AST::Function::Decl($2, *($3), *($5));
	}
	;

staticFunctionDef:
	STATIC AUTO NAME LROUNDBRACKET typeVarList RROUNDBRACKET scope
	{
		$$ = AST::Function::Def(AST::Type::UndefinedType(), *($3), *($5), $7);
	}
	| STATIC NAME LROUNDBRACKET typeVarList RROUNDBRACKET scope
	{
		$$ = AST::Function::Def(AST::Type::UndefinedType(), *($2), *($4), $6);
	}
	| STATIC type NAME LROUNDBRACKET typeVarList RROUNDBRACKET scope
	{
		$$ = AST::Function::Def($2, *($3), *($5), $7);
	}
	;
	
functionDecl:
	type NAME LROUNDBRACKET typeVarList RROUNDBRACKET SEMICOLON
	{
		$$ = AST::Function::Decl($1, *($2), *($4));
	}
	| type NAME LROUNDBRACKET nonEmptyTypeVarList COMMA DOT DOT DOT RROUNDBRACKET SEMICOLON
	{
		$$ = AST::Function::VarArgDecl($1, *($2), *($4));
	}
	| type NAME LROUNDBRACKET typeVarList RROUNDBRACKET error
	{
		parserContext->error("Function declaration must be terminated with a semicolon.");
		$$ = NULL;
	}
	| type NAME LROUNDBRACKET typeVarList COMMA DOT DOT DOT RROUNDBRACKET error
	{
		parserContext->error("Function declaration must be terminated with a semicolon.");
		$$ = NULL;
	}
	;
	
functionDef:
	type NAME LROUNDBRACKET typeVarList RROUNDBRACKET scope
	{
		$$ = AST::Function::Def($1, *($2), *($4), $6);
	}
	;
	
classFunctionDecl:
	staticFunctionDecl
	{
		$$ = $1;
	}
	| functionDecl
	{
		($1)->isMethod = true;
		$$ = $1;
	}
	;
	
classFunctionDef:
	staticFunctionDef
	{
		$$ = $1;
	}
	| functionDef
	{
		($1)->isMethod = true;
		$$ = $1;
	}
	;

classFunctionDeclList:
	// empty
	{
		$$ = new std::vector<AST::Function *>();
	}
	| classFunctionDeclList classFunctionDecl
	{
		($1)->push_back($2);
		$$ = $1;
	}
	;
	
classFunctionDefList:
	// empty
	{
		$$ = new std::vector<AST::Function *>();
	}
	| classFunctionDefList classFunctionDef
	{
		($1)->push_back($2);
		$$ = $1;
	}
	;

typeInstance:
	STRUCT NAME LCURLYBRACKET structVarList RCURLYBRACKET
	{
		$$ = AST::TypeInstance::Struct(*($2), *($4));
	}
	| CLASS NAME LCURLYBRACKET classFunctionDeclList RCURLYBRACKET
	{
		$$ = AST::TypeInstance::ClassDecl(*($2), *($4));
	}
	| CLASS NAME LROUNDBRACKET typeVarList RROUNDBRACKET LCURLYBRACKET classFunctionDefList RCURLYBRACKET
	{
		$$ = AST::TypeInstance::ClassDef(*($2), *($4), *($7));
	}
	| PRIMITIVE NAME LCURLYBRACKET classFunctionDeclList RCURLYBRACKET
	{
		$$ = AST::TypeInstance::Primitive(*($2), *($4));
	}
	;
	
fullName:
	NAME
	{
		$$ = new Locic::Name(Locic::Name::Relative() + *($1));
	}
	| COLON COLON NAME
	{
		$$ = new Locic::Name(Locic::Name::Absolute() + *($3));
	}
	| fullName COLON COLON NAME
	{
		$$ = new Locic::Name(*($1) + *($4));
	}
	;
	
typePrecision2:
	VOIDNAME
	{
		$$ = AST::Type::VoidType();
	}
	| fullName
	{
		const bool isMutable = true;
		$$ = AST::Type::Named(isMutable, *($1));
	}
	| CONST LROUNDBRACKET type RROUNDBRACKET
	{
		$$ = ($3)->applyTransitiveConst();
	}
	| LROUNDBRACKET type RROUNDBRACKET LROUNDBRACKET typeList RROUNDBRACKET
	{
		const bool isMutable = true;
		$$ = AST::Type::Function(isMutable, $2, *($5));
	}
	| LROUNDBRACKET type RROUNDBRACKET LROUNDBRACKET nonEmptyTypeList COMMA DOT DOT DOT RROUNDBRACKET
	{
		const bool isMutable = true;
		$$ = AST::Type::VarArgFunction(isMutable, $2, *($5));
	}
	| CONST LROUNDBRACKET error RROUNDBRACKET
	{
		parserContext->error("Invalid type.");
		$$ = NULL;
	}
	;

typePrecision1:
	typePrecision2
	{
		$$ = $1;
	}
	| typePrecision1 STAR
	{
		$$ = AST::Type::Pointer($1);
	}
	;
	
typePrecision0:
	typePrecision1
	{
		$$ = $1;
	}
	| CONST typePrecision1
	{
		$$ = ($2)->applyTransitiveConst();
	}
	;

type:
	typePrecision0
	{
		$$ = $1;
	}
	;
	
nonEmptyTypeList:
	type
	{
		$$ = new std::vector<AST::Type *>(1, $1);
	}
	| nonEmptyTypeList COMMA type
	{
		($1)->push_back($3);
		$$ = $1;
	}
	;
	
typeList:
	// empty
	{
		$$ = new std::vector<AST::Type *>();
	}
	| nonEmptyTypeList
	{
		$$ = $1;
	}
	;
	
typeVar:
	type NAME
	{
		$$ = new AST::TypeVar($1, *($2));
	}
	;
	
typeVarList:
	// empty
	{
		$$ = new std::vector<AST::TypeVar *>();
	}
	| nonEmptyTypeVarList
	{
		$$ = $1;
	}
	;
	
nonEmptyTypeVarList:
	typeVar
	{
		$$ = new std::vector<AST::TypeVar *>(1, $1);
	}
	| nonEmptyTypeVarList COMMA typeVar
	{
		($1)->push_back($3);
		$$ = $1;
	}
	;
	
valueList:
	// empty
	{
		$$ = new std::vector<AST::Value *>();
	}
	| nonEmptyValueList
	{
		$$ = $1;
	}
	;
	
nonEmptyValueList:
	value
	{
		$$ = new std::vector<AST::Value *>(1, $1);
	}
	| nonEmptyValueList COMMA value
	{
		($1)->push_back($3);
		$$ = $1;
	}
	;
	
scope:
	LCURLYBRACKET statementList RCURLYBRACKET
	{
		$$ = new AST::Scope(*($2));
	}
	;
	
statementList:
	// empty
	{
		$$ = new std::vector<AST::Statement *>();
	}
	| statementList scopedStatement
	{
		($1)->push_back($2);
		$$ = $1;
	}
	| statementList normalStatement SEMICOLON
	{
		($1)->push_back($2);
		$$ = $1;
	}
	| statementList normalStatement error
	{
		parserContext->error("Statement must be terminated with semicolon.");
		($1)->push_back($2);
		$$ = $1;
	}
	| statementList SEMICOLON
	{
		$$ = $1;
	}
	| statementList error
	{
		parserContext->error("Invalid statement.");
		$$ = $1;
	}
	;
	
scopedStatement:
	scope
	{
		$$ = AST::Statement::ScopeStmt($1);
	}
	| IF LROUNDBRACKET value RROUNDBRACKET scope
	{
		// One sided if statement (i.e. nothing happens in 'else' case).
		$$ = AST::Statement::If($3, $5, new AST::Scope());
	}
	| IF LROUNDBRACKET value RROUNDBRACKET scope ELSE scope
	{
		$$ = AST::Statement::If($3, $5, $7);
	}
	| FOR LROUNDBRACKET type NAME COLON value RROUNDBRACKET scope
	{
		// TODO
		assert(false && "For loops not implemented yet");
		$$ = NULL;
	}
	| WHILE LROUNDBRACKET value RROUNDBRACKET scope
	{
		$$ = AST::Statement::While($3, $5);
	}
	;
	
normalStatement:
	AUTO NAME SETEQUAL value
	{
		$$ = AST::Statement::AutoVarDecl(*($2), $4);
	}
	
	/*
	 * 'dprec 2' ensures that variable name definitions
	 * are preferred over assignments when there is an
	 * ambiguity.
	 *
	 * For example, 'T * p = null' is actually ambiguous,
	 * since it could mean defining a variable p as a (null)
	 * pointer to type 'T', or it could mean assigning
	 * null to the lvalue result of 'T * p', where 'T' and 'p'
	 * are both values of some kind.
	 * 
	 * In future this should be fixed to create an 'ambiguous
	 * statement' object in the AST tree that is resolved by
	 * semantic analysis.
	 */
	| type NAME SETEQUAL value %dprec 2
	{
		$$ = AST::Statement::VarDecl($1, *($2), $4);
	}
	| value SETEQUAL value %dprec 1
	{
		$$ = AST::Statement::Assign($1, $3);
	}
	
	| value ADDEQUAL value
	{
		$$ = AST::Statement::Assign($1, AST::Value::BinaryOp("operatorAdd", $1, $3));
	}
	| value SUBEQUAL value
	{
		$$ = AST::Statement::Assign($1, AST::Value::BinaryOp("operatorSubtract", $1, $3));
	}
	| value MULEQUAL value
	{
		$$ = AST::Statement::Assign($1, AST::Value::BinaryOp("operatorMultiply", $1, $3));
	}
	| value DIVEQUAL value
	{
		$$ = AST::Statement::Assign($1, AST::Value::BinaryOp("operatorDivide", $1, $3));
	}
	| value
	{
		$$ = AST::Statement::ValueStmt($1);
	}
	| RETURN
	{
		$$ = AST::Statement::ReturnVoid();
	}
	| RETURN value
	{
		$$ = AST::Statement::Return($2);
	}
	;
	
precision7:
	LROUNDBRACKET precision0 RROUNDBRACKET
	{
		$$ = $2;
	}
	| fullName
	{
		$$ = AST::Value::NameRef(*($1));
	}
	| AT NAME
	{
		$$ = AST::Value::MemberRef(*($2));
	}
	| BOOLCONSTANT
	{
		$$ = AST::Value::BoolConstant($1);
	}
	| INTCONSTANT
	{
		$$ = AST::Value::IntConstant($1);
	}
	| FLOATCONSTANT
	{
		$$ = AST::Value::FloatConstant($1);
	}
	| CSTRINGCONSTANT
	{
		$$ = AST::Value::CStringConstant(*($1));
	}
	| NULLVAL
	{
		$$ = AST::Value::NullConstant();
	}
	| CAST LTRIBRACKET type RTRIBRACKET LROUNDBRACKET value RROUNDBRACKET
	{
		$$ = AST::Value::Cast($3, $6);
	}
	;
	
precision6:
	precision7
	{
		$$ = $1;
	}
	| precision6 DOT NAME
	{
		$$ = AST::Value::MemberAccess($1, *($3));
	}
	| precision6 PTRACCESS NAME
	{
		$$ = AST::Value::MemberAccess(AST::Value::Dereference($1), *($3));
	}
	| precision6 LROUNDBRACKET valueList RROUNDBRACKET
	{
		$$ = AST::Value::FunctionCall($1, *($3));
	}
	;
	
precision5:
	precision6
	{
		$$ = $1;
	}
	| PLUS precision5
	{
		$$ = AST::Value::UnaryOp("operatorPlus", $2);
	}
	| MINUS precision5
	{
		$$ = AST::Value::UnaryOp("operatorMinus", $2);
	}
	| EXCLAIMMARK precision5
	{
		$$ = AST::Value::UnaryOp("operatorNot", $2);
	}
	| AMPERSAND precision5
	{
		$$ = AST::Value::AddressOf($2);
	}
	| STAR precision5
	{
		$$ = AST::Value::Dereference($2);
	}
	;
	
precision4:
	precision5
	{
		$$ = $1;
	}
	| precision4 STAR precision5
	{
		$$ = AST::Value::BinaryOp("operatorMultiply", $1, $3);
	}
	| precision4 FORWARDSLASH precision5
	{
		$$ = AST::Value::BinaryOp("operatorDivide", $1, $3);
	}
	| precision4 PERCENT precision5
	{
		$$ = AST::Value::BinaryOp("operatorModulo", $1, $3);
	}
	;
	
precision3:
	precision4
	{
		$$ = $1;
	}
	| precision3 PLUS precision4
	{
		$$ = AST::Value::BinaryOp("operatorAdd", $1, $3);
	}
	| precision3 MINUS precision4
	{
		$$ = AST::Value::BinaryOp("operatorSubtract", $1, $3);
	}
	;
	
precision2:
	precision3
	{
		$$ = $1;
	}
	| precision3 ISEQUAL precision3
	{
		$$ = AST::Value::BinaryOp("operatorIsEqual", $1, $3);
	}
	| precision3 NOTEQUAL precision3
	{
		$$ = AST::Value::BinaryOp("operatorNotEqual", $1, $3);
	}
	| precision3 LTRIBRACKET precision3
	{
		$$ = AST::Value::BinaryOp("operatorIsLess", $1, $3);
	}
	| precision3 RTRIBRACKET precision3
	{
		$$ = AST::Value::BinaryOp("operatorIsGreater", $1, $3);
	}
	
	/*
	 * In an attempt to reduce the number of necessary
	 * methods to implement, we use the (assumed) identities
	 * that 'i <= j' is identical to '!(i > j)', and that
	 * 'i >= j' is identical to '!(i < j)'.
	 */
	| precision3 LESSOREQUAL precision3
	{
		$$ = AST::Value::UnaryOp("operatorNot", AST::Value::BinaryOp("operatorIsGreater", $1, $3));
	}
	| precision3 GREATEROREQUAL precision3
	{
		$$ = AST::Value::UnaryOp("operatorNot", AST::Value::BinaryOp("operatorIsLess", $1, $3));
	}
	;
	
precision1:
	precision2
	{
		$$ = $1;
	}
	| precision2 QUESTIONMARK precision1 COLON precision1
	{
		$$ = AST::Value::Ternary($1, $3, $5);
	}
	;
	
precision0:
	precision1
	{
		$$ = $1;
	}
	;
	
value:
	precision0
	{
		$$ = $1;
	}
	;
	
%%

int Locic_Parser_GeneratedParser_lex(Locic::Parser::Token * token, void * lexer, Locic::Parser::Context * parserContext){
	const int result = Locic::Parser::LexGetToken(lexer);
	*token = parserContext->token;
	return result;
}

int Locic_Parser_GeneratedParser_error(void * scanner, Locic::Parser::Context * parserContext, const char *s){
	parserContext->error(s);
	return 0;
}
