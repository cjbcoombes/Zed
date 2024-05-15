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
			GROUP
		};

		struct Match {
			MatchType type;

			Match(MatchType type) : type(type) {}
			virtual ~Match() {}
		};

		struct TokenMatch : Match {
			Token* token;

			TokenMatch(Token* token) : Match(MatchType::TOKEN), token(token) {}
		};

		struct ThreeMatch : Match {
			Match* left;
			Match* middle;
			Match* right;

			ThreeMatch(MatchType type, Match* left, Match* middle, Match* right) : Match(type), left(left), middle(middle), right(right) {}
		};

		struct GroupMatch : Match {
			enum class GroupType {
				SQUARE, PAREN, CURLY
			};

			GroupType groupType;
			std::list<Match*> matches;

			GroupMatch(GroupType groupType) : Match(MatchType::GROUP), groupType(groupType) {}
		};

		class MatchData {
		public:
			std::list<std::unique_ptr<Match>> matches;
			std::list<Match*> tree;

			template<class M>
			M* add(std::unique_ptr<M> match);
		};

		// ~~~~~~~~~~~~~~~~~~~~~~~~~~~
		// Patterns

		class Pattern {
		public:
			virtual int match(std::list<Match*>& matches, MatchData& matchData, CompilerStatus& status, CompilerSettings& settings, std::ostream& stream);
		};

		class GroupingPattern : public Pattern {
		public:
			virtual int match(std::list<Match*>& matches, MatchData& matchData, CompilerStatus& status, CompilerSettings& settings, std::ostream& stream);
		};

		int applyPatterns(std::list<Match*>& matches, MatchData& matchData, CompilerStatus& status, CompilerSettings& settings, std::ostream& stream);
	}
}