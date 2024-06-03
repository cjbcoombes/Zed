#pragma once
#include "nodes.h"
#include <list>

namespace compiler {
	struct CompilerSettings;
	class CompilerStatus;

	namespace tokens {
		class TokenData;
	}

	namespace ast {
		class MatchData;

		struct Node;
	}
}

namespace compiler::ast {
	// ~~~~~~~~~~~~~~~~~~~~~~~~~~~
	// Tree

	class Tree {
		std::list<std::unique_ptr<Node>> nodes;
		Node* root;

	public:
		TypeData typeData;

		Tree();

		template<class N>
		N* add(std::unique_ptr<N> node);

		template<class N, class... Args>
		N* addNode(Args&&... args);

		void print(std::ostream& stream) const;
		void setRoot(Node* const root);
	};

	// Turns the patterns into an actual AST
	int formTree(ast::Tree& tree, const ast::MatchData& matchData, CompilerStatus& status, const CompilerSettings& settings, std::ostream& stream);
}
