#pragma once
#include "../utils/io_utils.h"

namespace compiler {
	class CompilerStatus;
	struct CompilerSettings;

	namespace tokens {
		class TokenData;
	}

	namespace ast {
		class MatchData;

		class Tree;
	}
}

namespace compiler::ast {
	// ~~~~~~~~~~~~~~~~~~~~~~~~~~~
	// AST Functions

	// Make the AST from the TokenData
	int makeAST(ast::Tree& tree, const tokens::TokenData& tokenData, CompilerStatus& status, const CompilerSettings& settings, std::ostream& stream);
}


/*

assembler
disassembler
executor
compiler
v
main








*/