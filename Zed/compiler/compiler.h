#include "..\utils\utils.h"
#include "tokenizer.h"

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
			Count
		};

		static constexpr int errorTypeCount = static_cast<int>(ErrorType::Count);
		static constexpr const char* const errorTypeStrings[] = {
			"I don't really know how this happened but it shouldn't have"
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
}
