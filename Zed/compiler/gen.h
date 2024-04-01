#include "..\utils\utils.h"

namespace compiler {
	namespace gen {

		// ~~~~~~~~~~~~~~~~~~~~~~~~~~~
		// Stack Frame Management

		class Frame {
		public:
			bool wordRegs[bytecode::NUM_WORD_REGISTERS];
			bool byteRegs[bytecode::NUM_BYTE_REGISTERS];
			int nextFreeWord = 0;
			int nextFreeByte = 0;

			Frame();

			bytecode::types::reg_t allocw();
			bytecode::types::reg_t allocb();
			void freew(bytecode::types::reg_t reg);
			void freeb(bytecode::types::reg_t reg);
		};

		// ~~~~~~~~~~~~~~~~~~~~~~~~~~~
		// Generation

		struct GenOut {
		public:
			int byteCounter;
			std::iostream& outputFile;
			// Will also need:
			// - Global memory

			GenOut(std::iostream& outputFile) : byteCounter(0), outputFile(outputFile) {}

			template<typename T>
			void write(T obj);
		};
	}
}