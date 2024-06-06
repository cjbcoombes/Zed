#include "compiler.h"
#include "tokenizer.h"
#include "treeform.h"
#include "ast.h"
#include <fstream>

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Compiler Issues

compiler::CompilerIssue::CompilerIssue(const Type& type, const code_location loc) : type(type), loc(loc), extra() {}
compiler::CompilerIssue::CompilerIssue(const Type& type, const code_location loc, const char* const& extra) : type(type), loc(loc), extra(extra) {}
compiler::CompilerIssue::CompilerIssue(const Type& type, const code_location loc, std::string extra) : type(type), loc(loc), extra(std::move(extra)) {}

std::string compiler::CompilerIssue::what() const {
	if (extra.length() == 0) {
		return typeStrings[static_cast<int>(type)];
	} else {
		return typeStrings[static_cast<int>(type)] + (" : " + extra);
	}
}

void compiler::CompilerIssue::print(const tokens::TokenData& tokenData, std::ostream& stream) const {
	const char* starter = IO_INFO;
	switch (level()) {
		case Level::WARN:
			starter = IO_WARN;
			break;
		case Level::ERROR:
		case Level::ABORT:
			starter = IO_ERR;
			break;
		case Level::INFO:
			// starter = IO_INFO; // it's already IO_INFO
			break;
	}

	stream << starter;
	if (loc.isValid()) {
		const bool multiline = loc.endLine != loc.startLine;

		if (multiline) {
			stream << '@' << (loc.startLine + 1) << ':' << (loc.startCol + 1) << '-' << (loc.endLine + 1) << ':' << (loc.endCol + 1) << "  " << what() << '\n';
		} else {
			stream << '@' << (loc.startLine + 1) << ':' << (loc.startCol + 1) << '-' << (loc.endCol + 1) << "  " << what() << '\n';
		}

		const auto line = tokenData.getLine(loc.startLine);
		// -1 for the \n, and -1 again because we're treating the endpoint as inclusive
		const int end = multiline ? static_cast<int>(line.length() - 2) : loc.endCol;

		stream << starter << "     | ";
		std::fill_n(std::ostream_iterator<char>(stream), loc.startCol, ' ');
		std::fill_n(std::ostream_iterator<char>(stream), end - loc.startCol + 1, 'v');
		if (multiline) stream << "...";
		stream << '\n';

		stream << starter << std::setw(5) << (loc.startLine + 1) << "| ";
		stream << line;

		stream << starter << "     | ";
		std::fill_n(std::ostream_iterator<char>(stream), loc.startCol, ' ');
		std::fill_n(std::ostream_iterator<char>(stream), end - loc.startCol + 1, '^');
		if (multiline) stream << "...";
		stream << '\n';

		stream << IO_NORM;
	} else {
		stream << what() << IO_NORM "\n";
	}
}

compiler::CompilerIssue::Level compiler::CompilerIssue::level() const {
	const int i = static_cast<int>(type);

	if (i >= firstAbort) return Level::ABORT;
	if (i >= firstError) return Level::ERROR;
	if (i >= firstWarning) return Level::WARN;
	return Level::INFO;
	// if (i >= firstInfo) return Level::INFO;
}

compiler::CompilerStatus::CompilerStatus() : issues(), abort(false) {}

void compiler::CompilerStatus::print(const tokens::TokenData& tokenData, std::ostream& stream) const {
	for (const CompilerIssue& issue : issues) {
		issue.print(tokenData, stream);
	}
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Compiler Functions

int compiler::compile(const char* const& inputPath, const char* const& outputPath, CompilerSettings& settings) {
	using std::cout;
	cout << IO_MAIN "Attempting to compile file \"" << inputPath << "\" into output file \"" << outputPath << "\"\n" IO_NORM;

	// Open streams for the input and output files
	std::fstream inputFile, outputFile;

	inputFile.open(inputPath, std::ios::in);
	if (!inputFile.is_open()) {
		cout << IO_ERR "Could not open file \"" << inputPath << "\"" IO_NORM IO_END;
		return 1;
	}

	outputFile.open(outputPath, std::ios::out | std::ios::binary | std::ios::trunc);
	if (!outputFile.is_open()) {
		cout << IO_ERR "Could not open file \"" << outputPath << "\"" IO_NORM IO_END;
		return 1;
	}

	try {
		int out = compile_(inputFile, outputFile, settings, std::cout);
		cout << IO_MAIN "Compilation finished with code: " << out << IO_NORM IO_END;
		return out;
	} catch (std::exception& e) {
		cout << IO_ERR "An unknown error occurred during compilation. This error is most likely an issue with the c++ compiler code, not your code. Sorry. The provided error message is as follows:\n" << e.what() << IO_NORM IO_END;
	}

	return 1;
}

int compiler::compile_(std::iostream& inputFile, std::iostream& outputFile, CompilerSettings& settings, std::ostream& stream) {
	// Flags/settings
	const bool isDebug = settings.flags.hasFlags(Flags::FLAG_DEBUG);
	int out = 0;

	CompilerStatus status;

	tokens::TokenData tokenData;
	stream << IO_MAIN "Tokenizing..." IO_NORM "\n";
	out = tokenize(inputFile, tokenData, status, settings, stream);
	stream << IO_MAIN "Tokenization finished with code: " << out << IO_NORM "\n";
	if (out) {
		status.print(tokenData, stream);
		return out;
	} else if (isDebug && settings.flags.hasFlags(FLAG_DEBUG_TOKENIZER)) {
		stream << IO_DEBUG "Tokenizer output\n" IO_DEBUG "---------------------------------------------" IO_NORM "\n";
		printTokens(tokenData, stream);
		stream << IO_DEBUG "---------------------------------------------" IO_NORM "\n";
	}

	stream << IO_MAIN "Creating AST..." IO_NORM "\n";

	ast::Tree tree;
	out = makeAST(tree, tokenData, status, settings, stream);
	stream << IO_MAIN "AST Construction finished with code: " << out << IO_NORM "\n";
	if (out) {
		status.print(tokenData, stream);
		return out;
	} else if (isDebug && settings.flags.hasFlags(FLAG_DEBUG_AST)) {
		stream << IO_DEBUG "AST output\n" IO_DEBUG "---------------------------------------------" IO_NORM "\n";
		tree.print(stream);
		stream << IO_DEBUG "---------------------------------------------" IO_NORM "\n";
	}

	stream << IO_MAIN "Generating bytecode..." IO_NORM "\n";
	//out = generateBytecode(outputFile, astTree, settings, stream);
	stream << IO_MAIN "Bytecode Generation finished with code: " << out << IO_NORM "\n";
	if (out) {
		status.print(tokenData, stream);
		return out;
	}

	status.print(tokenData, stream);
	return 0;
}