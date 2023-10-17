#include "..\utils\utils.h"
#include "..\utils\bytecode.h"

namespace executor {
	// ~~~~~~~~~~~~~~~~~~~~~~~~~~~
	// Constants?

	// ~~~~~~~~~~~~~~~~~~~~~~~~~~~
	// Assembler Settings

	// Holds settings info about the assembly process
	struct ExecutorSettings {
		Flags flags;
		unsigned int stackSize;

		ExecutorSettings() : stackSize(0x1000) {}
	};

	// ~~~~~~~~~~~~~~~~~~~~~~~~~~~
	// Executor Exceptions

	class ExecutorException : public std::exception {
	public:
		enum class ErrorType {
			UNKNOWN_OPCODE,
			DIVIDE_BY_ZERO,
			BAD_ALLOC
		};

		static constexpr const char* const errorTypeStrings[] = {
			"Unknown opcode",
			"Division (or modulo) by zero",
			"Dynamic memory allocation error"
		};

		const ErrorType eType;
		const int loc;
		std::string extra;

		ExecutorException(const ErrorType& eType, const int& loc) : eType(eType), loc(loc), extra("") {}
		ExecutorException(const ErrorType& eType, const int& loc, char* const& extra) : eType(eType), loc(loc), extra(extra) {}
		ExecutorException(const ErrorType& eType, const int& loc, const char* const& extra) : eType(eType), loc(loc), extra(extra) {}
		ExecutorException(const ErrorType& eType, const int& loc, const std::string& extra) : eType(eType), loc(loc), extra(extra) {}


		virtual const char* what();
	};

	
}