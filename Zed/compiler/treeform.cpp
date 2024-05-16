#include "compiler.h"


// ~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Tree

template<class N>
N* compiler::ast::Tree::add(std::unique_ptr<N> node) {
	N* temp = node.get();
	nodes.emplace_back(std::move(node));
	return temp;
}

template<class N, class ...Args>
N* compiler::ast::Tree::make(Args && ...args) {
	return add<N>(std::make_unique<N>(std::forward<Args>(args)...));
}


// ~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Match Tree Formation

std::pair<compiler::ast::Node*, int> compiler::ast::Match::formTree(Tree& tree, CompilerStatus& status, CompilerSettings& settings, std::ostream& stream) {
	if (settings.flags.hasFlags(Flags::FLAG_DEBUG)) {
		stream << IO_DEBUG "Missing treeform implementation" IO_NORM "\n";
	}
	return { tree.add(std::make_unique<UnimplNode>("Generic match", line, column)), 0 };
}

std::pair<compiler::ast::Node*, int> compiler::ast::TokenMatch::formTree(Tree& tree, CompilerStatus& status, CompilerSettings& settings, std::ostream& stream) {
	return { tree.add(std::make_unique<TokenNode>(token)), 0 };
}

std::pair<compiler::ast::Node*, int> compiler::ast::GroupMatch::formTree(Tree& tree, CompilerStatus& status, CompilerSettings& settings, std::ostream& stream) {
	// The parent match may decide to do something different with the group.

	std::pair<compiler::ast::Node*, int> tempRes;
	BlockNode* tempBlockNode;

	switch (type) {
		case MatchType::ROOT_GROUP:
		case MatchType::CURLY_GROUP:
			// block
			tempBlockNode = tree.add(std::make_unique<BlockNode>(line, column));
			for (Match* match : matches) {
				tempRes = match->formTree(tree, status, settings, stream);
				if (tempRes.second) {
					return { nullptr, tempRes.second };
				}
				tempBlockNode->nodes.push_back(tempRes.first);
			}
			return { tempBlockNode, 0 };
		case MatchType::SQUARE_GROUP:
			return { tree.add(std::make_unique<UnimplNode>("Square group", line, column)), 0 };
		case MatchType::PAREN_GROUP:
			if (matches.size() == 1) {
				return matches.front()->formTree(tree, status, settings, stream);
			} else {
				return { tree.add(std::make_unique<UnimplNode>("Paren group with multiple internal matches", line, column)), 0 };
			}
			break;
		default:
			throw std::logic_error("Group has unexpected MatchType");
	}
	return { nullptr, 0 };
}

int compiler::ast::formTree(ast::Tree& tree, ast::MatchData& matchData, CompilerStatus& status, CompilerSettings& settings, std::ostream& stream) {
	auto res = matchData.root->formTree(tree, status, settings, stream);
	tree.root = res.first;

	return res.second;
}