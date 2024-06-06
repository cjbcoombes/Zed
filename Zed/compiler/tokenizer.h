#pragma once
#include "../utils/code_location.h"
#include "../utils/bytecode.h"
#include <sstream>
#include <list>
#include <vector>
#include <span>

namespace compiler {
	class CompilerStatus;
	struct CompilerSettings;
}

namespace compiler::tokens {
	// ~~~~~~~~~~~~~~~~~~~~~~~~~~~
	// Tokenizer Constants?
	constexpr int MAX_TOKEN_STR_LEN = 1024;

	// ~~~~~~~~~~~~~~~~~~~~~~~~~~~
	// Token Type Info

	enum class TokenType {
		IDENTIFIER,
		STRING,
		CHAR,
		NUM_UNIDENTIFIED,
		NUM_INT,
		NUM_FLOAT,

		// 1-Symbols
		TILDE,
		BTICK,
		EXPT,
		AT,
		HASH,
		DOLLAR,
		PCT,
		CARET,
		AMP,
		STAR,
		USCORE,
		DASH,
		PLUS,
		EQUALS,
		PIPE,
		BSLASH,
		COLON,
		SEMICOLON,
		COMMA,
		PERIOD,
		QMARK,
		SLASH,
		LEFT_PAREN,
		RIGHT_PAREN,
		LEFT_SQUARE,
		RIGHT_SQUARE,
		LEFT_CURLY,
		RIGHT_CURLY,
		LEFT_ANGLE,
		RIGHT_ANGLE,

		// Multi-Symbols (decreasing size order)
		EASTER_EGG,
		EQ_EQ_EQUALS,
		PLUS_EQUALS,
		DASH_EQUALS,
		STAR_EQUALS,
		SLASH_EQUALS,
		PCT_EQUALS,
		EQ_EQUALS,
		LEFT_ANGLE_EQUALS,
		RIGHT_ANGLE_EQUALS,
		PLUS_PLUS,
		DASH_DASH,
		DASH_RIGHT_ANGLE,
		SLASH_SLASH,
		SLASH_STAR,
		STAR_SLASH,

		// Keywords
		RETURN,
		WHILE,
		FOR,
		IF,
		ELSE,
		ELIF,
		AND,
		OR,
		TRUE,
		FALSE,
		FUN,
		TYPE,
		MIXED,
		NARROW,
		LET,
		DEFAULT,
		// Primitive typenames (included in keywords)
		TYPE_INT,
		TYPE_FLOAT,
		TYPE_BOOL,
		TYPE_CHAR,
		TYPE_VOID,

		Count
	};

	constexpr int tokenCount = static_cast<int>(TokenType::Count);

	// Single-char symbols
	constexpr int firstSymbol = static_cast<int>(TokenType::TILDE);
	constexpr int symbolCount = static_cast<int>(TokenType::RIGHT_ANGLE) + 1 - firstSymbol;
	constexpr char symbolChars[] = {
		'~', '`', '!', '@', '#', '$', '%',  '^', '&', '*', '_', '-', '+', '=',
		'|', '\\', ':', ';', ',', '.', '?', '/',
		'(', ')', '[', ']', '{', '}', '<', '>'
	};
	static_assert(sizeof(symbolChars) / sizeof(char) == symbolCount,
				  "Number of symbols does not match list of symbols");

	// Symbols composed of multiple single-char symbols
	constexpr int firstMultiSymbol = static_cast<int>(TokenType::EASTER_EGG);
	constexpr int multiSymbolCount = static_cast<int>(TokenType::STAR_SLASH) + 1 - firstMultiSymbol;
	// Must be in decreasing size order
	constexpr const char* const multiSymbolStrings[] = {
		"\\(^-^)/", // This is here as an easter egg and to test large tokens
		"===",
		"+=", "-=", "*=", "/=", "%=",
		"==", "<=", ">=",
		"++", "--",
		"->",
		"//", "/*", "*/"
	};
	constexpr int maxMultiSymbolLen = 7; // THIS HAS TO BE THE SIZE OF THE LARGEST SYMBOL
	static_assert(sizeof(multiSymbolStrings) / sizeof(const char*) == multiSymbolCount,
				  "Number of multi-symbols does not match list of multi-symbols");

	// Keywords, including names of primitives
	constexpr int firstKeyword = static_cast<int>(TokenType::RETURN);
	constexpr int keywordCount = static_cast<int>(TokenType::TYPE_VOID) + 1 - firstKeyword;
	constexpr const char* const keywordStrings[] = {
		"return",
		"while",
		"for",
		"if",
		"else",
		"elif",
		"and",
		"or",
		"true",
		"false",
		"fun",
		"type",
		"mixed",
		"narrow",
		"let",
		"default",
		"int",
		"float",
		"bool",
		"char",
		"void"
	};
	constexpr std::span<const char* const> keywordSpan(keywordStrings, keywordCount);
	static_assert(sizeof(keywordStrings) / sizeof(const char*) == keywordCount,
				  "Number of keywords does not match list of keywords");

	// ~~~~~~~~~~~~~~~~~~~~~~~~~~~
	// Token Data Types

	// Represents a single token, once parsed
	struct Token {
		TokenType type;
		// Line and column for helpful errors
		code_location loc;

		// The token will have one and only one of these as data
		union {
			// bytecode::types::word_t word; // Might not be necessary
			bytecode::types::float_t float_;
			bytecode::types::int_t int_;

			// bytecode::types::byte_t byte; // Might not be necessary
			//bytecode::types::bool_t bool_; // Might not be necessary
			bytecode::types::char_t char_;

			// A pointer to a string in the strList of a TokenData
			std::string* str;
		};

		Token(const TokenType type, const code_location loc) : type(type), loc(loc), str(nullptr) {}
	};

	// Holds a list of tokens and a list of strings
	// that those tokens may refer to
	class TokenData {
	private:
		std::list<std::string> strList;
		std::list<Token> tokens;
		std::stringstream content;
		std::vector<int> lineStarts;

		Token* put(const TokenType type, const code_location& loc);

	public:
		void putInt(const int val, const code_location& loc);
		void putFloat(const float val, const code_location& loc);
		void putChar(const char val, const code_location& loc);
		void putType(const TokenType type, const code_location& loc);
		void putStr(const TokenType type, const code_location& loc, std::string str);

		void newLine(const int location);
		[[nodiscard]] std::string_view getLine(const int line) const;
		void newChar(const char c);

		[[nodiscard]] const std::list<Token>& getTokens() const noexcept;

		TokenData() = default;
		TokenData(const TokenData&) = delete;
		TokenData& operator=(const TokenData&) = delete;
	};

	// ~~~~~~~~~~~~~~~~~~~~~~~~~~~
	// Tokenizer Functions

	// Turns a file into a list of tokens
	int tokenize(std::iostream& inputFile, TokenData& outputData, CompilerStatus& status, const CompilerSettings& settings, std::ostream& stream);

	// Prints tokens nicely
	void printTokens(const TokenData& tokenData, std::ostream& stream);
	void printToken(const Token& token, std::ostream& stream);
}