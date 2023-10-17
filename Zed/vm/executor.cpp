#include "executor.h"

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Executor Exceptions

const char* executor::ExecutorException::what() {
	if (extra.length() == 0) {
		return errorTypeStrings[static_cast<int>(eType)];
	} else {
		extra.insert(0, " : ");
		extra.insert(0, errorTypeStrings[static_cast<int>(eType)]);
		return extra.c_str();
	}
}