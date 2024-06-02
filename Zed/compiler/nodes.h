#pragma once
#include "../utils/code_location.h"
#include "../utils/bytecode.h"
#include <string>
#include <vector>

namespace compiler {
	namespace tokens {
		struct Token;
		class TokenData;
	}
}

namespace compiler::ast {
	// ~~~~~~~~~~~~~~~~~~~~~~~~~~~
	// Types

	// A type in the type system
	// Currently not fleshed-out. Just primitives
	struct ExprType {
		enum class PrimType {
			NONE, ERR, VOID, INT, FLOAT, BOOL, CHAR
		};

		static const ExprType primNoType;
		static const ExprType primErrType;
		static const ExprType primVoid;
		static const ExprType primInt;
		static const ExprType primFloat;
		static const ExprType primBool;
		static const ExprType primChar;

		PrimType type;

		ExprType();
		explicit ExprType(const PrimType type);

		void printSimple(std::ostream& stream) const;
	};

	bool sameType(const ExprType& a, const ExprType& b);

	// ~~~~~~~~~~~~~~~~~~~~~~~~~~~
	// Nodes

	enum class NodeType {
		UNIMPL,
		TOKEN,
		BLOCK,
		ARITH_BINOP,
		LITERAL,
		MACRO
	};

	// Parent Node
	struct Node {
		NodeType type;
		ExprType exprType;
		code_location loc;

		Node(const NodeType type, const code_location loc);
		Node(const NodeType type, const ExprType exprType, const code_location loc);
		virtual ~Node() = default;

		virtual void print(tokens::TokenData& tokenData, std::ostream& stream, std::string&& indent, bool last) const;
		virtual void printSimple(tokens::TokenData& tokenData, std::ostream& stream) const;
		void printType(std::ostream& stream) const;
	};

	// A Node for currently unimplemented cases
	struct UnimplNode : Node {
		std::string msg;
		std::vector<Node*> nodes;

		UnimplNode(const char* const msg, const code_location loc);

		void print(tokens::TokenData& tokenData, std::ostream& stream, std::string&& indent, bool last) const override;
	};

	// A Node for a block (just a group of other Nodes)
	struct BlockNode : Node {
		std::vector<Node*> nodes;

		explicit BlockNode(const code_location loc);

		void print(tokens::TokenData& tokenData, std::ostream& stream, std::string&& indent, bool last) const override;
	};

	// A Node for a token
	struct TokenNode : Node {
		const tokens::Token* token;

		explicit TokenNode(const tokens::Token* const token);

		void printSimple(tokens::TokenData& tokenData, std::ostream& stream) const override;
	};

	// A Node for a binary arithmetic operation
	struct ArithBinopNode : Node {
		enum class Type {
			ADD, SUB, MUL, DIV
		};

		Node* left;
		Node* right;
		Type opType;

		ArithBinopNode(const Type opType, const ExprType exprType, Node* const left, Node* const right, const code_location loc);

		void print(tokens::TokenData& tokenData, std::ostream& stream, std::string&& indent, bool last) const override;
	};

	// A Node for literal values
	struct LiteralNode : Node {
		enum class Type {
			INT, FLOAT, BOOL, CHAR
		};

		union {
			bytecode::types::float_t float_;
			bytecode::types::int_t int_;

			bytecode::types::bool_t bool_; // Might not be necessary
			bytecode::types::char_t char_;
		};

		Type litType;

		explicit LiteralNode(const tokens::Token* const token);

		void printSimple(tokens::TokenData& tokenData, std::ostream& stream) const override;
	};

	// A Node for Macros
	struct MacroNode : Node {
		enum class Type {
			HELLO_WORLD,
			PRINT_I,

			Count,
			UNKNOWN
		};

		static constexpr int macroCount = static_cast<int>(Type::Count);
		static constexpr const char* const macroStrings[] = {
			"hello_world",
			"printi"
		};
		static constexpr std::span<const char* const> macroSpan{ macroStrings, macroCount };
		static_assert(sizeof(macroStrings) / sizeof(const char*) == macroCount,
		              "Number of macros does not match list of macros");

		Type macroType;
		Node* arg;

		MacroNode(const Type macroType, const ExprType exprType, Node* const arg, const code_location loc);

		void print(tokens::TokenData& tokenData, std::ostream& stream, std::string&& indent, bool last) const override;
	};
}
