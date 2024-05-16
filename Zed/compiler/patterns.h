#include "..\utils\utils.h"
#include "tokenizer.h"
#include "nodes.h"

namespace compiler {
	struct CompilerSettings;
	class CompilerStatus;

	namespace ast {

		// ~~~~~~~~~~~~~~~~~~~~~~~~~~~
		// Matches

		enum class MatchType {
			TOKEN,
			ROOT_GROUP,
			PAREN_GROUP,
			SQUARE_GROUP,
			CURLY_GROUP
		};

		// Parent Match
		struct Match {
			MatchType type;
			int line;
			int column;

			Match(MatchType type, int line, int column) : type(type), line(line), column(column) {}
			virtual ~Match() {}

			virtual std::pair<Node*, int> formTree(Tree& tree, CompilerStatus& status, CompilerSettings& settings, std::ostream& stream);
		};

		// A Match of just a token
		struct TokenMatch : Match {
			Token* token;

			TokenMatch(Token* token) : Match(MatchType::TOKEN, token->line, token->column), token(token) {}

			virtual std::pair<Node*, int> formTree(Tree& tree, CompilerStatus& status, CompilerSettings& settings, std::ostream& stream);
		};

		// A Match of three tokens in a row
		struct ThreeMatch : Match {
			Match* left;
			Match* middle;
			Match* right;

			ThreeMatch(MatchType type, Match* left, Match* middle, Match* right, int line, int column) 
				: Match(type, line, column), left(left), middle(middle), right(right) {}
		};

		// A Match of a group of tokens
		struct GroupMatch : Match {
			std::list<Match*> matches;

			GroupMatch(MatchType type, int line, int column) : Match(type, line, column) {}


			virtual std::pair<Node*, int> formTree(Tree& tree, CompilerStatus& status, CompilerSettings& settings, std::ostream& stream);
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

		// Applies the patterns to reduce a list of matches
		int applyPatterns(std::list<Match*>& matches, MatchData& matchData, CompilerStatus& status, CompilerSettings& settings, std::ostream& stream);
	}
}