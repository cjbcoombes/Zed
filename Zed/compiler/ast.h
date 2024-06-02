#pragma once
#include "../utils/io_utils.h"

namespace compiler {
	class CompilerStatus;
	struct CompilerSettings;

	class TokenData;

	namespace ast {
		class MatchData;

		class Tree;
	}
}

namespace compiler::ast {
	// ~~~~~~~~~~~~~~~~~~~~~~~~~~~
	// AST Functions

	// Make the AST from the TokenData
	int makeAST(ast::Tree& tree, const TokenData& tokenData, CompilerStatus& status, const CompilerSettings& settings, std::ostream& stream);
}


/*

assembler
disassembler
executor
compiler
v
main








*/