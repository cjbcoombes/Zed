#include "compiler.h"

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Compiler Exceptions

const char* compiler::CompilerIssue::what() {
	if (extra.length() == 0) {
		return typeStrings[static_cast<int>(type)];
	} else {
		extra.insert(0, " : ");
		extra.insert(0, typeStrings[static_cast<int>(type)]);
		return extra.c_str();
	}
}

void compiler::CompilerIssue::print(TokenData& tokenData, std::ostream& stream) {
	const char* starter;
	switch (level()) {
		case Level::WARN:
			starter = IO_WARN;
			break;
		case Level::ERROR:
		case Level::ABORT:
			starter = IO_ERR;
			break;
		case Level::INFO:
		default:
			starter = IO_INFO;
			break;
	}

	stream << starter;
	if (line >= 0) {
		stream << '@' << (line + 1) << ':' << (column + 1) << "  " << what() << '\n';
		stream << starter << "     | " << std::setw(column) << "v" << '\n';
		stream << starter << std::setw(5) << (line + 1) << "| ";
		if (tokenData.lineStarts.size() > line + 1) {
			stream << tokenData.content.substr(tokenData.lineStarts[line], tokenData.lineStarts[line + 1] - tokenData.lineStarts[line]);
		} else {
			stream << tokenData.content.substr(tokenData.lineStarts[line]);
		}
		stream << starter << "     | " << std::setw(column) << "^" << '\n';
		stream << IO_NORM;
	} else {
		stream << what() << IO_NORM "\n";
	}
}

compiler::CompilerIssue::Level compiler::CompilerIssue::level() {
	const int i = static_cast<int>(type);

	if (i >= firstAbort) return Level::ABORT;
	if (i >= firstError) return Level::ERROR;
	if (i >= firstWarning) return Level::WARN;
	return Level::INFO;
	// if (i >= firstInfo) return Level::INFO;
}

void compiler::CompilerStatus::addIssue(CompilerIssue&& issue) {
	if (issue.level() == CompilerIssue::Level::ABORT) {
		abort = true;
	}
	issues.push_back(issue);
}

void compiler::CompilerStatus::print(TokenData& tokenData, std::ostream& stream) {
	for (CompilerIssue& issue : issues) {
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
		cout << IO_ERR "An unknown error ocurred during compilation. This error is most likely an issue with the c++ compiler code, not your code. Sorry. The provided error message is as follows:\n" << e.what() << IO_NORM IO_END;
	}

	return 1;
}

int compiler::compile_(std::iostream& inputFile, std::iostream& outputFile, CompilerSettings& settings, std::ostream& stream) {
	// Flags/settings
	const bool isDebug = settings.flags.hasFlags(Flags::FLAG_DEBUG);
	int out = 0;

	CompilerStatus status;

	TokenData tokenData;
	stream << IO_MAIN "Tokenizing..." IO_NORM "\n";
	out = tokenize(inputFile, tokenData, status, settings, stream);
	if (isDebug) {
		stream << IO_DEBUG "Tokenizer output\n" IO_DEBUG "---------------------------------------------" IO_NORM "\n";
		printTokens(tokenData, stream);
		stream << IO_DEBUG "---------------------------------------------" IO_NORM "\n";
	}
	stream << IO_MAIN "Tokenization finished with code: " << out << IO_NORM "\n";
	if (out) {
		status.print(tokenData, stream);
		return out;
	}

	stream << IO_MAIN "Creating AST..." IO_NORM "\n";

	ast::MatchData matchData;
	out = matchPatterns(tokenData, matchData, status, settings, stream);
	if (isDebug) {
		stream << IO_DEBUG "AST output\n" IO_DEBUG "---------------------------------------------" IO_NORM "\n";
		//astTree.print(stream);
		stream << IO_DEBUG "---------------------------------------------" IO_NORM "\n";
	}
	stream << IO_MAIN "AST Construction finished with code: " << out << IO_NORM "\n";
	if (out) {
		status.print(tokenData, stream);
		return out;
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