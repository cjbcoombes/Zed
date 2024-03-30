#include "..\utils\utils.h"

namespace compiler {
	namespace gen {
		// ~~~~~~~~~~~~~~~~~~~~~~~~~~~
		// Register Management

		struct RegManager {

		};

		// ~~~~~~~~~~~~~~~~~~~~~~~~~~~
		struct GenState {
		public:
			int byteCounter;
			RegManager regs;
			std::iostream& outputFile;
			// Will also need:
			// - Global memory
			// - Scopes?

			GenState(std::iostream& outputFile) : byteCounter(0), regs(), outputFile(outputFile) {}

			template<typename T>
			void write(T obj);
		};
	}
}