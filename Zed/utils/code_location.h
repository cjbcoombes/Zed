#pragma once

struct code_location {
	int line;
	int column;

	code_location(const int line, const int column);
	code_location();
};