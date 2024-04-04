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

			std::vector<char> bytes;
			// std::vector<std::pair<?, int>> markers; // markers to addresses to be overwritten
			// Will also need:
			// - Global memory

			Frame();

			template<typename T>
			void write(T obj);

			bytecode::types::reg_t allocw();
			bytecode::types::reg_t allocb();
			void freew(bytecode::types::reg_t reg);
			void freeb(bytecode::types::reg_t reg);
		};

		class GenOut {
		public:
			std::list<std::unique_ptr<Frame>> frames;

			Frame& addFrame();
		};
	}
}