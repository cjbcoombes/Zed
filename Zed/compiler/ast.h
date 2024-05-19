#include "patterns.h"

namespace compiler {

	namespace ast {
		// ~~~~~~~~~~~~~~~~~~~~~~~~~~~
		// AST Functions

		// Turns the list of tokens into a tree of matched patterns
		int matchPatterns(TokenData& tokenData, ast::MatchData& matchData, CompilerStatus& status, CompilerSettings& settings, std::ostream& stream);
		// Turns the patterns into an actual AST
		int formTree(ast::Tree& tree, ast::MatchData& matchData, CompilerStatus& status, CompilerSettings& settings, std::ostream& stream);
	}
}