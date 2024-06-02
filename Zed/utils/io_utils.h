#pragma once
#include <iostream>
#include <iomanip>

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~
// ANSI codes (and other io utils)

#define IO_END "\n\n\n"

#define IO_NORM "\033[0m"
#define IO_BLACK "\033[30m"
#define IO_RED "\033[31m"
#define IO_GREEN "\033[32m"
#define IO_YELLOW "\033[33m"
#define IO_BLUE "\033[34m"
#define IO_MAGENTA "\033[38;2;245;107;250m"
// "\033[35m"
#define IO_CYAN "\033[36m"
#define IO_WHITE "\033[37m"
#define IO_GRAY "\033[90m"

#define IO_FMT_STRING(x) IO_YELLOW "\"" << x << "\"" IO_NORM
#define IO_FMT_ID(x) IO_RED << x << IO_NORM
#define IO_FMT_INT(x) IO_CYAN << x << IO_NORM
#define IO_FMT_FLOAT(x) IO_BLUE << x << IO_NORM
#define IO_FMT_CHAR(x) IO_YELLOW "\'" << x << "\'" IO_NORM
#define IO_FMT_SYMB(x) IO_WHITE << x << IO_NORM
#define IO_FMT_MULTISYMB(x) IO_GRAY << x << IO_NORM
#define IO_FMT_KEYWORD(x) IO_MAGENTA << x << IO_NORM
#define IO_FMT_ERR(x) IO_RED << x << IO_NORM

#define IO_ERR IO_RED "[ERROR] "
#define IO_WARN IO_YELLOW "[WARNING] "
#define IO_MAIN IO_GREEN "[MAIN] "
#define IO_DEBUG IO_CYAN "[DEBUG] "
#define IO_INFO IO_GRAY "[INFO] "

#define IO_HEX std::hex
#define IO_DEC std::dec