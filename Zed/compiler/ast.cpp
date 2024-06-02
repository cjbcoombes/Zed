#include "compiler.h"

compiler::ast::Tree::Tree() : nodes(), root(nullptr) {}

void compiler::ast::Tree::setRoot(Node* const root) {
	if (this->root != nullptr) {
		throw std::logic_error("Tree root was set multiple times");
	} else {
		this->root = root;
	}
}

int compiler::makeAST(ast::Tree& tree, const TokenData& tokenData, CompilerStatus& status, const CompilerSettings& settings, std::ostream& stream) {
	ast::MatchData matchData(tokenData);

	int out = ast::matchPatterns(tokenData, matchData, status, settings, stream);

	if (out) return out;

	out = ast::formTree(tree, matchData, status, settings, stream);

	return out;
}