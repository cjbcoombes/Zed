#pragma once
#include "nodes.h"
#include "../utils/flags.h"
#include <list>

namespace compiler {
	struct CompilerSettings;
	class CompilerStatus;

	namespace tokens {
		class TokenData;
	}

	namespace ast {
		enum class MatchType;
		class MatchData;

		struct Node;
	}
}

namespace compiler::ast {
	// ~~~~~~~~~~~~~~~~~~~~~~~~~~~
	// Context

	struct TreeContext {
		MatchType parentType;
		Flags flags;

		static constexpr Flags::bits_t FLAG_TYPE_DECL = 1;

		TreeContext();
		TreeContext(const MatchType parentType, const TreeContext& other);
	};

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
