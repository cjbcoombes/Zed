#include "main.h"

int main(int argc, const char* args[]) {
    using namespace std;

    cout << IO_NORM;

    // cout << args[0] << "\n";

    for (int clarg, i = 01; i < argc; i++) {
        clarg = lookupString(args[i], clargStrings, clargCount);
        if (clarg == -1) {
            cout << IO_ERR "Unknown command line argument >> " << args[i] << " << " IO_NORM "\n";
            break;
        }
        switch (static_cast<CLArg>(clarg)) {
            case CLArg::DEBUG:
                // TODO: Set debug flags
                break;
            case CLArg::NODEBUG:
                // TODO: Unset debug flags
                break;
            case CLArg::ASSEMBLE:
                // TODO: Assembler
                break;
            case CLArg::DISASSEMBLE:
                // TODO: Disassembler
                break;
            case CLArg::EXECUTE:
                // TODO: VM
                break;
            case CLArg::COMPILE:
                // TODO: Compiler
                break;
        }
    }

    return 0;
}