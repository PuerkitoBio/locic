#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <fstream>

#include <boost/program_options.hpp>

#include <Locic/AST.hpp>
#include <Locic/Parser/DefaultParser.hpp>
#include <Locic/CodeGen/CodeGen.hpp>
#include <Locic/SemanticAnalysis.hpp>

using namespace Locic;

namespace po = boost::program_options;

int main(int argc, char* argv[]) {
	try {
		std::vector<std::string> inputFileNames;
		int optimisationLevel = 0;
		std::string outputFileName;
		std::string semDebugFileName;
		std::string codeGenDebugFileName;
		std::string optDebugFileName;
		
		po::options_description visibleOptions("Options");
		visibleOptions.add_options()
		("help,h", "Display help information")
		("output-file,o", po::value<std::string>(&outputFileName)->default_value("out.bc"), "Set output file name")
		("optimisation,O", po::value<int>(&optimisationLevel)->default_value(0), "Set optimization level")
		("sem-debug-file", po::value<std::string>(&semDebugFileName), "Set Semantic Analysis SEM tree debug output file")
		("codegen-debug-file", po::value<std::string>(&codeGenDebugFileName), "Set CodeGen LLVM IR debug output file")
		("opt-debug-file", po::value<std::string>(&optDebugFileName), "Set Optimiser LLVM IR debug output file")
		;
		
		po::options_description hiddenOptions;
		hiddenOptions.add_options()
		("input-file", po::value< std::vector<std::string> >(&inputFileNames), "Set input file names")
		;
		
		po::options_description allOptions;
		allOptions.add(visibleOptions).add(hiddenOptions);
		
		po::positional_options_description optionsPositions;
		optionsPositions.add("input-file", -1);
		
		po::variables_map variableMap;
		
		try {
			po::store(po::command_line_parser(argc, argv).options(allOptions).positional(optionsPositions).run(), variableMap);
			po::notify(variableMap);
		} catch (const po::error& e) {
			std::cout << "Command line parser error: " << e.what() << std::endl;
			return 1;
		}
		
		if (!variableMap["help"].empty()) {
			printf("Usage: locic [options] file...\n");
			std::cout << visibleOptions << std::endl;
			return 1;
		}
		
		if (inputFileNames.empty()) {
			printf("locic: No files provided.\n");
			printf("Usage: locic [options] file...\n");
			std::cout << visibleOptions << std::endl;
			return 1;
		}
		
		if (optimisationLevel < 0 || optimisationLevel > 3) {
			printf("locic: Invalid optimisation level '%d'.\n", optimisationLevel);
			printf("Usage: locic [options] file...\n");
			std::cout << visibleOptions << std::endl;
			return 1;
		}
		
		inputFileNames.push_back("BuiltInTypes.loci");
		
		AST::NamespaceList astRootNamespaceList;
		
		// Parse all source files.
		for (std::size_t i = 0; i < inputFileNames.size(); i++) {
			const std::string filename = inputFileNames.at(i);
			FILE* file = fopen(filename.c_str(), "rb");
			
			if (file == NULL) {
				printf("Parser Error: Failed to open file '%s'.\n", filename.c_str());
				return 1;
			}
			
			Parser::DefaultParser parser(astRootNamespaceList, file, filename);
			
			if (!parser.parseFile()) {
				const auto errors = parser.getErrors();
				assert(!errors.empty());
				
				printf("Parser Error: Failed to parse file '%s' with %lu errors:\n", filename.c_str(), errors.size());
				
				for (const auto & error : errors) {
					printf("Parser Error (at %s): %s\n", error.location.toString().c_str(), error.message.c_str());
				}
				
				return 1;
			}
		}
		
		// Perform semantic analysis.
		SEM::Namespace* rootSEMNamespace = SemanticAnalysis::Run(astRootNamespaceList);
		assert(rootSEMNamespace != NULL);
		
		if (!semDebugFileName.empty()) {
			// If requested, dump SEM tree information.
			std::ofstream ofs(semDebugFileName.c_str(), std::ios_base::binary);
			ofs << formatMessage(rootSEMNamespace->toString());
		}
		
		const std::string outputName = "output";
		
		CodeGen::TargetInfo targetInfo = CodeGen::TargetInfo::DefaultTarget();
		CodeGen::CodeGenerator codeGenerator(targetInfo, outputName);
		codeGenerator.genNamespace(rootSEMNamespace);
		
		if (!codeGenDebugFileName.empty()) {
			// If requested, dump LLVM IR prior to optimisation.
			codeGenerator.dumpToFile(codeGenDebugFileName);
		}
		
		codeGenerator.applyOptimisations(optimisationLevel);
		
		if (!optDebugFileName.empty()) {
			// If requested, dump LLVM IR after optimisation.
			codeGenerator.dumpToFile(optDebugFileName);
		}
		
		codeGenerator.writeToFile(outputFileName);
		
	} catch (const Exception& e) {
		printf("Compilation failed (errors should be shown above).\n");
		return 1;
	}
	
	return 0;
}

