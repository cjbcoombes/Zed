#include "..\utils\utils.h"
#include "tokenizer.h"
#include "ast.h"

namespace compiler {
	// ~~~~~~~~~~~~~~~~~~~~~~~~~~~
	// Constants?

	// ~~~~~~~~~~~~~~~~~~~~~~~~~~~
	// Compiler Settings

	struct CompilerSettings {
		Flags flags;
	};

	// ~~~~~~~~~~~~~~~~~~~~~~~~~~~
	// Compiler Exceptions

	// An error raised by the compiler
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

			BAD_TYPE_ADD,
			BAD_TYPE_SUB,
			BAD_TYPE_MUL,
			BAD_TYPE_DIV,

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

			"Cannot add value of type (TODO: more info)",
			"Cannot subtract value of type (TODO: more info)",
			"Cannot multiply value of type (TODO: more info)",
			"Cannot divide value of type (TODO: more info)",

			"Ran out of registers"
		};

		const ErrorType eType;
		const int line;
		const int column;
		std::string extra;

		CompilerException(const ErrorType& eType, const int& line, const int& column) : eType(eType), line(line), column(column), extra("") {}
		CompilerException(const ErrorType& eType, const int& line, const int& column, char* const& extra) : eType(eType), line(line), column(column), extra(extra) {}
		CompilerException(const ErrorType& eType, const int& line, const int& column, const char* const& extra) : eType(eType), line(line), column(column), extra(extra) {}
		CompilerException(const ErrorType& eType, const int& line, const int& column, const std::string& extra) : eType(eType), line(line), column(column), extra(extra) {}

		virtual const char* what();
	};

	// ~~~~~~~~~~~~~~~~~~~~~~~~~~~
	// Compiler Functions

	// Compiles from an input file to an output file
	int compile(const char* const& inputPath, const char* const& outputPath, CompilerSettings& settings);
	int compile_(std::iostream& inputFile, std::iostream& outputFile, CompilerSettings& settings, std::ostream& stream);

	// ~~~~~~~~~~~~~~~~~~~~~~~~~~~
	// Tokenizer Functions

	// Turns a file into a list of tokens
	int tokenize(std::iostream& inputFile, TokenData& ouputData, CompilerSettings& settings, std::ostream& stream);
	// Prints tokens nicely
	void printTokens(TokenData& tokenData, std::ostream& stream);
	void printToken(Token& token, TokenData& tokenData, std::ostream& stream);

	// ~~~~~~~~~~~~~~~~~~~~~~~~~~~
	// AST Functions

	// Constructs a base AST from a token list, with all parentheses/brackets/etc. matched
	int initAST(ast::Tree& astTree, CompilerSettings& settings, std::ostream& stream);
	// Applies rules to make the full AST tree
	int constructAST(ast::Tree& astTree, CompilerSettings& settings, std::ostream& stream);

	// ~~~~~~~~~~~~~~~~~~~~~~~~~~~
	// Bytecode Generation

	int generateBytecode(std::iostream& outputFile, ast::Tree& astTree, CompilerSettings& settings, std::ostream& stream);
}
