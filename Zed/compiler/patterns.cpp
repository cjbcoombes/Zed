#include "patterns.h"
#include "compiler.h"
#include "tokenizer.h"
#include <stack>

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Matches and MatchData

compiler::ast::Match::Match(const MatchType type, const code_location loc) : type(type), loc(loc) {}

compiler::ast::TokenMatch::TokenMatch(const tokens::Token* const token) : Match(MatchType::TOKEN, token->loc), token(token) {}

compiler::ast::GroupMatch::GroupMatch(const MatchType type, const code_location loc) : Match(type, loc) {}

compiler::ast::FixedSizeMatch::FixedSizeMatch(const MatchType type, const code_location loc) : Match(type, loc), matches() {}

compiler::ast::MatchData::MatchData(const tokens::TokenData& tokenData) : matches(), root(nullptr) {
	GroupMatch* const tempRoot = add(std::make_unique<GroupMatch>(MatchType::ROOT_GROUP, code_location()));
	for (const tokens::Token& token : tokenData.getTokens()) {
		tempRoot->matches.push_back(add(std::make_unique<TokenMatch>(&token)));
	}

	root = tempRoot;
}

compiler::ast::GroupMatch* compiler::ast::MatchData::getRoot() const noexcept {
	return root;
}

template<class M>
M* compiler::ast::MatchData::add(std::unique_ptr<M> match) {
	M* temp = match.get();
	matches.emplace_back(std::move(match));
	return temp;
}

template<class M, class... Args>
M* compiler::ast::MatchData::addMatch(Args&&... args) {
	return add<M>(std::make_unique<M>(std::forward<Args>(args)...));
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Patterns

int compiler::ast::Pattern::match(std::list<const Match*>& matches, MatchData& matchData, CompilerStatus& status, const CompilerSettings& settings, std::ostream& stream) {
	if (settings.flags.hasFlags(Flags::FLAG_DEBUG | compiler::FLAG_DEBUG_AST)) {
		stream << IO_DEBUG "Missing pattern match implementation" IO_NORM "\n";
	}
	return 0;
}

int compiler::ast::GroupingPattern::match(std::list<const Match*>& matches, MatchData& matchData, CompilerStatus& status, const CompilerSettings& settings, std::ostream& stream) {
	using tokens::TokenType;
	using tokens::Token;

	std::stack<std::pair<MatchType, std::list<const Match*>::iterator>> openings;
	auto it = matches.begin();

	auto closeGroup = [&](MatchType groupType, CompilerIssue::Type errorType) -> int {
		if (openings.empty()) {
			const Token* token = dynamic_cast<const TokenMatch*>(*it)->token;
			status.addIssue(errorType, token->loc);
			return 1;
		} else {
			const Token* token = dynamic_cast<const TokenMatch*>(*openings.top().second)->token;
			if (openings.top().first == groupType) {
				GroupMatch* group = matchData.addMatch<GroupMatch>(
					groupType,
					code_location(token->loc, dynamic_cast<const TokenMatch*>(*it)->token->loc));

				group->matches.splice(group->matches.begin(), matches, std::next(openings.top().second), it);
				const int out = applyPatterns(group->matches, matchData, status, settings, stream);

				matches.insert(it, group);
				--it;
				matches.erase(openings.top().second);
				matches.erase(std::next(it));
				openings.pop();

				return out;
			} else {
				switch (openings.top().first) {
					case MatchType::PAREN_GROUP:
						status.addIssue(CompilerIssue::Type::UNMATCHED_PAREN, token->loc);
						break;
					case MatchType::SQUARE_GROUP:
						status.addIssue(CompilerIssue::Type::UNMATCHED_SQUARE, token->loc);
						break;
					case MatchType::CURLY_GROUP:
						status.addIssue(CompilerIssue::Type::UNMATCHED_CURLY, token->loc);
						break;
					default:
						throw std::logic_error("Group has unexpected MatchType");
				}
				return 1;
			}
		}
		};

	int out = 0;
	for (; it != matches.end(); ++it) {
		if ((*it)->type == MatchType::TOKEN) {
			const TokenType tokenType = dynamic_cast<const TokenMatch*>(*it)->token->type;

			switch (tokenType) {
				case TokenType::LEFT_PAREN:
					openings.emplace(MatchType::PAREN_GROUP, it);
					break;

				case TokenType::RIGHT_PAREN:
					out = closeGroup(MatchType::PAREN_GROUP, CompilerIssue::Type::INVALID_CLOSING_PAREN);
					if (out) return out;
					break;

				case TokenType::LEFT_SQUARE:
					openings.emplace(MatchType::SQUARE_GROUP, it);
					break;

				case TokenType::RIGHT_SQUARE:
					out = closeGroup(MatchType::SQUARE_GROUP, CompilerIssue::Type::INVALID_CLOSING_SQUARE);
					if (out) return out;
					break;

				case TokenType::LEFT_CURLY:
					openings.emplace(MatchType::CURLY_GROUP, it);
					break;

				case TokenType::RIGHT_CURLY:
					out = closeGroup(MatchType::CURLY_GROUP, CompilerIssue::Type::INVALID_CLOSING_CURLY);
					if (out) return out;
					break;

				default:
					// skip intentionally
					break;
			}
		}
	}

	if (!openings.empty()) {
		const Token* token = dynamic_cast<const TokenMatch*>(*openings.top().second)->token;
		switch (openings.top().first) {
			case MatchType::PAREN_GROUP:
				status.addIssue(CompilerIssue::Type::UNMATCHED_PAREN, token->loc);
				break;
			case MatchType::SQUARE_GROUP:
				status.addIssue(CompilerIssue::Type::UNMATCHED_SQUARE, token->loc);
				break;
			case MatchType::CURLY_GROUP:
				status.addIssue(CompilerIssue::Type::UNMATCHED_CURLY, token->loc);
				break;
			default:
				throw std::logic_error("Group has unexpected MatchType");
		}
		return 1;
	}

	return 0;
}

compiler::ast::FixedSizePattern::FixedSizePattern(il_t predicates, const MatchType matchType) : predicates(predicates), matchType(matchType) {}

int compiler::ast::FixedSizePattern::match(std::list<const Match*>& matches, MatchData& matchData, CompilerStatus& status, const CompilerSettings& settings, std::ostream& stream) {
	for (auto start = matches.begin(); start != matches.end();) {
		auto ptr = start;
		bool matched = true;

		for (auto pred = predicates.cbegin(); pred != predicates.cend(); ++pred, ++ptr) {
			if (ptr == matches.end()) return 0; // no more matches possible
			if (!(*pred)(*ptr)) {
				matched = false;
				break;
			}
		}

		if (matched) {
			FixedSizeMatch* match = matchData.addMatch<FixedSizeMatch>(
				matchType,
				code_location((*start)->loc, (*std::prev(ptr))->loc));
			std::copy(start, ptr, std::back_inserter(match->matches));

			matches.insert(start, match);
			start = matches.erase(start, ptr);

			// Backtrack in case the new group now should be put in a match
			size_t n = predicates.size();
			while (n > 0 && start != matches.begin()) {
				--start;
				--n;
			}
		} else {
			++start;
		}
	}

	return 0;
}

int compiler::ast::applyPatterns(std::list<const Match*>& matches, MatchData& matchData, CompilerStatus& status, const CompilerSettings& settings, std::ostream& stream) {
	typedef FixedSizePattern::pred_t pred_t;
	// Allows type deduction for use of initializer_list in make_unique
	typedef FixedSizePattern::il_t il_t;
	using tokens::TokenType;

	pred_t predTrue = [](const Match* m) -> bool { return true; };
	auto predToken = [](TokenType t) -> pred_t {
		return [=](const Match* m) -> bool {
			return m->type == MatchType::TOKEN && dynamic_cast<const TokenMatch*>(m)->token->type == t;
			};
		};
	auto predTokens = [](std::initializer_list<TokenType> tokens) -> pred_t {
		return [=](const Match* m) -> bool {
			if (m->type != MatchType::TOKEN) return false;
			const TokenType tt = dynamic_cast<const TokenMatch*>(m)->token->type;
			for (const TokenType t : tokens) {
				if (t == tt) return true;
			}
			return false;
			};
		};

	const std::unique_ptr<Pattern> patterns[] = {
		// Groups {} () []
		std::make_unique<GroupingPattern>(),
		// Property accessor .
		std::make_unique<FixedSizePattern, il_t>({ predTrue, predToken(TokenType::PERIOD), predTrue }, MatchType::PROP_ACCESS),
		// Mult/Div * /
		std::make_unique<FixedSizePattern, il_t>({ predTrue, predTokens({ TokenType::STAR, TokenType::SLASH }), predTrue }, MatchType::ARITH_BINOP),
		// Add/Sub + -
		std::make_unique<FixedSizePattern, il_t>({ predTrue, predTokens({ TokenType::PLUS, TokenType::DASH }), predTrue }, MatchType::ARITH_BINOP),
		// Macros #
		std::make_unique<FixedSizePattern, il_t>({ predToken(TokenType::HASH), predToken(TokenType::IDENTIFIER), predTrue}, MatchType::MACRO),
		// Return type Annotation :
		std::make_unique<FixedSizePattern, il_t>({ predTrue, predToken(TokenType::DASH_RIGHT_ANGLE), predTrue }, MatchType::RETURN_TYPE),
		// Type Annotation :
		std::make_unique<FixedSizePattern, il_t>({ predTrue, predToken(TokenType::COLON), predTrue }, MatchType::TYPE_ANNOTATION),
	};

	int out = 0;

	for (const std::unique_ptr<Pattern>& pattern : patterns) {
		out = pattern->match(matches, matchData, status, settings, stream);
		if (out) return out;
	}

	return 0;
}

int compiler::ast::matchPatterns(const tokens::TokenData& tokenData, ast::MatchData& matchData, CompilerStatus& status, const CompilerSettings& settings, std::ostream& stream) {
	using namespace compiler::ast;

	const int out = applyPatterns(matchData.getRoot()->matches, matchData, status, settings, stream);

	return out;
}