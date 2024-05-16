#include "compiler.h"


int compiler::makeAST(ast::Tree& tree, TokenData& tokenData, CompilerStatus& status, CompilerSettings& settings, std::ostream& stream) {
	ast::MatchData matchData;
	int out = ast::matchPatterns(tokenData, matchData, status, settings, stream);

	if (out) return out;

	out = ast::formTree(tree, matchData, status, settings, stream);

	return out;
}