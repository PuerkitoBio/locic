set(REQ_LIBS
	${CLANG_REQ_LIBS}
	${LLVM_REQ_LIBS}
	boost_filesystem
	boost_program_options
	boost_system
	)

# Build compiler.
add_executable(locic
	locic.cpp
	)

target_link_libraries(locic
	locic-parser
	locic-semanalysis
	locic-codegen
	locic-sem
	locic-ast
	locic-support
	${REQ_LIBS}
	)

# Build test tool.
add_executable(loci-test
	loci-test.cpp
	)

target_link_libraries(loci-test
	locic-parser
	locic-semanalysis
	locic-codegen
	locic-sem
	locic-ast
	locic-support
	${REQ_LIBS}
	)

# Build vtable generator.
add_executable(loci-vtable
	loci-vtable.cpp
	)

target_link_libraries(loci-vtable
	locic-parser
	locic-semanalysis
	locic-codegen
	locic-sem
	locic-support
	${REQ_LIBS}
	)
