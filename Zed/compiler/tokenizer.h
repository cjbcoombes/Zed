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
		"int",
		"float",
		"bool",
		"char"
	};
	static_assert(sizeof(keywordStrings) / sizeof(const char*) == keywordCount,
				  "Number of keywords does not match list of keywords");

	bool isIdChar(char c);
	bool isSymbolChar(char c);

	// ~~~~~~~~~~~~~~~~~~~~~~~~~~~
	// Token Data Types

	// Represents a single token, once parsed
	struct Token {
		TokenType type;
		// Line and column for helpful errors
		const int line;
		const int column;

		// The token will have one and only one of these as data
		union {
			// bytecode::types::word_t word; // Might not be necessary
			bytecode::types::float_t float_;
			bytecode::types::int_t int_;

			// bytecode::types::byte_t byte; // Might not be necessary
			bytecode::types::char_t char_;

			// The index of a string in the strList of a TokenData
			int strIndex;
		};

		Token(TokenType type, int line, int column) : type(type), line(line), column(column), strIndex(-1) {}
	};

	// Holds a list of tokens and a list of strings
	// that those tokens may refer to
	class TokenData {
	public:
		std::vector<std::string> strList;
		std::vector<Token> tokens;

		void putInt(int val, int line, int column);
		void putFloat(float val, int line, int column);
		void putChar(char val, int line, int column);
		void putType(TokenType type, int line, int column);
		void putStr(TokenType type, int line, int column, std::string str);

	private:
		// Adds a token and returns a pointer to that token
		// IMPORTANT that the pointer does not get used after
		// the tokens vector is modified. (That's why it's
		// private)
		Token* put(TokenType type, int line, int column);
	};
}