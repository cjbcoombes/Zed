#include "compiler.h"


// ~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Tree

template<class N>
N* compiler::ast::Tree::add(std::unique_ptr<N> node) {
	N* temp = node.get();
	nodes.emplace_back(std::move(node));
	return temp;
}


// ~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Match Tree Formation

std::pair<compiler::ast::Node*, int> compiler::ast::Match::formTree(Tree& tree, CompilerStatus& status, CompilerSettings& settings, std::ostream& stream) {
	if (settings.flags.hasFlags(Flags::FLAG_DEBUG)) {
		stream << IO_DEBUG "Missing treeform implementation" IO_NORM "\n";
	}
	return { nullptr, 0 };
}

std::pair<compiler::ast::Node*, int> compiler::ast::TokenMatch::formTree(Tree& tree, CompilerStatus& status, CompilerSettings& settings, std::ostream& stream) {
	return { tree.add<TokenNode>(std::make_unique<TokenNode>(token)), 0 };
}