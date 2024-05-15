#include "compiler.h"

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Matches

template<class M>
M* compiler::ast::MatchData::add(std::unique_ptr<M> match) {
	M* temp = match.get();
	matches.emplace_back(std::move(match));
	return temp;
}


// ~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Patterns

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Matching

void compiler::ast::Pattern::match(std::list<Match*>& matches, MatchData& matchData, CompilerSettings& settings, std::ostream& stream) {
	if (settings.flags.hasFlags(Flags::FLAG_DEBUG)) {
		stream << IO_DEBUG "Missing pattern match implementation" IO_NORM "\n";
	}
}

void compiler::ast::GroupingPattern::match(std::list<Match*>& matches, MatchData& matchData, CompilerSettings& settings, std::ostream& stream) {

	std::stack<std::pair<GroupMatch::GroupType, std::list<Match*>::iterator>> openings;

	for (auto it = matches.begin(); it != matches.end(); it++) {
		if ((*it)->type == MatchType::TOKEN) {
			TokenType tokenType = dynamic_cast<TokenMatch*>(*it)->token->type;
			
			switch (tokenType) {
				case TokenType::LEFT_PAREN:
					openings.emplace(GroupMatch::GroupType::PAREN, it);
					break;

				case TokenType::RIGHT_PAREN:
					if (!openings.empty() && openings.top().first == GroupMatch::GroupType::PAREN) {
						GroupMatch* match = matchData.add(std::make_unique<GroupMatch>(GroupMatch::GroupType::PAREN));
						// TODO: splice from opening to it into match
					} else {
						// error
					}
					break;
			}
		}
	}
}

void compiler::ast::applyPatterns(std::list<Match*>& matches, MatchData& matchData, CompilerSettings& settings, std::ostream& stream) {
	const std::unique_ptr<Pattern> patterns[] = {
		std::make_unique<GroupingPattern>() // temp
	};

	for (const std::unique_ptr<Pattern>& pattern : patterns) {
		pattern->match(matches, matchData, settings, stream);
	}
}

int compiler::matchPatterns(TokenData& tokenData, ast::MatchData& matchData, CompilerSettings& settings, std::ostream& stream) {
	using namespace compiler::ast;

	for (Token& token : tokenData.tokens) {
		matchData.tree.push_back(matchData.add(std::make_unique<TokenMatch>(&token)));
	}

	applyPatterns(matchData.tree, matchData, settings, stream);

	return 0;
}