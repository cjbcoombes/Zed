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

void compiler::ast::Pattern::match(std::list<Match*>& matches, MatchData& matchData, CompilerSettings& settings, std::ostream& stream) {
	if (settings.flags.hasFlags(Flags::FLAG_DEBUG)) {
		stream << IO_DEBUG "Missing pattern match implementation" IO_NORM "\n";
	}
}

void compiler::ast::GroupingPattern::match(std::list<Match*>& matches, MatchData& matchData, CompilerSettings& settings, std::ostream& stream) {

	std::stack<std::pair<GroupMatch::GroupType, std::list<Match*>::iterator>> openings;
	auto it = matches.begin();

	auto closeGroup = [&](GroupMatch::GroupType groupType, CompilerException::ErrorType errorType) {
		if (!openings.empty() && openings.top().first == groupType) {
			GroupMatch* group = matchData.add(std::make_unique<GroupMatch>(groupType));

			group->matches.splice(group->matches.begin(), matches, std::next(openings.top().second), it);
			applyPatterns(group->matches, matchData, settings, stream);

			matches.insert(it, group);
			it--;
			matches.erase(openings.top().second);
			matches.erase(std::next(it));
			openings.pop();
		} else {
			Token* token = dynamic_cast<TokenMatch*>(*it)->token;
			throw CompilerException(errorType, token->line, token->column);
		}
	};

	for (; it != matches.end(); it++) {
		if ((*it)->type == MatchType::TOKEN) {
			TokenType tokenType = dynamic_cast<TokenMatch*>(*it)->token->type;

			switch (tokenType) {
				case TokenType::LEFT_PAREN:
					openings.emplace(GroupMatch::GroupType::PAREN, it);
					break;

				case TokenType::RIGHT_PAREN:
					closeGroup(GroupMatch::GroupType::PAREN, CompilerException::ErrorType::INVALID_CLOSING_PAREN);
					break;

				case TokenType::LEFT_SQUARE:
					openings.emplace(GroupMatch::GroupType::SQUARE, it);
					break;

				case TokenType::RIGHT_SQUARE:
					closeGroup(GroupMatch::GroupType::SQUARE, CompilerException::ErrorType::INVALID_CLOSING_SQUARE);
					break;

				case TokenType::LEFT_CURLY:
					openings.emplace(GroupMatch::GroupType::CURLY, it);
					break;

				case TokenType::RIGHT_CURLY:
					closeGroup(GroupMatch::GroupType::CURLY, CompilerException::ErrorType::INVALID_CLOSING_CURLY);
					break;
			}
		}
	}

	if (!openings.empty()) {
		Token* token = dynamic_cast<TokenMatch*>(*openings.top().second)->token;
		switch (openings.top().first) {
			case GroupMatch::GroupType::PAREN:
				throw CompilerException(CompilerException::ErrorType::UNMATCHED_PAREN, token->line, token->column);
			case GroupMatch::GroupType::SQUARE:
				throw CompilerException(CompilerException::ErrorType::UNMATCHED_SQUARE, token->line, token->column);
			case GroupMatch::GroupType::CURLY:
				throw CompilerException(CompilerException::ErrorType::UNMATCHED_CURLY, token->line, token->column);
		}
	}
}

void compiler::ast::applyPatterns(std::list<Match*>& matches, MatchData& matchData, CompilerSettings& settings, std::ostream& stream) {
	const std::unique_ptr<Pattern> patterns[] = {
		std::make_unique<GroupingPattern>()
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