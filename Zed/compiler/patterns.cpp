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

int compiler::ast::Pattern::match(std::list<Match*>& matches, MatchData& matchData, CompilerStatus& status, CompilerSettings& settings, std::ostream& stream) {
	if (settings.flags.hasFlags(Flags::FLAG_DEBUG)) {
		stream << IO_DEBUG "Missing pattern match implementation" IO_NORM "\n";
	}
	return 0;
}

int compiler::ast::GroupingPattern::match(std::list<Match*>& matches, MatchData& matchData, CompilerStatus& status, CompilerSettings& settings, std::ostream& stream) {

	std::stack<std::pair<GroupMatch::GroupType, std::list<Match*>::iterator>> openings;
	auto it = matches.begin();

	auto closeGroup = [&](GroupMatch::GroupType groupType, CompilerIssue::Type errorType) -> int {
		if (openings.empty()) {
			Token* token = dynamic_cast<TokenMatch*>(*it)->token;
			status.addIssue(CompilerIssue(errorType, token->line, token->column));
			return 1;
		} else {
			if (openings.top().first == groupType) {
				GroupMatch* group = matchData.add(std::make_unique<GroupMatch>(groupType));

				group->matches.splice(group->matches.begin(), matches, std::next(openings.top().second), it);
				int out = applyPatterns(group->matches, matchData, status, settings, stream);

				matches.insert(it, group);
				it--;
				matches.erase(openings.top().second);
				matches.erase(std::next(it));
				openings.pop();

				return out;
			} else {
				Token* token = dynamic_cast<TokenMatch*>(*openings.top().second)->token;
				switch (openings.top().first) {
					case GroupMatch::GroupType::PAREN:
						status.addIssue(CompilerIssue(CompilerIssue::Type::UNMATCHED_PAREN, token->line, token->column));
						break;
					case GroupMatch::GroupType::SQUARE:
						status.addIssue(CompilerIssue(CompilerIssue::Type::UNMATCHED_SQUARE, token->line, token->column));
						break;
					case GroupMatch::GroupType::CURLY:
						status.addIssue(CompilerIssue(CompilerIssue::Type::UNMATCHED_CURLY, token->line, token->column));
						break;
				}
				return 1;
			}
		}
	};

	int out = 0;
	for (; it != matches.end(); it++) {
		if ((*it)->type == MatchType::TOKEN) {
			TokenType tokenType = dynamic_cast<TokenMatch*>(*it)->token->type;

			switch (tokenType) {
				case TokenType::LEFT_PAREN:
					openings.emplace(GroupMatch::GroupType::PAREN, it);
					break;

				case TokenType::RIGHT_PAREN:
					out = closeGroup(GroupMatch::GroupType::PAREN, CompilerIssue::Type::INVALID_CLOSING_PAREN);
					if (out) return out;
					break;

				case TokenType::LEFT_SQUARE:
					openings.emplace(GroupMatch::GroupType::SQUARE, it);
					break;

				case TokenType::RIGHT_SQUARE:
					out = closeGroup(GroupMatch::GroupType::SQUARE, CompilerIssue::Type::INVALID_CLOSING_SQUARE);
					if (out) return out;
					break;

				case TokenType::LEFT_CURLY:
					openings.emplace(GroupMatch::GroupType::CURLY, it);
					break;

				case TokenType::RIGHT_CURLY:
					out = closeGroup(GroupMatch::GroupType::CURLY, CompilerIssue::Type::INVALID_CLOSING_CURLY);
					if (out) return out;
					break;
			}
		}
	}

	if (!openings.empty()) {
		Token* token = dynamic_cast<TokenMatch*>(*openings.top().second)->token;
		switch (openings.top().first) {
			case GroupMatch::GroupType::PAREN:
				status.addIssue(CompilerIssue(CompilerIssue::Type::UNMATCHED_PAREN, token->line, token->column));
				break;
			case GroupMatch::GroupType::SQUARE:
				status.addIssue(CompilerIssue(CompilerIssue::Type::UNMATCHED_SQUARE, token->line, token->column));
				break;
			case GroupMatch::GroupType::CURLY:
				status.addIssue(CompilerIssue(CompilerIssue::Type::UNMATCHED_CURLY, token->line, token->column));
				break;
		}
		return 1;
	}

	return 0;
}

int compiler::ast::applyPatterns(std::list<Match*>& matches, MatchData& matchData, CompilerStatus& status, CompilerSettings& settings, std::ostream& stream) {
	const std::unique_ptr<Pattern> patterns[] = {
		std::make_unique<GroupingPattern>()
	};

	int out = 0;

	for (const std::unique_ptr<Pattern>& pattern : patterns) {
		out = pattern->match(matches, matchData, status, settings, stream);
		if (out) return out;
	}

	return 0;
}

int compiler::matchPatterns(TokenData& tokenData, ast::MatchData& matchData, CompilerStatus& status, CompilerSettings& settings, std::ostream& stream) {
	using namespace compiler::ast;

	for (Token& token : tokenData.tokens) {
		matchData.tree.push_back(matchData.add(std::make_unique<TokenMatch>(&token)));
	}

	int out = applyPatterns(matchData.tree, matchData, status, settings, stream);

	return out;
}