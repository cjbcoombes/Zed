#include "tokenizer.h"
#include "nodes.h"

namespace compiler {
	struct CompilerSettings;
	class CompilerStatus;

	namespace ast {
		typedef std::pair<Node*, int> treeres_t;

		// ~~~~~~~~~~~~~~~~~~~~~~~~~~~
		// Matches

		enum class MatchType {
			TOKEN,
			ROOT_GROUP,
			PAREN_GROUP,
			SQUARE_GROUP,
			CURLY_GROUP,

			ARITH_BINOP,
			MACRO
		};

		// Parent Match
		struct Match {
			MatchType type;
			code_location loc;

			Match(MatchType type, code_location loc) : type(type), loc(loc) {}
			virtual ~Match() {}

			virtual treeres_t formTree(Tree& tree, CompilerStatus& status, CompilerSettings& settings, std::ostream& stream);
		};

		// A Match of just a token
		struct TokenMatch : Match {
			Token* token;

			TokenMatch(Token* token) : Match(MatchType::TOKEN, token->loc), token(token) {}

			virtual treeres_t formTree(Tree& tree, CompilerStatus& status, CompilerSettings& settings, std::ostream& stream);
		};

		// A Match of a group of tokens
		struct GroupMatch : Match {
			std::list<Match*> matches;

			GroupMatch(MatchType type, code_location loc) : Match(type, loc) {}

			virtual treeres_t formTree(Tree& tree, CompilerStatus& status, CompilerSettings& settings, std::ostream& stream);
		};

		// A Match of a fixed-size group of tokens
		struct FixedSizeMatch : Match {
			std::vector<Match*> matches;

			FixedSizeMatch(MatchType type, code_location loc) : Match(type, loc), matches() {}

			virtual treeres_t formTree(Tree& tree, CompilerStatus& status, CompilerSettings& settings, std::ostream& stream);
		};

		// Collects matches in a way that avoids memory leaks
		class MatchData {
		public:
			std::list<std::unique_ptr<Match>> matches;
			Match* root;

			MatchData() : matches(), root(nullptr) {}

			template<class M>
			M* add(std::unique_ptr<M> match);
		};

		// ~~~~~~~~~~~~~~~~~~~~~~~~~~~
		// Patterns

		// Parent Pattern
		class Pattern {
		public:
			virtual int match(std::list<Match*>& matches, MatchData& matchData, CompilerStatus& status, CompilerSettings& settings, std::ostream& stream);
		};

		// A Pattern for paren/square/curly groups
		class GroupingPattern : public Pattern {
		public:
			virtual int match(std::list<Match*>& matches, MatchData& matchData, CompilerStatus& status, CompilerSettings& settings, std::ostream& stream);
		};

		// A general Pattern for fixed-size matches
		class FixedSizePattern : public Pattern {
		public:
			typedef std::function<bool(Match*)> pred_t;
			typedef std::initializer_list<pred_t>&& il_t;

			std::vector<pred_t> predicates;
			MatchType matchType;
			int linecolsource;

			FixedSizePattern(std::vector<pred_t> predicates, MatchType matchType, int linecolsource)
				: predicates(predicates), matchType(matchType), linecolsource(linecolsource) {}
			FixedSizePattern(std::initializer_list<pred_t>&& predicates, MatchType matchType, int linecolsource)
				: predicates(predicates), matchType(matchType), linecolsource(linecolsource) {}

			virtual int match(std::list<Match*>& matches, MatchData& matchData, CompilerStatus& status, CompilerSettings& settings, std::ostream& stream);
		};

		// Applies the patterns to reduce a list of matches
		int applyPatterns(std::list<Match*>& matches, MatchData& matchData, CompilerStatus& status, CompilerSettings& settings, std::ostream& stream);
	}
}