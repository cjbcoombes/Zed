#include "compiler.h"

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Compiler Exceptions

const char* compiler::CompilerException::what() {
	if (extra.length() == 0) {
		return errorTypeStrings[static_cast<int>(eType)];
	} else {
		extra.insert(0, " : ");
		extra.insert(0, errorTypeStrings[static_cast<int>(eType)]);
		return extra.c_str();
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
	} catch (CompilerException& e) {
		cout << IO_ERR "Error during compilation at LINE " << e.line << ", COLUMN " << e.column << " : " << e.what() << IO_NORM IO_END;
	} catch (std::exception& e) {
		cout << IO_ERR "An unknown error ocurred during compilation. This error is most likely an issue with the c++ compiler code, not your code. Sorry. The provided error message is as follows:\n" << e.what() << IO_NORM IO_END;
	}

	return 1;
}

int compiler::compile_(std::iostream& inputFile, std::iostream& outputFile, CompilerSettings& settings, std::ostream& stream) {
	// Flags/settings
	const bool isDebug = settings.flags.hasFlags(Flags::FLAG_DEBUG);

	stream << "[Insert Compiler Here]\n";

	return 0;
}