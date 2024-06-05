#pragma once

struct code_location {
	int startLine;
	int startCol;
	int endLine;
	int endCol;

	code_location(const int line, const int column);
	code_location(const int startLine, const int startCol, const int endLine, const int endCol);
	code_location(const code_location& start, const code_location& end);
	code_location();

	[[nodiscard]] bool isValid() const;
};