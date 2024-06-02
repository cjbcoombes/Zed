#include "treeform.h"

namespace compiler::ast {

	

	// ~~~~~~~~~~~~~~~~~~~~~~~~~~~
	// AST Functions

	// Turns the list of tokens into a tree of matched patterns
	int matchPatterns(const TokenData& tokenData, ast::MatchData& matchData, CompilerStatus& status, const CompilerSettings& settings, std::ostream& stream);
	// Turns the patterns into an actual AST
	int formTree(ast::Tree& tree, const ast::MatchData& matchData, CompilerStatus& status, const CompilerSettings& settings, std::ostream& stream);
}
