#include "ast.h"
#include "patterns.h"
#include "treeform.h"

int compiler::ast::makeAST(ast::Tree& tree, const TokenData& tokenData, CompilerStatus& status, const CompilerSettings& settings, std::ostream& stream) {
	ast::MatchData matchData(tokenData);

	int out = ast::matchPatterns(tokenData, matchData, status, settings, stream);

	if (out) return out;

	out = ast::formTree(tree, matchData, status, settings, stream);

	return out;
}
