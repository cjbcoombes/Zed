#include "ast.h"

namespace compiler {
	// ~~~~~~~~~~~~~~~~~~~~~~~~~~~
	// Constants?

	// ~~~~~~~~~~~~~~~~~~~~~~~~~~~
	// Compiler Settings

	static constexpr int FLAG_DEBUG_TOKENIZER = Flags::FLAG_FIRST_FREE;
	static constexpr int FLAG_DEBUG_AST = FLAG_DEBUG_TOKENIZER << 1;
	static constexpr int FLAG_DEBUG_BYTECODE = FLAG_DEBUG_AST << 1;

	struct CompilerSettings {
		Flags flags;
	};

	struct CompilerIssue {
		enum class Level {
			INFO,
			WARN,
			ERROR,
			ABORT
		};

		enum class Type {
			// Info
			GEN_INFO,
			UNKNOWN,
			EASTER_EGG,

			// Warnings
			GEN_WARN,

			// Errors
			GEN_ERROR,
			INVALID_MACRO,
			// : with types
			INVALID_TYPE_MACRO,
			INVALID_TYPE_ARITH_BINOP,

			// Abortive
			GEN_ABORT,
			TOKEN_TOO_LONG,
			UNCLOSED_STRING,
			UNCLOSED_CHAR,
			INVALID_CHAR,

			INVALID_CLOSING_PAREN,
			INVALID_CLOSING_SQUARE,
			INVALID_CLOSING_CURLY,
			INVALID_CLOSING_ANGLE,

			UNMATCHED_PAREN,
			UNMATCHED_SQUARE,
			UNMATCHED_CURLY,
			UNMATCHED_ANGLE,

			Count
		};

		static constexpr int firstInfo = static_cast<int>(Type::GEN_INFO);
		static constexpr int firstWarning = static_cast<int>(Type::GEN_WARN);
		static constexpr int firstError = static_cast<int>(Type::GEN_ERROR);
		static constexpr int firstAbort = static_cast<int>(Type::GEN_ABORT);
		static constexpr int typeCount = static_cast<int>(Type::Count);
		static constexpr const char* const typeStrings[] = {
			"General info",
			"Unknown",
			"Easter egg",

			"General warning",

			"General error",
			"Unknown macro name",
			"Invalid type for macro argument",
			"Invalid (illegal or non-matching) types for arithmetic binop",

			"General abortive error",
			"Maximum token length exceeded",
			"Unclosed string",
			"Unclosed char",
			"Invalid char (must have length 1)",

			"Invalid closing parenthesis",
			"Invalid closing square bracket",
			"Invalid closing curly bracket",
			"Invalid closing angle bracket",

			"Unmatched parenthesis",
			"Unmatched square bracket",
			"Unmatched curly bracket",
			"Unmatched angle bracket"
		};

		static_assert(sizeof(typeStrings) / sizeof(const char*) == typeCount,
					  "Number of issue types does not match list of issue types");

		const Type type;
		const code_location loc;
		std::string extra;

		CompilerIssue(const Type& type, const code_location loc) : type(type), loc(loc), extra("") {}
		CompilerIssue(const Type& type, const code_location loc, char* const& extra) : type(type), loc(loc), extra(extra) {}
		CompilerIssue(const Type& type, const code_location loc, const char* const& extra) : type(type), loc(loc), extra(extra) {}
		CompilerIssue(const Type& type, const code_location loc, const std::string& extra) : type(type), loc(loc), extra(extra) {}

		const char* what();
		Level level() const;
		void print(TokenData& tokenData, std::ostream& stream);
	};

	class CompilerStatus {
	public:
		std::list<CompilerIssue> issues;
		bool abort;

		CompilerStatus() : issues(), abort(false) {}

		void addIssue(CompilerIssue&& issue);
		void print(TokenData& tokenData, std::ostream& stream);
	};

	// ~~~~~~~~~~~~~~~~~~~~~~~~~~~
	// Compiler Exceptions

	// An error raised by the compiler
	/*
	class CompilerException : public std::exception {
	public:
		enum class ErrorType {
			UNKNOWN,
			STRING_TOO_LONG,
			INVALID_CHAR,
			UNCLOSED_STRING,
			UNCLOSED_CHAR,

			INVALID_CLOSING_PAREN,
			INVALID_CLOSING_SQUARE,
			INVALID_CLOSING_CURLY,
			INVALID_CLOSING_ANGLE,

			UNMATCHED_PAREN,
			UNMATCHED_SQUARE,
			UNMATCHED_CURLY,
			UNMATCHED_ANGLE,

			INVALID_TYPE_TOKEN,
			INVALID_MACRO,
			DUPLICATE_SYMBOL,

			BAD_TYPE,
			MISMATCH_TYPE,
			BAD_FORM,
			BAD_TYPE_ADD,
			BAD_TYPE_SUB,
			BAD_TYPE_MUL,
			BAD_TYPE_DIV,
			BAD_TYPE_MACRO,

			OUT_OF_REGISTERS,

			Count
		};

		static constexpr int errorTypeCount = static_cast<int>(ErrorType::Count);
		static constexpr const char* const errorTypeStrings[] = {
			"I don't really know how this happened but it shouldn't have",
			"A string or identifier name is too long",
			"A char must have length 1",
			"Unclosed string",
			"Unclosed char",

			"Invalid closing parenthesis",
			"Invalid closing square bracket",
			"Invalid closing curly bracket",
			"Invalid closing angle bracket",

			"Unmatched parenthesis",
			"Unmatched square bracket",
			"Unmatched curly bracket",
			"Unmatched angle bracket",

			"Invalid token in typespec",
			"Invalid macro (after #)",
			"Duplicate symbol",

			"Bad type",
			"Mismatched types",
			"Bad form",
			"Cannot add value of type (TODO: more info)",
			"Cannot subtract value of type (TODO: more info)",
			"Cannot multiply value of type (TODO: more info)",
			"Cannot divide value of type (TODO: more info)",
			"Cannot apply macro to type (TODO: more info)",

			"Ran out of registers"
		};
	};
	*/

	// ~~~~~~~~~~~~~~~~~~~~~~~~~~~
	// Compiler Functions

	// Compiles from an input file to an output file
	int compile(const char* const& inputPath, const char* const& outputPath, CompilerSettings& settings);
	int compile_(std::iostream& inputFile, std::iostream& outputFile, CompilerSettings& settings, std::ostream& stream);

	// ~~~~~~~~~~~~~~~~~~~~~~~~~~~
	// Tokenizer Functions

	// Turns a file into a list of tokens
	int tokenize(std::iostream& inputFile, TokenData& outputData, CompilerStatus& status, const CompilerSettings& settings, std::ostream& stream);
	// Prints tokens nicely
	void printTokens(const TokenData& tokenData, std::ostream& stream);
	void printToken(const Token& token, std::ostream& stream);

	// ~~~~~~~~~~~~~~~~~~~~~~~~~~~
	// AST Functions

	int makeAST(ast::Tree& tree, TokenData& tokenData, CompilerStatus& status, CompilerSettings& settings, std::ostream& stream);

	// ~~~~~~~~~~~~~~~~~~~~~~~~~~~
	// Bytecode Generation

}
