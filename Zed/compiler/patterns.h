#pragma once
#include "../utils/code_location.h"
#include <utility>
#include <list>
#include <vector>
#include <functional>

namespace compiler {
	struct CompilerSettings;
	class CompilerStatus;

	namespace tokens {
		struct Token;
		class TokenData;
	}

	namespace ast {
		struct Node;

		class Tree;
		struct TreeContext;
	}
}

namespace compiler::ast {
	typedef std::pair<Node*, int> treeres_t;

	// ~~~~~~~~~~~~~~~~~~~~~~~~~~~
	// Matches

	enum class MatchType {
		NONE,
		TOKEN,
		ROOT_GROUP,
		IMPLICIT_GROUP,
		PAREN_GROUP,
		SQUARE_GROUP,
		CURLY_GROUP,

		PROP_ACCESS,
		ARITH_BINOP,
		MACRO,
		TYPE_ANNOTATION,
		RETURN_TYPE
	};

	// Parent Match
	struct Match {
		MatchType type;
		code_location loc;

		Match(const MatchType type, const code_location loc);
		virtual ~Match() = default;

		// Forms the tree version of this match, which means returning a Node* (or an error, hence treeres_t)
		[[nodiscard]] virtual treeres_t formTree(Tree& tree, const TreeContext& context, CompilerStatus& status, const CompilerSettings& settings, std::ostream& stream) const;
	};

	// A Match of just a token
	struct TokenMatch : Match {
		const tokens::Token* token;

		explicit TokenMatch(const tokens::Token* const token);

		[[nodiscard]] treeres_t formTree(Tree& tree, const TreeContext& context, CompilerStatus& status, const CompilerSettings& settings, std::ostream& stream) const override;
	};

	// A Match of a group of tokens
	struct GroupMatch : Match {
		std::list<const Match*> matches;

		GroupMatch(const MatchType type, const code_location loc);

		[[nodiscard]] treeres_t formTree(Tree& tree, const TreeContext& context, CompilerStatus& status, const CompilerSettings& settings, std::ostream& stream) const override;
	};

	// A Match of a fixed-size group of tokens
	struct FixedSizeMatch : Match {
		std::vector<const Match*> matches;

		FixedSizeMatch(const MatchType type, const code_location loc);

		[[nodiscard]] treeres_t formTree(Tree& tree, const TreeContext& context, CompilerStatus& status, const CompilerSettings& settings, std::ostream& stream) const override;
	};

	// Collects matches in a way that avoids memory leaks
	class MatchData {
		std::list<std::unique_ptr<const Match>> matches;
		GroupMatch* root;

	public:
		explicit MatchData(const tokens::TokenData& tokenData);

		template<class M>
		M* add(std::unique_ptr<M> match);

		template<class M, class... Args>
		M* addMatch(Args&&... args);

		[[nodiscard]] GroupMatch* getRoot() const noexcept;
	};

	// ~~~~~~~~~~~~~~~~~~~~~~~~~~~
	// Patterns

	// Parent Pattern
	class Pattern {
	public:
		virtual ~Pattern() = default;

		// Matches this pattern and splices the changes directly into the provided matches list
		virtual int match(std::list<const Match*>& matches, MatchData& matchData, CompilerStatus& status, const CompilerSettings& settings, std::ostream& stream);
	};

	// A Pattern for paren/square/curly groups
	class GroupingPattern : public Pattern {
	public:
		int match(std::list<const Match*>& matches, MatchData& matchData, CompilerStatus& status, const CompilerSettings& settings, std::ostream& stream) override;
	};


	// TODO: finish refactors for this class (privacy, const, no copying?)
	// A general Pattern for fixed-size matches, which match based on a provided list of predicates
	class FixedSizePattern : public Pattern {
	public:
		typedef std::function<bool(const Match*)> pred_t;
		typedef std::initializer_list<pred_t>&& il_t;

	private:
		std::vector<pred_t> predicates;
		MatchType matchType;

	public:
		FixedSizePattern(il_t predicates, const MatchType matchType);

		int match(std::list<const Match*>& matches, MatchData& matchData, CompilerStatus& status, const CompilerSettings& settings, std::ostream& stream) override;
	};

	int applyGrouping(std::list<const Match*>& matches, MatchData& matchData, CompilerStatus& status, const CompilerSettings& settings, std::ostream& stream);
	// Applies the patterns to reduce a list of matches
	int applyPatterns(std::list<const Match*>& matches, MatchData& matchData, CompilerStatus& status, const CompilerSettings& settings, std::ostream& stream);

	// Turns the list of tokens into a tree of matched patterns
	int matchPatterns(const tokens::TokenData& tokenData, ast::MatchData& matchData, CompilerStatus& status, const CompilerSettings& settings, std::ostream& stream);
}
