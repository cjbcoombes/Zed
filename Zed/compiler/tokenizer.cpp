#include "tokenizer.h"
#include "../utils/string_lookup.h"
#include "compiler.h"

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Tokenizer Helper Functions

static bool isIdChar(const char c) {
	return ('a' <= c && c <= 'z') || ('A' <= c && c <= 'Z') || ('0' <= c && c <= '9') || c == '_';
}

static bool isSymbolChar(const char c) {
	for (const char& symb : compiler::tokens::symbolChars) {
		if (symb == c) return true;
	}
	return false;
}

static bool isTypeToken(const compiler::tokens::TokenType type) {
	using namespace compiler::tokens;

	return type == TokenType::TYPE_BOOL
		|| type == TokenType::TYPE_CHAR
		|| type == TokenType::TYPE_FLOAT
		|| type == TokenType::TYPE_INT;
}

static int parseInt(char str[], const int strlen, const int base) {
	int out = 0;
	for (int i = 0; i < strlen; i++) {
		out *= base;
		if ('0' <= str[i] && str[i] <= '9') {
			out += str[i] - '0';
		} else if ('A' <= str[i] && str[i] <= 'Z') {
			out += str[i] - 'A' + 10;
		} else if ('a' <= str[i] && str[i] <= 'z') {
			out += str[i] - 'a' + 10;
		}
	}
	return out;
}

static float parseFloat(char str[], const int strlen, const float base) {
	float out = 0;
	int i = 0;
	while (i < strlen && str[i] != '.') {
		out *= base;
		if ('0' <= str[i] && str[i] <= '9') {
			out += str[i] - '0';
		} else if ('A' <= str[i] && str[i] <= 'Z') {
			out += str[i] - 'A' + 10;
		} else if ('a' <= str[i] && str[i] <= 'z') {
			out += str[i] - 'a' + 10;
		}
		i++;
	}
	i++;
	float place = 1;
	while (i < strlen) {
		place /= base;
		if ('0' <= str[i] && str[i] <= '9') {
			out += (str[i] - '0') * place;
		} else if ('A' <= str[i] && str[i] <= 'Z') {
			out += (str[i] - 'A' + 10) * place;
		} else if ('a' <= str[i] && str[i] <= 'z') {
			out += (str[i] - 'a' + 10) * place;
		}
		i++;
	}
	return out;
}

static void putSymbols(const char* const str, const int slen, const code_location& loc, compiler::tokens::TokenData& outputData) {
	using namespace compiler::tokens;
	if (slen == 0) return;
	if (slen == 1) {
		for (int i = 0; i < symbolCount; i++) {
			if (symbolChars[i] == str[0]) {
				outputData.putType(static_cast<TokenType>(i + firstSymbol), code_location(loc.startLine, loc.startCol));
				break;
			}
		}
		return;
	}

	int currlen = slen > maxMultiSymbolLen ? maxMultiSymbolLen : slen;
	int i = 0, j;
	while (static_cast<int>(strlen(multiSymbolStrings[i])) > currlen && i < multiSymbolCount) i++;
	while (i < multiSymbolCount) {
		currlen = strlen(multiSymbolStrings[i]);
		j = 0;
		while (true) {
			if (multiSymbolStrings[i][j] != str[j]) break;
			if (j >= currlen - 1) {
				// PUT symbol
				outputData.putType(static_cast<TokenType>(i + firstMultiSymbol),
								   code_location(loc.startLine, loc.startCol, loc.startLine, loc.startCol + currlen - 1));
				putSymbols(str + currlen, slen - currlen, code_location(loc.startLine, loc.startCol + currlen), outputData);
				return;
			}
			j++;
		}
		i++;
	}
	putSymbols(str, 1, loc, outputData);
	putSymbols(str + 1, slen - 1, code_location(loc.startLine, loc.startCol + 1), outputData);
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~
// TokenData

compiler::tokens::Token* compiler::tokens::TokenData::put(const TokenType type, const code_location& loc) {
	tokens.emplace_back(type, loc);
	return &tokens.back();
}
void compiler::tokens::TokenData::putInt(const int val, const code_location& loc) {
	put(TokenType::NUM_INT, loc)->int_ = val;
}
void compiler::tokens::TokenData::putFloat(const float val, const code_location& loc) {
	put(TokenType::NUM_FLOAT, loc)->float_ = val;
}
void compiler::tokens::TokenData::putChar(const char val, const code_location& loc) {
	put(TokenType::CHAR, loc)->char_ = val;
}
void compiler::tokens::TokenData::putType(const TokenType type, const code_location& loc) {
	tokens.emplace_back(type, loc);
}
void compiler::tokens::TokenData::putStr(const TokenType type, const code_location& loc, std::string str) {
	std::string* strPtr = nullptr;
	for (auto i = strList.begin(); i != strList.end(); ++i) {
		if ((*i) == str) {
			strPtr = &(*i);
			break;
		}
	}

	if (strPtr == nullptr) {
		strList.emplace_back(std::move(str));
		strPtr = &strList.back();
	}

	put(type, loc)->str = strPtr;
}

void compiler::tokens::TokenData::newLine(const int location) {
	lineStarts.push_back(location);
}

std::string_view compiler::tokens::TokenData::getLine(const int line) const {
	const int size = lineStarts.size();

	if (line >= size) {
		throw std::logic_error("Requested line that does not exist");
	} else if (line + 1 >= size) {
		return content.view().substr(lineStarts.at(line));
	} else {
		return content.view().substr(lineStarts.at(line), lineStarts.at(line + 1) - lineStarts.at(line));
	}
}

void compiler::tokens::TokenData::newChar(const char c) {
	content << c;
}

const std::list<compiler::tokens::Token>& compiler::tokens::TokenData::getTokens() const noexcept {
	return tokens;
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Tokenizer Functions

int compiler::tokens::tokenize(std::iostream& inputFile, TokenData& outputData, CompilerStatus& status, const CompilerSettings& settings, std::ostream& stream) {
	// Go to the start of the file
	inputFile.clear();
	inputFile.seekg(0, std::ios::beg);

	/*
	Keep track of current type:
	NONE, STRING, CHAR, NUMBER, IDENTIFIER, SYMBOLS, COMMENT, BLOCK_COMMENT

	For none:
	" starts string
	// or /* starts comment
	' starts char
	0-9 starts number
	a-z, A-Z (and what else? _?) starts identifier
	symbol starts symbols

	For string:
	do escaping
	" ends string

	For comment
	Newline ends //
	* / ends /*

	For number:
	Know if there's been a .
	Check base patterns (0x, 0b)
	End on non-number

	For identifier:
	a-z, A-Z, 0-9 (and what else?) can be in an identifier
	End on non-identifier char

	For symbols:
	End on non-symbol char
	Helper function to push symbols, greedily take largest
	Ex:
	+===       +===      +===            ==
	^^^^ (no)  ^^^ (no)  ^^ (yes, push)  ^^ (yes, push)


	*/

	// Keeping track of chars
	char c;
	char currGroup[MAX_TOKEN_STR_LEN];
	int currGroupLen = 0;
	enum class GroupType {
		NONE,
		SKIP,
		STRING,
		CHAR,
		NUMBER,
		SYMBOL,
		ID,
		COMMENT,
		BLOCK_COMMENT
	};
	GroupType currGroupType = GroupType::NONE;

	// Specific to certain types
	bool isEscaped = false;
	bool hasDecimal = false;
	int numBase = 10;
	int keyword;

	// Location of the current token
	code_location loc;
	int line = 0;
	// has to start at -1 so it gets set to 0 on the first iteration
	int col = -1;
	int i = 0;
	outputData.newLine(0);
	// end
	bool end = inputFile.eof();

	// In each loop, it does something based on the current type and the current char
	// If it processes the char and retains its type, the char is simply appended to currGroup
	// If it processes the char and sets type to GroupType::NONE, it means it doesn't want the char
	// and the char will be used to start a new token
	// if it processes the char and sets type to GroupType::SKIP, it means it doesn't want the char
	// but also not to use the char to start a new token
	while (!end) {
		inputFile.get(c);
		end = inputFile.eof();
		outputData.newChar(end ? '\n' : c);
		i++;
		if (c == '\n' || end) {
			outputData.newLine(i);
			line++;
			// has to start at -1 so it gets set to 0 on the next iteration
			col = -1;
		} else {
			col++;
		}

		switch (currGroupType) {
			case GroupType::NONE:
				break;

			case GroupType::STRING:
				// Match certain escape sequences
				if (end) {
					status.addIssue(CompilerIssue::Type::UNCLOSED_STRING, loc);
					return 1;
				} else if (isEscaped) {
					if (c == 'n') {
						c = '\n';
						// line--;
					} else if (c == 'c') {
						c = '\033';
					} else if (c == '0') {
						c = '\0';
					} else if (c == 't') {
						c = '\t';
					}
					// For anything else the escape doesn't change anything
					// That means for example \" will become " but it won't terminate the string

					isEscaped = false;
				} else if (c == '"') {
					// PUT string
					currGroup[currGroupLen] = '\0';
					outputData.putStr(TokenType::STRING, loc, std::string(currGroup));
					currGroupType = GroupType::SKIP;
				} else {
					if (c == '\\') {
						isEscaped = true;
						// Don't add this char to the group
						continue;
					}
					// Other stuff specific to string management?


				}
				break;

			case GroupType::CHAR:
				// Match certain escape sequences
				if (end) {
					status.addIssue(CompilerIssue::Type::UNCLOSED_CHAR, loc);
					return 1;
				} if (isEscaped) {
					if (c == 'n') {
						c = '\n';
						// line--;
					} else if (c == 'c') {
						c = '\033';
					} else if (c == '0') {
						c = '\0';
					} else if (c == 't') {
						c = '\t';
					}
					// For anything else the escape doesn't change anything
					// That means for example \" will become " but it won't terminate the string

					isEscaped = false;
				} else if (c == '\'') {
					if (currGroupLen != 1) {
						status.addIssue(CompilerIssue::Type::INVALID_CHAR, loc);
						return 1;
					}
					outputData.putChar(currGroup[0], loc);
					currGroupType = GroupType::SKIP;
				} else {
					if (currGroupLen != 0) {
						status.addIssue(CompilerIssue::Type::INVALID_CHAR, loc);
						return 1;
					}
					if (c == '\\') {
						isEscaped = true;
						// Don't add this char to the group
						continue;
					}
				}
				break;

			case GroupType::NUMBER:
				if (currGroupLen == 1 && currGroup[0] == '0' && (c == 'x' || c == 'b')) {
					if (c == 'x') {
						numBase = 16;
					} else if (c == 'b') {
						numBase = 2;
					}
					// Don't add this char to the group
					continue;
				} else if (!hasDecimal && c == '.') {
					hasDecimal = true;
				} else if (!end && (('0' <= c && c <= '9') ||
									('A' <= c && c < ('A' + numBase - 10)) ||
									('a' <= c && c < ('a' + numBase - 10)))) {
					// All good
				} else {
					// PUT number
					if (!hasDecimal) {
						outputData.putInt(parseInt(currGroup, currGroupLen, numBase), loc);
					} else {
						outputData.putFloat(parseFloat(currGroup, currGroupLen, static_cast<float>(numBase)), loc);
					}
					currGroup[currGroupLen] = '\0';
					currGroupType = GroupType::NONE;
				}
				break;

			case GroupType::SYMBOL:
				if (!end && currGroupLen >= 1) {
					if (currGroup[currGroupLen - 1] == '/' && c == '/') {
						currGroupType = GroupType::COMMENT;
						currGroupLen--;
					} else if (currGroup[currGroupLen - 1] == '/' && c == '*') {
						currGroupType = GroupType::BLOCK_COMMENT;
						currGroupLen--;
					}
				}
				if (end || currGroupType != GroupType::SYMBOL || !isSymbolChar(c)) {
					// PUT symbols (greedy thing)
					currGroup[currGroupLen] = '\0';
					putSymbols(currGroup, currGroupLen, loc, outputData);
					currGroupType = currGroupType == GroupType::SYMBOL ? GroupType::NONE : currGroupType;
				}
				break;

			case GroupType::ID:
				if (end || !isIdChar(c)) {
					currGroup[currGroupLen] = '\0';
					keyword = lookupString(currGroup, keywordSpan);
					if (keyword == -1) {
						outputData.putStr(TokenType::IDENTIFIER, loc, std::string(currGroup));
					} else {
						outputData.putType(static_cast<TokenType>(keyword + firstKeyword), loc);
					}
					currGroupType = GroupType::NONE;
				}
				break;

			case GroupType::COMMENT:
				if (end || c == '\n') {
					// Don't PUT comment
					currGroupType = GroupType::SKIP;
				}
				break;

			case GroupType::BLOCK_COMMENT:
				if (end || (currGroupLen > 0 && currGroup[0] == '*' && c == '/')) {
					// Don't PUT comment
					currGroupType = GroupType::SKIP;
				}
				break;
		}

		if (end) break;
		loc.endLine = line;
		loc.endCol = col;
		if (currGroupType == GroupType::NONE) {
			// Start a new token
			currGroupLen = 1;
			currGroup[0] = c;
			loc.startLine = line;
			loc.startCol = col;
			if (c == '"') {
				currGroupLen = 0;
				currGroupType = GroupType::STRING;
				isEscaped = false;
			} else if (c == '\'') {
				currGroupLen = 0;
				currGroupType = GroupType::CHAR;
				isEscaped = false;
			} else if ('0' <= c && c <= '9') {
				currGroupType = GroupType::NUMBER;
				hasDecimal = false;
				numBase = 10;
			} else if (isIdChar(c)) {
				currGroupType = GroupType::ID;
			} else if (isSymbolChar(c)) {
				currGroupType = GroupType::SYMBOL;
			}
			// Anything else, like whitespace, is ignored and GroupType::NONE persists on the next loop iteration
		} else if (currGroupType == GroupType::SKIP) {
			// Start a new token on the NEXT iteration
			currGroupType = GroupType::NONE;
			currGroupLen = 0;
		} else {
			// Add current char to the current group
			if (currGroupLen >= MAX_TOKEN_STR_LEN) {
				status.addIssue(CompilerIssue::Type::TOKEN_TOO_LONG, loc);
				return 1;
			}
			// For comments we only care about the single previous character (to check for the ending sequence)
			if (currGroupType == GroupType::COMMENT || currGroupType == GroupType::BLOCK_COMMENT) {
				currGroupLen = 0;
			}
			currGroup[currGroupLen] = c;
			currGroupLen++;
		}
	}

	return 0;
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Printing Functions

void compiler::tokens::printTokens(const TokenData& tokenData, std::ostream& stream) {
	int line = 0;
	stream << IO_NORM;
	for (const Token& t : tokenData.getTokens()) {
		if (line != t.loc.startLine) stream << '\n';
		line = t.loc.startLine;
		printToken(t, stream);
		stream << ' ';
	}
	stream << '\n';
}

void compiler::tokens::printToken(const Token& t, std::ostream& stream) {
	const int type = static_cast<int>(t.type);

	if (t.type == TokenType::STRING) {
		stream << IO_FMT_STRING(*t.str);
	} else if (t.type == TokenType::IDENTIFIER) {
		stream << IO_FMT_ID(*t.str);
	} else if (t.type == TokenType::NUM_INT) {
		stream << IO_FMT_INT(t.int_);
	} else if (t.type == TokenType::NUM_FLOAT) {
		stream << IO_FMT_FLOAT(t.float_);
	} else if (t.type == TokenType::CHAR) {
		stream << IO_FMT_CHAR(t.char_);
	} else if (t.type == TokenType::LAMBDA) {
		stream << "\033[38;2;255;82;82ml"
			"\033[38;2;255;255;82ma"
			"\033[38;2;82;255;82mm"
			"\033[38;2;82;255;255mb"
			"\033[38;2;82;82;255md"
			"\033[38;2;255;82;255ma" IO_NORM;
	} else if (type >= firstSymbol && type - firstSymbol < symbolCount) {
		stream << IO_FMT_SYMB(symbolChars[type - firstSymbol]);
	} else if (type >= firstMultiSymbol && type - firstMultiSymbol < multiSymbolCount) {
		stream << IO_FMT_MULTISYMB(multiSymbolStrings[type - firstMultiSymbol]);
	} else if (type >= firstKeyword && type - firstKeyword < keywordCount) {
		stream << IO_FMT_KEYWORD(keywordStrings[type - firstKeyword]);
	}

}