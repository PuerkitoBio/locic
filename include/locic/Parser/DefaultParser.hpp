#ifndef LOCIC_PARSER_DEFAULTPARSER_HPP
#define LOCIC_PARSER_DEFAULTPARSER_HPP

#include <cstdio>
#include <string>

#include <locic/AST.hpp>
#include <locic/Parser/Context.hpp>

namespace locic {
	
	class StringHost;
	
	namespace Parser {
		
		class DefaultParser {
			public:
				DefaultParser(const StringHost& stringHost, AST::NamespaceList& rootNamespaceList, FILE * file, const std::string& fileName);
				~DefaultParser();
				
				bool parseFile();
				
				std::vector<Error> getErrors();
				
			private:
				void * lexer_;
				Context context_;
				
		};
		
	}
	
}

#endif
