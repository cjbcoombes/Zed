#include "..\utils\utils.h"
#include "..\utils\bytecode.h"

namespace compiler {
	// ~~~~~~~~~~~~~~~~~~~~~~~~~~~
	// Tokenizer Constants?
	constexpr const int MAX_TOKEN_STR_LEN = 1024;

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
		// Primitive typenames (included in keywords)
		TYPE_INT,
		TYPE_FLOAT,
		TYPE_BOOL,
		TYPE_CHAR,

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
		"//", "/*", "*/"
	};
	constexpr int maxMultiSymbolLen = 7; // THIS HAS TO BE THE SIZE OF THE LARGEST SYMBOL
	static_assert(sizeof(multiSymbolStrings) / sizeof(const char*) == multiSymbolCount,
				  "Number of multi-symbols does not match list of multi-symbols");

	// Keywords, including names of primitives
	constexpr int firstKeyword = static_cast<int>(TokenType::RETURN);
	constexpr int keywordCount = static_cast<int>(TokenType::TYPE_CHAR) + 1 - firstKeyword;
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
		"int",
		"float",
		"bool",
		"char"
	};
	static_assert(sizeof(keywordStrings) / sizeof(const char*) == keywordCount,
				  "Number of keywords does not match list of keywords");

	bool isIdChar(char c);
	bool isSymbolChar(char c);
	bool isTypeToken(TokenType type);

	// ~~~~~~~~~~~~~~~~~~~~~~~~~~~
	// Token Data Types

	struct code_location {
		int line;
		int column;

		code_location(int line, int column) : line(line), column(column) {}
		code_location() : line(-1), column(-1) {}
	};

	// Represents a single token, once parsed
	struct Token {
		TokenType type;
		// Line and column for helpful errors
		const code_location loc;

		// The token will have one and only one of these as data
		union {
			// bytecode::types::word_t word; // Might not be necessary
			bytecode::types::float_t float_;
			bytecode::types::int_t int_;

			// bytecode::types::byte_t byte; // Might not be necessary
			//bytecode::types::bool_t bool_; // Might not be necessary
			bytecode::types::char_t char_;

			// The index of a string in the strList of a TokenData
			int strIndex;
		};

		Token(TokenType type, code_location loc) : type(type), loc(loc), strIndex(-1) {}
	};

	// Holds a list of tokens and a list of strings
	// that those tokens may refer to
	class TokenData {
	public:
		std::vector<std::string> strList;
		std::list<Token> tokens;
		std::string content;
		std::vector<int> lineStarts;

		void putInt(int val, code_location loc);
		void putFloat(float val, code_location loc);
		void putChar(char val, code_location loc);
		void putType(TokenType type, code_location loc);
		void putStr(TokenType type, code_location loc, std::string str);

	private:
		// Adds a token and returns a pointer to that token
		// IMPORTANT that the pointer does not get used after
		// the tokens vector is modified. (That's why it's
		// private)
		Token* put(TokenType type, code_location loc);
	};
}