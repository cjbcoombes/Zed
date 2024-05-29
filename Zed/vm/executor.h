#include "..\utils\utils.h"
#include "..\utils\bytecode.h"

namespace executor {
	// ~~~~~~~~~~~~~~~~~~~~~~~~~~~
	// Constants?

	// ~~~~~~~~~~~~~~~~~~~~~~~~~~~
	// Executor Settings

	static constexpr int FLAG_CHECK_MEM = Flags::FLAG_FIRST_FREE;

	// Holds settings info about the assembly process
	struct ExecutorSettings {
		Flags flags;
		unsigned int stackSize;

		ExecutorSettings() noexcept;
	};

	// ~~~~~~~~~~~~~~~~~~~~~~~~~~~
	// Executor Exceptions

	class ExecutorException : public std::runtime_error {
	public:
		enum class ErrorType {
			UNKNOWN_OPCODE,
			DIVIDE_BY_ZERO,
			BAD_ALLOC
		};

	private:
		const ErrorType eType;
		const int loc;
		std::string extra;

	public:
		static constexpr const char* const errorTypeStrings[] = {
			"Unknown opcode",
			"Division (or modulo) by zero",
			"Dynamic memory allocation error"
		};

		ExecutorException(const ErrorType eType, const int loc);
		ExecutorException(const ErrorType eType, const int loc, const char* const extra);
		//ExecutorException(const ErrorType eType, const int loc, const std::string& extra);

		int getLoc() const noexcept;
	};

	// ~~~~~~~~~~~~~~~~~~~~~~~~~~~
	// The Stack

	// Acts as the stack memory for the program execution
	class Stack {
	private:
		std::unique_ptr<char[]> owner;

	public:
		Stack(const int size);

		char* begin() noexcept;
	};

	// ~~~~~~~~~~~~~~~~~~~~~~~~~~~
	// Executor Functions

	int exec(const char* const path, const ExecutorSettings& settings);
	int exec_(std::iostream& file, const ExecutorSettings& settings, std::ostream& outstream, std::istream& instream);
}