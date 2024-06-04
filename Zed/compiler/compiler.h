#pragma once
#include "../utils/code_location.h"
#include "../utils/flags.h"
#include "../utils/io_utils.h"
#include <list>

namespace compiler {
	// ~~~~~~~~~~~~~~~~~~~~~~~~~~~
	// Forward Declarations

	namespace tokens {
		struct Token;
		class TokenData;
	}

	namespace ast {
		class Tree;
	}
}

namespace compiler {
	// ~~~~~~~~~~~~~~~~~~~~~~~~~~~
	// Constants?

	// ~~~~~~~~~~~~~~~~~~~~~~~~~~~
	// Compiler Settings

	static constexpr int FLAG_DEBUG_TOKENIZER = Flags::FLAG_FIRST_FREE;
	static constexpr int FLAG_DEBUG_AST = FLAG_DEBUG_TOKENIZER << 1;
	static constexpr int FLAG_DEBUG_BYTECODE = FLAG_DEBUG_AST << 1;

	// Holds settings info about the compilation process
	struct CompilerSettings {
		Flags flags;
	};

	// Describes an issue (info, warning, error, etc.) raised during compilation
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
			INVALID_TYPE_TYPE_LIST,
			MISSING_COMMA_TYPE_LIST,
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
			"Invalid type in type list",
			"Missing comma in type list",
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

		Type type;
		code_location loc;
		std::string extra;

		CompilerIssue(const Type& type, const code_location loc);
		CompilerIssue(const Type& type, const code_location loc, const char* const& extra);
		CompilerIssue(const Type& type, const code_location loc, std::string extra);

		[[nodiscard]] std::string what() const;
		[[nodiscard]] Level level() const;
		void print(const tokens::TokenData& tokenData, std::ostream& stream) const;
	};

	// The compiler status, which collects compiler issues throughout compilation
	class CompilerStatus {
	public:
		std::list<CompilerIssue> issues;
		bool abort;

		CompilerStatus();
		CompilerStatus(const CompilerStatus&) = delete;
		CompilerStatus& operator=(const CompilerStatus&) = delete;

		template<class... Args>
		void addIssue(Args&&... args) {
			issues.emplace_back(std::forward<Args&&>(args)...);
			if (issues.back().level() == CompilerIssue::Level::ABORT) {
				abort = true;
			}
		}

		void print(const tokens::TokenData& tokenData, std::ostream& stream) const;
	};

	// ~~~~~~~~~~~~~~~~~~~~~~~~~~~
	// Compiler Functions

	// Compiles from an input file to an output file
	int compile(const char* const& inputPath, const char* const& outputPath, CompilerSettings& settings);
	int compile_(std::iostream& inputFile, std::iostream& outputFile, CompilerSettings& settings, std::ostream& stream);


}
